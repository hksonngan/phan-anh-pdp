// "pdp"
#include "mitkActiveSurfaceTool.h"
#include "mitkActiveSurfaceTool.xpm"
#include "QmitkActiveSurfaceToolGUI.h"

// MITK
#include "mitkToolManager.h"
#include <mitkSegTool2D.h>
#include <mitkExtractImageFilter.h>
#include <mitkITKImageImport.h>
#include "mitkImage.h"
#include "mitkBoundingObjectCutter.h"
#include <mitkCuboid.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageWriter.h>
//#include <mitkGeometryClipImageFilter.h>
#include "mitkSurfaceToImageFilter.h"

// VTK
#include "itkVTKPolyDataWriter.h"
#include "vtkImageData.h"

// ITK
#include "itkScalarChanAndVeseSparseLevelSetImageFilter.h"
#include "itkScalarChanAndVeseLevelSetFunction.h"
#include "itkScalarChanAndVeseLevelSetFunctionData.h"
#include "itkConstrainedRegionBasedLevelSetFunctionSharedData.h"

#include <itkImageDuplicator.h>
#include <itkImageRegionIterator.h>
#include <itkMultiplyImageFilter.h>

#include <itkBinaryCrossStructuringElement.h>
#include <itkGrayscaleMorphologicalClosingImageFilter.h>


mitk::ActiveSurfaceTool::ActiveSurfaceTool()
//:Tool("MyPositionTrackerWithPicking")
:Tool("MyPositionTrackerWithPickingAndLeftClick")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );
	CONNECT_ACTION(1003, SetBubble );
	CONNECT_ACTION(1004, RemoveBubbleWithMouse );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_Ut = new pdp::Utilities(m_Ext);
	m_Initialize = true;
	m_StepSize = 1;
	m_DirectedPlay = 0;
	m_CurrentNumberOfIterations = 0;
	m_Curvature_weight = 2500.0;
	m_PropagationForce = 1.0;
	m_Area_weight = 1000.0;
	m_Volume_weight = 0.1;
}

mitk::ActiveSurfaceTool::~ActiveSurfaceTool()
{}

const char** mitk::ActiveSurfaceTool::GetXPM() const
{
	return mitkActiveSurfaceTool_xpm;
}

const char* mitk::ActiveSurfaceTool::GetName() const
{
	return "Active Surface Tool";
}

const char* mitk::ActiveSurfaceTool::GetGroup() const
{
	return "default";
}

void mitk::ActiveSurfaceTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::ActiveSurfaceTool, int>(this, &mitk::ActiveSurfaceTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::ActiveSurfaceTool, int>(this, &mitk::ActiveSurfaceTool::UpdateRadiusSlider);

	GetCurrentImages.Send(GetReferenceImages());
	SetCurvatureForce(m_Curvature_weight);
	SetAreaForce(m_Area_weight);
	SetVolumeForce(m_Volume_weight);

	// Zoom out of thickening selection
	//Zoom(150);
}

void mitk::ActiveSurfaceTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::ActiveSurfaceTool, int>(this, &mitk::ActiveSurfaceTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::ActiveSurfaceTool, int>(this, &mitk::ActiveSurfaceTool::UpdateRadiusSlider);
}

void mitk::ActiveSurfaceTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::ActiveSurfaceTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	
	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

std::vector<std::string> mitk::ActiveSurfaceTool::GetReferenceImages()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	bool first = true;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Ext->GetDataStorage()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
		{
			std::string currentImageName = allNodes->at(i)->GetName();
			m_CurrentImages.push_back(currentImageName);

			if(first)
			{
				m_ReferenceImageNode = allNodes->at(i);
				m_ReferenceImage = (mitk::Image*)allNodes->at(i)->GetData();
				first = false;

				// TODO: set opacity
				//m_ReferenceImageNode->SetOpacity(0.5);
			}
		}
	}
	return m_CurrentImages;	
}

