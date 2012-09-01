// "pdp"
#include "mitkMoveROITool.h"
#include "mitkMoveROITool.xpm"
#include <../../../../RenderingManagerProperty.h>

// MITK
#include "mitkToolManager.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkDataStorage.h"
#include "mitkGlobalInteraction.h"
#include <mitkNodePredicateProperty.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkAction.h>
#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkInteractionConst.h>
#include <mitkRenderingManager.h>
#include <mitkVector.h> // for PointDataType 
#include <mitkProperties.h>
#include <mitkBoundingObject.h>
#include <mitkSurfaceToImageFilter.h>
#include "mitkVtkScalarModeProperty.h"
#include <mitkSurfaceGLMapper2D.h>
// VTK
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAppendPolyData.h>
#include <vtkClipPolyData.h>
#include "vtkPlane.h" 
#include <vtkSphere.h>
#include "vtkPointData.h"
#include <vtkPolyDataNormals.h>
#include <vtkDoubleArray.h>
#include <vtkFillHolesFilter.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkTriangleFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkLine.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkCellLocator.h>
#include <vtkTriangle.h>
#include <vtkSmoothPolyDataFilter.h>

#include <vtkButterflySubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkLinearSubdivisionFilter.h>

#include <vtkExtractEdges.h>
#include <vtkStripper.h>

#include <cmath> 
#include <vector>

// test
//#include <mitkReduceContourSetFilter.h>


mitk::MoveROITool::MoveROITool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	//PoinSetNode for MouseOrientation
	m_PositionTrackerNode = mitk::DataNode::New();
	m_PositionTrackerNode->SetProperty("name", mitk::StringProperty::New("Mouse Position"));
	m_PositionTrackerNode->SetData( mitk::PointSet::New() );
	m_PositionTrackerNode->SetColor(1.0,0.33,0.0);
	m_PositionTrackerNode->SetProperty("layer", mitk::IntProperty::New(1001));
	m_PositionTrackerNode->SetVisibility(true);
	m_PositionTrackerNode->SetProperty("inputdevice", mitk::BoolProperty::New(true) );
	m_PositionTrackerNode->SetProperty("BaseRendererMapperID", mitk::IntProperty::New(0) );//point position 2D mouse
	m_PositionTrackerNode->SetProperty("baserenderer", mitk::StringProperty::New("N/A"));

	m_ContourResolution = 10;
	m_Counter = 0;
	m_Radius = 20;
	m_bulging = false;
	m_SurfaceHadBeenHit = false;
	m_SmoothIteration = 50;
	m_RadiusFactor = 3;
	m_JumpToPosition = false;

	m_OldVoidCenterVectorIndex = 0;
	m_MouseMovementVectorSumLength = 1;
	m_OldVOICenterVector = new double [3];
	m_OldVOICenterVectors = new double [m_MouseMovementVectorSumLength*3];
	for(int i = 0; i < m_MouseMovementVectorSumLength; i++)
	{
		m_OldVOICenterVectors[i*3] = 0.0;
		m_OldVOICenterVectors[i*3+1] = 0.0;
		m_OldVOICenterVectors[i*3+2] = 0.0;
	}
}

mitk::MoveROITool::~MoveROITool()
{}

const char** mitk::MoveROITool::GetXPM() const
{
	return mitkMoveROITool_xpm;
}

const char* mitk::MoveROITool::GetName() const
{
	return "Move ROI";
}

const char* mitk::MoveROITool::GetGroup() const
{
	return "default";
}

