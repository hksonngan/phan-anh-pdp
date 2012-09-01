// "pdp"
#include "mitkActiveContourTool.h"
#include "mitkActiveContourTool.xpm"
#include "QmitkActiveContourToolGUI.h"
//#include "StartStopChanVese.h"

// MITK
#include "mitkToolManager.h"
#include <mitkSegTool2D.h>
#include <mitkExtractImageFilter.h>
#include <mitkITKImageImport.h>
#include "mitkImage.h"

#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageWriter.h>

// VTK

#include "itkVTKPolyDataWriter.h"

// ITK
#include "itkScalarChanAndVeseSparseLevelSetImageFilter.h"
#include "itkScalarChanAndVeseLevelSetFunction.h"
#include "itkScalarChanAndVeseLevelSetFunctionData.h"
#include "itkConstrainedRegionBasedLevelSetFunctionSharedData.h"
//#include "itkImageFileReader.h"
//#include "itkImageFileWriter.h"
//#include "itkImage.h"
//#include "itkAtanRegularizedHeavisideStepFunction.h"
#include "itkFastMarchingImageFilter.h"



mitk::ActiveContourTool::ActiveContourTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_Ut = new pdp::Utilities(m_Ext);
	m_Initialize = true;
	m_StepSize = 1;
	m_DirectedPlay = 0;
	m_CurrentNumberOfIterations = 0;
}

mitk::ActiveContourTool::~ActiveContourTool()
{}

const char** mitk::ActiveContourTool::GetXPM() const
{
	return mitkActiveContourTool_xpm;
}

const char* mitk::ActiveContourTool::GetName() const
{
	return "Active Contour Tool";
}

const char* mitk::ActiveContourTool::GetGroup() const
{
	return "default";
}

void mitk::ActiveContourTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::ActiveContourTool, int>(this, &mitk::ActiveContourTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::ActiveContourTool, int>(this, &mitk::ActiveContourTool::UpdateRadiusSlider);
}

void mitk::ActiveContourTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::ActiveContourTool, int>(this, &mitk::ActiveContourTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::ActiveContourTool, int>(this, &mitk::ActiveContourTool::UpdateRadiusSlider);
}