void mitk::ActiveSurfaceTool::SelectImage(int selection)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Get new surface
	if(m_Ext->GetDataStorage()->Exists(m_Ext->GetDataStorage()->GetNamedNode(m_CurrentImages[selection])))
	{
		m_ReferenceImageNode = m_Ext->GetDataStorage()->GetNamedNode(m_CurrentImages[selection]);
		m_ReferenceImage = (mitk::Image*)m_Ext->GetDataStorage()->GetNamedNode(m_CurrentImages[selection])->GetData();

		// TODO: set opacity
		m_ReferenceImageNode->SetOpacity(0.5);
	}
	else
	{
		std::cout << "Error: Selected surface does not exist anymore!\n";
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::CreateReferenceImageVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Create VOI feature input image according to the current bounding box of the thickening surface
	mitk::Cuboid::Pointer CroppingObject = mitk::Cuboid::New();
	CroppingObject->FitGeometry(m_Ext->GetSurface()->GetGeometry());

	// image and bounding object ok
	mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
	cutter->SetBoundingObject( CroppingObject );
	cutter->SetInput( m_ReferenceImage );
	cutter->AutoOutsideValueOff();
	cutter->Update();

	// cutting successful
	m_FeatureImage = cutter->GetOutput();
	m_FeatureImage->DisconnectPipeline();

	m_FeatureImageNode = mitk::DataNode::New();
	m_FeatureImageNode->SetData(m_FeatureImage);
	m_FeatureImageNode->SetName("Feature Image");
	int newId = m_Ext->GetUniqueId();
	m_FeatureImageNode->SetIntProperty("UniqueID", newId);
	m_FeatureImageNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	
	m_Ext->GetDataStorage()->Add(m_FeatureImageNode);	

	m_SavedGeometry = m_FeatureImage->Clone()->GetGeometry();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool mitk::ActiveSurfaceTool::SetBubble(Action* action, const StateEvent* stateEvent)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
	if (displayPositionEvent == NULL)
		return false;
	mitk::BaseRenderer* sender = displayPositionEvent->GetSender();
	std::string renderer4 = "stdmulti.widget4";
	if(renderer4.compare(sender->GetName()) == 0)
	{
		// 3D window -> do nothing
		//std::cout << "3D renderer -> skip\n";
		return ok;
	}

	mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
	mitk::Point3D worldPoint = posEvent->GetWorldPosition();

	std::cout << "Set Bubble at position: " << worldPoint << "\n";

	vtkSphereSource* BubbleSphere = vtkSphereSource::New();
	BubbleSphere->SetThetaResolution(30);
	BubbleSphere->SetPhiResolution(30);
	BubbleSphere->SetRadius(m_Ext->GetRadius());
	BubbleSphere->SetCenter(worldPoint[0], worldPoint[1], worldPoint[2]);
	BubbleSphere->Update();

	mitk::Surface::Pointer BubbleSurface = mitk::Surface::New();
	BubbleSurface->SetVtkPolyData(BubbleSphere->GetOutput());
	mitk::DataNode::Pointer BubbleNode = mitk::DataNode::New();
	BubbleNode->SetName("Seed bubble");
	BubbleNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	// Set ball visible in renderer 1 to 3 and visible=false in renderer 4
	//BubbleNode->SetVisibility(false, m_Ext->GetMitkView()->GetRenderWindow4()->GetRenderer());
	BubbleNode->SetData(BubbleSurface);
	// Add unique identifier for data nodes
	int newId = m_Ext->GetUniqueId();
	BubbleNode->SetIntProperty("UniqueID", newId);
	m_Ext->GetDataStorage()->Add(BubbleNode);
	
	m_BubbleNodes.push_back(BubbleNode);
	m_Bubbles.push_back(BubbleSurface);
	m_BubbleRadii.push_back(m_Ext->GetRadius());
	m_BubbleOrigins.push_back(worldPoint);

	m_Ext->SetImagesToBottom();
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	
	return ok;
}

void mitk::ActiveSurfaceTool::RemoveBubble()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_BubbleNodes.size() == 0)
		return;
	m_Ext->GetDataStorage()->Remove(m_BubbleNodes[m_BubbleNodes.size()-1]);
	m_BubbleNodes.pop_back();
	m_Bubbles.pop_back();
	m_BubbleRadii.pop_back();
	m_BubbleOrigins.pop_back();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool mitk::ActiveSurfaceTool::RemoveBubbleWithMouse(Action* action, const StateEvent* stateEvent)
{
	RemoveBubble();
	return true;
}

