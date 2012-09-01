#include "app/itk_filter_progress_watcher.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include <mitkImageCast.h>
#include "itkContourExtractor2DImageFilter.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkPointData.h>
#include <vtkLinearExtrusionFilter.h>
#include <itkVTKImageToImageFilter.h>
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "itkImageFileWriter.h"
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellData.h>
#include <vtkScalarsToColors.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vector>
#include <vtkMetaImageReader.h>
#include <vtkPolyDataReader.h>
#include <vtkJPEGWriter.h>
#include <vtkImageCast.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include "itkBinaryShapeKeepNObjectsImageFilter.h"


/*
* Supervised Range Constrained Otsu thresholding
* inside value corresponds to the background
*/
template <class PixelType, int dim>
typename itk::Image<unsigned char, dim>::Pointer pdp::OtsuStep::SVRC_Otsu(typename itk::Image<PixelType, dim>::Pointer img, float lowerBound, float upperBound)
{
	typedef itk::Image<PixelType, dim> InputImageType;
	typedef itk::Image<unsigned char, dim> OutputImageType;

	typedef itk::OtsuThresholdImageFilter<InputImageType, OutputImageType> OtsuFilterType;
	OtsuFilterType::Pointer otsuFilter = OtsuFilterType::New();

    itkFilterProgressWatcher watcher(otsuFilter, this, 0.2f, 0.9f);

	otsuFilter->SetInput(img);
	//inside value corresponds to the background
	otsuFilter->SetInsideValue(0);
	otsuFilter->SetOutsideValue(255);
	otsuFilter->SetNumberOfHistogramBins(256);
	otsuFilter->SetLowerBound(lowerBound);
	otsuFilter->SetUpperBound(upperBound);
	otsuFilter->Update();
	return otsuFilter->GetOutput();
}

/*
* for each slice perform 3 times morphological opening and 2 times morphological closing
* with a box structuring element of 3x3 (radius 1x1)
* use sliceExtractor to extract each slice, modify it and assemble again to get the result
* 3D morphological operations possible but must be tested for the correctness
*/
itk::Image<unsigned char, 3>::Pointer pdp::OtsuStep::Morphologies(itk::Image<unsigned char, 3>::Pointer img)
{
	pdpItkSliceExtractor<unsigned char> extractor("performing morphological operations for each slice");
	extractor.extractSlice(img);

	typedef itk::Image<unsigned char, 2> IType;

	//create a box structuring element of 3x3
	typedef itk::FlatStructuringElement<2> StructuringElementType;
	StructuringElementType::RadiusType elementRadius;
	//radius 1x1 is a box of 3x3
	elementRadius.Fill(1);
	StructuringElementType structuringElement = StructuringElementType::Box(elementRadius);

	computeNumberOfLogicalProcessors();
	
	//int tid;

	#pragma omp parallel for// private(tid)
	for (int i = 0; i < extractor.getNumberOfSlices(); i++)
	{

		/* Obtain and print thread id */
		//tid = omp_get_thread_num();
		//printf("Hello World from thread = %d\n", tid);

		IType::Pointer aSlice = extractor.getSlice(i);

		//create an opening filter
		typedef itk::BinaryMorphologicalOpeningImageFilter< IType, IType, StructuringElementType > OpenType;
		OpenType::Pointer open1 = OpenType::New();
		open1->SetInput( aSlice );
		open1->SetKernel( structuringElement );
		open1->SetForegroundValue( 255 );

		OpenType::Pointer open2 = OpenType::New();
		open2->SetInput( open1->GetOutput() );
		open2->SetKernel( structuringElement );
		open2->SetForegroundValue( 255 );

		OpenType::Pointer open3 = OpenType::New();
		open3->SetInput( open2->GetOutput() );
		open3->SetKernel( structuringElement );
		open3->SetForegroundValue( 255 );

		//create a closing filter
		typedef itk::BinaryMorphologicalClosingImageFilter< IType, IType, StructuringElementType > CloseType;
		CloseType::Pointer close1 = CloseType::New();
		close1->SetInput( open3->GetOutput() );
		close1->SetKernel( structuringElement );
		close1->SetForegroundValue( 255 );

		CloseType::Pointer close2 = CloseType::New();
		close2->SetInput( close1->GetOutput() );
		close2->SetKernel( structuringElement );
		close2->SetForegroundValue( 255 );

		close2->Update();
		//reassign the result to the extractor
		//extractor.setSlice(i, close2->GetOutput());
		//throax filler
		IType::Pointer throaxMask = fillThroax(close2->GetOutput());
		extractor.setSlice(i, throaxMask);
	}

	return extractor.assembleSlice();
}

