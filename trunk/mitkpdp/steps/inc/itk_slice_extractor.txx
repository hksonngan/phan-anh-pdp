#include "steps/inc/contour_filter.h"

#include <mitkImage.h>
#include <mitkImageCast.h>
#include <itkChangeInformationImageFilter.h>
#include <itkLineIterator.h>

template <class PixelType>
pdp::pdpItkSliceExtractor<PixelType>::pdpItkSliceExtractor(QString info)
    : info(info)
{ 
	ImageType::Pointer referencedImage = ImageType::New();
}

template <class PixelType>
QString pdp::pdpItkSliceExtractor<PixelType>::name() const
{
    return "Convex hull step: " + info;
}

template <class PixelType>
float pdp::pdpItkSliceExtractor<PixelType>::relativePercent() const
{
    return 1.0;
}

//slice is inverted not by std::vector but by the input process
template <class PixelType>
typename itk::Image<PixelType, 2>::Pointer pdp::pdpItkSliceExtractor<PixelType>::getSlice(int sliceNumber)
{
	return sliceList.at(sliceNumber);
}

template <class PixelType>
void pdp::pdpItkSliceExtractor<PixelType>::setSlice(int sliceNumber, typename itk::Image<PixelType, 2>::Pointer slice)
{
	sliceList[sliceNumber] = slice;
}

template <class PixelType>
void pdp::pdpItkSliceExtractor<PixelType>::extractSlice(typename itk::Image<PixelType, 3>::Pointer img)
{

	//referencedImage->CopyInformation(img);

	//clear slice list
	sliceList.clear();

	//init Region information, create an ExtractImageFilter
	ImageType::RegionType requestedRegion = img->GetLargestPossibleRegion();
	//index is often zero except for the collapsed index
	index = requestedRegion.GetIndex();
	size = requestedRegion.GetSize();
	//collapse in the z dimension: size = 0 and index is the point of collapse
	int length = size[2];
	size[2] = 0;

	std::cout << "length " << length << std::endl;
	sliceList.resize(length);
	
	size2D[0] = size[0];
	size2D[1] = size[1];

	typedef itk::ExtractImageFilter< ImageType, ImageType2D > FilterType;
	FilterType::Pointer filter = FilterType::New();
	filter->SetInput(img);

	for (int i = 0; i < length; i++){
		index[2] = length - i - 1;
		ImageType::RegionType region;
		region.SetSize( size );
		region.SetIndex( index );
		filter->SetExtractionRegion(region);
		filter->Update();
		ImageType2D::Pointer tempSlice = filter->GetOutput();
		tempSlice->DisconnectPipeline();
		sliceList[i] = tempSlice;
	}
}

template <class PixelType>
typename itk::Image<PixelType, 3>::Pointer pdp::pdpItkSliceExtractor<PixelType>::assembleSlice()
{
	return assembleSlice(0, sliceList.size()-1);
}

template <class PixelType>
typename itk::Image<PixelType, 3>::Pointer pdp::pdpItkSliceExtractor<PixelType>::assembleSlice(int start, int end)
{
	if (sliceList.size() == 0)
	{
		std::cout << "slice stack empty, please extract slices into stack" << std::endl;
		throw;
	}

	//tiler to stack up 2D images
	typedef itk::TileImageFilter< ImageType2D, ImageType > TilerType;
	
	TilerType::Pointer tiler = TilerType::New();
	
	//set layout = (1, 1, 0)
	//this is not a spacing
	//the spacing is taken from the first image
	//the third spacing is set to 1
	//see itkChangeInformationImageFilter to change the information
	itk::FixedArray< unsigned int, 3 > layout; 
	layout[0] = 1;
	layout[1] = 1;
	layout[2] = 0;
	tiler->SetLayout( layout );

	//be careful with the order of image slices not the std::vector
	for(int i=end; i >= start; i--)
	{
		tiler->SetInput(end - i, sliceList[i] );		
	}
	
	tiler->Update();

	sliceList.clear();

	ImageType::Pointer img = tiler->GetOutput();

	typedef itk::ChangeInformationImageFilter<ImageType> ChangeType;
	ChangeType::Pointer changer = ChangeType::New();
	changer->SetInput(img);
	changer->SetOutputSpacing(spacing);
	changer->SetOutputOrigin(origin);
	//changer->ChangeSpacingOn();
	//changer->ChangeOriginOn();
	changer->Update();
	return changer->GetOutput();

	//img->CopyInformation(referencedImage);
	//return img;

}

template <class PixelType>
void pdp::pdpItkSliceExtractor<PixelType>::work(LungDataset& input, LungDataset& output)
{	
	
//	demoConvexHull(input);
	
    emit stepProgress(1.00f);
}

template <class PixelType>
void pdp::pdpItkSliceExtractor<PixelType>::setPathToImageParam(LungDataset& input)
{
	mitk::Image::Pointer mitkImage = input.getImage(inputStepName);
	mitkImage->DisconnectPipeline();

	itk::Image<PixelType, 3>::Pointer img;
	mitk::CastToItkImage(mitkImage, img);
	const ImageType::SpacingType& sp = img->GetSpacing();
	spacing[0] = sp[0];
	spacing[1] = sp[1];
	spacing[2] = sp[2];

	const ImageType::PointType& orgn = img->GetOrigin();
	origin[0] = orgn[0];
	origin[1] = orgn[1];
	origin[2] = orgn[2];
	//setting region
	typename ImageType2D::IndexType regionIndex;
	typename ImageType2D::SizeType regionSize;
	extractSlice(img);
	ImageType2D::RegionType requestedRegion = getSlice(0)->GetLargestPossibleRegion();
	regionIndex = requestedRegion.GetIndex();
	regionSize = requestedRegion.GetSize();
	region.SetSize( regionSize );
	region.SetIndex( regionIndex );
}

