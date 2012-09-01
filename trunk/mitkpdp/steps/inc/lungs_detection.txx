#include "app/itk_filter_progress_watcher.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

//! \todo remove?
//#include "itkMinimumMaximumImageCalculator.h"
//#include "itkRegionOfInterestImageFilter.h"
//#include "itkImageLinearConstIteratorWithIndex.h"


pdp::LungsDetection::LungsDetection(QString info)
    : info(info)
{ }

QString pdp::LungsDetection::name() const
{
    return "Lungs detection step: " + info;
}

float pdp::LungsDetection::relativePercent() const
{
    return 1.0;
}

template <class PixelType, int dim>
typename itk::Image<unsigned char, dim>::Pointer pdp::LungsDetection::binaryThreshold(typename itk::Image<PixelType, dim>::Pointer img)
{
	typedef itk::Image<PixelType, dim> InputImageType;
	typedef itk::Image<unsigned char, dim> OutputImageType;

	typedef itk::BinaryThresholdImageFilter<InputImageType, OutputImageType> BinaryFilterType;
	BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

    itkFilterProgressWatcher watcher(binaryFilter, this, 0.2f, 0.9f);

	binaryFilter->SetInput(img);

	binaryFilter->SetLowerThreshold(itk::NumericTraits<PixelType>::NonpositiveMin());
	binaryFilter->SetUpperThreshold(-550);
	binaryFilter->SetInsideValue(255);
	binaryFilter->SetOutsideValue(0);

	binaryFilter->Update();
	return binaryFilter->GetOutput();
}


void pdp::LungsDetection::work(LungDataset& input, LungDataset& output)
{
	std::cout << "lungs detection....................." << std::endl;

	mitk::Image::Pointer mitkImage = input.getImage(inputStepName);
	mitkImage->DisconnectPipeline();
	typedef itk::Image<float, 3> ImageType;
	typedef itk::Image<unsigned char, 2> ImageType2D;
	ImageType::Pointer img;
	mitk::CastToItkImage(mitkImage, img);

	//see otsu for max min intensity values
	//check if it is the HU already
	itk::Image<unsigned char, 3>::Pointer imgBin = binaryThreshold<float, 3>(img);

	//count the number of lung pixels for each slice
	typedef itk::ImageRegionConstIterator< ImageType2D > ConstIteratorType;

	pdpItkSliceExtractor<unsigned char> extractor("detect start and end slices");
	extractor.extractSlice(imgBin);

	m_LungStartSlice = 0;
	m_Slices = extractor.getNumberOfSlices();
	m_LungEndSlice = m_Slices - 1;

	float nSliceNorm;
	QString cLungField;
	bool startFound = false;
	bool endFound = false;

	for (int i = 0; i < m_Slices; i++)
	{
		int lungArea = 0;
		GetSliceLungNorm(0, extractor.getNumberOfSlices(), i, nSliceNorm, cLungField);
		//throax region has the pixel value of 0
		ImageType2D::RegionType inputRegion = computeThroaxRegion(extractor.getSlice(i), nSliceNorm, 0);
		ConstIteratorType inputIt( extractor.getSlice(i), inputRegion);
		computeMinLungArea(extractor.getSlice(i));

		for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
		{
			if ( inputIt.Get() == 255) lungArea++;
		}

		if (!startFound && lungArea >= minLungAreaUp && cLungField == "O")
		{
			startFound = true;
			m_LungStartSlice = i;
		}

		if (!endFound && lungArea < minLungAreaDown && cLungField == "U" && startFound)
		{
			endFound = true;
			m_LungEndSlice = i - 1;
			break;
		}

	}

	std::cout << "start " << m_LungStartSlice << " end " << m_LungEndSlice << " size " << m_Slices << std::endl;
	
	pdpItkSliceExtractor<float> extractor1("truncate start and end slices");
	extractor1.extractSlice(img);
	std::cout << "truncated size " << m_LungEndSlice - m_LungStartSlice + 1 << std::endl;
	ImageType::Pointer imgOut = extractor1.assembleSlice(m_LungStartSlice, m_LungEndSlice);

	QByteArray byteArray = inputStepName.toUtf8();
	const char* cString = byteArray.constData();
	output.getDataStore()->Remove(output.getDataStore()->GetNamedNode(cString));
	
	// compute image max and min
	//max 2095.76 min -1024.00
	//typedef itk::MinimumMaximumImageCalculator<ImageType> RangeCalculator;
	//RangeCalculator::Pointer rangeCalculator = RangeCalculator::New();
	//rangeCalculator->SetImage( img );
	//rangeCalculator->Compute();
	//float imageMin = rangeCalculator->GetMinimum();
	//float imageMax = rangeCalculator->GetMaximum();
	//std::cout << "max " << imageMax << " min " << imageMin << std::endl;

	//only for debugging, when done the final result will be imgOut
	//imgBin = extractor.assembleSlice(m_LungStartSlice, m_LungEndSlice);
	
	//Cast the ITK -> MITK image and add it to the datatree.
	mitk::Image::Pointer mitkOut = mitk::Image::New();
	mitk::CastToMitkImage(imgOut, mitkOut);
    output.addImage(mitkOut, "Lungs");
}