void mitk::MoveROITool::Activated()
{
	Superclass::Activated();
	std::cout << "Activated.\n";

	m_DataStorage = m_ToolManager->GetDataStorage();

	// Mouse position
	m_DataStorage->Add(m_PositionTrackerNode);

	// Get Surfaces and send to GUI
	m_CurrentSurfaces.clear();
	// Get recent surface, if available
	bool currentSurfaceAvailable = false;
	mitk::DataStorage::SetOfObjects::ConstPointer allSurfaceNodes = m_DataStorage->GetAll();
	for(unsigned int i = 0; i < allSurfaceNodes->size(); i++)
	{
		if(strcmp(allSurfaceNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			allSurfaceNodes->at(i)->GetBoolProperty("CurrentSurface", currentSurfaceAvailable);
			if(currentSurfaceAvailable)
			{
				m_Surface = (mitk::Surface*)allSurfaceNodes->at(i)->GetData();
				m_SurfaceNode = allSurfaceNodes->at(i);

				std::string currentSurfaceName = allSurfaceNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allSurfaceNodes->at(i)->SetBoolProperty("CurrentSurface", true);

				std::cout << "Current Surface found!\n";
				break;
			}
		}
	}
	// If there is no current surface, try to find another
	bool anySurfaceFound = false;
	if(!currentSurfaceAvailable)
	{
		for(unsigned int i = 0; i < allSurfaceNodes->size(); i++)
		{
			//allSurfaceNodes->at(i)->Print(std::cout);
			
			if(strcmp(allSurfaceNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
			{
				m_Surface = (mitk::Surface*)allSurfaceNodes->at(i)->GetData();
				m_SurfaceNode = allSurfaceNodes->at(i);

				std::string currentSurfaceName = allSurfaceNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allSurfaceNodes->at(i)->SetBoolProperty("CurrentSurface", true);

				std::cout << "Surface found!\n";
				anySurfaceFound = true;
				break;
			}
		}
	
		if(!anySurfaceFound)
		{
			std::cout << "There is no surface available. Please load a surface first.\n"; // Todo, e.g. send gui popup and return
			return;
		}
	}

	// add all other surfaces to current surface vector
	for(unsigned int i = 0; i < allSurfaceNodes->size(); i++)
	{	
		if(strcmp(allSurfaceNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			if(!std::binary_search(m_CurrentSurfaces.begin(), m_CurrentSurfaces.end(), allSurfaceNodes->at(i)->GetName()))
			{
				if((allSurfaceNodes->at(i)->GetName()).compare("IntersectionOutput 2D") == 0)
					continue;
				if((allSurfaceNodes->at(i)->GetName()).compare("IntersectionOutput 3D") == 0)
					continue;
				std::string currentSurfaceName = allSurfaceNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allSurfaceNodes->at(i)->SetBoolProperty("CurrentSurface", false);
				allSurfaceNodes->at(i)->SetProperty("color",mitk::ColorProperty::New(255.0/255.0, 255.0/255.0, 255.0/255.0));
			}
		}
	}
	GetCurrentSurfaces.Send(m_CurrentSurfaces);

	// Set Old Sphere Radius
	int tmpRadius;
	if(m_SurfaceNode->GetIntProperty("SphereRadius", tmpRadius))
		m_Radius = tmpRadius;
	SetSliderToLastRadius.Send((int)(m_Radius*m_RadiusFactor-1));

	// Some surface properties
	unsigned int id = 1;
	mitk::Mapper::Pointer newMapper = NULL;
	newMapper = mitk::SurfaceGLMapper2D::New();
	m_SurfaceNode->SetMapper(id, newMapper);

	m_SurfaceNode->SetProperty("color",mitk::ColorProperty::New(1.0/255.0, 254.0/255.0, 1.0/255.0));
	m_SurfaceNode->SetProperty("back color",mitk::ColorProperty::New(1.0/255.0, 1.0/255.0, 254.0/255.0));
	//m_SurfaceNode->SetProperty("draw normals 2d", true); 
	m_SurfaceNode->SetBoolProperty("draw normals 2d", true);
	//m_SurfaceNode->SetBoolProperty("scalar visibility", true);
	m_SurfaceNode->SetBoolProperty("show points", true);

	m_SurfaceNode->SetProperty("scalar visibility", mitk::BoolProperty::New(true));
    m_SurfaceNode->SetProperty("color mode", mitk::BoolProperty::New(true));

	m_SurfaceNode->AddProperty( "line width", IntProperty::New(2));
	m_SurfaceNode->AddProperty( "scalar mode", VtkScalarModeProperty::New());
	m_SurfaceNode->AddProperty( "draw normals 2D", BoolProperty::New(true));
	m_SurfaceNode->AddProperty( "invert normals", BoolProperty::New(false));
	m_SurfaceNode->AddProperty( "front color", ColorProperty::New(0.0, 1.0, 0.0));
	m_SurfaceNode->AddProperty( "back color", ColorProperty::New(1.0, 0.0, 0.0));
	m_SurfaceNode->AddProperty( "front normal lenth (px)", FloatProperty::New(10.0));
	m_SurfaceNode->AddProperty( "back normal lenth (px)", FloatProperty::New(10.0));
	m_SurfaceNode->AddProperty( "layer", mitk::IntProperty::New(100));



	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	m_MitkView = properties->GetStdMultiWidget();

	m_Ball = mitk::Surface::New();
	m_BallNode = mitk::DataNode::New();
	m_BallNode->SetName("IntersectionOutput 2D");
	m_BallNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	// Set ball visible in renderer 1 to 3 and visible=false in renderer 4
	
	// add ball to renderer
	m_BallNode->SetVisibility(false, m_MitkView->GetRenderWindow4()->GetRenderer());

	m_BallNode->SetData(m_Ball);
	m_DataStorage->Add(m_BallNode, m_DataStorage->GetNamedNode("peternode"));

	m_IntersectionData = mitk::Surface::New();
	m_IntersectionNode = mitk::DataNode::New();

	m_IntersectionNode->SetData(m_IntersectionData);
	m_IntersectionNode->SetName("IntersectionOutput 3D");
	m_IntersectionNode->SetProperty( "outline width", mitk::FloatProperty::New( 5.0 ) );
	m_IntersectionNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	m_DataStorage->Add(m_IntersectionNode, m_DataStorage->GetNamedNode("peternode"));

	m_IntersectionPolyDataFilter =	vtkSmartPointer<vtkIntersectionPolyDataFilter>::New();
	m_IntersectionPolyDataFilter->SetInput(0,  m_Surface->GetVtkPolyData());

	m_ContourSphere = vtkSphereSource::New();
	m_ContourSphere->SetThetaResolution(m_ContourResolution);
	m_ContourSphere->SetPhiResolution(m_ContourResolution);

	m_Ball->SetVtkPolyData(m_ContourSphere->GetOutput());

	m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
	m_IntersectionPolyDataFilter->Update();
	m_IntersectionData->SetVtkPolyData(m_IntersectionPolyDataFilter->GetOutput());
}

void mitk::MoveROITool::Deactivated()
{
	std::cout << "Deactivated.\n";

	m_DataStorage->Remove(m_PositionTrackerNode);
	m_DataStorage->Remove(m_BallNode);
	m_DataStorage->Remove(m_IntersectionNode);

	// Save Radius to m_Surface
	m_SurfaceNode->SetIntProperty("SphereRadius", m_Radius);

	Superclass::Deactivated();
}

bool mitk::MoveROITool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	bool ok = false;
	const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

	mitk::DataNode::Pointer  dtnode;
	if (displayPositionEvent == NULL)
		return false;

	if (stateEvent->GetEvent()->GetSender()!=NULL)
	{
		m_DataStorage = stateEvent->GetEvent()->GetSender()->GetDataStorage();
	}
	else
	{
		itkWarningMacro(<<"StateEvent::GetSender()==NULL - setting timeInMS to 0");
		return false;
	}
	if (m_DataStorage == NULL) 
		return false;

	// Looking for desired point set
	dtnode = m_DataStorage->GetNode(mitk::NodePredicateProperty::New("inputdevice", mitk::BoolProperty::New(true)));
	if (dtnode.IsNull())
		return false;

	dtnode->SetIntProperty("BaseRendererMapperID", stateEvent->GetEvent()->GetSender()->GetMapperID());
	mitk::PointSet* ps = dynamic_cast<mitk::PointSet*>(dtnode->GetData());
	if (ps == NULL)
		return false;

	int position = 0;
	if( ps->GetPointSet()->GetPoints()->IndexExists( position )) //first element
	{
		ps->GetPointSet()->GetPoints()->SetElement( position, displayPositionEvent->GetWorldPosition());                            
	}
	else
	{
		mitk::PointSet::PointDataType pointData = {position , false /*selected*/, mitk::PTUNDEFINED};
		ps->GetPointSet()->GetPointData()->InsertElement(position, pointData);
		ps->GetPointSet()->GetPoints()->InsertElement(position, displayPositionEvent->GetWorldPosition());
	}
	ps->Modified();

	// Position
	mitk::Point3D worldPoint = displayPositionEvent->GetWorldPosition();
    m_ContourSphere->SetRadius(m_Radius);
	m_ContourSphere->SetCenter(worldPoint[0], worldPoint[1], worldPoint[2]);
    m_ContourSphere->Update();
    //m_Ball->SetVtkPolyData(ContourSphere->GetOutput());	

	mitk::Point3D point3 = displayPositionEvent->GetWorldPosition();
	//m_MitkView->MoveCrossToPosition(point3);  

	// Calculate intersection
	m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
	m_IntersectionPolyDataFilter->Update();
	//vtkPolyData* polydata = m_IntersectionPolyDataFilter->GetOutput();
	//m_IntersectionData->SetVtkPolyData(polydata);

	//float bounds[6] = {0,20,0,20,0,20};
	//m_Surface->GetGeometry()->SetFloatBounds(bounds);
	//m_Surface->GetGeometry()->SetBounds(m_Ball->GetGeometry()->GetBounds());
	
	m_Surface->GetGeometry()->GetBoundingBox();

	//m_Surface->Print(std::cout);
	//m_Ball->Print(std::cout);
	
	//std::cout << "Counter: " << m_Counter << "\n";
	m_Counter++;
	//mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	// Test: Reduce number of cells in the surface
	//mitk::ReduceContourSetFilter::Pointer reductionFilter = mitk::ReduceContourSetFilter::new(); 

	//m_DataStorage->Remove(m_BallNode);
	//m_DataStorage->Add(m_BallNode);

	if(m_bulging)
	{
		//std::cout << "Bulging\n";
		if(0)
		{
			// Use direction of the radius
			vtkIdType inputNumPoints = m_Surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();

			double* center = m_Ball->GetVtkPolyData()->GetCenter();
			double translateX;
			double translateY;
			double translateZ;
			double translateFactor;

			std::cout << "Center: " << center[0] << " " << center[1] << " " << center[2] << "\n"; 

			for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
			{
				double pt[3];
				m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);
						
				// Use distance			
				double distance = sqrt((center[0]-pt[0])*(center[0]-pt[0])+(center[1]-pt[1])*(center[1]-pt[1])+(center[2]-pt[2])*(center[2]-pt[2]));
				if(distance < m_Radius)
				{
					// move cells				
					translateX = pt[0] - center[0];
					translateY = pt[1] - center[1];
					translateZ = pt[2] - center[2];
					translateFactor = m_Radius / sqrt(translateX*translateX+translateY*translateY+translateZ*translateZ);

					if(1)
					{
						std::cout << "Translate: " << translateX << " " << translateY << " " << translateZ << "\n";
						std::cout << "Point: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n"; 
						std::cout << "Translation Factor: " << translateFactor << "\n";
					}

					pt[0] = (translateFactor * translateX + center[0]);
					pt[1] = (translateFactor * translateY + center[1]);
					pt[2] = (translateFactor * translateZ + center[2]);

					std::cout << "New Point: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n"; 

					m_Surface->GetVtkPolyData()->GetPoints()->SetPoint(ptId, pt);
				}
			}
		}
		if(1)
		{
			bool pointHasBeenMoved = false;
			// Use direction of the mouse movement
			vtkIdType inputNumPoints = m_Surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
			double* center = m_Ball->GetVtkPolyData()->GetCenter();

			// Mouse Movement
			m_MouseMovementVector[0] = center[0] - m_OldVOICenterVector[0];
			m_MouseMovementVector[1] = center[1] - m_OldVOICenterVector[1];
			m_MouseMovementVector[2] = center[2] - m_OldVOICenterVector[2];

			// Mouse Movement with multiple vectors
			double oldVOICenterVectorSum[3];
			oldVOICenterVectorSum[0] = 0.0;
			oldVOICenterVectorSum[1] = 0.0;
			oldVOICenterVectorSum[2] = 0.0;
			for(int i = 0; i < m_MouseMovementVectorSumLength; i++)
			{
				oldVOICenterVectorSum[0] += m_OldVOICenterVectors[0+i*3];
				oldVOICenterVectorSum[1] += m_OldVOICenterVectors[1+i*3];
				oldVOICenterVectorSum[2] += m_OldVOICenterVectors[2+i*3];
			}		
			//std::cout << "oldVOICenterVectorSum: " << oldVOICenterVectorSum[0] << " " << oldVOICenterVectorSum[1] << " " << oldVOICenterVectorSum[2] << "\n";
			m_MouseMovementVectorSum[0] = center[0] - oldVOICenterVectorSum[0]/m_MouseMovementVectorSumLength;
			m_MouseMovementVectorSum[1] = center[1] - oldVOICenterVectorSum[1]/m_MouseMovementVectorSumLength;
			m_MouseMovementVectorSum[2] = center[2] - oldVOICenterVectorSum[2]/m_MouseMovementVectorSumLength;

			if(0)
			{
				//std::cout << "Old Center: " << m_oldVOICenterVector[0] << " " << m_oldVOICenterVector[1] << " " << m_oldVOICenterVector[2] << "\n"; 
				//std::cout << "Current Center: " << center[0] << " " << center[1] << " " << center[2] << "\n"; 
				std::cout << "Mouse Movement Vector: " << m_MouseMovementVector[0] << " " << m_MouseMovementVector[1] << " " << m_MouseMovementVector[2] << "\n"; 
				std::cout << "Mouse Movement VectorSum: " << m_MouseMovementVectorSum[0] << " " << m_MouseMovementVectorSum[1] << " " << m_MouseMovementVectorSum[2] << "\n"; 
			}

			for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
			{
				double pt[3];
				m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);
						
				// Use distance			
				double centerToPointVector[3];
				centerToPointVector[0] = pt[0] - center[0];
				centerToPointVector[1] = pt[1] - center[1];
				centerToPointVector[2] = pt[2] - center[2];
				double centerToPointVectorLength = sqrt((centerToPointVector[0])*(centerToPointVector[0])+(centerToPointVector[1])*(centerToPointVector[1])+(centerToPointVector[2])*(centerToPointVector[2]));

				if(centerToPointVectorLength < m_Radius)
				{
					pointHasBeenMoved = true;

					// Calculate dot product between mouse movement vector and center to point vector. (Skalarprodukt)
					double dotProduct = m_MouseMovementVector[0]*centerToPointVector[0] + m_MouseMovementVector[1]*centerToPointVector[1] + m_MouseMovementVector[2]*centerToPointVector[2];
					//std::cout << "Dot Product: " << dotProduct << "\n";

					// Calculate length of Mouse Movement Vector
					double mouseMovementVectorLength = sqrt(m_MouseMovementVector[0]*m_MouseMovementVector[0]+m_MouseMovementVector[1]*m_MouseMovementVector[1]+m_MouseMovementVector[2]*m_MouseMovementVector[2]);

					// Calculate angle between mouse movement vector and center to point vector. (beta)
					double beta = asin(/*-*/(dotProduct / (centerToPointVectorLength*mouseMovementVectorLength)));   // FIXME: What about the minus? cos(a+90) = -sin(a) ???
					//std::cout << "Beta: " << beta << "\n";

					// Calculate angle between mouse movement vector and center to NEW point vector. (beta')
					double newBeta = acos((cos(beta) * centerToPointVectorLength) / m_Radius);
					//std::cout << "Beta': " << newBeta << "\n";

					// Calculate translation length between old point and translated point.
					double oldPointToNewPointLength = (sin(newBeta) * m_Radius) - (sin(beta) * centerToPointVectorLength);
					//test
					//double oldPointToNewPointLength = mouseMovementVectorLength;

					// Calculate translation vector.
					double tranlationVector[3];
					tranlationVector[0] = oldPointToNewPointLength * m_MouseMovementVector[0] / mouseMovementVectorLength;  
					tranlationVector[1] = oldPointToNewPointLength * m_MouseMovementVector[1] / mouseMovementVectorLength;  
					tranlationVector[2] = oldPointToNewPointLength * m_MouseMovementVector[2] / mouseMovementVectorLength;  
			
					// Calculate translated point.
					pt[0] = pt[0] + tranlationVector[0];
					pt[1] = pt[1] + tranlationVector[1];
					pt[2] = pt[2] + tranlationVector[2];

					if(0)
					{
						std::cout << "Mouse Movement Vector Length: " << mouseMovementVectorLength << "\n";
						// Translation length of the point should be <= mouse movement length 
						std::cout << "Old Point To New Point Length: " << oldPointToNewPointLength << "\n";
						std::cout << "New Point: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n"; 
					}

					m_Surface->GetVtkPolyData()->GetPoints()->SetPoint(ptId, pt);
				}
			}
			// Mouse Movement
			m_OldVOICenterVector[0] = center[0];
			m_OldVOICenterVector[1] = center[1];
			m_OldVOICenterVector[2] = center[2];

			// Mouse Movement with multiple vectors
			m_OldVOICenterVectors[0+m_OldVoidCenterVectorIndex] = center[0];
			m_OldVOICenterVectors[1+m_OldVoidCenterVectorIndex] = center[1];
			m_OldVOICenterVectors[2+m_OldVoidCenterVectorIndex] = center[2];			
			// Increase m_OldVoidCenterVectorIndex appropriate
			if(m_OldVoidCenterVectorIndex == m_MouseMovementVectorSumLength*3)
			{
				m_OldVoidCenterVectorIndex = 0;
			}
			else
			{
				m_OldVoidCenterVectorIndex += 3;
			}
			//std::cout << "OldVoidCenterVectorIndex: " << m_OldVoidCenterVectorIndex << "\n";

			// NEW: Calculate if ball hit faces or edges but no point has been moved
			if(1)
			{
				if(m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints())
				{
					
					if(!m_SurfaceHadBeenHit)
					{				
						m_SurfaceHadBeenHit = true;
						std::cout << "Surface hit for the first time.\n";

						if(!pointHasBeenMoved)
						{
							std::cout << "Number of points hit by the ball: " << m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() << "\n";
							// 1: Calculate Ball "Center-Surface"-Point and find associated cell. Subdivide this cell.
							if(0)
							{
								double surfaceCentralHitPoint[3];
								double* center = m_Ball->GetVtkPolyData()->GetCenter();
								
								//std::cout << "m_MouseMovementVector: " << m_MouseMovementVector[0] << " " << m_MouseMovementVector[1] << " " << m_MouseMovementVector[2] << "\n";
								//std::cout << "m_MouseMovementVectorSum: " << m_MouseMovementVectorSum[0] << " " << m_MouseMovementVectorSum[1] << " " << m_MouseMovementVectorSum[2] << "\n";

								vtkMath::Normalize(m_MouseMovementVector);
								vtkMath::Normalize(m_MouseMovementVectorSum);

								surfaceCentralHitPoint[0] = center[0]+m_MouseMovementVectorSum[0]*m_Radius;
								surfaceCentralHitPoint[1] = center[1]+m_MouseMovementVectorSum[1]*m_Radius;
								surfaceCentralHitPoint[2] = center[2]+m_MouseMovementVectorSum[2]*m_Radius;

								// Test: Display surfaceCentralHitPoint as point-node in renderer
								mitk::PointSet::Pointer surfaceCentralHitPointSet = mitk::PointSet::New();
								mitk::PointSet::PointType point;
								point[0] = surfaceCentralHitPoint[0];
								point[1] = surfaceCentralHitPoint[1];
								point[2] = surfaceCentralHitPoint[2];

								surfaceCentralHitPointSet->InsertPoint(0,point);
								mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
								pointSetNode->SetData(surfaceCentralHitPointSet);
								pointSetNode->SetName("SurfaceHitPoint");

								// Add the node to the tree
								m_DataStorage->Add(pointSetNode);

								if(1)
								{
									std::cout << "Center: " << center[0] << " " << center[1] << " " << center[2] << "\n";
									std::cout << "m_MouseMovementVector(norm): " << m_MouseMovementVector[0] << " " << m_MouseMovementVector[1] << " " << m_MouseMovementVector[2] << "\n";
									std::cout << "m_MouseMovementVectorSum(norm): " << m_MouseMovementVectorSum[0] << " " << m_MouseMovementVectorSum[1] << " " << m_MouseMovementVectorSum[2] << "\n";
									std::cout << "surfaceCentralHitPoint: " << surfaceCentralHitPoint[0] << " " << surfaceCentralHitPoint[1] << " " << surfaceCentralHitPoint[2] << "\n";
									std::cout << "point: " << point[0] << " " << point[1] << " " << point[2] << "\n";

								}
							}

							// 2: take a look at the surface intersection and search for the next points in the mesh
							if(1)
							{
								std::cout << "Second try: Use intersection points\n";
								double surfaceCentralHitPoint[3];
								surfaceCentralHitPoint[0] = 0.0;
								surfaceCentralHitPoint[1] = 0.0;
								surfaceCentralHitPoint[2] = 0.0;
								for(int i = 0; i < m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints(); i++)
								{
									double* pt;
									pt = m_IntersectionData->GetVtkPolyData()->GetPoints()->GetPoint(i);
									//std::cout << "Points: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
									surfaceCentralHitPoint[0] += pt[0];
									surfaceCentralHitPoint[1] += pt[1];
									surfaceCentralHitPoint[2] += pt[2];
								}
								surfaceCentralHitPoint[0] /= m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
								surfaceCentralHitPoint[1] /= m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
								surfaceCentralHitPoint[2] /= m_IntersectionData->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
								//std::cout << "surfaceCentralHitPoint: " << surfaceCentralHitPoint[0] << " " << surfaceCentralHitPoint[1] << " " << surfaceCentralHitPoint[2] << "\n";
						
								// Get Cell to surfaceCentralHitPoint
								vtkSmartPointer<vtkCellLocator> cellLocator = 
								vtkSmartPointer<vtkCellLocator>::New();
								cellLocator->SetDataSet(m_Surface->GetVtkPolyData());
								cellLocator->BuildLocator();

								//Find the closest points to surfaceCentralHitPoint
								double closestPoint[3];//the coordinates of the closest point will be returned here
								double closestPointDist2; //the squared distance to the closest point will be returned here
								vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
								int subId; //this is rarely used (in triangle strips only, I believe)
								cellLocator->FindClosestPoint(surfaceCentralHitPoint, closestPoint, cellId, subId, closestPointDist2);

								std::cout << "Distance to closest point in surface: " << closestPointDist2 << "\n";

								// Test: Display surfaceCentralHitPoint as point-node in renderer
								if(0)
								{
									mitk::PointSet::Pointer surfaceCentralHitPointSet = mitk::PointSet::New();
									mitk::PointSet::PointType point;
									point[0] = surfaceCentralHitPoint[0];
									point[1] = surfaceCentralHitPoint[1];
									point[2] = surfaceCentralHitPoint[2];
									mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
									pointSetNode->SetData(surfaceCentralHitPointSet);
									pointSetNode->SetName("SurfaceHitPoint");
									// Add the node to the tree
									m_DataStorage->Add(pointSetNode);
									// add closed point to pointset
									point[0] = closestPoint[0];
									point[1] = closestPoint[1];
									point[2] = closestPoint[2];
									surfaceCentralHitPointSet->InsertPoint(0,point);
								}

								if(0) // Color hit cell
								{
									vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
									faceColors->SetNumberOfComponents(3);
									vtkIdType inputNumCells = m_Surface->GetVtkPolyData()->GetNumberOfCells();
									for(int i = 0; i < inputNumCells; i++)
									{
										faceColors->InsertNextTuple3(255, 0, 0);
									}
									faceColors->InsertTuple3(cellId,0, 255, 0);
									m_Surface->GetVtkPolyData()->GetCellData()->SetScalars(faceColors);
								}

								// Get Points of hit cell
								vtkIdType ntps, *pts;
								m_Surface->GetVtkPolyData()->GetCellPoints(cellId, ntps, pts);
								std::cout << "Number of Points in Cell: " << ntps << "\n";
								if(ntps != 3)
									std::cout << "Error -> Triangulate!\n"; // TODO
								double pt1[3], pt2[3], pt3[3];
								m_Surface->GetVtkPolyData()->GetPoint(pts[0], pt1);
								m_Surface->GetVtkPolyData()->GetPoint(pts[1], pt2);
								m_Surface->GetVtkPolyData()->GetPoint(pts[2], pt3);
								
								// Calculate distance to cell points and compare to radius
								double dist1 = vtkMath::Distance2BetweenPoints(surfaceCentralHitPoint, pt1);
								double dist2 = vtkMath::Distance2BetweenPoints(surfaceCentralHitPoint, pt2);
								double dist3 = vtkMath::Distance2BetweenPoints(surfaceCentralHitPoint, pt3);

								// Decide if a new point should be added to the surface
								double maxDist = m_Radius / 2;  // TODO: Find appropriate Value
								if(dist1 > maxDist || dist2 > maxDist || dist3 > maxDist)
								{
									std::cout << "Nearest Surface Point is two far away. -> Insert new Point!\n";
										
									// Delete hit cell
									m_Surface->GetVtkPolyData()->DeleteCell(cellId);
									m_Surface->GetVtkPolyData()->RemoveDeletedCells(); 	
									m_Surface->GetVtkPolyData()->Update();

									// Add three new cells with new point
									vtkIdType *pts1, *pts2, *pts3, ptsNewPoint, pt1Id, pt2Id, pt3Id;
									pts1 = new vtkIdType[3];
									pts2 = new vtkIdType[3];
									pts3 = new vtkIdType[3];		
								
									vtkSmartPointer<vtkPolyData> newTriangles = vtkSmartPointer<vtkPolyData>::New();
									vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
									ptsNewPoint = points->InsertNextPoint(surfaceCentralHitPoint[0], surfaceCentralHitPoint[1], surfaceCentralHitPoint[2]);
									pt1Id = points->InsertNextPoint(pt1);
									pt2Id = points->InsertNextPoint(pt2);
									pt3Id = points->InsertNextPoint(pt3);

									pts1[0] = ptsNewPoint;
									pts1[1] = pt2Id;
									pts1[2] = pt3Id; 
									pts2[0] = pt1Id;
									pts2[1] = ptsNewPoint;
									pts2[2] = pt3Id; 
									pts3[0] = pt1Id;
									pts3[1] = pt2Id;
									pts3[2] = ptsNewPoint;
									newTriangles->SetPoints ( points );
									
									newTriangles->Allocate();
									newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts1);
									newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts2);
									newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts3);
									
									if(0)
									{
										mitk::Surface::Pointer newTriangleSurface = mitk::Surface::New();
										newTriangleSurface->SetVtkPolyData(newTriangles);
										mitk::DataNode::Pointer newTriangleNode = mitk::DataNode::New();
										newTriangleNode->SetData(newTriangleSurface);
										newTriangleNode->SetName("New Triangles");
										m_DataStorage->Add(newTriangleNode);
									}

									// Append to original data set
									vtkAppendPolyData *app = vtkAppendPolyData::New();
									app->AddInput(m_Surface->GetVtkPolyData());
									app->AddInput(newTriangles);
									app->Update();									
									//m_Surface->SetVtkPolyData(app->GetOutput());


									// Remove duplicated points
									vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
									removeDuplicatedPoints->SetInput(app->GetOutput());
									removeDuplicatedPoints->Update();
									m_Surface->SetVtkPolyData(removeDuplicatedPoints->GetOutput());						
								}
							}
						}
					}
				}
				else
				{
					// Surface has not been hit
					m_SurfaceHadBeenHit = false;
				}
			}
		}
		if(0)
		{
			// Third idea:	1) Calculate length of intersection contour which is an approximation to the area of the enclosed surface
			//				2) Compare length to the radius of the surface and define a threshold
			//				3) Clip out part of the surface, use subdivision filter, append filtered part to the original surface

			// 1) Calculate length of intersection contour which is an approximation to the area of the enclosed surface
			double intersectionContourLength = 0.0;

			std::cout << "Number of lines in intersection contour: " << m_IntersectionData->GetVtkPolyData()->GetNumberOfLines() << "\n";
			vtkIdType ntps, *pts;
			m_IntersectionData->GetVtkPolyData()->GetLines()->InitTraversal();
			while(m_IntersectionData->GetVtkPolyData()->GetLines()->GetNextCell(ntps, pts))
			{
				if(ntps != 2)
					std::cout << "Error: Number of points in intersection contour line != 2.\n";

				double pt1[3], pt2[3];
				m_IntersectionData->GetVtkPolyData()->GetPoints()->GetPoint(pts[0], pt1);
				m_IntersectionData->GetVtkPolyData()->GetPoints()->GetPoint(pts[1], pt2);
				intersectionContourLength += vtkMath::Distance2BetweenPoints(pt1, pt2);

				if(0)
				{
					std::cout << "Point 1: " << pt1[0] << " " << pt1[1] << " " << pt1[2] << "\n";
					std::cout << "Point 2: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << "\n";
					std::cout << "Distance between two points: " << vtkMath::Distance2BetweenPoints(pt1, pt2) << "\n";
				}
			}
			std::cout << "Intersection Contour Length: " << intersectionContourLength << "\n";
			std::cout << "Radius: " << m_Radius << "\n";

			// TODO: Check for multiple points or lines (clean ploy data)
			
			// 2) Compare length to the radius of the surface and define a threshold (Theory: u = 2 * pi * r ca. 7*r)
			//if(intersectionContourLength > 3*m_Radius)
			if(true)
			{
			if(1)
			{
				std::cout << "Intersection contour longer than 3*Radius!\n";

				// Problem: Subdivision filter brauch mindestens einen knoten um zu arbeiten

				// Idee: bestimme hit-cell, cut out hit-cell, give hit-cell to subdivision filter, replace hit-cells

				// TODO

				// 3) Clip out part of the surface, use subdivision filter, append filtered part to the original surface
				vtkClipPolyData *clip = vtkClipPolyData::New();
				clip->AddInput(m_Surface->GetVtkPolyData());

				vtkSphere *sphere1 = vtkSphere::New();
				sphere1->SetRadius(m_ContourSphere->GetRadius());
				sphere1->SetCenter(m_ContourSphere->GetCenter());

				clip->SetClipFunction(sphere1);
				clip->GenerateClippedOutputOn();
				clip->Update();

				if(0)
				{
					vtkPolyData* outputCopy = clip->GetOutput(0);
					mitk::Surface::Pointer outSurface = mitk::Surface::New();
					outSurface->SetVtkPolyData(outputCopy);
					mitk::DataNode::Pointer outNode = mitk::DataNode::New();
					outNode->SetData(outSurface);
					outNode->SetName("Appended Surfaces");
					m_DataStorage->Add(outNode);
				}
				if(0)
				{
					vtkPolyData* outputCopy2 = clip->GetOutput(1);
					mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
					outSurface2->SetVtkPolyData(outputCopy2);
					mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
					outNode2->SetData(outSurface2);
					outNode2->SetName("Appended Surfaces 2");
					m_DataStorage->Add(outNode2);
				}

				vtkSmartPointer<vtkLinearSubdivisionFilter> subdivisionFilter;
				subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
				subdivisionFilter->SetInput(clip->GetOutput(1));
				//subdivisionFilter->SetNumberOfSubdivisions(2);
				subdivisionFilter->SetNumberOfSubdivisions(1);
				subdivisionFilter->Update();

				// vtkAppendPolyData
				vtkAppendPolyData *app = vtkAppendPolyData::New();
				app->AddInput(clip->GetOutput(0));
				app->AddInput(subdivisionFilter->GetOutput());
				app->Update();			
				m_Surface->SetVtkPolyData(app->GetOutput());
		
				if(0)
				{
					vtkPolyData* outputCopy = subdivisionFilter->GetOutput();
					mitk::Surface::Pointer outSurface = mitk::Surface::New();
					outSurface->SetVtkPolyData(outputCopy);
					mitk::DataNode::Pointer outNode = mitk::DataNode::New();
					outNode->SetData(outSurface);
					outNode->SetName("Appended Surfaces");
					m_DataStorage->Add(outNode);
				}

				if(0)
				{
					// set color
					vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
					faceColors->SetNumberOfComponents(3);
					faceColors->InsertNextTuple3(255, 0, 0);
					faceColors->InsertNextTuple3(0, 255, 0);
					/*for(int i = 0; i < inputNumCells; i++)
					{
						faceColors->InsertNextTuple3(255, 0, 0);
					}*/
					m_Surface->GetVtkPolyData()->GetCellData()->SetScalars(faceColors);
				}
			}
			}
		}

		//Send out modified
		m_SurfaceNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
	}

	//UpdateRenderer();
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	
	//std::cout << "Move ROI Mouse Moved\n";
	return ok;
}