template <class PixelType>
typename itk::Image<PixelType, 2>::Pointer pdp::pdpItkSliceExtractor<PixelType>::itkPathToImage(std::vector<typename itk::PolyLineParametricPath<2>::Pointer> paths)
{

	//init path to image filter
	typedef itk::TracePathOnImageFilter< PathType, ImageType2D > PathToImageType;
	PathToImageType::Pointer path2Img = PathToImageType::New();
	path2Img->CalculatePathRegionOff();
	
	ImageType2D::RegionType region;
	ImageType2D::IndexType index2d;
	index2d[0] = index[0];
	index2d[1] = index[1];
	region.SetSize( size2D );
	region.SetIndex( index2d );
	path2Img->GetOutput()->SetRegions(region);

	path2Img->SetPathValue(1000);
	ImageType2D::SpacingType spacing2;
	ImageType2D::PointType origin2;
	spacing2[0] = spacing[0];
	spacing2[1] = spacing[1];
	origin2[0] = origin[0];
	origin2[1] = origin[1];
	path2Img->GetOutput()->SetOrigin(origin2);
	path2Img->GetOutput()->SetSpacing(spacing2);
	path2Img->setNumberOfPaths(paths.size());
    std::cout << "PathToImage: size "<<paths.size()<<std::endl;
	for (int i = 0; i < paths.size(); ++i)
	{
        //std::cout << "PathToImage: inside the loop"<< std::endl;
		path2Img->SetInput(i, paths[i]);
	}
	if(paths.size()==0){
		typedef itk::Image<PixelType, 2> Image2D;
		Image2D::Pointer img = Image2D::New();
		img->SetOrigin(origin2);
		img->SetSpacing(spacing2);
		return img;
	}
    std::cout << "PathToImage: before update"<< std::endl;
	path2Img->Update();
    std::cout << "PathToImage: after the update"<< std::endl;
	
	
	typedef itk::Image<PixelType, 2>::Pointer Image2D;
	Image2D img = path2Img->GetOutput();
	
	for(int i = 2; i < paths.size(); i++)
	{
		img = drawLine(paths[i], img);
	}

	return img;
}

template <class PixelType>
typename itk::Image<PixelType, 2>::Pointer pdp::pdpItkSliceExtractor<PixelType>::manualPathToImage(std::vector<typename itk::PolyLineParametricPath<2>::Pointer> paths){	

	typedef itk::Image<PixelType, 2>::IndexType IndexType;
	ImageType2D::PointType point, point1;
	IndexType index, index1;

	ImageType2D::SpacingType spacing2;
	ImageType2D::PointType origin2;
	spacing2[0] = spacing[0];
	spacing2[1] = spacing[1];
	origin2[0] = origin[0];
	origin2[1] = origin[1];
	
	typedef itk::Image<PixelType, 2> Image2D;
	Image2D::Pointer imgo = Image2D::New();
	imgo->SetOrigin(origin2);
	imgo->SetSpacing(spacing2);
	imgo->SetRegions(region);
	imgo->Allocate();
	int cSize;
	const PathType::VertexListType * vertexList;

	/*Image2D::PixelType pixVal;
	pixVal[0]=6;
	pixVal[1]=15;*/
	//double d = 0.01;
	for (int i = 0; i < paths.size(); ++i)
	{
        vertexList=paths[i]->GetVertexList();
		cSize = vertexList->size();
			for(int l = 0; l < cSize; l++){
				if(l==cSize-1){
					imgo->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
					imgo->TransformPhysicalPointToIndex(point, index);
					imgo->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(0), point1);
					imgo->TransformPhysicalPointToIndex(point1, index1);
				}
				else{
					imgo->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
					imgo->TransformPhysicalPointToIndex(point, index);
					imgo->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l+1), point1);
					imgo->TransformPhysicalPointToIndex(point1, index1);
				}
				itk::LineIterator<ImageType2D> lit(imgo, index, index1);
				lit.GoToBegin();
				while (!lit.IsAtEnd())
				{
					lit.Set(255);
					//std::cout<<imgo->GetPixel(index)<<" ";
					++lit;
					//d+=0.02;
				}
			}
	}
	return imgo;
}

template <class PixelType>
typename itk::Image<PixelType, 2>::Pointer pdp::pdpItkSliceExtractor<PixelType>::drawLine(typename itk::PolyLineParametricPath<2>::Pointer bbox, typename itk::Image<PixelType, 2>::Pointer img)
{
	typedef itk::Image<PixelType, 2> ImageType;
	typedef ImageType::IndexType IndexType;
	typedef itk::PolyLineParametricPath< 2 > PathType;

	ImageType::PointType point;
	IndexType startIndex, endIndex, moveIndex;


	const PathType::VertexListType * vertexList = bbox->GetVertexList ();

	img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(0), point);
	img->TransformPhysicalPointToIndex(point, startIndex);
	img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(vertexList->size()-1), point);
	img->TransformPhysicalPointToIndex(point, endIndex);

	int steps = endIndex[1] - startIndex[1];
	moveIndex = startIndex;

	//std::cout << "startindex " << startIndex[0] << " " << startIndex[1] << " endIndex " << endIndex[0] << " " << endIndex[1] << std::endl;

	for (int i = 1; i < steps; i++)
	{
		moveIndex[1] += 1;
		img->SetPixel(moveIndex, 1000.0);
	}
	return img;
}