/*
* compute the throax contour
* take the largest one, if it is open then merge to the second largest one
* it's easier to modify the contour in itk than vtk
* the longest contours are measured using bounding box length (squared diagonal)
*/
itk::PolyLineParametricPath<2>::Pointer pdp::OtsuStep::throaxContour(itk::Image<unsigned char, 2>::Pointer img)
{

	typedef itk::Image<unsigned char, 2> ImageType;
	typedef itk::PolyLineParametricPath<2> PathType;

	typedef itk::ContourExtractor2DImageFilter <ImageType>
          ContourExtractor2DImageFilterType;
	ContourExtractor2DImageFilterType::Pointer contourExtractor2DImageFilter
          = ContourExtractor2DImageFilterType::New();
	contourExtractor2DImageFilter->SetInput(img);
	contourExtractor2DImageFilter->SetContourValue(0);	//the throax is white-255
	//thus we have to reverse the contour orientation
	contourExtractor2DImageFilter->ReverseContourOrientationOn();
	contourExtractor2DImageFilter->Update();

	//the longest contours are measured using bounding box length (squared diagonal)
	double max = -1;
	int maxIndex = -1;
	double second = -2;	
	int secondIndex = -2;
	for(int j = 0; j < contourExtractor2DImageFilter->GetNumberOfOutputs(); j++){

		double bounds[6];
		itkContourToVtkContour(contourExtractor2DImageFilter->GetOutput(j), false)->GetCellBounds(0, bounds);
		double diag = GetLength2(bounds);

		if (diag > max)
		{
			max = diag;
			maxIndex = j;
		}
	}


	//dummy algorithm to find the second max
	for(int j = 0; j < contourExtractor2DImageFilter->GetNumberOfOutputs(); j++){

		double bounds[6];
		itkContourToVtkContour(contourExtractor2DImageFilter->GetOutput(j), false)->GetCellBounds(0, bounds);
		double diag = GetLength2(bounds);
		
		if (diag > second && j != maxIndex)
		{
			second = diag;
			secondIndex = j;
		}
	}



	PathType::Pointer first = contourExtractor2DImageFilter->GetOutput(maxIndex);
	if (first->GetVertexList()->GetElement(0) == first->GetVertexList()->GetElement(first->GetVertexList()->size()-1))
	{
		return first;
	}
	else if (second < 0.5*max) // half open contour
	{
		return first;
	}
	else
	{
		PathType::Pointer second = contourExtractor2DImageFilter->GetOutput(secondIndex);
		//head-tail adding pattern
		//black to the left and white to the right
		PathType::VertexListType::ConstIterator vertexIterator = second->GetVertexList()->Begin();
		while(vertexIterator != second->GetVertexList()->End())
		{
			//get() and value() return the vertexType which is ContinuousIndexType
			//in image coordinate
			//std::cout << vertexIterator->Value() << std::endl;
			
			first->AddVertex( vertexIterator->Value() );
			++vertexIterator;	
		}
		//close the loop
		//first->AddVertex(first->vertexList->GetElement(0));
		return first;
	}

}