void mitk::ActiveContourTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::ActiveContourTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	
	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::ActiveContours()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Input images
	// get image
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("OriginalImageThickenings");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		std::cout << "OriginalImageThickenings found!\n";
	}
	mitk::Image::Pointer image = (mitk::Image*)imageNode->GetData();
	std::cout << "Reference image name: " << imageNode->GetName() << "\n";

	mitk::Point3D savedMitkOrigin;
	savedMitkOrigin = image->GetGeometry()->GetOrigin();

	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	int affectedDimension( -1 );
	int affectedSlice( -1 );
	SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

	std::cout << "Affected dimension: " << affectedDimension << ", affected slice: " << affectedSlice << "\n";

	// 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice;// = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

	try
	{
		// now we extract the correct slice from the volume, resulting in a 2D image
		 mitk::ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
		extractor->SetInput( image );
		extractor->SetSliceDimension( affectedDimension );
		extractor->SetSliceIndex( affectedSlice );
		extractor->SetTimeStep( 0 );
		extractor->Update();

		// here we have a single slice that can be modified
		slice = extractor->GetOutput();
	}
	catch(...)
	{
		// not working
		return;
    }




	// The use of the ScalarChanAndVeseSparseLevelSetImageFilter is
	// illustrated in the following example. The implementation of this filter in
	// ITK is based on the paper by Chan And Vese.  This
	// implementation extends the functionality of the
	// level-set filters in ITK by using region-based variational techniques. These methods
	// do not rely on the presence of edges in the images.
	//
	// ScalarChanAndVeseSparseLevelSetImageFilter expects two inputs.  The first is
	// an initial level set in the form of an \doxygen{Image}. The second input
	// is a feature image. For this algorithm, the feature image is the original
	// raw or preprocessed image. Several parameters are required by the algorithm
	// for regularization and weights of different energy terms. The user is encouraged to
	// change different parameter settings to optimize the code example on their images.
	//
	// Let's start by including the headers of the main filters involved in the
	// preprocessing.
	//

	//if( argc < 6 )
	//  {
	//  std::cerr << "Missing arguments" << std::endl;
	//  std::cerr << "Usage: " << std::endl;
	//  std::cerr << argv[0] << " featureImage outputImage";
	//  std::cerr << " startx starty seedValue" << std::endl;
	//  return EXIT_FAILURE;
	//  }

	unsigned int nb_iteration = 500;
	double rms = 0.;
	double epsilon = 1.;
	double curvature_weight = 0.;
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	//
	//  We now define the image type using a particular pixel type and
	//  dimension. In this case the \code{float} type is used for the pixels
	//  due to the requirements of the smoothing filter.
	//
	const unsigned int Dimension = 2;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;


	typedef itk::ScalarChanAndVeseLevelSetFunctionData< InternalImageType,
	InternalImageType > DataHelperType;

	typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData<
	InternalImageType, InternalImageType, DataHelperType > SharedDataHelperType;

	typedef itk::ScalarChanAndVeseLevelSetFunction< InternalImageType,
	InternalImageType, SharedDataHelperType > LevelSetFunctionType;


	//  We declare now the type of the numerically discretized Step and Delta functions that
	//  will be used in the level-set computations for foreground and background regions
	//
	typedef itk::AtanRegularizedHeavisideStepFunction< ScalarPixelType,
	ScalarPixelType >  DomainFunctionType;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	// We instantiate reader and writer types in the following lines.
	//
	//typedef itk::ImageFileReader< InternalImageType > ReaderType;
	//typedef itk::ImageFileWriter< InternalImageType > WriterType;

	//ReaderType::Pointer reader = ReaderType::New();
	//WriterType::Pointer writer = WriterType::New();

	//reader->SetFileName( "D:\\hachmann\\SVN\\ThreeDEditing\\Tools\\ActiveContourTool\\src\\release\\Data\\example2D.png"/*argv[1]*/ );
	//reader->SetFileName( "D:\\hachmann\\SVN\\ThreeDEditing\\Tools\\ActiveContourTool\\src\\release\\Data\\30.png"/*argv[1]*/ );
	//reader->Update();

	// add input image to datastorage
	/*mitk::Image::Pointer input = mitk::ImportItkImage(reader->GetOutput());
	mitk::DataNode::Pointer inNode = mitk::DataNode::New();
	inNode->SetData(input);
	inNode->SetName("Input");
	m_Ext->GetDataStorage()->Add(inNode);*/

	mitk::DataNode::Pointer internalInputNode = mitk::DataNode::New();
	internalInputNode->SetData(slice);
	internalInputNode->SetName("Internal Input");
	m_Ext->GetDataStorage()->Add(internalInputNode);

	//slice->Print(std::cout);

	mitk::Vector3D spacing;
	spacing.Fill(1.0);
	slice->GetGeometry()->SetSpacing(spacing);
	mitk::Point3D origin;
	origin.Fill(0.0);
	slice->SetOrigin(origin);

	//mitk::ImageToItk<InternalImageType>::Pointer convert = mitk::ImageToItk<InternalImageType>::New();
	//convert->SetInput(slice);
	////convert->UpdateOutputInformation();
	//convert->Update();
	//InternalImageType::Pointer featureImage = convert->GetOutput();


	InternalImageType::Pointer featureImage;
	mitk::CastToItkImage( slice, featureImage );
	//featureImage = reader->GetOutput();
	//featureImage->Print(std::cout);


	//  We declare now the type of the FastMarchingImageFilter that
	//  will be used to generate the initial level set in the form of a distance
	//  map.
	//
	typedef  itk::FastMarchingImageFilter< InternalImageType, InternalImageType >
	FastMarchingFilterType;

	FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();

	//  The FastMarchingImageFilter requires the user to provide a seed
	//  point from which the level set will be generated. The user can actually
	//  pass not only one seed point but a set of them. Note the the
	//  FastMarchingImageFilter is used here only as a helper in the
	//  determination of an initial level set. We could have used the
	//  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
	//
	//  The seeds are passed stored in a container. The type of this
	//  container is defined as \code{NodeContainer} among the
	//  FastMarchingImageFilter traits.
	//
	typedef FastMarchingFilterType::NodeContainer  NodeContainer;
	typedef FastMarchingFilterType::NodeType       NodeType;

	NodeContainer::Pointer seeds = NodeContainer::New();

	InternalImageType::IndexType  seedPosition;

	//seedPosition[0] = 100;//atoi( argv[3] );
	//seedPosition[1] = 100;//atoi( argv[4] );

	//seedPosition[0] = 0;//atoi( argv[3] );
	//seedPosition[1] = 0;//atoi( argv[4] );

	//seedPosition[0] = 200;//atoi( argv[3] ); // 200 nach rechts
	//seedPosition[1] = 400;//atoi( argv[4] ); // 400 nach unten

	seedPosition[0] = 500;//atoi( argv[3] ); // 500 nach rechts
	seedPosition[1] = 100;//atoi( argv[4] ); // 100 nach unten

	//const double initialDistance = 1000.0; //atof( argv[5] );
	const double initialDistance = 500.0; //atof( argv[5] );
	//const double initialDistance = 100.0; //atof( argv[5] );
	//const double initialDistance = 10.0; //atof( argv[5] );
	//const double initialDistance = 1.0; //atof( argv[5] );

	NodeType node;

	const double seedValue = - initialDistance;

	node.SetValue( seedValue );
	node.SetIndex( seedPosition );

	//  The list of nodes is initialized and then every node is inserted using
	//  the \code{InsertElement()}.
	//
	seeds->Initialize();
	seeds->InsertElement( 0, node );

	
	//  The set of seed nodes is passed now to the
	//  FastMarchingImageFilter with the method
	//  \code{SetTrialPoints()}.
	//
	fastMarching->SetTrialPoints(  seeds  );


	//  Since the FastMarchingImageFilter is used here just as a
	//  Distance Map generator. It does not require a speed image as input.
	//  Instead the constant value $1.0$ is passed using the
	//  \code{SetSpeedConstant()} method.
	//
	fastMarching->SetSpeedConstant( 1.0 );

	//  The FastMarchingImageFilter requires the user to specify the
	//  size of the image to be produced as output. This is done using the
	//  \code{SetOutputSize()}. Note that the size is obtained here from the
	//  output image of the smoothing filter. The size of this image is valid
	//  only after the \code{Update()} methods of this filter has been called
	//  directly or indirectly.
	//
	fastMarching->SetOutputSize(
	featureImage->GetBufferedRegion().GetSize() );
	fastMarching->Update();

	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.

	typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter< InternalImageType,
	InternalImageType, InternalImageType, LevelSetFunctionType,
	SharedDataHelperType > MultiLevelSetType;

	MultiLevelSetType::Pointer levelSetFilter = MultiLevelSetType::New();

	//  We set the function count to 1 since a single level-set is being evolved.
	//
	levelSetFilter->SetFunctionCount( 1 );

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	/*levelSetFilter->SetFeatureImage( featureImage2 );
	levelSetFilter->SetLevelSet( 0, featureImage2 );*/

	levelSetFilter->SetFeatureImage( featureImage );
	levelSetFilter->SetLevelSet( 0, fastMarching->GetOutput() );

	//  Once activiated the level set evolution will stop if the convergence
	//  criteria or if the maximum number of iterations is reached.  The
	//  convergence criteria is defined in terms of the root mean squared (RMS)
	//  change in the level set function. The evolution is said to have
	//  converged if the RMS change is below a user specified threshold.  In a
	//  real application is desirable to couple the evolution of the zero set
	//  to a visualization module allowing the user to follow the evolution of
	//  the zero set. With this feedback, the user may decide when to stop the
	//  algorithm before the zero set leaks through the regions of low gradient
	//  in the contour of the anatomical structure to be segmented.
	//
	levelSetFilter->SetNumberOfIterations( nb_iteration );
	levelSetFilter->SetMaximumRMSError( rms );

	//  Often, in real applications, images have different pixel resolutions. In such
	//  cases, it is best to use the native spacings to compute derivatives etc rather
	//  than sampling the images.
	//
	//levelSetFilter->SetUseImageSpacing( 0.79 );
	levelSetFilter->SetUseImageSpacing( 1 );

	//  For large images, we may want to compute the level-set over the initial supplied
	//  level-set image. This saves a lot of memory.
	//
	levelSetFilter->SetInPlace( false );
	//levelSetFilter->SetInPlace( true);

	//  For the level set with phase 0, set different parameters and weights. This may
	//  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	//
	levelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	levelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( curvature_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );

	levelSetFilter->Update();
	
	// add output to data storage
	//mitk::Image::Pointer output = mitk::ImportItkImage(featureImage);
	mitk::Image::Pointer output = mitk::ImportItkImage(levelSetFilter->GetOutput());
	mitk::DataNode::Pointer outNode = mitk::DataNode::New();
	outNode->SetData(output);
	outNode->SetName("Output");
	m_Ext->GetDataStorage()->Add(outNode);

	mitk::Image::Pointer output2 = mitk::ImportItkImage(fastMarching->GetOutput());
	mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
	outNode2->SetData(output2);
	outNode2->SetName("Fast Marching Output");
	m_Ext->GetDataStorage()->Add(outNode2);



	//mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
	//writer->SetInput(mitk::ImportItkImage(featureImage));
	//writer->SetFileName("FeatureImage");
	//writer->SetExtension(".png");
	//writer->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::ActiveContoursVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Input images
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("OriginalImageThickenings");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		std::cout << "OriginalImageThickenings found!\n";
	}
	mitk::Image::Pointer image = (mitk::Image*)imageNode->GetData();
	std::cout << "Reference image name: " << imageNode->GetName() << "\n";

	mitk::Point3D savedMitkOrigin;
	savedMitkOrigin = image->GetGeometry()->GetOrigin();

	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	int affectedDimension( -1 );
	int affectedSlice( -1 );
	SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

	std::cout << "Affected dimension: " << affectedDimension << ", affected slice: " << affectedSlice << "\n";

	// 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice;
	try
	{
		// now we extract the correct slice from the volume, resulting in a 2D image
		 mitk::ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
		extractor->SetInput( image );
		extractor->SetSliceDimension( affectedDimension );
		extractor->SetSliceIndex( affectedSlice );
		extractor->SetTimeStep( 0 );
		extractor->Update();

		// here we have a single slice that can be modified
		slice = extractor->GetOutput();
	}
	catch(...)
	{
		// not working
		return;
    }

	mitk::DataNode::Pointer internalInputNode = mitk::DataNode::New();
	internalInputNode->SetData(slice);
	internalInputNode->SetName("Internal Input");
	internalInputNode->SetOpacity(0.5);
	m_Ext->GetDataStorage()->Add(internalInputNode);

	mitk::DataNode::Pointer outNode = mitk::DataNode::New();
	outNode->SetName("Output");
	m_Ext->GetDataStorage()->Add(outNode);

	mitk::Vector3D spacing;
	spacing.Fill(1.0);
	slice->GetGeometry()->SetSpacing(spacing);
	mitk::Point3D origin;
	origin.Fill(0.0);
	slice->SetOrigin(origin);

	const unsigned int Dimension = 2;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;

	// Convert to float
	InternalImageType::Pointer featureImage;
	mitk::CastToItkImage( slice, featureImage );
	//featureImage->Print(std::cout);

	// loop
	int myNumberOfIterations = 100;
	for(int i = 0; i < myNumberOfIterations; i++)
	{
		outNode->SetData(Iteration(i*5, featureImage));
	
		// show
		mitk::RenderingManager::GetInstance()->InitializeViews( outNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
		// try FORce updata ^^
		//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}
	
	mitk::RenderingManager::GetInstance()->InitializeViews( outNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


mitk::Image::Pointer mitk::ActiveContourTool::Iteration(int iteration, itk::Image<float,2>::Pointer input)
{
	std::cout << "Iteration: " << iteration << "\n";
	
	unsigned int nb_iteration = 500;
	double rms = 0.;
	double epsilon = 1.;
	double curvature_weight = 0.;
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	//
	//  We now define the image type using a particular pixel type and
	//  dimension. In this case the \code{float} type is used for the pixels
	//  due to the requirements of the smoothing filter.
	//
	const unsigned int Dimension = 2;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;

	typedef itk::ScalarChanAndVeseLevelSetFunctionData< InternalImageType,
	InternalImageType > DataHelperType;

	typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData<
	InternalImageType, InternalImageType, DataHelperType > SharedDataHelperType;

	typedef itk::ScalarChanAndVeseLevelSetFunction< InternalImageType,
	InternalImageType, SharedDataHelperType > LevelSetFunctionType;


	//  We declare now the type of the numerically discretized Step and Delta functions that
	//  will be used in the level-set computations for foreground and background regions
	//
	typedef itk::AtanRegularizedHeavisideStepFunction< ScalarPixelType,
	ScalarPixelType >  DomainFunctionType;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	//  We declare now the type of the FastMarchingImageFilter that
	//  will be used to generate the initial level set in the form of a distance
	//  map.
	typedef  itk::FastMarchingImageFilter< InternalImageType, InternalImageType >
	FastMarchingFilterType;

	FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();

	//  The FastMarchingImageFilter requires the user to provide a seed
	//  point from which the level set will be generated. The user can actually
	//  pass not only one seed point but a set of them. Note the the
	//  FastMarchingImageFilter is used here only as a helper in the
	//  determination of an initial level set. We could have used the
	//  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
	//
	//  The seeds are passed stored in a container. The type of this
	//  container is defined as \code{NodeContainer} among the
	//  FastMarchingImageFilter traits.
	typedef FastMarchingFilterType::NodeContainer  NodeContainer;
	typedef FastMarchingFilterType::NodeType       NodeType;

	NodeContainer::Pointer seeds = NodeContainer::New();

	InternalImageType::IndexType  seedPosition;

	seedPosition[0] = 500;//atoi( argv[3] ); // 500 nach rechts
	seedPosition[1] = 100;//atoi( argv[4] ); // 100 nach unten

	//const double initialDistance = 1000.0; //atof( argv[5] );
	//const double initialDistance = 500.0; //atof( argv[5] );
	//const double initialDistance = 100.0; //atof( argv[5] );
	//const double initialDistance = 10.0; //atof( argv[5] );
	//const double initialDistance = 1.0; //atof( argv[5] );
	double initialDistance = iteration;

	NodeType node;

	const double seedValue = - initialDistance;

	node.SetValue( seedValue );
	node.SetIndex( seedPosition );

	//  The list of nodes is initialized and then every node is inserted using
	//  the \code{InsertElement()}.
	//
	seeds->Initialize();
	seeds->InsertElement( 0, node );

	
	//  The set of seed nodes is passed now to the
	//  FastMarchingImageFilter with the method
	//  \code{SetTrialPoints()}.
	//
	fastMarching->SetTrialPoints(  seeds  );


	//  Since the FastMarchingImageFilter is used here just as a
	//  Distance Map generator. It does not require a speed image as input.
	//  Instead the constant value $1.0$ is passed using the
	//  \code{SetSpeedConstant()} method.
	//
	fastMarching->SetSpeedConstant( 1.0 );

	//  The FastMarchingImageFilter requires the user to specify the
	//  size of the image to be produced as output. This is done using the
	//  \code{SetOutputSize()}. Note that the size is obtained here from the
	//  output image of the smoothing filter. The size of this image is valid
	//  only after the \code{Update()} methods of this filter has been called
	//  directly or indirectly.
	//
	fastMarching->SetOutputSize( input->GetBufferedRegion().GetSize() );
	fastMarching->Update();

	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.

	typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter< InternalImageType,	InternalImageType, InternalImageType, LevelSetFunctionType,	SharedDataHelperType > MultiLevelSetType;
	MultiLevelSetType::Pointer levelSetFilter = MultiLevelSetType::New();

	typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter< InternalImageType,	InternalImageType, InternalImageType, LevelSetFunctionType,	SharedDataHelperType >::TimeStepType TimeStepType;


	//  We set the function count to 1 since a single level-set is being evolved.
	//
	levelSetFilter->SetFunctionCount( 1 );

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	//levelSetFilter->SetFeatureImage( featureImage2 );
	//levelSetFilter->SetLevelSet( 0, featureImage2 );

	levelSetFilter->SetFeatureImage( input );
	levelSetFilter->SetLevelSet( 0, fastMarching->GetOutput() );

	//  Once activiated the level set evolution will stop if the convergence
	//  criteria or if the maximum number of iterations is reached.  The
	//  convergence criteria is defined in terms of the root mean squared (RMS)
	//  change in the level set function. The evolution is said to have
	//  converged if the RMS change is below a user specified threshold.  In a
	//  real application is desirable to couple the evolution of the zero set
	//  to a visualization module allowing the user to follow the evolution of
	//  the zero set. With this feedback, the user may decide when to stop the
	//  algorithm before the zero set leaks through the regions of low gradient
	//  in the contour of the anatomical structure to be segmented.
	//
	levelSetFilter->SetNumberOfIterations( nb_iteration );
	levelSetFilter->SetMaximumRMSError( rms );

	//  Often, in real applications, images have different pixel resolutions. In such
	//  cases, it is best to use the native spacings to compute derivatives etc rather
	//  than sampling the images.
	//
	levelSetFilter->SetUseImageSpacing( 1 );

	//  For large images, we may want to compute the level-set over the initial supplied
	//  level-set image. This saves a lot of memory.
	//
	levelSetFilter->SetInPlace( false );

	//  For the level set with phase 0, set different parameters and weights. This may
	//  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	//
	levelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	levelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( curvature_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	levelSetFilter->Update();

	return mitk::ImportItkImage(levelSetFilter->GetOutput());
}






void mitk::ActiveContourTool::Reset()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Delete & remove old images
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("Output");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		m_Ext->GetDataStorage()->Remove(imageNode);
		//imageNode->Delete();
	}

	unsigned int nb_iteration = 500;
	double rms = 0.;
	double epsilon = 1.;
	double curvature_weight = 0.; 
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	InternalImageType::Pointer featureImage = GetInputImage();
	InternalImageType::Pointer distanceImage = CreateDistanceMap(featureImage);	

	// add feature and distance image to datastorage
	
	//AddToDataStorage(featureImage, "Feature Image", 0.5);
	//AddToDataStorage(distanceImage, "Distance Image", 0.2);

	
	m_LevelSetFilter = MultiLevelSetType::New();	
	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.
	//  We set the function count to 1 since a single level-set is being evolved.
	//

	m_LevelSetFilter->SetFunctionCount( 1 );

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	m_LevelSetFilter->SetFeatureImage( featureImage );
	m_LevelSetFilter->SetLevelSet( 0, distanceImage );

	//  Once activiated the level set evolution will stop if the convergence
	//  criteria or if the maximum number of iterations is reached.  The
	//  convergence criteria is defined in terms of the root mean squared (RMS)
	//  change in the level set function. The evolution is said to have
	//  converged if the RMS change is below a user specified threshold.  In a
	//  real application is desirable to couple the evolution of the zero set
	//  to a visualization module allowing the user to follow the evolution of
	//  the zero set. With this feedback, the user may decide when to stop the
	//  algorithm before the zero set leaks through the regions of low gradient
	//  in the contour of the anatomical structure to be segmented.
	//
	m_LevelSetFilter->SetNumberOfIterations( nb_iteration );
	m_LevelSetFilter->SetMaximumRMSError( rms );

	//  Often, in real applications, images have different pixel resolutions. In such
	//  cases, it is best to use the native spacings to compute derivatives etc rather
	//  than sampling the images.
	//
	m_LevelSetFilter->SetUseImageSpacing( 1 );

	//  For large images, we may want to compute the level-set over the initial supplied
	//  level-set image. This saves a lot of memory.
	//
	m_LevelSetFilter->SetInPlace( false );

	//  For the level set with phase 0, set different parameters and weights. This may
	//  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	//


	m_LevelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( curvature_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	m_LevelSetFilter->Update();

	m_OutNode = AddToDataStorage(m_LevelSetFilter->GetOutput(), "Output", 1.0);


	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::Initialize()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	unsigned int nb_iteration = 500;
	double rms = 0.;
	double epsilon = 1.;
	double curvature_weight = 0.; 
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	InternalImageType::Pointer featureImage = GetInputImage();
	InternalImageType::Pointer distanceImage = CreateDistanceMap(featureImage);	

	// add feature and distance image to datastorage
	AddToDataStorage(featureImage, "Feature Image", 0.5);
	AddToDataStorage(distanceImage, "Distance Image", 0.2);

	m_LevelSetFilter = MultiLevelSetType::New();	
	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.
	//  We set the function count to 1 since a single level-set is being evolved.
	//
	m_LevelSetFilter->SetFunctionCount( 1 );

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	m_LevelSetFilter->SetFeatureImage( featureImage );
	m_LevelSetFilter->SetLevelSet( 0, distanceImage );

	//  Once activiated the level set evolution will stop if the convergence
	//  criteria or if the maximum number of iterations is reached.  The
	//  convergence criteria is defined in terms of the root mean squared (RMS)
	//  change in the level set function. The evolution is said to have
	//  converged if the RMS change is below a user specified threshold.  In a
	//  real application is desirable to couple the evolution of the zero set
	//  to a visualization module allowing the user to follow the evolution of
	//  the zero set. With this feedback, the user may decide when to stop the
	//  algorithm before the zero set leaks through the regions of low gradient
	//  in the contour of the anatomical structure to be segmented.
	//
	m_LevelSetFilter->SetNumberOfIterations( nb_iteration );
	m_LevelSetFilter->SetMaximumRMSError( rms );

	//  Often, in real applications, images have different pixel resolutions. In such
	//  cases, it is best to use the native spacings to compute derivatives etc rather
	//  than sampling the images.
	//
	m_LevelSetFilter->SetUseImageSpacing( 1 );

	//  For large images, we may want to compute the level-set over the initial supplied
	//  level-set image. This saves a lot of memory.
	//
	m_LevelSetFilter->SetInPlace( false );

	//  For the level set with phase 0, set different parameters and weights. This may
	//  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	//
	m_LevelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( curvature_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	m_LevelSetFilter->Update();

	m_OutNode = AddToDataStorage(m_LevelSetFilter->GetOutput(), "Output", 1.0);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::Run()
{
	while( m_DirectedPlay == 1)
	{
		std::cout << "Number of Iterations: " << m_CurrentNumberOfIterations << "\n";
		SetNumberOfIterations(m_CurrentNumberOfIterations);

		m_LevelSetFilter->Run(m_StepSize);
		m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput()));
	
		// show
		mitk::RenderingManager::GetInstance()->InitializeViews( m_OutNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		m_CurrentNumberOfIterations += m_StepSize;
		QCoreApplication::processEvents();
	}
}

itk::Image< float, 2 >::Pointer mitk::ActiveContourTool::GetInputImage()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Input images
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("OriginalImageThickenings");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		std::cout << "OriginalImageThickenings found!\n";
	}
	mitk::Image::Pointer image = (mitk::Image*)imageNode->GetData();
	std::cout << "Reference image name: " << imageNode->GetName() << "\n";

	mitk::Point3D savedMitkOrigin;
	savedMitkOrigin = image->GetGeometry()->GetOrigin();

	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	int affectedDimension( -1 );
	int affectedSlice( -1 );

	SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

	affectedSlice = 198;	// todo

	std::cout << "Affected dimension: " << affectedDimension << ", affected slice: " << affectedSlice << "\n";

	// 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
	// now we extract the correct slice from the volume, resulting in an 2D image
	ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
	extractor->SetInput( image );
	extractor->SetSliceDimension( affectedDimension );
	extractor->SetSliceIndex( affectedSlice );
	extractor->SetTimeStep( 0 );
	extractor->Update();
	// here we have a single slice that can be modified
	Image::Pointer slice = extractor->GetOutput();

	std::cout << "1\n" << std::endl;

	mitk::Vector3D spacing;
	spacing.Fill(1.0);
	slice->GetGeometry()->SetSpacing(spacing);
	mitk::Point3D origin;
	origin.Fill(0.0);
	slice->SetOrigin(origin);

	std::cout << "2\n" << std::endl;

	const unsigned int Dimension = 2;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;

	std::cout << "3\n" << std::endl;

	// Convert to float
	InternalImageType::Pointer featureImage;
	mitk::CastToItkImage( slice, featureImage );

	std::cout << "Return from get input image\n" << std::endl;

	return featureImage;
}

itk::Image< float, 2 >::Pointer mitk::ActiveContourTool::CreateDistanceMap(itk::Image< float, 2 >::Pointer input)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	const unsigned int Dimension = 2;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;

	//  We declare now the type of the FastMarchingImageFilter that
	//  will be used to generate the initial level set in the form of a distance
	//  map.
	typedef  itk::FastMarchingImageFilter< InternalImageType, InternalImageType >
	FastMarchingFilterType;

	FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();

	//  The FastMarchingImageFilter requires the user to provide a seed
	//  point from which the level set will be generated. The user can actually
	//  pass not only one seed point but a set of them. Note the the
	//  FastMarchingImageFilter is used here only as a helper in the
	//  determination of an initial level set. We could have used the
	//  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
	//
	//  The seeds are passed stored in a container. The type of this
	//  container is defined as \code{NodeContainer} among the
	//  FastMarchingImageFilter traits.
	typedef FastMarchingFilterType::NodeContainer  NodeContainer;
	typedef FastMarchingFilterType::NodeType       NodeType;

	NodeContainer::Pointer seeds = NodeContainer::New();

	InternalImageType::IndexType  seedPosition;

	seedPosition[0] = 400;//atoi( argv[3] ); // 500 nach rechts
	seedPosition[1] = 300;//atoi( argv[4] ); // 100 nach unten

	//const double initialDistance = 1000.0; //atof( argv[5] );
	//const double initialDistance = 500.0; //atof( argv[5] );
	//const double initialDistance = 100.0; //atof( argv[5] );
	const double initialDistance = 10.0; //atof( argv[5] );
	//const double initialDistance = 1.0; //atof( argv[5] );
	//double initialDistance = iteration;

	NodeType node;

	const double seedValue = - initialDistance;

	node.SetValue( seedValue );
	node.SetIndex( seedPosition );


	InternalImageType::IndexType  seedPosition2;
	seedPosition2[0] = 200;//atoi( argv[3] ); // 500 nach rechts
	seedPosition2[1] = 300;
	NodeType node2;
	node2.SetValue( seedValue );
	node2.SetIndex( seedPosition2 );

	//  The list of nodes is initialized and then every node is inserted using
	//  the \code{InsertElement()}.
	//
	seeds->Initialize();
	seeds->InsertElement( 0, node );
	seeds->InsertElement( 1, node2 );


	/*InternalImageType::IndexType  seedPosition3;
	seedPosition2[0] = 100;
	seedPosition2[1] = 100;
	double radius = 2;
	SetBubble(seeds, 1, radius);*/



	// Bubble aus seed points

	
	//  The set of seed nodes is passed now to the
	//  FastMarchingImageFilter with the method
	//  \code{SetTrialPoints()}.
	//
	fastMarching->SetTrialPoints(  seeds  );


	//  Since the FastMarchingImageFilter is used here just as a
	//  Distance Map generator. It does not require a speed image as input.
	//  Instead the constant value $1.0$ is passed using the
	//  \code{SetSpeedConstant()} method.
	//
	fastMarching->SetSpeedConstant( 1.0 );

	//  The FastMarchingImageFilter requires the user to specify the
	//  size of the image to be produced as output. This is done using the
	//  \code{SetOutputSize()}. Note that the size is obtained here from the
	//  output image of the smoothing filter. The size of this image is valid
	//  only after the \code{Update()} methods of this filter has been called
	//  directly or indirectly.
	//
	fastMarching->SetOutputSize( input->GetBufferedRegion().GetSize() );
	fastMarching->Update();

	return fastMarching->GetOutput();
}

mitk::DataNode::Pointer mitk::ActiveContourTool::AddToDataStorage(itk::Image< float, 2 >::Pointer data, char* name, double opacity)
{
	mitk::DataNode::Pointer inputNode = mitk::DataNode::New();
	inputNode->SetData(mitk::ImportItkImage(data));
	inputNode->SetName(name);
	inputNode->SetOpacity(opacity);
	int newId = GetUniqueId();
	inputNode->SetIntProperty("UniqueID", newId);
	inputNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(inputNode);

	return inputNode;
}

 int mitk::ActiveContourTool::GetUniqueId()
 {
	// Search for free index
	std::list<int> ids;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Ext->GetDataStorage()->GetAll();
	for(unsigned int j = 0; j < allNodes->size(); j++)
	{
		int id = -1;
		if(!allNodes->at(j)->GetIntProperty("UniqueID", id))
		{
			std::cout << "Error: Data found without unique id: " << allNodes->at(j)->GetName() << "\n";
			continue;
		}
		ids.push_back(id);	
	}
	ids.sort();
	int counter = 0;
	for(std::list<int>::iterator it = ids.begin(); it != ids.end(); it++)
	{
		//std::cout << "Counter: " << counter << "\n";
		if(*it == counter)
		{
			counter++;
		}
		else
		{
			// free id found
			break;
		}
	}
	return counter;
}

//void mitk::ActiveContourTool::SetBubble(NodeContainer::Pointer seeds, InternalImageType::IndexType newSeedPoint, double radius)
//{
//	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	// Create Circle with radius




	////const double initialDistance = 1000.0; //atof( argv[5] );
	////const double initialDistance = 500.0; //atof( argv[5] );
	////const double initialDistance = 100.0; //atof( argv[5] );
	//const double initialDistance = 10.0; //atof( argv[5] );
	////const double initialDistance = 1.0; //atof( argv[5] );
	////double initialDistance = iteration;

	//NodeType node;

	//const double seedValue = - initialDistance;

	//node.SetValue( seedValue );
	//node.SetIndex( seedPosition );


	//InternalImageType::IndexType  seedPosition2;
	//seedPosition2[0] = 200;//atoi( argv[3] ); // 500 nach rechts
	//seedPosition2[1] = 300;
	//NodeType node2;
	//node2.SetValue( seedValue );
	//node2.SetIndex( seedPosition2 );

	////  The list of nodes is initialized and then every node is inserted using
	////  the \code{InsertElement()}.
	////
	//seeds->Initialize();
	//seeds->InsertElement( 0, node );
	//seeds->InsertElement( 1, node2 );
	
//	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
//}

void mitk::ActiveContourTool::Stop()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CurrentNumberOfIterations = 0;
	SetNumberOfIterations(m_CurrentNumberOfIterations);
	m_DirectedPlay = 0;
	Reset();
	m_Initialize = false;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::StepBack()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::ReversePlay()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_DirectedPlay = 2;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::Pause()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DirectedPlay = 0;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::Play()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DirectedPlay = 1;
	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}
	Run();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::StepForward()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	std::cout << "Number of Iterations: " << m_CurrentNumberOfIterations << "\n";
	SetNumberOfIterations(m_CurrentNumberOfIterations);

	m_LevelSetFilter->Run(m_StepSize);
	m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput()));

	// show
	mitk::RenderingManager::GetInstance()->InitializeViews( m_OutNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	m_CurrentNumberOfIterations += m_StepSize;
	QCoreApplication::processEvents();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::SelectStepSize(int stepSize)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_StepSize = stepSize;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveContourTool::ActiveContour3d()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Input images
	// get image
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("klein");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		std::cout << "OriginalImageThickenings found!\n";
	}
	mitk::Image::Pointer image = (mitk::Image*)imageNode->GetData();
	std::cout << "Reference image name: " << imageNode->GetName() << "\n";

	mitk::Point3D savedMitkOrigin;
	savedMitkOrigin = image->GetGeometry()->GetOrigin();

	unsigned int nb_iteration = 80;
	double rms = 0.;
	double epsilon = 1.;
	double curvature_weight = 0.;
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	//
	//  We now define the image type using a particular pixel type and
	//  dimension. In this case the \code{float} type is used for the pixels
	//  due to the requirements of the smoothing filter.
	//
	const unsigned int Dimension = 3;
	typedef float ScalarPixelType;
	typedef itk::Image< ScalarPixelType, Dimension > InternalImageType;


	typedef itk::ScalarChanAndVeseLevelSetFunctionData< InternalImageType,
	InternalImageType > DataHelperType;

	typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData<
	InternalImageType, InternalImageType, DataHelperType > SharedDataHelperType;

	typedef itk::ScalarChanAndVeseLevelSetFunction< InternalImageType,
	InternalImageType, SharedDataHelperType > LevelSetFunctionType;


	//  We declare now the type of the numerically discretized Step and Delta functions that
	//  will be used in the level-set computations for foreground and background regions
	//
	typedef itk::AtanRegularizedHeavisideStepFunction< ScalarPixelType,
	ScalarPixelType >  DomainFunctionType;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	
	//mitk::Vector3D spacing;
	//spacing.Fill(1.0);
	//slice->GetGeometry()->SetSpacing(spacing);
	//mitk::Point3D origin;
	//origin.Fill(0.0);
	//slice->SetOrigin(origin);

	//mitk::ImageToItk<InternalImageType>::Pointer convert = mitk::ImageToItk<InternalImageType>::New();
	//convert->SetInput(slice);
	////convert->UpdateOutputInformation();
	//convert->Update();
	//InternalImageType::Pointer featureImage = convert->GetOutput();


	InternalImageType::Pointer featureImage;
	mitk::CastToItkImage( image, featureImage );
	//featureImage = reader->GetOutput();
	//featureImage->Print(std::cout);


	//  We declare now the type of the FastMarchingImageFilter that
	//  will be used to generate the initial level set in the form of a distance
	//  map.
	//
	typedef  itk::FastMarchingImageFilter< InternalImageType, InternalImageType >
	FastMarchingFilterType;

	FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();

	//  The FastMarchingImageFilter requires the user to provide a seed
	//  point from which the level set will be generated. The user can actually
	//  pass not only one seed point but a set of them. Note the the
	//  FastMarchingImageFilter is used here only as a helper in the
	//  determination of an initial level set. We could have used the
	//  \doxygen{DanielssonDistanceMapImageFilter} in the same way.
	//
	//  The seeds are passed stored in a container. The type of this
	//  container is defined as \code{NodeContainer} among the
	//  FastMarchingImageFilter traits.
	//
	typedef FastMarchingFilterType::NodeContainer  NodeContainer;
	typedef FastMarchingFilterType::NodeType       NodeType;

	NodeContainer::Pointer seeds = NodeContainer::New();
	InternalImageType::IndexType  seedPosition;
	seedPosition[0] = 50;
	seedPosition[1] = 50;
	seedPosition[2] = 30;

	//const double initialDistance = 1000.0; 
	//const double initialDistance = 500.0; 
	//const double initialDistance = 100.0; 
	const double initialDistance = 10.0; 
	//const double initialDistance = 1.0; 

	NodeType node;
	const double seedValue = - initialDistance;
	node.SetValue( seedValue );
	node.SetIndex( seedPosition );

	////  The list of nodes is initialized and then every node is inserted using
	////  the \code{InsertElement()}.
	////
	seeds->Initialize();
	seeds->InsertElement( 0, node );

	//
	////  The set of seed nodes is passed now to the
	////  FastMarchingImageFilter with the method
	////  \code{SetTrialPoints()}.
	////
	fastMarching->SetTrialPoints(  seeds  );


	////  Since the FastMarchingImageFilter is used here just as a
	////  Distance Map generator. It does not require a speed image as input.
	////  Instead the constant value $1.0$ is passed using the
	////  \code{SetSpeedConstant()} method.
	////
	fastMarching->SetSpeedConstant( 1.0 );

	////  The FastMarchingImageFilter requires the user to specify the
	////  size of the image to be produced as output. This is done using the
	////  \code{SetOutputSize()}. Note that the size is obtained here from the
	////  output image of the smoothing filter. The size of this image is valid
	////  only after the \code{Update()} methods of this filter has been called
	////  directly or indirectly.
	////
	fastMarching->SetOutputSize(featureImage->GetBufferedRegion().GetSize() );
	fastMarching->Update();

	////  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	////  will be used to generate a segmentation.
	typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter< InternalImageType,	InternalImageType, InternalImageType, LevelSetFunctionType,	SharedDataHelperType > MultiLevelSetType;
	MultiLevelSetType::Pointer levelSetFilter = MultiLevelSetType::New();

	////  We set the function count to 1 since a single level-set is being evolved.
	////
	levelSetFilter->SetFunctionCount( 1 );

	////  Set the feature image and initial level-set image as output of the
	////  fast marching image filter.
	////
	levelSetFilter->SetFeatureImage( featureImage );
	levelSetFilter->SetLevelSet( 0, fastMarching->GetOutput() );

	////  Once activiated the level set evolution will stop if the convergence
	////  criteria or if the maximum number of iterations is reached.  The
	////  convergence criteria is defined in terms of the root mean squared (RMS)
	////  change in the level set function. The evolution is said to have
	////  converged if the RMS change is below a user specified threshold.  In a
	////  real application is desirable to couple the evolution of the zero set
	////  to a visualization module allowing the user to follow the evolution of
	////  the zero set. With this feedback, the user may decide when to stop the
	////  algorithm before the zero set leaks through the regions of low gradient
	////  in the contour of the anatomical structure to be segmented.
	////
	levelSetFilter->SetNumberOfIterations( nb_iteration );
	levelSetFilter->SetMaximumRMSError( rms );

	////  Often, in real applications, images have different pixel resolutions. In such
	////  cases, it is best to use the native spacings to compute derivatives etc rather
	////  than sampling the images.
	////
	////levelSetFilter->SetUseImageSpacing( 0.79 );
	levelSetFilter->SetUseImageSpacing( 1 );

	////  For large images, we may want to compute the level-set over the initial supplied
	////  level-set image. This saves a lot of memory.
	////
	levelSetFilter->SetInPlace( false );
	////levelSetFilter->SetInPlace( true);

	////  For the level set with phase 0, set different parameters and weights. This may
	////  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	////
	levelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	levelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( curvature_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	levelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	levelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );

	levelSetFilter->Update();
	//
	//// add output to data storage
	////mitk::Image::Pointer output = mitk::ImportItkImage(featureImage);
	mitk::Image::Pointer output = mitk::ImportItkImage(levelSetFilter->GetOutput());
	mitk::DataNode::Pointer outNode = mitk::DataNode::New();
	outNode->SetData(output);
	outNode->SetName("Output");
	m_Ext->GetDataStorage()->Add(outNode);

	mitk::Image::Pointer output2 = mitk::ImportItkImage(fastMarching->GetOutput());
	mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
	outNode2->SetData(output2);
	outNode2->SetName("Fast Marching Output");
	m_Ext->GetDataStorage()->Add(outNode2);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}