void mitk::MoveROITool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceNode = m_DataStorage->GetNamedNode(m_CurrentSurfaces[selection]);
	m_Surface = (mitk::Surface*)m_DataStorage->GetNamedNode(m_CurrentSurfaces[selection])->GetData();

	mitk::DataStorage::SetOfObjects::ConstPointer allSurfaceNodes = m_DataStorage->GetAll();
	for(unsigned int i = 0; i < allSurfaceNodes->size(); i++)
	{
		if(strcmp(allSurfaceNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			if((allSurfaceNodes->at(i)->GetName()).compare("IntersectionOutput 2D") == 0)
				continue;
			if((allSurfaceNodes->at(i)->GetName()).compare("IntersectionOutput 3D") == 0)
				continue;

			allSurfaceNodes->at(i)->SetBoolProperty("CurrentSurface", false);
			allSurfaceNodes->at(i)->SetProperty("color",mitk::ColorProperty::New(255.0/255.0, 255.0/255.0, 255.0/255.0));
		}
	}
	m_SurfaceNode->SetBoolProperty("CurrentSurface", true);
	m_IntersectionPolyDataFilter->SetInput(0,  m_Surface->GetVtkPolyData());

	// Highlight selected surface by color
	m_SurfaceNode->SetProperty("color",mitk::ColorProperty::New(1.0/255.0, 254.0/255.0, 1.0/255.0));
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void mitk::MoveROITool::RadiusChanged(int value)
{
	std::cout << "RadiusChanged in TOOL: " << value << "\n";

	m_Radius = value / m_RadiusFactor + 1;

	// UPDATE 
	// OnMouseMoved(Action* action, const StateEvent* stateEvent)
}

void mitk::MoveROITool::CombineSurfaces()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// vtkClipPolyData
	if(1)
	{
		vtkClipPolyData *clip = vtkClipPolyData::New();
		clip->AddInput(m_Surface->GetVtkPolyData());
		//clip->AddInput(m_Ball->GetVtkPolyData());
		
		vtkPlane *plane=vtkPlane::New(); 
		plane->SetOrigin(0.25, 0, 0); 
		plane->SetNormal(-1, 1, 0); 

		//clip->SetClipFunction(plane); 
		//clip->SetClipFunction(m_Ball->GetVtkPolyData());

		vtkSphere *sphere1 = vtkSphere::New();
		sphere1->SetRadius(m_ContourSphere->GetRadius());
		sphere1->SetCenter(m_ContourSphere->GetCenter());

		clip->SetClipFunction(sphere1);
		clip->GenerateClippedOutputOn();
		
		clip->Update();

		vtkPolyData* outputCopy = clip->GetOutput(0);
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Appended Surfaces");
		m_DataStorage->Add(outNode);

		vtkPolyData* outputCopy2 = clip->GetOutput(1);
		mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
		outSurface2->SetVtkPolyData(outputCopy2);
		mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
		outNode2->SetData(outSurface2);
		outNode2->SetName("Appended Surfaces 2");
		m_DataStorage->Add(outNode2);
	}

	// vtkAppendPolyData
	if(0)
	{
		vtkAppendPolyData *app = vtkAppendPolyData::New();
		app->AddInput(m_Surface->GetVtkPolyData());
		app->AddInput(m_Ball->GetVtkPolyData());
		app->Update();
		vtkPolyData* outputCopy = app->GetOutput();
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);
		
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Appended Surfaces");
		m_DataStorage->Add(outNode);
	}
	// Identify surface vertices inside the bounding ball and move those vertices
	if(0)
	{
		// statistics
		vtkPoints *inPts = m_Surface->GetVtkPolyData()->GetPoints();
		vtkCellData *inCD  = m_Surface->GetVtkPolyData()->GetCellData();
		vtkIdType numCells = m_Surface->GetVtkPolyData()->GetNumberOfCells();
		vtkIdType numVerts = m_Surface->GetVtkPolyData()->GetNumberOfVerts();
		vtkIdType inputNumPoints = m_Surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
		vtkIdType cellId = 0;
		vtkPolyData *output = vtkPolyData::New();
		output->Initialize();

		if(1)
		{
			std::cout << "Number of Vertices in Surface: " << numVerts << "\n";
			std::cout << "Number of Cells in Surface: " << numCells << "\n";
			std::cout << "Number of Vertices (Points) in Surface: " << numCells << "\n";
		}
		
		// loop over all vertices from the target mesh
		vtkPointData *inPD  = m_Surface->GetVtkPolyData()->GetPointData();

		vtkPointData *outPD = output->GetPointData();
		outPD->CopyAllocate(inPD, m_Surface->GetVtkPolyData()->GetNumberOfPoints()); // Too much!
		vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
		points->Allocate(100);
		output->SetPoints(points);

		double* center = m_Ball->GetVtkPolyData()->GetCenter();
		std::cout << "Center Coordinates: " << center[0] << " " << center[0] << " " << center[0] << "\n";
		std::cout << "Radius: " << m_Radius << "\n";

		// Calculate Normal of the surface

		// Pick nearest Surface Point from position event -> calc normal
		vtkSmartPointer<vtkPolyDataNormals> skinNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
		skinNormals->SetInput(m_Surface->GetVtkPolyData());
		//skinNormals->SetInputConnection(m_Surface->GetVtkPolyData());
		skinNormals->ComputePointNormalsOn();
		skinNormals->ComputeCellNormalsOff();
		skinNormals->SetFeatureAngle(60.0);
		skinNormals->Update();

		vtkPolyData* normals = skinNormals->GetOutput();
		mitk::Surface::Pointer normalsSurface = mitk::Surface::New();
		normalsSurface->SetVtkPolyData(normals);
		mitk::DataNode::Pointer normalsNode = mitk::DataNode::New();
		normalsNode->SetData(normalsSurface);
		normalsNode->SetName("Normals Surfaces");
		m_DataStorage->Add(normalsNode);

		 // Double normals in an array
		vtkDoubleArray* normalDataDouble = vtkDoubleArray::SafeDownCast(normals->GetPointData()->GetArray("Normals"));
		if(normalDataDouble)
		{
			int nc = normalDataDouble->GetNumberOfTuples();
			std::cout << "There are " << nc
				<< " components in normalDataDouble" << std::endl;
		}
		else
		{
			std::cout << "Normals not found!\n";
		}

		vtkDataArray* normalsGeneric = normals->GetPointData()->GetNormals(); //works
		if(normalsGeneric)
		{
			std::cout << "There are " << normalsGeneric->GetNumberOfTuples()
			  << " normals in normalsGeneric" << std::endl;
		} 

		vtkDoubleArray* normalsDouble = vtkDoubleArray::SafeDownCast(normals->GetPointData()->GetNormals());
		if(normalsDouble)
		{
			std::cout << "There are " << normalsDouble->GetNumberOfComponents()
				  << " components in normalsDouble" << std::endl;
		}

		double testDouble[3];
		normalsGeneric->GetTuple(0, testDouble);
 
		std::cout << "Double: " << testDouble[0] << " "
              << testDouble[1] << " " << testDouble[2] << std::endl;

		// Check if center is part of the surface, otherwise call nearest vertex
		bool centerPointInSurface = false;
		vtkIdType centerPtId;
		for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
		{
			double pt[3];
			m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);

			if(center[0] == pt[0] && center[1] == pt[1] && center[2] == pt[2])
			{
				centerPtId = ptId;
				centerPointInSurface = true;
				std::cout << "Center Point found in Surface!\n";
			}
		}
		if(!centerPointInSurface)
		{
			double minDistance = 9999999;
			std::cout << "Find nearest Surface Point to Center Point.\n";
			for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
			{
				double pt[3];
				m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);

				double distance = sqrt((pt[0]-center[0])*(pt[0]-center[0]) + (pt[1]-center[1])*(pt[1]-center[1]) + (pt[2]-center[2])*(pt[2]-center[2]));
				
				if(minDistance > distance)
				{
					minDistance = distance;
					centerPtId = ptId;
					std::cout << minDistance << std::endl;
				}

			}
		}
		std::cout << "Center Point ID: " << centerPtId << std::endl;
		double showPoint[3];
		m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(centerPtId, showPoint);
		std::cout << "Center Point Coordinates: " << showPoint[0] << " " << showPoint[0] << " " << showPoint[0] << "\n";

		// Get Normal to Center Point
		double centerPointNormal[3];
		normalsGeneric->GetTuple(centerPtId, centerPointNormal); // seems to work only for non-doublicated vertices


		// Copy over the point data from the input
		for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
		{
			double pt[3];
			m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);
			//std::cout << "Surface Points Coordinates: " << pt[0] << " " << pt[0] << " " << pt[0] << "\n";
			mitk::Point3D point;
			point[0] = pt[0];
			point[1] = pt[1];
			point[2] = pt[2];
					
			if(abs(center[0]-point[0]) <= m_Radius && abs(center[1]-point[1]) <= m_Radius && abs(center[2]-point[2]) <= m_Radius)
			{
				// move cells
				//pt[0] += 20;
				//pt[1] -= 20;

				pt[0] += centerPointNormal[0] * 10;
				pt[1] += centerPointNormal[1] * 10;
				pt[2] += centerPointNormal[2] * 10;

				output->GetPoints()->InsertNextPoint(pt);
				outPD->CopyData(inPD, ptId, ptId);
				//std::cout << "Surface Points Coordinates: " << pt[0] << " " << pt[0] << " " << pt[0] << "\n";
			}
			else
			{
				output->GetPoints()->InsertNextPoint(pt);
				outPD->CopyData(inPD, ptId, ptId);
			}
		}
		
		vtkCellArray *cells = m_Surface->GetVtkPolyData()->GetPolys();
		output->SetPolys(cells);

		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(output);
		
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Appended Surfaces");
		m_DataStorage->Add(outNode);
	}
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::PatchHoles()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	// Patch hole using Rami's Methodes: Advancing Front Mesh combined with the "third" weight function used by the umbrella operator.

	// identify boundary vertices and boundary edges
	//vtkIdType inputNumPoints = m_Surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();

	//for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	//{
	//	double pt[3];
	//	m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);
	//
	//}


	
	// Triangulate
	// todo
	vtkSmartPointer<vtkTriangleFilter> triangleFilter =
    vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInput(m_Surface->GetVtkPolyData());
	triangleFilter->Update();
	m_Surface->SetVtkPolyData(triangleFilter->GetOutput(0));



	
	vtkIdType inputNumCells = m_Surface->GetVtkPolyData()->GetNumberOfCells();
	//std::cout << "Number of Cells in Surface: " << inputNumCells << "\n";

	vtkPolyData* input = m_Surface->GetVtkPolyData();
	vtkCellArray* cells = input->GetPolys();
	
	//cells->Print(std::cout);
	vtkIdType *ptIds = cells->GetPointer();
	//std::cout << "Number of Cells in Surface: " << cells->GetNumberOfCells() << "\n";

	
	for (vtkIdType id0 = 0; id0 < inputNumCells; id0++)
    {
		//vtkIdType cellId0 = m_Surface->GetVtkPolyData()->Cells->GetId(id0);

		

		//int type0 = m_Surface->GetVtkPolyData()->GetCellType(cellId0);
		//vtkCell *cell0 = m_Surface->GetVtkPolyData()->GetCell(cellId0);

		//std::cout << "Cell Id: " << cellId0 << "\n";
		
		//if (type0 == VTK_TRIANGLE)
		{
			//std::cout << "Triangle!\n";
			/*vtkIdType npts0, *triPtIds0;
			mesh0->GetCellPoints(cellId0, npts0, triPtIds0);
			double triPts0[3][3];
			for (vtkIdType id = 0; id < npts0; id++)
			{
				mesh0->GetPoint(triPtIds0[id], triPts0[id]);
			}
			vtkCell *cell0 = mesh0->GetCell(cellId0);*/
		}
	}

	int count = 0;
	vtkIdType *pts, *pPolys, npts, ptId, cellId, numNei, p1, p2;
	
	vtkIdList *neighbors = vtkIdList::New();
	neighbors->Allocate(VTK_CELL_SIZE);

	input->BuildLinks();

	vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
	faceColors->SetNumberOfComponents(3);
	faceColors->InsertNextTuple3(255, 0, 0);
	faceColors->InsertNextTuple3(0, 255, 0);
	/*for(int i = 0; i < inputNumCells; i++)
	{
		faceColors->InsertNextTuple3(255, 0, 0);
	}*/
	//input->GetCellData()->SetScalars(faceColors);

	for(cellId=0, cells->InitTraversal(); cells->GetNextCell(npts,pts); cellId++)
	{
		//count++;
		if(npts != 3)
		{
			std::cout << "Number of Points in Cell is not 3. Triangulate?\n";
		}
		//std::cout << "Number of Points: " << npts << "\n";

		for (int i=0; i < npts; i++) 
		{
			p1 = pts[i];
			//p2 = pts[(i+1)%npts];
			p2 = pts[(i+1)];

			input->GetCellEdgeNeighbors(cellId,p1,p2, neighbors);
			numNei = neighbors->GetNumberOfIds();
			//std::cout << numNei << " ";

			//if(numNei > 3)
			//{
			//	input->GetCell(cellId)->Print(std::cout);
			//	std::cout << "num edges: " << input->GetCell(cellId)->GetNumberOfEdges() << "\n";

			//	faceColors->InsertTuple3(cellId,0, 255, 0);
			//	input->GetCellData()->SetScalars(faceColors);
	
			//	input->IsTriangle()
			//}

			//if(numNei < 1)
			//{
			//	count++;
			//}

			/*if(numNei == 0)
			{
				faceColors->InsertTuple3(cellId,0, 255, 0);
				input->GetCellData()->SetScalars(faceColors);
			}
			if(numNei == 1)
			{
				faceColors->InsertTuple3(cellId,255, 255, 0);
				input->GetCellData()->SetScalars(faceColors);
			}
			if(numNei == 2)
			{
				faceColors->InsertTuple3(cellId,0, 255, 255);
				input->GetCellData()->SetScalars(faceColors);
			}
			if(numNei == 3)
			{
				faceColors->InsertTuple3(cellId,0, 0, 255);
				input->GetCellData()->SetScalars(faceColors);
			}
			if(numNei == 4)
			{
				faceColors->InsertTuple3(cellId,255, 0, 0);
				input->GetCellData()->SetScalars(faceColors);
			}*/
		}

		//vtkCell *cell0 = input->GetCell(cellId0);

	}
	std::cout << "Counter: " << count << "\n";

	// insert new triangles at the boundary according to the algorithm (1,2 or 3 new triangles) untill the hole is closed

	// apply mesh fairing in order to smooth the surface, adjust triangle density and smooth curvature of the surface


	// try feature edges
	if(1)
	{
		vtkSmartPointer<vtkPolyData> outputCopy2 = vtkSmartPointer<vtkPolyData>::New();

		vtkSmartPointer<vtkFeatureEdges> featureEdges =
		vtkSmartPointer<vtkFeatureEdges>::New();
		//featureEdges->SetInputConnection(diskSource->GetOutputPort());
		featureEdges->SetInput(m_Surface->GetVtkPolyData());
		featureEdges->BoundaryEdgesOn();
		featureEdges->FeatureEdgesOff();
		featureEdges->ManifoldEdgesOff();
		//featureEdges->ManifoldEdgesOn();
		featureEdges->NonManifoldEdgesOff();
		featureEdges->Update();

		vtkPolyData* outputCopy = featureEdges->GetOutput();
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);

		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Feature Edge");
		m_DataStorage->Add(outNode);

		//std::cout << "There are " << outputCopy->GetNumberOfLines() << " lines." << std::endl;