vtkSmartPointer<vtkPolyData> pdp::OtsuStep::itkContourToVtkContour(itk::PolyLineParametricPath<2>::Pointer contour, bool closeTheLoop)
{
 // vtkSmartPointer<vtkPolyData> circle = stripper->GetOutput();

 // vtkSmartPointer<vtkPoints> cirPoints = circle->GetPoints();
 // vtkSmartPointer<vtkCellArray> cirPolys = circle->GetLines();
 // vtkIdType* conns;
	//vtkIdType length;
 // cirPolys->GetNextCell(length, conns);


	typedef itk::PolyLineParametricPath<2> PathType;

	//define the geometry and topology smart pointers
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> polys =
		vtkSmartPointer<vtkCellArray>::New();
	vtkFloatArray* pcoords = vtkFloatArray::New();
	pcoords->SetNumberOfComponents(3);

	//the poly data object will combine geometry and topology
    vtkSmartPointer<vtkPolyData> vtkThroax = 
		vtkSmartPointer<vtkPolyData>::New();
	//put some intensity in it
	//vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();

	if (closeTheLoop) polys->InsertNextCell( contour->GetVertexList()->size()+1 );
	else polys->InsertNextCell( contour->GetVertexList()->size());

	pcoords->SetNumberOfTuples(contour->GetVertexList()->size());

	int j = 0;
	PathType::VertexListType::ConstIterator vertexIterator = contour->GetVertexList()->Begin();
	while(vertexIterator != contour->GetVertexList()->End())
    {		
		float tuple[3] = {vertexIterator->Value()[0],vertexIterator->Value()[1], 0.0};
		pcoords->SetTupleValue(j, tuple);

		polys->InsertCellPoint( j );

		//scalars->InsertTuple1(j,255);

		j++;
		++vertexIterator;	
    }

	//close the loop
	if (closeTheLoop) polys->InsertCellPoint( 0 );

	points->SetData(pcoords);

	//std::cout << "number of points " << points->GetNumberOfPoints() << std::endl;


	//polys->InsertNextCell( cirPoints->GetNumberOfPoints()+1 );

	//  for (int i = 0; i < cirPoints->GetNumberOfPoints(); i++) 
 // {

	//  points->InsertPoint(i, cirPoints->GetPoint(i));
	//  polys->InsertCellPoint(conns[i]);
	//  scalars->InsertTuple1(i,0);
 // }

	//polys->InsertCellPoint(conns[length-1]);


	// We now assign the pieces to the vtkPolyData.
	vtkThroax->SetPoints(points);
    vtkThroax->SetLines(polys);
    //vtkThroax->GetPointData()->SetScalars(scalars);
	return vtkThroax;
}


// Compute Length squared of cell (i.e., bounding box diagonal squared).
double pdp::OtsuStep::GetLength2 (double Bounds[6])
{

	//just consider the width
	return Bounds[1] - Bounds[0];

  double diff, l=0.0;
  int i;

  for (i=0; i<3; i++)
    {
    diff = Bounds[2*i+1] - Bounds[2*i];
    l += diff * diff;
    }
  return l;
}