void mitk::ActiveSurfaceTool::Reset()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	unsigned int nb_iteration = 1;
	double rms = 0.;
	double epsilon = 1.;
	//double curvature_weight = 0.; 
	double area_weight = 0.;
	double reinitialization_weight = 0.;
	double volume_weight = 0.;
	double volume = 0.;
	double l1 = 1.;
	double l2 = 1.;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	// Search data storage for old images and remove if found
	/*if(m_Ext->GetDataStorage()->Exists(m_FeatureImageNode))
	{
		m_Ext->GetDataStorage()->Remove(m_FeatureImageNode);
	}
	if(m_Ext->GetDataStorage()->Exists(m_DistanceImageNode))
	{
		m_Ext->GetDataStorage()->Remove(m_DistanceImageNode);
	}*/

	InternalImageType::Pointer featureImage = GetInputImage();
	InternalImageType::Pointer distanceImage = CreateDistanceMap(featureImage);	

	// add feature and distance image to datastorage
	//m_Ext->GetDataStorage()->Add(featureImageNode);
	//m_Ext->GetDataStorage()->Add(distanceImageNode);
	AddToDataStorage(featureImage, "featureImage", 1.0);
	AddToDataStorage(distanceImage, "distanceImage", 1.0);

	
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
	m_LevelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( m_Curvature_weight/*curvature_weight*/ );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( area_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( volume_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	m_LevelSetFilter->Update();

	m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput(), m_SavedGeometry, true)); 
	mitk::RenderingManager::GetInstance()->InitializeViews( m_OutNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::SoftReset()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// History available
	std::cout << "History size: " << m_History.size() << "\n";
	if(m_History.size() < 2) // arrived at the beginning, there are no steps back
	{
		std::cout << "History empty\n";	
		m_DirectedPlay = 0;
		SetButtonsToPause(0);
		return;	
	}
	m_History.pop_back();

	// tmp
	//CreateReferenceImageVOI();


	unsigned int nb_iteration = 1;
	double rms = 0.;
	double epsilon = 1.;
	double reinitialization_weight = 0.;
	double volume = 100.0;
	double l1 = 1.;
	double l2 = 1.;

	std::cout << "bis hier 1 " << std::endl;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	std::cout << "bis hier 2 " << std::endl;
	
	m_LevelSetFilter = MultiLevelSetType::New();	

	std::cout << "bis hier 2.5 " << std::endl;

	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.
	//  We set the function count to 1 since a single level-set is being evolved.
	//
	m_LevelSetFilter->SetFunctionCount( 1 );

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	m_LevelSetFilter->SetFeatureImage( m_CurrentFeatureImage );
	std::cout << "bis hier 2.6 " << std::endl;
	m_LevelSetFilter->SetLevelSet( 0, m_History.back() );
	std::cout << "bis hier 2.7 " << std::endl;
	m_History.pop_back();

	std::cout << "bis hier 3 " << std::endl;

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
	m_LevelSetFilter->SetUseImageSpacing( 1 );

	//  For large images, we may want to compute the level-set over the initial supplied
	//  level-set image. This saves a lot of memory.
	m_LevelSetFilter->SetInPlace( false );

	//  For the level set with phase 0, set different parameters and weights. This may
	//  to be set in a loop for the case of multiple level-sets evolving simultaneously.
	m_LevelSetFilter->GetDifferenceFunction(0)->SetDomainFunction( domainFunction );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( m_Curvature_weight/*curvature_weight*/ );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( m_Area_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( m_Volume_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetOverlapPenaltyWeight(4000);

	std::cout << "bis hier 4 " << std::endl;
	m_LevelSetFilter->SetStarted(false);

	m_LevelSetFilter->Update();

	//m_OutNode = AddToDataStorage(m_LevelSetFilter->GetOutput(), "Output", 1.0);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Initialize()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	CreateReferenceImageVOI();

	unsigned int nb_iteration = 1;
	double rms = 0.;
	double epsilon = 1.;
	double reinitialization_weight = 0.;
	double volume = 100.0;
	double l1 = 1.;
	double l2 = 1.;

	DomainFunctionType::Pointer domainFunction = DomainFunctionType::New();
	domainFunction->SetEpsilon( epsilon );

	InternalImageType::Pointer featureImage = GetInputImage();
	InternalImageType::Pointer distanceImage = CreateDistanceMap(featureImage);	

	// calc range of pixel values
	//double upperbound = 0.0;
	//double lowerbound = 0.0;
	//itk::ImageRegionIterator< InternalImageType > itFeatureImageToPositiveValues( featureImage, featureImage->GetRequestedRegion() );
	//for (itFeatureImageToPositiveValues = itFeatureImageToPositiveValues.Begin(); !itFeatureImageToPositiveValues.IsAtEnd(); ++itFeatureImageToPositiveValues)
	//{
	//	//std::cout << "Original Feature Image Pixel: " << itFeatureImageToPositiveValues.Value() << "\n";
	//	if(itFeatureImageToPositiveValues.Value() < lowerbound)
	//	{
	//		lowerbound = itFeatureImageToPositiveValues.Value();
	//	}
	//	if(itFeatureImageToPositiveValues.Value() > upperbound)
	//	{
	//		upperbound = itFeatureImageToPositiveValues.Value();
	//	}
	//}
	//std::cout << "Upperbound: " << upperbound << ", Lowerbound: " << lowerbound << "\n";

	// add feature and distance image to datastorage
	AddToDataStorage(featureImage, "Feature Image", 0.5)->SetVisibility(false);
	AddToDataStorage(distanceImage, "Distance Image", 0.5)->SetVisibility(false);


	m_LevelSetFilter = MultiLevelSetType::New();	
	//  We declare now the type of the ScalarChanAndVeseSparseLevelSetImageFilter that
	//  will be used to generate a segmentation.
	//  We set the function count to 1 since a single level-set is being evolved.
	//
	m_LevelSetFilter->SetFunctionCount( 1 );

	InternalImageType::Pointer regionOfInterestMask = CreateThickeningMask(distanceImage);

	// Get all seedpoint masks and combine masks
	int bubbleCounter = 0;
	for(std::vector<mitk::Surface*>::iterator bubbleIt = m_Bubbles.begin(); bubbleIt != m_Bubbles.end(); ++bubbleIt)
	{
		InternalImageType::Pointer regionOfInterestMask2 = CreateSeedPointMask(distanceImage, bubbleCounter); // todo: for all seedpoints in list

		// Calculate combined mask
		itk::ImageRegionIterator< InternalImageType > it1( regionOfInterestMask, regionOfInterestMask->GetRequestedRegion() );
		itk::ImageRegionIterator< InternalImageType > it2( regionOfInterestMask2, regionOfInterestMask2->GetRequestedRegion() );
  		for (it1 = it1.Begin(), it2 = it2.Begin(); !it1.IsAtEnd(); ++it1, ++it2)
		{
			if(it2.Value() > 0)
			{
				it1.Set(+1);
			}
		}

		bubbleCounter++;
	}


	// Set feature image points outside combined mask to -1024
	itk::ImageRegionIterator< InternalImageType > it3( featureImage, featureImage->GetRequestedRegion() );
	itk::ImageRegionIterator< InternalImageType > it4( regionOfInterestMask, regionOfInterestMask->GetRequestedRegion() );
  	for (it3 = it3.Begin(), it4 = it4.Begin(); !it3.IsAtEnd(); ++it3, ++it4)
	{
		if(it4.Value() < 0)
		{
			it3.Set(-1024);
		}
	}


	// invert
	//itk::ImageRegionIterator< InternalImageType > itRegionOfInterestMask3( multiply->GetOutput(), multiply->GetOutput()->GetRequestedRegion() );
	//for (itRegionOfInterestMask3 = itRegionOfInterestMask3.Begin(); !itRegionOfInterestMask3.IsAtEnd(); ++itRegionOfInterestMask3)
	//{
	//	itRegionOfInterestMask3.Set(itRegionOfInterestMask3.Value() *= -1);
	//}

	// set all negative to -1
	//itk::ImageRegionIterator< InternalImageType > itRegionOfInterestMask4( multiply->GetOutput(), multiply->GetOutput()->GetRequestedRegion() );
	//itk::ImageRegionIterator< InternalImageType > itRegionOfInterestMask4( featureImage, featureImage->GetRequestedRegion() );
	//for (itRegionOfInterestMask4 = itRegionOfInterestMask4.Begin(); !itRegionOfInterestMask4.IsAtEnd(); ++itRegionOfInterestMask4)
	//{
	//	if(itRegionOfInterestMask4.Value() < 0)
	//	{
	//		itRegionOfInterestMask4.Set(-1);
	//	}
	//	else
	//	{
	//		itRegionOfInterestMask4.Set(100);
	//	}
	//}


	//AddToDataStorage(multiply->GetOutput(), "DistanceImageWithMask", 1.0)->SetVisibility(false);
	AddToDataStorage(featureImage, "DistanceImageWithMask", 1.0)->SetVisibility(false);
	AddToDataStorage(regionOfInterestMask, "regionOfInterestMask", 1.0)->SetVisibility(false);

	m_CurrentFeatureImage = featureImage;
	AddToDataStorage(m_CurrentFeatureImage, "m_CurrentFeatureImage", 1.0)->SetVisibility(false);



	// insert featureImage
	typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();
	duplicator->SetInputImage(featureImage);
	duplicator->Update();

	InternalImageType::Pointer invertedFeatureImage = duplicator->GetOutput();
	// invert and make binary
	itk::ImageRegionIterator< InternalImageType > itInvert( invertedFeatureImage, invertedFeatureImage->GetRequestedRegion() );
	for (itInvert = itInvert.Begin(); !itInvert.IsAtEnd(); ++itInvert)
	{
		//itInvert.Set(itInvert.Value() *= -1);

		// set to zero
		if(itInvert.Value() == -1024)
		{
			itInvert.Set(0);
		}
		else
		{
			itInvert.Set(1);
		}
	}
	AddToDataStorage(invertedFeatureImage, "invertedFeatureImage", 1.0)->SetVisibility(false);

	//  Set the feature image and initial level-set image as output of the
	//  fast marching image filter.
	//
	m_LevelSetFilter->SetFeatureImage( featureImage );
	//m_LevelSetFilter->SetFeatureImage( multiply->GetOutput() );
	//m_LevelSetFilter->SetFeatureImage( regionOfInterestMask );


	m_LevelSetFilter->SetLevelSet( 0, distanceImage );	// ist das richtige
	//m_LevelSetFilter->SetLevelSet( 0, invertedFeatureImage );
	//m_LevelSetFilter->SetLevelSet( 0, regionOfInterestMask2 );
	//m_LevelSetFilter->SetLevelSet( 0, multiply->GetOutput() );
	//m_LevelSetFilter->SetLevelSet(0, regionOfInterestMask);


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
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( m_Curvature_weight/*curvature_weight*/ );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight( m_Area_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetReinitializationSmoothingWeight( reinitialization_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight( m_Volume_weight );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolume( volume );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda1( l1 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetLambda2( l2 );
	m_LevelSetFilter->GetDifferenceFunction(0)->SetOverlapPenaltyWeight(4000);
	m_LevelSetFilter->Update();

	m_OutNode = AddToDataStorage(m_LevelSetFilter->GetOutput(), "Output", 1.0);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Run()
{
	m_Ext->SetImagesToBottom();
	while( m_DirectedPlay == 1)
	{
		std::cout << "Number of Iterations: " << m_CurrentNumberOfIterations << "\n";
		SetNumberOfIterations(m_CurrentNumberOfIterations);

		m_LevelSetFilter->Run(m_StepSize);
		m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput(), m_SavedGeometry, true));
		//mitk::RenderingManager::GetInstance()->InitializeViews(m_SavedGeometry, mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		m_CurrentNumberOfIterations += m_StepSize;
		QCoreApplication::processEvents();

		// save old image
		typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;
		DuplicatorType::Pointer duplicator = DuplicatorType::New();
		duplicator->SetInputImage(m_LevelSetFilter->GetLevelSet(0));
		duplicator->Update();
		m_History.push_back(duplicator->GetOutput());
	}
}

itk::Image< float, 3 >::Pointer mitk::ActiveSurfaceTool::GetInputImage()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Killed Origin: " << m_FeatureImage->GetGeometry()->GetOrigin() << "\n";
	mitk::Point3D origin;
	origin.Fill(0.0);
	m_FeatureImage->SetOrigin(origin);

	// Convert to float
	InternalImageType::Pointer featureImage;
	mitk::CastToItkImage( m_FeatureImage, featureImage );

	//std::cout << "Return from get input image\n" << std::endl;
	return featureImage;
}

itk::Image< float, 3 >::Pointer mitk::ActiveSurfaceTool::CreateDistanceMap(itk::Image< float, 3 >::Pointer input)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//  We declare now the type of the FastMarchingImageFilter that
	//  will be used to generate the initial level set in the form of a distance
	//  map.
	//typedef  itk::FastMarchingImageFilter< InternalImageType, InternalImageType > FastMarchingFilterType;
	m_FastMarching = FastMarchingFilterType::New();

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
	seeds->Initialize();

	// Bubble aus seed points
	int i = 0;
	std::vector<mitk::DataNode*>::iterator nodesIt = m_BubbleNodes.begin();
	std::vector<double>::iterator radiiIt =  m_BubbleRadii.begin();
	std::vector<mitk::Point3D>::iterator originsIt =  m_BubbleOrigins.begin();
	for(std::vector<mitk::Surface*>::iterator it = m_Bubbles.begin(); it != m_Bubbles.end(); it++, nodesIt++, radiiIt++, originsIt++)
	{
		InternalImageType::IndexType  seedPosition;
		seedPosition[0] = (*originsIt)[0];
		seedPosition[1] = (*originsIt)[1];
		seedPosition[2] = (*originsIt)[2];

		// Coordinate systems ->convert
		seedPosition[0] -= m_Ext->GetSurface()->GetGeometry()->GetBounds()[0] - 1;
		seedPosition[1] -= m_Ext->GetSurface()->GetGeometry()->GetBounds()[2] - 1;
		seedPosition[2] -= m_Ext->GetSurface()->GetGeometry()->GetBounds()[4]; 

		// spacings !!!
		seedPosition[0] /= m_ReferenceImage->GetGeometry()->GetSpacing()[0];
		seedPosition[1] /= m_ReferenceImage->GetGeometry()->GetSpacing()[1];
		seedPosition[2] /= m_ReferenceImage->GetGeometry()->GetSpacing()[2];

		/*mitk::Point3D point = m_Ext->GetSurface()->GetGeometry()->GetCornerPoint();
		std::cout << "Corner point surface: " << point << "\n";
		std::cout << "Surface bounds: " << m_Ext->GetSurface()->GetGeometry()->GetBounds() << "\n";
		std::cout << "Seed origin: " << *originsIt << "\n";
		std::cout << "Seed position: " << seedPosition[0] << " " << seedPosition[1] << " " << seedPosition[2] << "\n";*/

		double initialDistance = *radiiIt;
		double seedValue = - initialDistance;
		NodeType node;
		node.SetValue( seedValue );
		node.SetIndex( seedPosition );
		
		seeds->InsertElement( i, node );
		i++;
	}
	
	//  The set of seed nodes is passed now to the
	//  FastMarchingImageFilter with the method
	//  \code{SetTrialPoints()}.
	//
	m_FastMarching->SetTrialPoints(  seeds  );


	//  Since the FastMarchingImageFilter is used here just as a
	//  Distance Map generator. It does not require a speed image as input.
	//  Instead the constant value $1.0$ is passed using the
	//  \code{SetSpeedConstant()} method.
	//
	m_FastMarching->SetSpeedConstant( m_PropagationForce );

	//  The FastMarchingImageFilter requires the user to specify the
	//  size of the image to be produced as output. This is done using the
	//  \code{SetOutputSize()}. Note that the size is obtained here from the
	//  output image of the smoothing filter. The size of this image is valid
	//  only after the \code{Update()} methods of this filter has been called
	//  directly or indirectly.
	//
	m_FastMarching->SetOutputSize( input->GetBufferedRegion().GetSize() );
	m_FastMarching->Update();

	return m_FastMarching->GetOutput();
}

mitk::DataNode::Pointer mitk::ActiveSurfaceTool::AddToDataStorage(itk::Image< float, 3 >::Pointer data, char* name, double opacity)
{
	mitk::DataNode::Pointer inputNode = mitk::DataNode::New();
	inputNode->SetData(mitk::ImportItkImage(data, m_SavedGeometry, true));
	
	// todo pixel type
	/*mitk::Image::Pointer mitkImage = mitk::Image::New();
	mitk::PixelType pixelType(typeid(short));
	mitkImage->Initialize(pixelType, *m_SavedGeometry);
	mitk::CastToMitkImage(data, mitkImage);
	inputNode->SetData(mitkImage);*/

	inputNode->SetName(name);
	inputNode->SetOpacity(opacity);
	int newId = m_Ext->GetUniqueId();
	inputNode->SetIntProperty("UniqueID", newId);
	inputNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(inputNode);

	return inputNode;
}

void mitk::ActiveSurfaceTool::Stop()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CurrentNumberOfIterations = 0;
	SetNumberOfIterations(m_CurrentNumberOfIterations);
	m_DirectedPlay = 0;
	//Reset();

	mitk::DataNode::Pointer node = m_Ext->GetDataStorage()->GetNamedNode("Output");
	if(m_Ext->GetDataStorage()->Exists(node))
	{
		m_Ext->GetDataStorage()->Remove(node);
	}

	Initialize();
	m_Initialize = false;
	m_Ext->SetImagesToBottom();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::StepBack()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	SoftReset();
	StepForward();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::ReversePlay()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_DirectedPlay = 2;
	while(m_DirectedPlay == 2)
	{
		SoftReset();
		m_CurrentNumberOfIterations -= m_StepSize;
		QCoreApplication::processEvents();
		
		// save old image
		typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;
		DuplicatorType::Pointer duplicator = DuplicatorType::New();
		duplicator->SetInputImage(m_LevelSetFilter->GetLevelSet(0));
		duplicator->Update();
		m_History.push_back(duplicator->GetOutput());
		//AddToDataStorage(m_History.back(), "LevelSetHistory", 1.0)->SetVisibility(false); // Tut es. Speichert die levelsets in unterschiedliche bilder

		std::cout << "Number of Iterations: " << m_CurrentNumberOfIterations << "\n";
		SetNumberOfIterations(m_CurrentNumberOfIterations);

		m_LevelSetFilter->Run(m_StepSize);
		m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput(), m_SavedGeometry, true));

		// show
		mitk::RenderingManager::GetInstance()->InitializeViews( m_OutNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		m_CurrentNumberOfIterations += m_StepSize;
		QCoreApplication::processEvents();
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Pause()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DirectedPlay = 0;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Play()
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

void mitk::ActiveSurfaceTool::StepForward()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_DirectedPlay = 0;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	// save old image
	typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();
	duplicator->SetInputImage(m_LevelSetFilter->GetLevelSet(0));
	duplicator->Update();
	m_History.push_back(duplicator->GetOutput());

	std::cout << "Number of Iterations: " << m_CurrentNumberOfIterations << "\n";
	SetNumberOfIterations(m_CurrentNumberOfIterations);

	m_LevelSetFilter->Run(m_StepSize);
	m_OutNode->SetData(mitk::ImportItkImage(m_LevelSetFilter->GetOutput(), m_SavedGeometry, true));

	// show
	mitk::RenderingManager::GetInstance()->InitializeViews( m_OutNode->GetData()->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	m_CurrentNumberOfIterations += m_StepSize;
	QCoreApplication::processEvents();

	//InternalImageType::Pointer oldLevelSet = m_LevelSetFilter->GetLevelSet(0);
	//AddToDataStorage(oldLevelSet, "oldLevelSet", 1.0);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::SelectStepSize(int stepSize)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_StepSize = stepSize;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

itk::Image< float, 3 >::Pointer mitk::ActiveSurfaceTool::CreateThickeningMask(itk::Image< float, 3 >::Pointer input)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode::Pointer node = mitk::DataNode::New();
	mitk::PixelType pixelType(typeid(short));
	mitk::Image::Pointer out = mitk::ImportItkImage(input, m_SavedGeometry, true);
	
	mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
	filter->SetInput(m_Ext->GetSurface());						
	filter->SetImage(out);
	filter->SetMakeOutputBinary(true);
	filter->Update();

	mitk::Image::Pointer mitkBinaryImage = (mitk::Image*)filter->GetOutput(0);
	node->SetData(mitkBinaryImage);
	node->SetName("ThickeningImage");
	node->SetVisibility(false);
	int newId = m_Ext->GetUniqueId();
	node->SetIntProperty("UniqueID", newId);
	node->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(node);

	// convert binary image to itk
	mitk::Point3D origin2;
	origin2.Fill(0.0);
	mitkBinaryImage->SetOrigin(origin2);
	InternalImageType::Pointer itkBinaryImage ;
	mitk::CastToItkImage( mitkBinaryImage, itkBinaryImage );
	AddToDataStorage(itkBinaryImage, "ItkBinaryImage", 1.0)->SetVisibility(false);

	// Create new itk image and copy values from binary image ----- Convert binary to gray values

	InternalImageType::Pointer itkGrayValueImage = InternalImageType::New();
    //itkGrayValueImage->SetRegions( itkBinaryImage->GetRequestedRegion() );
    //itkGrayValueImage->CopyInformation( itkBinaryImage );
	itkGrayValueImage->SetRegions( input->GetRequestedRegion() );
    itkGrayValueImage->CopyInformation( input );

    itkGrayValueImage->Allocate();

	itk::ImageRegionIterator< InternalImageType > itBinary( itkBinaryImage, itkBinaryImage->GetRequestedRegion() );
	itk::ImageRegionIterator< InternalImageType > itGrayValues( itkGrayValueImage, itkGrayValueImage->GetRequestedRegion() );

  	for (itBinary = itBinary.Begin(), itGrayValues = itGrayValues.Begin(); !itBinary.IsAtEnd(); ++itBinary, ++itGrayValues)
	{
		if(itBinary.Value() == 1)
		{
			itGrayValues.Set(1);
		}
		else
		{
			itGrayValues.Set(-1);
		}
	}

	AddToDataStorage(itkGrayValueImage, "ItkGrayValueImage", 1.0)->SetVisibility(false);

	return itkGrayValueImage;
}

itk::Image< float, 3 >::Pointer mitk::ActiveSurfaceTool::CreateSeedPointMask(itk::Image< float, 3 >::Pointer input, int seedPointNumber)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode::Pointer node = mitk::DataNode::New();
	mitk::PixelType pixelType(typeid(short));
	mitk::Image::Pointer out = mitk::ImportItkImage(input, m_SavedGeometry, true);
	
	// Create new surface around seedpoint
	double addToOriginalRadius = 0;
	vtkSphereSource* BubbleSphere = vtkSphereSource::New();
	BubbleSphere->SetThetaResolution(30);
	BubbleSphere->SetPhiResolution(30);
	BubbleSphere->SetRadius(m_BubbleRadii[seedPointNumber] += addToOriginalRadius);
	BubbleSphere->SetCenter(m_BubbleOrigins[seedPointNumber][0],m_BubbleOrigins[seedPointNumber][1],m_BubbleOrigins[seedPointNumber][2]);
	BubbleSphere->Update();
	mitk::Surface::Pointer BubbleSurface = mitk::Surface::New();
	BubbleSurface->SetVtkPolyData(BubbleSphere->GetOutput());
	
	//std::cout << "bis hier 1 " << std::endl;

	mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
	filter->SetInput(BubbleSurface);						
	filter->SetImage(out);
	filter->SetMakeOutputBinary(true);
	filter->Update();

	//std::cout << "bis hier 2 " << std::endl;

	mitk::Image::Pointer mitkBinaryImage = (mitk::Image*)filter->GetOutput(0);
	node->SetData(mitkBinaryImage);
	node->SetName("BubbleImage");
	node->SetVisibility(false);
	int newId = m_Ext->GetUniqueId();
	node->SetIntProperty("UniqueID", newId);
	node->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(node);

	//std::cout << "bis hier 3 " << std::endl;

	// convert binary image to itk
	mitk::Point3D origin2;
	origin2.Fill(0.0);
	mitkBinaryImage->SetOrigin(origin2);
	InternalImageType::Pointer itkBinaryImage ;
	mitk::CastToItkImage( mitkBinaryImage, itkBinaryImage );
	AddToDataStorage(itkBinaryImage, "ItkBubbleImage", 1.0)->SetVisibility(false);

	// Create new itk image and copy values from binary image ----- Convert binary to gray values

	InternalImageType::Pointer itkGrayValueImage = InternalImageType::New();
    //itkGrayValueImage->SetRegions( itkBinaryImage->GetRequestedRegion() );
    //itkGrayValueImage->CopyInformation( itkBinaryImage );
	itkGrayValueImage->SetRegions( input->GetRequestedRegion() );
    itkGrayValueImage->CopyInformation( input );

    itkGrayValueImage->Allocate();

	itk::ImageRegionIterator< InternalImageType > itBinary( itkBinaryImage, itkBinaryImage->GetRequestedRegion() );
	itk::ImageRegionIterator< InternalImageType > itGrayValues( itkGrayValueImage, itkGrayValueImage->GetRequestedRegion() );

  	for (itBinary = itBinary.Begin(), itGrayValues = itGrayValues.Begin(); !itBinary.IsAtEnd(); ++itBinary, ++itGrayValues)
	{
		if(itBinary.Value() == 1)
		{
			itGrayValues.Set(1);
		}
		else
		{
			itGrayValues.Set(-1);
		}
	}

	AddToDataStorage(itkGrayValueImage, "ItkBubbleGrayValueImage", 1.0)->SetVisibility(false);

	//std::cout << "bis hier 4 " << std::endl;

	return itkGrayValueImage;
}

void mitk::ActiveSurfaceTool::CurvatureForceCross(int force, int y)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Curvature_weight += (double)force;
	if(m_Curvature_weight < 0)
	{
		m_Curvature_weight = 0;
	}

	SetCurvatureForce(m_Curvature_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( m_Curvature_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::CurvatureForceLineEdit(double force)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Curvature_weight = force;
	if(m_Curvature_weight < 0)
	{
		m_Curvature_weight = 0;
	}

	SetCurvatureForce(m_Curvature_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetCurvatureWeight( m_Curvature_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::AreaForceCross(int force, int y)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Area_weight += (double)force / 5;
	if(m_Area_weight < 0)
	{
		m_Area_weight = 0;
	}

	SetAreaForce(m_Area_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight(m_Area_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::AreaForceLineEdit(double force)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Area_weight = force;
	if(m_Area_weight < 0)
	{
		m_Area_weight = 0;
	}

	SetAreaForce(m_Area_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetAreaWeight(m_Area_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::VolumeForceCross(int force, int y)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Volume_weight += ((double)((int)(((double)force / 15.0) * 100))) / 100.0;

	//m_Volume_weight += force / 100.0;

	//m_Volume_weight += ((int)(force*50)) / 100.0;

	//m_Volume_weight += force / 5;

	if(m_Volume_weight < 0)
	{
		m_Volume_weight = 0;
	}

	SetVolumeForce(m_Volume_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight(m_Volume_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::VolumeForceLineEdit(double force)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Initialize)
	{
		Initialize();
		m_Initialize = false;
	}

	m_Volume_weight = force;
	if(m_Volume_weight < 0)
	{
		m_Volume_weight = 0;
	}

	SetVolumeForce(m_Volume_weight);
	QCoreApplication::processEvents();
	
	m_LevelSetFilter->GetDifferenceFunction(0)->SetVolumeMatchingWeight(m_Volume_weight);
	m_LevelSetFilter->Update();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Closing()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::GrayscaleMorphologicalClosingImageFilter< SegmentationType, SegmentationType, CrossType > CrossClosingFilterType;

	SegmentationType::Pointer itkImage = SegmentationType::New();
	mitk::CastToItkImage((mitk::Image*)(m_OutNode->GetData()), itkImage);
	mitk::Image::Pointer new_image = mitk::Image::New();

	CrossType m_Cross;
    CrossClosingFilterType::Pointer m_CrossClosingFilter;

	m_Cross.SetRadius(1.0);
	m_Cross.CreateStructuringElement();

	m_CrossClosingFilter = CrossClosingFilterType::New();
	m_CrossClosingFilter->SetKernel(m_Cross);
	m_CrossClosingFilter->SetInput(itkImage);
	//m_CrossClosingFilter->SetForegroundValue(1);
	//m_CrossClosingFilter->SetBackgroundValue(0);
	m_CrossClosingFilter->SetSafeBorder(true);
	m_CrossClosingFilter->UpdateLargestPossibleRegion();

	mitk::CastToMitkImage(m_CrossClosingFilter->GetOutput(), new_image);

	mitk::DataNode::Pointer newImageNode = mitk::DataNode::New();
	newImageNode->SetData(new_image);
	newImageNode->SetName("ClosedImage");
	m_Ext->GetDataStorage()->Add(newImageNode);


	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ActiveSurfaceTool::Zoom(int zoom)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	double surrounding = (double)zoom / 10.0;

	mitk::Geometry3D::Pointer geo = m_Ext->GetSurface()->GetGeometry();	
	mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geo->GetBoundingBox());
	mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

	surfBounds[0] -= surrounding;
	surfBounds[1] += surrounding;
	surfBounds[2] -= surrounding;
	surfBounds[3] += surrounding;
	surfBounds[4] -= surrounding;
	surfBounds[5] += surrounding;
	geo->SetBounds(surfBounds);

	mitk::RenderingManager::GetInstance()->InitializeViews(
	geo/*basedata->GetTimeSlicedGeometry()*/, mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	m_Initialize = true;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}