if(1){
		vtkSmartPointer<vtkCellArray> firstHole =
			vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkLine> line =
			vtkSmartPointer<vtkLine>::New();
		
		bool cycleComplete = false;
		std::vector<double> addedLines;

		if(0)
		{
			outputCopy->GetLines()->InitTraversal();	
			outputCopy->GetLines()->GetNextCell(npts,pts);


			// randomly select a first line
			int lineID = 0;
			line->GetPointIds()->SetId(0,lineID);
			lineID++;
			line->GetPointIds()->SetId(0,lineID);
			lineID++;
			firstHole->InsertNextCell(line);

			double tmpP0[3];
			double p0[3];
			double p1[3];
			outputCopy->GetPoints()->GetPoint(pts[0],p0);
			outputCopy->GetPoints()->GetPoint(pts[1],p1);
			tmpP0[0] = p0[0];
			tmpP0[1] = p0[1];
			tmpP0[2] = p0[2];

			vtkSmartPointer<vtkPoints> points =
			vtkSmartPointer<vtkPoints>::New();
			
			// Inverted sequence due to algorithm work flow
			points->InsertNextPoint(p0);
			points->InsertNextPoint(p1);

			//std::cout << "Start Point: " << p0[0] << " " << p0[1] << " " << p0[2] << " " << pts[0] << "\n";
			//std::cout << "End Point: " << p1[0] << " " << p1[1] << " " << p1[2] << " " << pts[1] << "\n";
			
			addedLines.push_back(pts[0]);
			addedLines.push_back(pts[1]);			

			int cycles = 0;
			while(!cycleComplete)
			{
				while(outputCopy->GetLines()->GetNextCell(npts,pts))
				{
					if(npts != 2)
					{
						std::cout << "Number of Points in Line is not 2!\n";
					}

					// Add lines connected to the first line
					double p2[3];
					double p3[3];
					outputCopy->GetPoints()->GetPoint(pts[0],p2);
					outputCopy->GetPoints()->GetPoint(pts[1],p3);

					//std::cout << "Points1: " << p2[0] << " " << p2[1] << " " << p2[2] << " " << pts[0] << "\n";
					//std::cout << "Points2: " << p3[0] << " " << p3[1] << " " << p3[2] << " " << pts[1] << "\n";

					if(tmpP0[0] == p2[0] && tmpP0[1] == p2[1] && tmpP0[2] == p2[2])
					{
						//if(!std::binary_search(addedLines.begin(), addedLines.end(), pts[0]))
						//TODO liste anlegen, pruefen ob schon vorhanden, wenn noch nicht vorhanden -> hinzufügen!
						{
							//std::cout << "Connection Point found!\n";

							// Treffer auf p2 -> p3 hinzufügen
							points->InsertNextPoint(p3);

							vtkSmartPointer<vtkLine> line =
							vtkSmartPointer<vtkLine>::New();
							line->GetPointIds()->SetId(0,lineID-1);	// Syntax (0, first point id)
							line->GetPointIds()->SetId(1,lineID);	// Syntax (0, second point id)
							lineID++;

							firstHole->InsertNextCell(line);

							tmpP0[0] = p3[0];
							tmpP0[1] = p3[1];
							tmpP0[2] = p3[2];

							addedLines.push_back(pts[1]);

							//std::cout << "OtherPoint: " << p3[0] << " " << p3[1] << " " << p3[2] << " " << pts[1] << "\n";
						}
					}
					if(tmpP0[0] == p3[0] && tmpP0[1] == p3[1] && tmpP0[2] == p3[2])
					{
						//if(!std::binary_search(addedLines.begin(), addedLines.end(), pts[0]))
						//TODO liste anlegen, pruefen ob schon vorhanden, wenn noch nicht vorhanden -> hinzufügen!
						{

							//std::cout << "Connection Point found!\n";

							// Treffer auf p3 -> p2 hinzufügen
							points->InsertNextPoint(p2);

							vtkSmartPointer<vtkLine> line =
							vtkSmartPointer<vtkLine>::New();
							line->GetPointIds()->SetId(0,lineID-1);	// Syntax (0, first point id)
							line->GetPointIds()->SetId(1,lineID);	// Syntax (0, second point id)
							lineID++;

							firstHole->InsertNextCell(line);

							tmpP0[0] = p2[0];
							tmpP0[1] = p2[1];
							tmpP0[2] = p2[2];

							addedLines.push_back(pts[0]);

							//std::cout << "OtherPoint: " << p2[0] << " " << p2[1] << " " << p2[2] << " " << pts[0] << "\n";
						}
					}

					if((tmpP0[0] == p1[0] && tmpP0[1] == p1[1] && tmpP0[2] == p1[2]))
					{
						//std::cout << "End Point found!\n";
						cycleComplete = true;
						break;
					}
				}

				outputCopy->GetLines()->InitTraversal();

				if(cycles == 20)
					break;
				//std::cout << "Cycles: " << cycles << "\n";
				cycles++;
			}
			outputCopy2->SetPoints(points); 
			outputCopy2->SetLines(firstHole);
		}

		// Clean first hole polydata. There are manifold edges
		// use vtkCleanPolyData.h
		if(0)
		{
			vtkSmartPointer<vtkCleanPolyData> filledContours = vtkSmartPointer<vtkCleanPolyData>::New(); // Filter tuts! 
			filledContours->SetInput(outputCopy2);
			filledContours->Update();
			vtkPolyData* outputCopy3 = filledContours->GetOutput();
			mitk::Surface::Pointer outSurface3 = mitk::Surface::New();
			outSurface3->SetVtkPolyData(outputCopy3);
			mitk::DataNode::Pointer outNode3 = mitk::DataNode::New();
			outNode3->SetData(outSurface3);
			outNode3->SetName("Clean Poly Data Test");
			m_DataStorage->Add(outNode3);

			while(outputCopy3->GetLines()->GetNextCell(npts,pts))
			{
				std::cout << "Number of Points in Line: " << npts << "\n";

				double p2[3];
				double p3[3];
				outputCopy3->GetPoints()->GetPoint(pts[0],p2);
				outputCopy3->GetPoints()->GetPoint(pts[1],p3);

				std::cout << "Line1: " << p2[0] << " " << p2[1] << " " << p2[2] << " " << pts[0] << "\n";
				std::cout << "Line2: " << p3[0] << " " << p3[1] << " " << p3[2] << " " << pts[1] << "\n";
			}
		}

		// print added lines vector
		if(0)
		{
			for(std::vector<double>::iterator it = addedLines.begin(); it != addedLines.end(); it++)
			{
				std::cout << "pts of added lines: " << it[0] << "\n";
			}
		}

		if(0)
		{
			// Start hole patch algorithm on single closed contour (18 lines (cells), 19 points)
			int counter2 = 0;
			vtkIdType *oldPts, *newPts, *firstPts;
			double oldP1[3], oldP2[3], newP1[3], newP2[3], firstP1[3], firstP2[3];
			std::vector<double> angles;
			std::vector<std::vector<int>> firstEdgeId;
			std::vector<std::vector<int>> secondEdgeId;
			outputCopy2->GetLines()->InitTraversal();
			outputCopy2->GetLines()->GetNextCell(npts,firstPts);
			outputCopy2->GetPoints()->GetPoint(firstPts[0],firstP1);
			outputCopy2->GetPoints()->GetPoint(firstPts[1],firstP2);

			oldP1[0] = firstP1[0];
			oldP1[1] = firstP1[1];
			oldP1[2] = firstP1[2];
			oldP2[0] = firstP2[0];
			oldP2[1] = firstP2[1];
			oldP2[2] = firstP2[2];
			//oldPts[0] = firstPts[0]; // Anscheinend kann man pts nicht einfach kopieren
			//oldPts[1] = firstPts[1];
			//std::cout << "fist pts 0: " << firstPts[0] << ", first pts 1: " << firstPts[1] << "\n";
			int oldPtsInt0 = firstPts[0];
			int oldPtsInt1 = firstPts[1];
			//std::cout << "fist pts int 0: " << oldPtsInt0 << ", first pts int 1: " << oldPtsInt1 << "\n";

			if(1){
				while(outputCopy2->GetLines()->GetNextCell(npts,newPts))
				{
					outputCopy2->GetPoints()->GetPoint(newPts[0],newP1);
					outputCopy2->GetPoints()->GetPoint(newPts[1],newP2);
					
					// calculate angle between two lines
					double oldLineDirectionVector[3];
					oldLineDirectionVector[0] = oldP2[0] - oldP1[0];
					oldLineDirectionVector[1] = oldP2[1] - oldP1[1];
					oldLineDirectionVector[2] = oldP2[2] - oldP1[2];
					double newLineDirectionVector[3];
					newLineDirectionVector[0] = newP2[0] - newP1[0];
					newLineDirectionVector[1] = newP2[1] - newP1[1];
					newLineDirectionVector[2] = newP2[2] - newP1[2];

					angles.push_back(acos(vtkMath::Dot(oldLineDirectionVector, newLineDirectionVector) / (sqrt(vtkMath::Distance2BetweenPoints(oldP1,oldP2))*sqrt(vtkMath::Distance2BetweenPoints(newP1,newP2))))* 180.0 / 3.14159265);
					//std::cout << "Angle between: " << oldLineDirectionVector[0] << " "  << oldLineDirectionVector[1] << " "  << oldLineDirectionVector[2] << " and " << newLineDirectionVector[0] << " " << newLineDirectionVector[1] << " " << newLineDirectionVector[2] << " is " << angles.back() << "\n";
					std::vector<int> tmpOldVec;
					tmpOldVec.push_back(oldPts[0]);tmpOldVec.push_back(oldPts[1]);
					firstEdgeId.push_back(tmpOldVec);
					std::vector<int> tmpNewVec;
					tmpNewVec.push_back(newPts[0]);tmpNewVec.push_back(newPts[1]);
					secondEdgeId.push_back(tmpNewVec);
					// Loop parameter
					//oldPts[0] = newPts[0];
					//oldPts[1] = newPts[1];
					oldP1[0] = newP1[0];
					oldP1[1] = newP1[1];
					oldP1[2] = newP1[2];
					oldP2[0] = newP2[0];
					oldP2[1] = newP2[1];
					oldP2[2] = newP2[2];

					counter2++;
				}	

				// calculate angle between two lines
				double oldLineDirectionVector[3];
				oldLineDirectionVector[0] = oldP2[0] - oldP1[0];
				oldLineDirectionVector[1] = oldP2[1] - oldP1[1];
				oldLineDirectionVector[2] = oldP2[2] - oldP1[2];
				double firstLineDirectionVector[3];
				firstLineDirectionVector[0] = firstP2[0] - firstP1[0];
				firstLineDirectionVector[1] = firstP2[1] - firstP1[1];
				firstLineDirectionVector[2] = firstP2[2] - firstP1[2];

				angles.push_back(acos(vtkMath::Dot(oldLineDirectionVector, firstLineDirectionVector) / (sqrt(vtkMath::Distance2BetweenPoints(oldP1,oldP2))*sqrt(vtkMath::Distance2BetweenPoints(firstP1,firstP2))))* 180.0 / 3.14159265);
				//std::cout << "Angle between: " << oldLineDirectionVector[0] << " "  << oldLineDirectionVector[1] << " "  << oldLineDirectionVector[2] << " and " << firstLineDirectionVector[0] << " " << firstLineDirectionVector[1] << " " << firstLineDirectionVector[2] << " is " << angles.back() << "\n";

				std::vector<int> tmpOldVec;
				tmpOldVec.push_back(oldPts[0]);tmpOldVec.push_back(oldPts[1]);
				firstEdgeId.push_back(tmpOldVec);
				std::vector<int> tmpNewVec;
				tmpNewVec.push_back(newPts[0]);tmpNewVec.push_back(newPts[1]);
				secondEdgeId.push_back(tmpNewVec);

				counter2++;

				//std::cout << "Single closed contour counter: " << counter2 << "\n";
				// Print angles and line pts
				std::vector<double>::iterator it = angles.begin();
				std::vector<std::vector<int>>::iterator firstEdgeIdIt = firstEdgeId.begin();
				std::vector<std::vector<int>>::iterator secondEdgeIdIt = secondEdgeId.begin();
				for(; it != angles.end(); it++, firstEdgeIdIt++, secondEdgeIdIt++)
				{
					;//std::cout << "Angles vector: " << *it << ", first Edge: " << (*firstEdgeIdIt)[0] << " " << (*firstEdgeIdIt)[1] << ", second Edge: " << (*secondEdgeIdIt)[0] << " " << (*secondEdgeIdIt)[1] << "\n";
				}
			}

			
			// pic largest angle -> rami: smallest angle (angle bounds: l < 45; 45 < l < 105; 105 < l < 180)
			bool print = 0;
			for(std::vector<double>::iterator it = angles.begin(); it != angles.end(); it++)
			{
				if(*it < 45)
				{
					if(print){std::cout << "Angle > 45 found. -> Insert two points first, then insert lines.\n";}
				}
				else if(*it < 105)
				{
					if(print){std::cout << "Angle > 45 & Angle < 105 found. Insert one point first, then insert lines.\n";}

					// Insert one point

					// Connect point with two lines
				}
				else
				{
					if(print){std::cout << "Angle > 105 found. Insert line.\n";}
					
					// Directly insert new line between v(i-1) and v(i+1)
				}
			}


			//TODO First Edge and Second Edge, patchHoles() crashes if mouse enters the renderer windows after execution
		}


		// new dataNode
		//vtkPolyData* outputCopy2 = featureEdges->GetOutput();

		// create new data
		if(0)
		{
			double origin[3] = {0.0, 0.0, 0.0};
			double p0[3] = {1.0, 0.0, 0.0};
			double p1[3] = {0.0, 1.0, 0.0};
			double p2[3] = {0.0, 1.0, 2.0};
			double p3[3] = {1.0, 2.0, 3.0};

			// Create a vtkPoints object and store the points in it
			vtkSmartPointer<vtkPoints> points =
			vtkSmartPointer<vtkPoints>::New();
			points->InsertNextPoint(origin);
			points->InsertNextPoint(p0);
			points->InsertNextPoint(p1);
			points->InsertNextPoint(p2);
			points->InsertNextPoint(p3);

			// Create a cell array to store the lines in and add the lines to it
			vtkSmartPointer<vtkCellArray> lines =
			vtkSmartPointer<vtkCellArray>::New();

			for(unsigned int i = 0; i < 3; i++)
			{
			//Create the first line (between Origin and P0)
			vtkSmartPointer<vtkLine> line =
			vtkSmartPointer<vtkLine>::New();
			line->GetPointIds()->SetId(0,i);
			line->GetPointIds()->SetId(1,i+1);
			lines->InsertNextCell(line);
			}

			// Create a polydata to store everything in
			//vtkSmartPointer<vtkPolyData> linesPolyData =
			//vtkSmartPointer<vtkPolyData>::New();

			// Add the points to the dataset
			outputCopy2->SetPoints(points);

			// Add the lines to the dataset
			outputCopy2->SetLines(lines);
		}

		mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
		outSurface2->SetVtkPolyData(outputCopy2);

		mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
		outNode2->SetData(outSurface2);
		outNode2->SetName("First Hole");
		m_DataStorage->Add(outNode2);

		// print new line
		if(0)
		{
			std::cout << "New line numCells: " << firstHole->GetNumberOfCells() << "\n";
			outputCopy2->GetLines()->InitTraversal();
			while(outputCopy2->GetLines()->GetNextCell(npts,pts))
			{
				std::cout << "Number of Points in Line: " << npts << "\n";

				double p2[3];
				double p3[3];
				outputCopy2->GetPoints()->GetPoint(pts[0],p2);
				outputCopy2->GetPoints()->GetPoint(pts[1],p3);

				std::cout << "Line1: " << p2[0] << " " << p2[1] << " " << p2[2] << " " << pts[0] << "\n";
				std::cout << "Line2: " << p3[0] << " " << p3[1] << " " << p3[2] << " " << pts[1] << "\n";
			}
		}
	}


	// try VTK functions
	if(1)
	{
		vtkPolyData* outputCopy;
		if(1)
		{
			vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter = vtkSmartPointer<vtkFillHolesFilter>::New();

			std::cout << "Fill Holes Filter Input Data: " << m_SurfaceNode->GetName() << "\n";

			fillHolesFilter->SetInput(m_Surface->GetVtkPolyData());

			double maxHoleSize = 10000.0;
			fillHolesFilter->SetHoleSize(maxHoleSize);

			fillHolesFilter->Update();

			std::cout << "Hole Size: " << fillHolesFilter->GetHoleSize() << "\n";
			std::cout << "Number of Output Ports: " << fillHolesFilter->GetNumberOfOutputPorts() << "\n";
			
			m_Surface->SetVtkPolyData(fillHolesFilter->GetOutput());

			// Remove duplicated points
			vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
			removeDuplicatedPoints->SetInput(m_Surface->GetVtkPolyData());
			removeDuplicatedPoints->Update();
			m_Surface->SetVtkPolyData(removeDuplicatedPoints->GetOutput());

			// Clean Poly Data
			vtkSmartPointer<vtkCleanPolyData> filledContours = vtkSmartPointer<vtkCleanPolyData>::New(); // Filter tuts! 
			filledContours->SetInput(m_Surface->GetVtkPolyData());
			filledContours->Update();
			m_Surface->SetVtkPolyData(filledContours->GetOutput());

			// Triangulate
			vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
			//triangleFilter->SetInput(removeDuplicatedPoints->GetOutput());
			triangleFilter->SetInput(m_Surface->GetVtkPolyData());
			triangleFilter->Update();
			m_Surface->SetVtkPolyData(triangleFilter->GetOutput(0));


			/*mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(outputCopy);
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Surface with patched hole.");
			m_DataStorage->Add(outNode);*/

			m_Surface->Update();
			m_SurfaceNode->SetProperty("color",mitk::ColorProperty::New(1.0/255.0, 254.0/255.0, 1.0/255.0));
			UpdateRenderer();
		}
		if(0)
		{
			// TODO
			vtkSmartPointer<vtkSurfaceReconstructionFilter> surf = 
			vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();
			surf->SetInput(m_Surface->GetVtkPolyData());
			vtkImageData* tmp = surf->GetOutput();
			mitk::Image::Pointer outImage = mitk::Image::New();
			outImage->Initialize(tmp);
			
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outImage);
			outNode->SetName("Surface with patched hole.");
			m_DataStorage->Add(outNode);
			//outputCopy = surf->GetOutput();
		}

		// TODO
		if(0)
		{
			vtkSmartPointer<vtkExtractEdges> extractEdges = vtkSmartPointer<vtkExtractEdges>::New();
			extractEdges->SetInput(m_Surface->GetVtkPolyData());
			extractEdges->Update();

			vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
			stripper->SetInput(extractEdges->GetOutput());
			stripper->Update();

			vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::New();
			polydata->SetPoints(stripper->GetOutput()->GetPoints());
			polydata->SetPolys(stripper->GetOutput()->GetLines());

			vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
			triangleFilter->SetInput(polydata);
			triangleFilter->Update();

			vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
			append->AddInput(m_Surface->GetVtkPolyData());
			append->AddInput(triangleFilter->GetOutput());
			append->Update();

			mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(append->GetOutput());
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Surface with patched hole.");
			m_DataStorage->Add(outNode);
		}



		//vtkPolyData* outputCopy = fillHolesFilter->GetOutput(0);
		
		//mitk::Surface::Pointer outSurface = mitk::Surface::New();
		//outSurface->SetVtkPolyData(outputCopy);


		//mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		//outNode->SetData(outSurface);
		//outNode->SetName("Surface with patched hole.");
		//m_DataStorage->Add(outNode);
	}
}
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::SmoothROI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Smooth ROI: 1) Cut out VOI from Surface, 2) Smooth extraction with preserve edges on, 3) Append smoothed extraction to surface
	vtkClipPolyData *clip = vtkClipPolyData::New();
	clip->AddInput(m_Surface->GetVtkPolyData());

	vtkSphere *sphere1 = vtkSphere::New();
	sphere1->SetRadius(m_ContourSphere->GetRadius());
	sphere1->SetCenter(m_ContourSphere->GetCenter());

	clip->SetClipFunction(sphere1);
	clip->GenerateClippedOutputOn();
	clip->Update();

	if(0)
	{
		vtkPolyData* outputCopy = clip->GetOutput(0);
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Appended Surfaces");
		m_DataStorage->Add(outNode);
	}
	if(0)
	{
		vtkPolyData* outputCopy2 = clip->GetOutput(1);
		mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
		outSurface2->SetVtkPolyData(outputCopy2);
		mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
		outNode2->SetData(outSurface2);
		outNode2->SetName("Appended Surfaces 2");
		m_DataStorage->Add(outNode2);
	}

	// Smooth extraction
	vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
	// TODO: Decide which clipped surface is smaller
	//smoother->SetInput(clip->GetOutput(0));
	smoother->SetInput(clip->GetOutput(1));
	smoother->SetNumberOfIterations(m_SmoothIteration);
	smoother->SetRelaxationFactor( /*m_SmoothRelaxation*/ 0.1);
	smoother->SetFeatureAngle( 60 );
	smoother->FeatureEdgeSmoothingOff();
	//smoother->BoundarySmoothingOff();
	smoother->BoundarySmoothingOn();
	smoother->SetConvergence( 0 );
	smoother->Update();

	// vtkAppendPolyData
	vtkAppendPolyData *app = vtkAppendPolyData::New();
	app->AddInput(clip->GetOutput(0));
	app->AddInput(smoother->GetOutput());
	app->Update();
	
	m_Surface->SetVtkPolyData(app->GetOutput());
	UpdateRenderer();
	
	if(0)
	{
		vtkPolyData* outputCopy = app->GetOutput();
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Appended Surfaces");
		m_DataStorage->Add(outNode);
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::SmoothChanged(int value)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SmoothIteration = value;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::Subdivide()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Try VTK subdivision filters
	int i = 0;
	vtkSmartPointer<vtkPolyDataAlgorithm> subdivisionFilter;
    switch(i)
    {
      case 0:
        subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
        break;
      case 1:
        subdivisionFilter =  vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
        break;
      case 2: 
        subdivisionFilter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
        break;
      default:
        break;
    }

	subdivisionFilter->SetInput(m_Surface->GetVtkPolyData());

    //subdivisionFilter->SetNumberOfSubdivisions(2);
    subdivisionFilter->Update();

	if(0)
	{
		vtkPolyData* outputCopy = subdivisionFilter->GetOutput();
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(outputCopy);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName("Subdivided Surface");
		m_DataStorage->Add(outNode);
	}
	if(1)
	{
		m_Surface->SetVtkPolyData(subdivisionFilter->GetOutput());
	}
		
	UpdateRenderer();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::SubdivideVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Extend subdivide VOI abilities:
	// Problem: All three subdivision filters need an initial point in the mesh. They cannot work on single cells without a point inside.
	// Solution:	1) Check if there is one or more point(s) in the VOI
	//				2) Insert point if necessary
	//				3) Use subdivision filter if wanted 

	// Check for point(s) inside the VOI
	bool pointsAreInVOI = false;
	vtkIdType inputNumPoints = m_Surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
	double* center = m_Ball->GetVtkPolyData()->GetCenter();
	for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		double pt[3];
		m_Surface->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt);			
		// Use distance			
		double distance = sqrt((center[0]-pt[0])*(center[0]-pt[0])+(center[1]-pt[1])*(center[1]-pt[1])+(center[2]-pt[2])*(center[2]-pt[2]));
		if(distance < m_Radius)
		{
			pointsAreInVOI = true;
		}
	}
	if(pointsAreInVOI == true)
		std::cout << "There are one or more point(s) inside the VOI.\n";

	// Insert point(s)
	if(pointsAreInVOI == false)
	{
		std::cout << "There is no point inside the VOI. Subdivision filter will fail. Insert new point first.\n";

		// Test: Construct unsymmetric contour to test centroid function
		vtkSmartPointer<vtkPolyData> linesPolyData = createTestContour();

		double weightSum = 0.0;
		//int numberOfPoints = m_IntersectionData->GetVtkPolyData()->GetNumberOfLines() * 2;
		double currentWeight;
		double centroid[3] = {0.0, 0.0, 0.0};
		std::cout << "Centroid: " << centroid[0] << " " << centroid[0] << " " << centroid[0] << "\n";
		vtkIdType ntps, *pts;
		//linesPolyData->GetLines()->InitTraversal();
		//while(linesPolyData->GetLines()->GetNextCell(ntps, pts))
		m_IntersectionData->GetVtkPolyData()->GetLines()->InitTraversal();
		while(m_IntersectionData->GetVtkPolyData()->GetLines()->GetNextCell(ntps, pts))
		{
			if(ntps != 2)
				std::cout << "Error: Number of points in intersection contour line != 2.\n";

			double pt1[3], pt2[3];
			m_IntersectionData->GetVtkPolyData()->GetPoints()->GetPoint(pts[0], pt1);
			m_IntersectionData->GetVtkPolyData()->GetPoints()->GetPoint(pts[1], pt2);
			//linesPolyData->GetPoints()->GetPoint(pts[0], pt1);
			//linesPolyData->GetPoints()->GetPoint(pts[1], pt2);

			currentWeight = vtkMath::Distance2BetweenPoints(pt1, pt2);
			centroid[0] += pt1[0] * currentWeight;
			centroid[1] += pt1[1] * currentWeight;
			centroid[2] += pt1[2] * currentWeight;
			centroid[0] += pt2[0] * currentWeight;
			centroid[1] += pt2[1] * currentWeight;
			centroid[2] += pt2[2] * currentWeight;

			weightSum += currentWeight;
			
			if(0)
			{
				std::cout << "Point 1: " << pt1[0] << " " << pt1[1] << " " << pt1[2] << "\n";
				std::cout << "Point 2: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << "\n";
				std::cout << "Distance between two points: " << vtkMath::Distance2BetweenPoints(pt1, pt2) << "\n";
			}
		}
		centroid[0] /= (weightSum * 2);
		centroid[1] /= (weightSum * 2);
		centroid[2] /= (weightSum * 2);
		// std::cout << "Centroid: " << centroid[0] << " " << centroid[1] << " " << centroid[2] << "\n";

		if(0)
		{
			mitk::PointSet::Pointer surfaceCentralHitPointSet = mitk::PointSet::New();
			mitk::PointSet::PointType point;
			point[0] = centroid[0];
			point[1] = centroid[1];
			point[2] = centroid[2];
			surfaceCentralHitPointSet->InsertPoint(0,point);
			mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
			pointSetNode->SetData(surfaceCentralHitPointSet);
			pointSetNode->SetName("Weigthed Centroid");
			m_DataStorage->Add(pointSetNode);
		}

		// Add centroid point to surface
		// Get Cell to surfaceCentralHitPoint
		vtkSmartPointer<vtkCellLocator> cellLocator = 
		vtkSmartPointer<vtkCellLocator>::New();
		cellLocator->SetDataSet(m_Surface->GetVtkPolyData());
		cellLocator->BuildLocator();

		//Find the closest points to surfaceCentralHitPoint
		double closestPoint[3];//the coordinates of the closest point will be returned here
		double closestPointDist2; //the squared distance to the closest point will be returned here
		vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
		int subId; //this is rarely used (in triangle strips only, I believe)
		cellLocator->FindClosestPoint(centroid, closestPoint, cellId, subId, closestPointDist2);

		std::cout << "Distance to closest point in surface: " << closestPointDist2 << "\n";

		// Get Points of hit cell
		//vtkIdType ntps, *pts;
		m_Surface->GetVtkPolyData()->GetCellPoints(cellId, ntps, pts);
		std::cout << "Number of Points in Cell: " << ntps << "\n";
		if(ntps != 3)
			std::cout << "Error -> Triangulate!\n"; // TODO
		double pt1[3], pt2[3], pt3[3];
		m_Surface->GetVtkPolyData()->GetPoint(pts[0], pt1);
		m_Surface->GetVtkPolyData()->GetPoint(pts[1], pt2);
		m_Surface->GetVtkPolyData()->GetPoint(pts[2], pt3);
		
		// Delete hit cell
		m_Surface->GetVtkPolyData()->DeleteCell(cellId);
		m_Surface->GetVtkPolyData()->RemoveDeletedCells(); 	
		m_Surface->GetVtkPolyData()->Update();

		// Add three new cells with new point
		vtkIdType *pts1, *pts2, *pts3, ptsNewPoint, pt1Id, pt2Id, pt3Id;
		pts1 = new vtkIdType[3];
		pts2 = new vtkIdType[3];
		pts3 = new vtkIdType[3];		
	
		vtkSmartPointer<vtkPolyData> newTriangles = vtkSmartPointer<vtkPolyData>::New();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		ptsNewPoint = points->InsertNextPoint(centroid[0], centroid[1], centroid[2]);
		pt1Id = points->InsertNextPoint(pt1);
		pt2Id = points->InsertNextPoint(pt2);
		pt3Id = points->InsertNextPoint(pt3);

		pts1[0] = ptsNewPoint;
		pts1[1] = pt2Id;
		pts1[2] = pt3Id; 
		pts2[0] = pt1Id;
		pts2[1] = ptsNewPoint;
		pts2[2] = pt3Id; 
		pts3[0] = pt1Id;
		pts3[1] = pt2Id;
		pts3[2] = ptsNewPoint;
		newTriangles->SetPoints ( points );
		
		newTriangles->Allocate();
		newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts1);
		newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts2);
		newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts3);

		// Test: Rebuild old cell
		// Add three new cells with new point
		//vtkIdType *pts1, *pts2, *pts3, ptsNewPoint, pt1Id, pt2Id, pt3Id;
		//pts1 = new vtkIdType[3];
		//vtkSmartPointer<vtkPolyData> newTriangles = vtkSmartPointer<vtkPolyData>::New();
		//vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		////ptsNewPoint = points->InsertNextPoint(centroid[0], centroid[1], centroid[2]);
		//pt1Id = points->InsertNextPoint(pt1);
		//pt3Id = points->InsertNextPoint(pt2);
		//pt2Id = points->InsertNextPoint(pt3);
		//pts1[0] = pt1Id;
		//pts1[1] = pt2Id;
		//pts1[2] = pt3Id; 
		//newTriangles->SetPoints ( points );
		//newTriangles->Allocate();
		//newTriangles->InsertNextCell(VTK_TRIANGLE, 3, pts1);
		
		if(1)
		{
			mitk::Surface::Pointer newTriangleSurface = mitk::Surface::New();
			newTriangleSurface->SetVtkPolyData(newTriangles);
			mitk::DataNode::Pointer newTriangleNode = mitk::DataNode::New();
			newTriangleNode->SetData(newTriangleSurface);
			newTriangleNode->SetName("New Triangles");
			m_DataStorage->Add(newTriangleNode);
		}

		// Apply Subdivision filter here TODO -> so ist es viel besser!!!
		// Linear
		vtkSmartPointer<vtkLinearSubdivisionFilter> subdivisionFilter;
		subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
		subdivisionFilter->SetInput(newTriangles);
		subdivisionFilter->SetNumberOfSubdivisions(2);
		subdivisionFilter->Update();
		
		// Loop
		//vtkSmartPointer<vtkLoopSubdivisionFilter> subdivisionFilter;
		//subdivisionFilter = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
		//subdivisionFilter->SetInput(newTriangles);
		//subdivisionFilter->Update();

		// Butterfly
		//vtkSmartPointer<vtkButterflySubdivisionFilter> subdivisionFilter;
		//subdivisionFilter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
		//subdivisionFilter->SetInput(newTriangles);
		//subdivisionFilter->Update();

		//vtkAppendPolyData *app2 = vtkAppendPolyData::New();
		//app2->AddInput(m_Surface->GetVtkPolyData());
		////app2->AddInput(subdivisionFilter->GetOutput());
		//app2->AddInput(newTriangles);
		//app2->Update();
		//m_Surface->SetVtkPolyData(app2->GetOutput());

		vtkAppendPolyData *app = vtkAppendPolyData::New();
		app->AddInput(m_Surface->GetVtkPolyData());
		app->AddInput(subdivisionFilter->GetOutput());
		//app->AddInput(app2->GetOutput());
		//app->AddInput(newTriangles);
		app->Update();
		m_Surface->SetVtkPolyData(app->GetOutput());

		if(0)
		{
			mitk::Surface::Pointer newTriangleSurface = mitk::Surface::New();
			newTriangleSurface->SetVtkPolyData(subdivisionFilter->GetOutput());
			mitk::DataNode::Pointer newTriangleNode = mitk::DataNode::New();
			newTriangleNode->SetData(newTriangleSurface);
			newTriangleNode->SetName("Subdivision Output");
			m_DataStorage->Add(newTriangleNode);
		}

		// Append to original data set
		/*vtkAppendPolyData *app = vtkAppendPolyData::New();
		app->AddInput(m_Surface->GetVtkPolyData());
		app->AddInput(newTriangles);
		app->Update();									
		m_Surface->SetVtkPolyData(app->GetOutput());*/

		// Remove duplicated points
		vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
		removeDuplicatedPoints->SetInput(m_Surface->GetVtkPolyData());
		removeDuplicatedPoints->Update();
		m_Surface->SetVtkPolyData(removeDuplicatedPoints->GetOutput());

		// Clean Poly Data
		vtkSmartPointer<vtkCleanPolyData> filledContours = vtkSmartPointer<vtkCleanPolyData>::New(); // Filter tuts! 
		filledContours->SetInput(m_Surface->GetVtkPolyData());
		filledContours->Update();
		m_Surface->SetVtkPolyData(filledContours->GetOutput());

		// Triangulate
		vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
		//triangleFilter->SetInput(removeDuplicatedPoints->GetOutput());
		triangleFilter->SetInput(m_Surface->GetVtkPolyData());
		triangleFilter->Update();
		m_Surface->SetVtkPolyData(triangleFilter->GetOutput(0));

		// Clean Poly Data
		/*vtkSmartPointer<vtkCleanPolyData> filledContours = vtkSmartPointer<vtkCleanPolyData>::New(); // Filter tuts! 
		filledContours->SetInput(triangleFilter->GetOutput(0));
		filledContours->Update();
		m_Surface->SetVtkPolyData(filledContours->GetOutput());

		m_Surface->Update();*/
	}

	// Subdivision filter
	if(0)
	{
		vtkClipPolyData *clip = vtkClipPolyData::New();
		clip->AddInput(m_Surface->GetVtkPolyData());

		vtkSphere *sphere1 = vtkSphere::New();
		sphere1->SetRadius(m_ContourSphere->GetRadius());
		sphere1->SetCenter(m_ContourSphere->GetCenter());

		clip->SetClipFunction(sphere1);
		clip->GenerateClippedOutputOn();
		clip->Update();

		if(0)
		{
			vtkPolyData* outputCopy = clip->GetOutput(0);
			mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(outputCopy);
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Appended Surfaces");
			m_DataStorage->Add(outNode);
		}
		if(0)
		{
			vtkPolyData* outputCopy2 = clip->GetOutput(1);
			mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
			outSurface2->SetVtkPolyData(outputCopy2);
			mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
			outNode2->SetData(outSurface2);
			outNode2->SetName("Appended Surfaces 2");
			m_DataStorage->Add(outNode2);
		}

		
		//vtkSmartPointer<vtkPolyDataAlgorithm> subdivisionFilter;
		vtkSmartPointer<vtkLinearSubdivisionFilter> subdivisionFilter;

		// Linear
		subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
		subdivisionFilter->SetInput(clip->GetOutput(1));
		subdivisionFilter->SetNumberOfSubdivisions(5);
		subdivisionFilter->Update();

		// Loop
		/*subdivisionFilter =  vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
		subdivisionFilter->SetInput(clip->GetOutput(1));
		subdivisionFilter->Update();*/

		// Butterfly
		/*subdivisionFilter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
		subdivisionFilter->SetInput(clip->GetOutput(1));
		subdivisionFilter->Update();*/

		// vtkAppendPolyData
		vtkAppendPolyData *app = vtkAppendPolyData::New();
		app->AddInput(clip->GetOutput(0));
		app->AddInput(clip->GetOutput(1));
		//app->AddInput(subdivisionFilter->GetOutput());
		app->Update();			
		m_Surface->SetVtkPolyData(app->GetOutput());

		UpdateRenderer();

		if(1)
		{
			vtkPolyData* outputCopy = subdivisionFilter->GetOutput();
			mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(outputCopy);
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Appended Surfaces");
			m_DataStorage->Add(outNode);
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::ColorSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// set color TODO: only two colors?
	int inputNumCells = m_Surface->GetVtkPolyData()->GetNumberOfCells();
	std::cout << "Number of Cells in Surface: " << inputNumCells << "\n";
	vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
	faceColors->SetNumberOfComponents(3);
	//faceColors->InsertNextTuple3(255, 0, 0);
	//faceColors->InsertNextTuple3(0, 255, 0);
	for(int i = 0; i < inputNumCells+8; i += 8)
	{
		faceColors->InsertNextTuple3(0, 0, 0);
		faceColors->InsertNextTuple3(254, 0, 0);
		faceColors->InsertNextTuple3(0, 254, 0);
		faceColors->InsertNextTuple3(254, 254, 0);
		faceColors->InsertNextTuple3(0, 0, 254);
		faceColors->InsertNextTuple3(254, 0, 254);
		faceColors->InsertNextTuple3(0, 254, 254);
		faceColors->InsertNextTuple3(254, 254, 254);
	}
	m_Surface->GetVtkPolyData()->GetCellData()->SetScalars(faceColors);
	UpdateRenderer();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::JumpToPosition()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_JumpToPosition)
	{
		std::cout << "JumpToPosition: " << m_JumpToPosition << "\n";
		m_JumpToPosition = false;
	}
	else
	{
		std::cout << "JumpToPosition: " << m_JumpToPosition << "\n";
		m_JumpToPosition = true;
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::MoveROITool::UpdateRenderer()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// FIXME
	float opacity = 0.0;
	m_SurfaceNode->GetFloatProperty("opacity", opacity);
	opacity -= 0.000001;
	m_SurfaceNode->SetFloatProperty("opacity", opacity);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	opacity += 0.000001;
	m_SurfaceNode->SetFloatProperty("opacity", opacity);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

vtkSmartPointer<vtkPolyData> mitk::MoveROITool::createTestContour()
{
	// Test: Construct unsymmetric contour to test centroid function
		double origin[3] = {0.0, 0.0, 0.0};
		double p0[3] = {3.0, 1.0, 0.0};
		double p1[3] = {7.0, 1.0, 0.0};
		double p2[3] = {10.0, 0.0, 0.0};
		double p3[3] = {9.0, 3.0, 0.0};
		double p4[3] = {9.0, 7.0, 0.0};
		double p5[3] = {10.0, 10.0, 0.0};		
		double p6[3] = {0.0, 60.0, 100.0};
		// Create a vtkPoints object and store the points in it
		vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();
		points->InsertNextPoint(origin);
		points->InsertNextPoint(p0);
		points->InsertNextPoint(p1);
		points->InsertNextPoint(p2);
		points->InsertNextPoint(p3);
		points->InsertNextPoint(p4);
		points->InsertNextPoint(p5);
		points->InsertNextPoint(p6);
		// Create a cell array to store the lines in and add the lines to it
		vtkSmartPointer<vtkCellArray> lines =
		vtkSmartPointer<vtkCellArray>::New();
		for(unsigned int i = 0; i < 7; i++)
		{
			//Create the first line (between Origin and P0)
			vtkSmartPointer<vtkLine> line =
			vtkSmartPointer<vtkLine>::New();
			line->GetPointIds()->SetId(0,i);
			line->GetPointIds()->SetId(1,i+1);
			lines->InsertNextCell(line);
		}
		vtkSmartPointer<vtkLine> line =
		vtkSmartPointer<vtkLine>::New();
		line->GetPointIds()->SetId(0,0);
		line->GetPointIds()->SetId(1,7);
		lines->InsertNextCell(line);
		// Create a polydata to store everything in
		vtkSmartPointer<vtkPolyData> linesPolyData =
		vtkSmartPointer<vtkPolyData>::New();
		// Add the points to the dataset
		linesPolyData->SetPoints(points);
		// Add the lines to the dataset
		linesPolyData->SetLines(lines);
		mitk::Surface::Pointer linesPolyDataSurface = mitk::Surface::New();
		linesPolyDataSurface->SetVtkPolyData(linesPolyData);
		mitk::DataNode::Pointer linesPolyDataNode = mitk::DataNode::New();
		linesPolyDataNode->SetData(linesPolyDataSurface);
		linesPolyDataNode->SetName("Unsymmetric Contour");
		m_DataStorage->Add(linesPolyDataNode);
		
		return linesPolyData;
}