/*
* fill in the inner space of a throax by making a solid contour
* use vtk filters and vtk images
* http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/PolyDataContourToImageData
* @param img the image extracted from the morphologies step
*/
itk::Image<unsigned char, 2>::Pointer pdp::OtsuStep::fillThroax(itk::Image<unsigned char, 2>::Pointer img)
{
	typedef itk::Image<unsigned char, 2> ImageType;
	typedef itk::PolyLineParametricPath<2> PathType;

	//the open contour
	PathType::Pointer contour = throaxContour(img);
	vtkSmartPointer<vtkPolyData> vtkThroax = itkContourToVtkContour(contour, true);

	double spacing[3]; // desired volume spacing
	double origin[3];

	//spacing is one, because vertex has integer index
	const ImageType::SpacingType& sp = img->GetSpacing();
	//spacing[0] = sp[0];
	//spacing[1] = sp[1];
	//spacing[2] = sp[1];

	spacing[0] = 1;
	spacing[1] = 1;
	spacing[2] = 1;

	const ImageType::PointType& orgn = img->GetOrigin();
	double bounds[6];
	vtkThroax->GetBounds(bounds); 


	//origin[0] = orgn[0];
	//origin[1] = orgn[1];
	//origin[2] = 0;

	origin[0] = 0.0;
	origin[1] = 0.0;
	origin[2] = 0.5;


	int dim[3];

	ImageType::RegionType requestedRegion = img->GetLargestPossibleRegion();
	//index is often zero except for the collapsed index
	//index = requestedRegion.GetIndex();
	ImageType::SizeType size = requestedRegion.GetSize();
	dim[0] = size[0];
	dim[1] = size[1];
	dim[2] = 1;

	// prepare the binary image's voxel grid
	vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
	whiteImage->SetSpacing(spacing);
	//whiteImage->SetDimensions(dim);
	whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
	whiteImage->SetOrigin(origin);
	whiteImage->SetScalarTypeToUnsignedChar();
	whiteImage->AllocateScalars();
	// fill the image with foreground voxels:
	unsigned char inval = 255;
	unsigned char outval = 0;
	vtkIdType count = whiteImage->GetNumberOfPoints();
	for (vtkIdType i = 0; i < count; ++i)
	{
		whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
	}

    vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
        vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extruder->SetInput(vtkThroax);
    extruder->SetScaleFactor(1.);
    extruder->SetVector(0, 0, 10);
    extruder->Update();

    // polygonal data --> image stencil:
    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
    pol2stenc->SetInputConnection(extruder->GetOutputPort());
    pol2stenc->SetOutputOrigin(origin);
    pol2stenc->SetOutputSpacing(spacing);
    pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
    pol2stenc->Update();
 
    // cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInput(whiteImage);
    imgstenc->SetStencil(pol2stenc->GetOutput());
    imgstenc->ReverseStencilOff();
    imgstenc->SetBackgroundValue(outval);
    imgstenc->Update();

  	//convert to itk image
	//typedef itk::Image<unsigned char, 2> ImageType;
	typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
 
	VTKImageToImageType::Pointer vtkImageToImageFilter = VTKImageToImageType::New();
	vtkImageToImageFilter->SetInput(imgstenc->GetOutput());
	vtkImageToImageFilter->Update();

	//ImageType::Pointer image = ImageType::New();
    //image->Graft(vtkImageToImageFilter->GetOutput()); // Need to do this because QuickView can't accept const

	//vtkSmartPointer<vtkImageMapper> mapper = 
	//	vtkSmartPointer<vtkImageMapper>::New();
	//mapper->SetInput(imgstenc->GetOutput());
 //
	//vtkSmartPointer<vtkActor2D> actor = 
	//	vtkSmartPointer<vtkActor2D>::New();
	//actor->SetMapper(mapper);
 //
	//// Setup render window, renderer, and interactor
	//vtkSmartPointer<vtkRenderer> renderer = 
	//	vtkSmartPointer<vtkRenderer>::New();
	//vtkSmartPointer<vtkRenderWindow> renderWindow = 
	//	vtkSmartPointer<vtkRenderWindow>::New();
	//renderWindow->AddRenderer(renderer);
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//renderWindowInteractor->SetRenderWindow(renderWindow);
	//renderer->AddActor(actor);
 //
	//renderWindow->Render();
	//renderWindowInteractor->Start();

	//ImageType::DirectionType d = img->GetDirection();

	//typedef itk::ChangeInformationImageFilter<ImageType> ChangeType;
	//ChangeType::Pointer changer = ChangeType::New();
	//changer->SetInput(image);
	////changer->SetInput(vtkImageToImageFilter->GetOutput());
	//changer->SetOutputSpacing(sp);
	//changer->SetOutputOrigin(orgn);
	//changer->SetOutputDirection(d);
	//changer->ChangeDirectionOn();
	//changer->ChangeSpacingOn();
	//changer->ChangeOriginOn();
	//changer->Update();

	//ImageType::Pointer correctImage = changer->GetOutput();

	ImageType::Pointer correctImage = ImageType::New();
	//correctImage->Graft(vtkImageToImageFilter->GetOutput());
	correctImage->CopyInformation(img);
	
	typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
	ConstIteratorType inputIt(vtkImageToImageFilter->GetOutput(), vtkImageToImageFilter->GetOutput()->GetLargestPossibleRegion());
	typedef itk::ImageRegionIterator< ImageType> IteratorType;
	ImageType::RegionType outputRegion = img->GetLargestPossibleRegion();
	correctImage->SetRegions( outputRegion );
	correctImage->Allocate();
	IteratorType outputIt( correctImage, outputRegion);

	for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
	{
		outputIt.Set( inputIt.Get() );
	}

	return correctImage;
}





itk::Image<unsigned char, 3>::Pointer pdp::OtsuStep::labelling(itk::Image<unsigned char, 3>::Pointer img)
{
	typedef itk::Image<unsigned char, 3>  ImageType;
	typedef itk::BinaryShapeKeepNObjectsImageFilter< ImageType > BinaryOpeningType;
	BinaryOpeningType::Pointer opening = BinaryOpeningType::New();
	opening->SetInput( img );
	opening->SetForegroundValue( 255 );
	opening->SetBackgroundValue( 0 );
	opening->SetNumberOfObjects( 1 );
	//opening->SetFullyConnected(  );
	//opening->SetAttribute(  );	//default NumberOfPixels

	opening->Update();
	return opening->GetOutput();

}