//////////////////////////////////////////////////////////////////////////
///  Calculate normaliced slice
//////////////////////////////////////////////////////////////////////////
void pdp::LungsDetection::GetSliceLungNorm( int nSliceLungStart, int nSliceLungEnd, int nSlice, float &nSliceNorm, QString &cLungField )
{
	QString _cLungField = "";

	// init
	cLungField = "";
	nSliceNorm = 0.0;

	if( nSliceLungEnd > 0 )
	{
		nSliceNorm = ( (float) ( nSlice - nSliceLungStart + 1  ) ) / ( (float) ( nSliceLungEnd - nSliceLungStart + 1 ) ) * 100.0;
		//
		if( nSliceNorm <= ( 1.0 / 3.0 ) * 100.0 )
		{
			cLungField = "O";	// Oben	(upper)
		}
		else if( ( nSliceNorm > ( 1.0 / 3.0 ) * 100.0 ) && ( nSliceNorm <= ( 2.0 / 3.0 ) * 100.0 ) ) 
		{
			cLungField = "M";	// Mitte (middle)	
		}
		else
		{
			cLungField = "U";	// Unten (lower)
		}
	}
}

/*
* Minimum Lungs Area is 1/3 the number of pixels
*/
void pdp::LungsDetection::computeMinLungArea(itk::Image<unsigned char, 2>::Pointer img)
{
	typedef itk::Image<unsigned char, 2> ImageType;
	ImageType::SizeType size = img->GetLargestPossibleRegion().GetSize();
	//
	////const ImageType::SpacingType& sp = img->GetSpacing();

	////to cut more slices, decrease the divisions
	float divisionUp = 11;
	float divisionDown = 6;

	//std::cout << "index " << img->GetLargestPossibleRegion().GetIndex()[0] << " " << img->GetLargestPossibleRegion().GetIndex()[1] << " size " << size[0] << " " << size[1] << std::endl;

	minLungAreaUp = size[0]*size[1]/divisionUp;
	minLungAreaDown = size[0]*size[1]/divisionDown;
}


itk::Image<unsigned char, 2>::RegionType pdp::LungsDetection::computeThroaxRegion(itk::Image<unsigned char, 2>::Pointer img, float sliceNo, unsigned char pixelValue)
{
	typedef itk::Image<unsigned char, 2> ImageType;
	ImageType::RegionType region = img->GetLargestPossibleRegion();
	ImageType::SizeType size = region.GetSize();

	ImageType::RegionType resultRegion;
	ImageType::RegionType::IndexType pixelIndex;
	ImageType::RegionType::IndexType resultIndex;
	ImageType::RegionType::SizeType resultSize;

	int limit = 20;

	int y1 = 2*size[1]/5;
	int	y2 = 4*size[1]/5;
	int x1 = 2*size[0]/5;
	int	x2 = 4*size[0]/5;

	//std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;

	//avoid unwanted stuff
	if (sliceNo > 20)
	{
		pixelIndex[0] = size[0]/2;
	}
	else
	{
		pixelIndex[0] = size[0]/4;
	}
	
	pixelIndex[1] = 0;
	int consecutive = 0;
	bool previous = false;
	for ( int i = 0; i < size[1]; i++)
	{
		pixelIndex[1] = i;

		if (img->GetPixel( pixelIndex ) == pixelValue)
		{
			if (previous)
			{
				consecutive++;
				if (consecutive > limit)
				{
					y1 = i - limit;
					break;
				}
			}
			else
			{
				previous = true;
			}
		}
		else	//reset
		{
			consecutive = 0;
			previous = false;
		}
	}


	//y2

	//avoid unwanted stuff
	if (sliceNo > 20)
	{
		pixelIndex[0] = size[0]/2;
	}
	else 
	{
		pixelIndex[0] = size[0]/4;
	}
	
	pixelIndex[1] = size[1] - 1;
	consecutive = 0;
	previous = false;
	for ( int i = size[1] - 1; i >= 0; i--)
	{
		pixelIndex[1] = i;

		if (img->GetPixel( pixelIndex ) == pixelValue)
		{
			if (previous)
			{
				consecutive++;
				if (consecutive > limit)
				{
					y2 = i + limit;
					//std::cout << y2 << std::endl;
					break;
				}
			}
			else
			{
				previous = true;
			}
		}
		else	//reset
		{
			consecutive = 0;
			previous = false;
		}
	}


	//x1
	pixelIndex[0] = 0;
	pixelIndex[1] = size[1]/2;
	consecutive = 0;
	previous = false;
	for ( int i = 0; i < size[0]; i++)
	{
		pixelIndex[0] = i;

		if (img->GetPixel( pixelIndex ) == pixelValue)
		{
			if (previous)
			{
				consecutive++;
				if (consecutive > limit)
				{
					x1 = i - limit;
					break;
				}
			}
			else
			{
				previous = true;
			}
		}
		else	//reset
		{
			consecutive = 0;
			previous = false;
		}
	}


	//x2
	pixelIndex[0] = size[0] - 1;
	pixelIndex[1] = size[1]/2;
	consecutive = 0;
	previous = false;
	for ( int i = size[0] - 1; i >= 0; i--)
	{
		pixelIndex[0] = i;

		if (img->GetPixel( pixelIndex ) == pixelValue)
		{
			if (previous)
			{
				consecutive++;
				if (consecutive > limit)
				{
					x2 = i + limit;
					break;
				}
			}
			else
			{
				previous = true;
			}
		}
		else	//reset
		{
			consecutive = 0;
			previous = false;
		}
	}


	resultIndex[0] = x1;
	resultIndex[1] = y1;

	resultSize[0] = x2-x1;
	resultSize[1] = y2-y1;

	resultRegion.SetIndex(resultIndex);
	resultRegion.SetSize(resultSize);

	//std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;

	return resultRegion;
}