void pdp::OtsuStep::produceThroax(itk::Image<unsigned char, 3>::Pointer img, itk::Image<unsigned char, 3>::Pointer mask)
{

	typedef itk::Image<unsigned char, 3> ImageType;

	typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
	ConstIteratorType inputIt(mask, mask->GetLargestPossibleRegion());

	typedef itk::ImageRegionIterator< ImageType> IteratorType;
	IteratorType outputIt( img, img->GetLargestPossibleRegion());

	for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
	{
		
		if (inputIt.Get() == 0) outputIt.Set(0);
	}

	

}



/*
* Supervised Range Constrained Otsu thresholding with mask
* inside value corresponds to the background
*/
template <class PixelType, int dim>
typename itk::Image<unsigned char, dim>::Pointer pdp::OtsuStep::SVRC_OtsuWithMask(typename itk::Image<PixelType, dim>::Pointer img, float lowerBound, float upperBound, typename itk::Image<unsigned char, 3>::Pointer mask)
{
	typedef itk::Image<PixelType, dim> InputImageType;
	typedef itk::Image<unsigned char, dim> OutputImageType;

	typedef itk::OtsuThresholdImageFilter<InputImageType, OutputImageType> OtsuFilterType;
	OtsuFilterType::Pointer otsuFilter = OtsuFilterType::New();

    itkFilterProgressWatcher watcher(otsuFilter, this, 0.2f, 0.9f);

	otsuFilter->SetInput(img);
	//inside value corresponds to the background
	//otsuFilter->SetInsideValue(0);
	//otsuFilter->SetOutsideValue(255);

	otsuFilter->SetInsideValue(255);
	otsuFilter->SetOutsideValue(0);

	otsuFilter->SetNumberOfHistogramBins(256);
	otsuFilter->SetLowerBound(lowerBound);
	otsuFilter->SetUpperBound(upperBound);
	otsuFilter->SetMaskOnOff(true);
	otsuFilter->SetMask(mask);
	otsuFilter->Update();
	return otsuFilter->GetOutput();
}




pdp::OtsuStep::OtsuStep(QString info)
    : info(info)
{ }

QString pdp::OtsuStep::name() const
{
    return "Otsu step: " + info;
}

float pdp::OtsuStep::relativePercent() const
{
    return 1.0;
}

void pdp::OtsuStep::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the Otsu step!" << std::endl;	

	typedef itk::Image<unsigned char, 3> ImageType;
	
	// Cast the MITK -> ITK image
	//mitk::Image::Pointer mitkImage = input.getOriginalLungCTImage();
	mitk::Image::Pointer mitkImage = input.getImage(inputStepName);
	mitkImage->DisconnectPipeline();
	itk::Image<float, 3>::Pointer img;
	mitk::CastToItkImage(mitkImage, img);
	
    emit stepProgress(0.2f);
    
	ImageType::Pointer imgThroaxOtsu = SVRC_Otsu<float, 3>(img, pdp::THORAX_LOWER_BOUND, pdp::THORAX_UPPER_BOUND);

	ImageType::Pointer imgOut = Morphologies(imgThroaxOtsu);
    emit stepProgress(0.9f);


	ImageType::Pointer imgLungsOtsu = SVRC_OtsuWithMask<float, 3>(img, pdp::LUNGS_LOWER_BOUND, pdp::LUNGS_UPPER_BOUND, imgOut);
	produceThroax(imgLungsOtsu, imgOut);


	ImageType::Pointer labelled = labelling(imgLungsOtsu);


	// Cast the ITK -> MITK image and add it to the datatree.
	mitk::Image::Pointer mitkOut = mitk::Image::New();
	//mitk::CastToMitkImage(imgLungsOtsu, mitkOut);
	mitk::CastToMitkImage(labelled, mitkOut);
    output.addImage(mitkOut, "Otsu");



    emit stepProgress(1.00f);
}



