#include "mitkToolExtension.h"
#include "../../../../RenderingManagerProperty.h"

// MITK
#include "mitkPointSet.h"
#include <mitkSurfaceGLMapper2D.h>
#include <mitkNodePredicateProperty.h>
#include <mitkStateEvent.h>

// VTK
#include <vtkLine.h>
#include <vtkCellArray.h>


mitk::ToolExtension::ToolExtension()
{}

mitk::ToolExtension::~ToolExtension()
{}

void mitk::ToolExtension::InitTool()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//PoinSetNode for MouseOrientation
	m_PositionTrackerNode = mitk::DataNode::New();
	m_PositionTrackerNode->SetProperty("name", mitk::StringProperty::New("Mouse Position"));
	m_PositionTrackerNode->SetData( mitk::PointSet::New() );
	m_PositionTrackerNode->SetColor(1.0,0.33,0.0);
	m_PositionTrackerNode->SetProperty("layer", mitk::IntProperty::New(1001));
	//m_PositionTrackerNode->SetVisibility(true);
	m_PositionTrackerNode->SetVisibility(false);
	m_PositionTrackerNode->SetProperty("inputdevice", mitk::BoolProperty::New(true) );
	m_PositionTrackerNode->SetProperty("BaseRendererMapperID", mitk::IntProperty::New(0) );//point position 2D mouse
	m_PositionTrackerNode->SetProperty("baserenderer", mitk::StringProperty::New("N/A"));

	m_ContourResolution = 10;
	m_ReinitSelection = false;
	m_JumpToPosition = false;
	m_CalcIntersection = true;
	m_Radius = 5.5;
	m_RadiusFactor = 5;

	// Change radius interactor
	m_RadiusInteractorOn = true;
	m_ChangeRadius = new RadiusInteractor("MyRadiusInteractor");
	m_ChangeRadius->UpdateRadius += mitk::MessageDelegate1<mitk::ToolExtension, int>(this, &mitk::ToolExtension::UpdateRadiusViaInteractor);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::ActivateTool()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	m_MitkView = properties->GetStdMultiWidget();

	// Mouse position
	// Add unique identifier for data nodes
	int newId = GetUniqueId();
	m_PositionTrackerNode->SetIntProperty("UniqueID", newId);
	m_DataStorage->Add(m_PositionTrackerNode);

	// Get Surfaces and send to GUI
	m_CurrentSurfaces.clear();
	// Get recent surface, if available
	bool currentSurfaceAvailable = false;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_DataStorage->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			allNodes->at(i)->GetBoolProperty("CurrentSurface", currentSurfaceAvailable);
			if(currentSurfaceAvailable)
			{
				m_Surface = (mitk::Surface*)allNodes->at(i)->GetData();
				m_SurfaceNode = allNodes->at(i);

				std::string currentSurfaceName = allNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allNodes->at(i)->SetBoolProperty("CurrentSurface", true);

				//std::cout << "Current Surface found!\n";
				break;
			}
		}
	}
	// If there is no current surface, try to find another
	bool anySurfaceFound = false;
	if(!currentSurfaceAvailable)
	{
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			//allSurfaceNodes->at(i)->Print(std::cout);
			
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
			{
				m_Surface = (mitk::Surface*)allNodes->at(i)->GetData();
				m_SurfaceNode = allNodes->at(i);

				std::string currentSurfaceName = allNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allNodes->at(i)->SetBoolProperty("CurrentSurface", true);

				//std::cout << "Surface found!\n";
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
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{	
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			if(!std::binary_search(m_CurrentSurfaces.begin(), m_CurrentSurfaces.end(), allNodes->at(i)->GetName()))
			{
				if((allNodes->at(i)->GetName()).compare("IntersectionOutput 2D") == 0)
					continue;
				if((allNodes->at(i)->GetName()).compare("IntersectionOutput 3D") == 0)
					continue;

				//std::cout << "Another surface found. -> add to vector.\n";

				std::string currentSurfaceName = allNodes->at(i)->GetName();
				m_CurrentSurfaces.push_back(currentSurfaceName);

				allNodes->at(i)->SetBoolProperty("CurrentSurface", false);
				allNodes->at(i)->SetProperty("color",mitk::ColorProperty::New(255.0/255.0, 255.0/255.0, 255.0/255.0));
			}
		}
	}

	// Some surface properties
	unsigned int id = 1;
	mitk::Mapper::Pointer newMapper = NULL;
	newMapper = mitk::SurfaceGLMapper2D::New();
	m_SurfaceNode->SetMapper(id, newMapper);
	m_SurfaceNode->SetProperty("color",mitk::ColorProperty::New(1.0/255.0, 254.0/255.0, 1.0/255.0));
	m_SurfaceNode->SetProperty("back color",mitk::ColorProperty::New(1.0/255.0, 1.0/255.0, 254.0/255.0));
	//m_SurfaceNode->SetProperty("draw normals 2d", true); 
	//m_SurfaceNode->SetBoolProperty("draw normals 2d", true);
	m_SurfaceNode->SetBoolProperty("draw normals 2d", false);
	//m_SurfaceNode->SetBoolProperty("scalar visibility", true);
	m_SurfaceNode->SetBoolProperty("show points", true);
	m_SurfaceNode->SetProperty("scalar visibility", mitk::BoolProperty::New(true));
    m_SurfaceNode->SetProperty("color mode", mitk::BoolProperty::New(true));
	m_SurfaceNode->AddProperty( "line width", IntProperty::New(2));
    //m_SurfaceNode->AddProperty( "scalar mode", VtkScalarModeProperty::New());
	//m_SurfaceNode->AddProperty( "draw normals 2D", BoolProperty::New(true));
	m_SurfaceNode->AddProperty( "draw normals 2D", BoolProperty::New(false));
	m_SurfaceNode->AddProperty( "invert normals", BoolProperty::New(false));
	m_SurfaceNode->AddProperty( "front color", ColorProperty::New(0.0, 1.0, 0.0));
	m_SurfaceNode->AddProperty( "back color", ColorProperty::New(1.0, 0.0, 0.0));
	m_SurfaceNode->AddProperty( "front normal lenth (px)", FloatProperty::New(10.0));
	m_SurfaceNode->AddProperty( "back normal lenth (px)", FloatProperty::New(10.0));
	m_SurfaceNode->AddProperty( "layer", mitk::IntProperty::New(100));

	bool jmpTpPos;
	if(m_SurfaceNode->GetBoolProperty("JumpToPosition", jmpTpPos))
	{
		m_JumpToPosition = jmpTpPos;
	}
	else
	{
		m_SurfaceNode->SetBoolProperty("JumpToPosition", m_JumpToPosition);
	}

	float sphereRadius;
	if(m_SurfaceNode->GetFloatProperty("SphereRadius", sphereRadius))
	{
		m_Radius = sphereRadius;
	}
	else
	{
		m_SurfaceNode->SetFloatProperty("SphereRadius", m_Radius);
	}

	m_Ball = mitk::Surface::New();
	m_BallNode = mitk::DataNode::New();
	m_BallNode->SetName("IntersectionOutput 2D");
	m_BallNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	// Set ball visible in renderer 1 to 3 and visible=false in renderer 4
	m_BallNode->SetVisibility(false, m_MitkView->GetRenderWindow4()->GetRenderer());
	m_BallNode->SetData(m_Ball);
	// Add unique identifier for data nodes
	newId = GetUniqueId();
	m_BallNode->SetIntProperty("UniqueID", newId);
	m_DataStorage->Add(m_BallNode, m_DataStorage->GetNamedNode("peternode"));

	m_IntersectionData = mitk::Surface::New();
	m_IntersectionNode = mitk::DataNode::New();
	m_IntersectionNode->SetData(m_IntersectionData);
	m_IntersectionNode->SetName("IntersectionOutput 3D");
	m_IntersectionNode->SetProperty( "outline width", mitk::FloatProperty::New( 3.0 ) );
	m_IntersectionNode->SetProperty( "material.wireframeLineWidth", mitk::FloatProperty::New( 3.0 ) );
	m_IntersectionNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	// Add unique identifier for data nodes
	newId = GetUniqueId();
	m_IntersectionNode->SetIntProperty("UniqueID", newId);
	m_DataStorage->Add(m_IntersectionNode, m_DataStorage->GetNamedNode("peternode"));

	m_IntersectionPolyDataFilter = vtkSmartPointer<vtkIntersectionPolyDataFilter>::New();
	m_IntersectionPolyDataFilter->SetInput(0, m_Surface->GetVtkPolyData());

	m_ContourSphere = vtkSphereSource::New();
	m_ContourSphere->SetThetaResolution(m_ContourResolution);
	m_ContourSphere->SetPhiResolution(m_ContourResolution);
	m_Ball->SetVtkPolyData(m_ContourSphere->GetOutput());

	m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
	m_IntersectionPolyDataFilter->Update();
	m_IntersectionData->SetVtkPolyData(m_IntersectionPolyDataFilter->GetOutput());

	SetImagesToBottom();
	
	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->AddListener(m_ChangeRadius);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::DeactivateTool()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_DataStorage->Remove(m_PositionTrackerNode);
	m_DataStorage->Remove(m_BallNode);
	m_DataStorage->Remove(m_IntersectionNode);

	// Save Radius to m_Surface
	m_SurfaceNode->SetFloatProperty("SphereRadius", m_Radius);
	m_SurfaceNode->SetBoolProperty("JumpToPosition", m_JumpToPosition);

	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->RemoveListener(m_ChangeRadius);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool mitk::ToolExtension::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	bool ok = false;
	const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

	mitk::DataNode::Pointer  dtnode;
	if (displayPositionEvent == NULL)
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
	if(m_JumpToPosition)
	{
		m_MitkView->MoveCrossToPosition(worldPoint);  
	} 


	m_ContourSphere->SetRadius(m_Radius);
	m_ContourSphere->SetCenter(worldPoint[0], worldPoint[1], worldPoint[2]);
	m_ContourSphere->Update();

	if(m_CalcIntersection)
	{
		// Calculate intersection
		m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
		m_IntersectionPolyDataFilter->Update();

		//UpdateRenderer();
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return ok;
}

void mitk::ToolExtension::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Save old jmpToPos & old radius  
	m_SurfaceNode->SetBoolProperty("JumpToPosition", m_JumpToPosition);
	m_SurfaceNode->SetFloatProperty("SphereRadius", m_Radius);

	// Get new surface
	if(m_DataStorage->Exists(m_DataStorage->GetNamedNode(m_CurrentSurfaces[selection])))
	{
		m_SurfaceNode = m_DataStorage->GetNamedNode(m_CurrentSurfaces[selection]);
		m_Surface = (mitk::Surface*)m_DataStorage->GetNamedNode(m_CurrentSurfaces[selection])->GetData();
	}
	else
	{
		std::cout << "Error: Selected surface does not exist anymore!\n";
	}

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

	// Reinit view to selected surface
	if(m_ReinitSelection)
	{
		// Get Surrounding
		const char* propertyKey = "Properties"; 
		pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
		int surrounding =  properties->GetReinitSurrounding();

		mitk::BaseData::Pointer basedata = m_SurfaceNode->GetData();
		if (basedata.IsNotNull())
		{
			mitk::Geometry3D::Pointer geo = basedata->GetTimeSlicedGeometry();
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
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}

	// Get Jump to Position Property
	bool jmpTpPos;
	if(m_SurfaceNode->GetBoolProperty("JumpToPosition", jmpTpPos))
	{
		m_JumpToPosition = jmpTpPos;
	}
	else
	{
		m_SurfaceNode->SetBoolProperty("JumpToPosition", m_JumpToPosition);
	}

	float sphereRadius;
	if(m_SurfaceNode->GetFloatProperty("SphereRadius", sphereRadius))
	{
		m_Radius = sphereRadius;
		std::cout << "Old radius found: " << sphereRadius << "\n";
	}
	else
	{
		m_SurfaceNode->SetFloatProperty("SphereRadius", m_Radius);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::SelectSurfaceViaMouse()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "SelectSurfaceViaMouse\n";

	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_DataStorage->GetAll();
	for(int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0) // Surfaces only
		{
			// create interactor
			m_SelectViaMouseInteractor = mitk::SelectSurfaceWithMouseInteractor::New("SelectAndMoveObjectWithArrowKeys",allNodes->at(i), this);

			//activate interactor at interaction controller:
			mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SelectViaMouseInteractor);
			m_CurrentIteractors.push_back(m_SelectViaMouseInteractor);
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	UpdateSelectedSurface.Send(selection);
	for(std::vector<mitk::SelectSurfaceWithMouseInteractor::Pointer>::iterator it = m_CurrentIteractors.begin(); it != m_CurrentIteractors.end(); it++)
	{
		mitk::GlobalInteraction::GetInstance()->RemoveInteractor(*it);
	}
	std::cout << "Deactivated!\n";
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	if(m_ReinitSelection)
	{
		std::cout << "ReinitSelection: " << m_ReinitSelection << "\n";		
		m_ReinitSelection = false;
	}
	else
	{
		std::cout << "ReinitSelection: " << m_ReinitSelection << "\n";
		m_ReinitSelection = true;
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	if(m_JumpToPosition)
	{
		std::cout << "JumpToPosition: " << m_JumpToPosition << "\n";		
		m_SurfaceNode->SetBoolProperty("JumpToPosition", false);		
		m_JumpToPosition = false;
	}
	else
	{
		std::cout << "JumpToPosition: " << m_JumpToPosition << "\n";
		m_SurfaceNode->SetBoolProperty("JumpToPosition", true);
		m_JumpToPosition = true;
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	//std::cout << "RadiusChanged in TOOL: " << value << "\n";
	m_Radius = value / m_RadiusFactor + 0.1;
	std::cout << "RadiusChanged in TOOL (m_Radius): " << m_Radius << "\n";

	m_ContourSphere->SetRadius(m_Radius);
	m_ContourSphere->Update();

	// Calculate intersection
	m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
	m_IntersectionPolyDataFilter->Update();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool mitk::ToolExtension::UniquePoint(double* first, double* second)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool tmp = false;
	if(first[0] == second[0] && first[1] == second[1] && first[2] == second[2])
	{
		tmp = true;
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return tmp;
}

int mitk::ToolExtension::GetUniqueId()
 {
	// Search for free index
	std::list<int> ids;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_DataStorage->GetAll();
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

void mitk::ToolExtension::SetImagesToBottom()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//mitk::DataNode::Pointer surfaceNode = m_DataStorage->GetNamedNode("GoldStandard");
	//if(m_DataStorage->Exists(surfaceNode))
	//{
	//	std::cout << "GoldStandard found.\n";
	//	m_DataStorage->Remove(surfaceNode);
	//	m_DataStorage->Add(surfaceNode);
	//}

	
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_DataStorage->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
		{
			// Image found?
			//std::cout << "Reference Image found? : " << allNodes->at(i)->GetName() << "\n";

			// Remove image from dataStorage
			m_DataStorage->Remove(allNodes->at(i));
			// "Re-"Add image to dataStorage
			m_DataStorage->Add(allNodes->at(i));
		}
	}
	// If OriginalThickening available -> set to bottom
	mitk::DataNode::Pointer node1 = m_DataStorage->GetNamedNode("Scenario_1_ReferenceImage");
	if(m_DataStorage->Exists(node1))
	{
		std::cout << "Scenario_1_ReferenceImage found.\n";
		m_DataStorage->Remove(node1);
		m_DataStorage->Add(node1);
	}
	mitk::DataNode::Pointer node2 = m_DataStorage->GetNamedNode("Scenario_2_ReferenceImage");
	if(m_DataStorage->Exists(node2))
	{
		std::cout << "Scenario_2_ReferenceImage found.\n";
		m_DataStorage->Remove(node2);
		m_DataStorage->Add(node2);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::UpdateRenderer()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	// FIXME
	if(0)
	{
		bool up = false;
		if(!m_SurfaceNode->GetBoolProperty("OpacityCountUp", up))
			m_SurfaceNode->SetBoolProperty("OpacityCountUp", false);

		float opacity = 0.0;
		m_SurfaceNode->GetFloatProperty("opacity", opacity);
		if(opacity == 1.0)
			up = false;
		if(opacity == 0.0)
			up = true;
		
		//up = true;

		if(up)
		{
			opacity += 0.000001;
			m_SurfaceNode->SetFloatProperty("opacity", opacity);
			mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
			m_SurfaceNode->SetBoolProperty("OpacityCountUp", false);
		}
		else
		{
			opacity -= 0.000001;
			m_SurfaceNode->SetFloatProperty("opacity", opacity);
			mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
			m_SurfaceNode->SetBoolProperty("OpacityCountUp", true);
		}
	}

	// old
	if(1)
	{
		float opacity = 0.0;
		m_SurfaceNode->GetFloatProperty("opacity", opacity);
		
		opacity -= 0.000001;
		m_SurfaceNode->SetFloatProperty("opacity", opacity);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		opacity += 0.000001;
		m_SurfaceNode->SetFloatProperty("opacity", opacity);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		if(opacity > 0.99)
		{
			m_SurfaceNode->SetFloatProperty("opacity", 1.0);
			mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
		}

		if(opacity < 0.01)
		{
			m_SurfaceNode->SetFloatProperty("opacity", 0.0);
			mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
		}
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::Reinit()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Get Surrounding
	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	int surrounding =  properties->GetReinitSurrounding();

	mitk::BaseData::Pointer basedata = m_SurfaceNode->GetData();
	if (basedata.IsNotNull())
	{
		mitk::Geometry3D::Pointer geo = basedata->GetTimeSlicedGeometry();
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
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::UpdateRadiusViaInteractor(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	//std::cout << "Value: " << value << "\n";

	if(m_RadiusInteractorOn)
	{
		double radiusAddend = 0.1;

		if(value > 0)
		{
			m_Radius += m_Radius/10.0;
		}
		if(value < 0)
		{
			m_Radius -= m_Radius/10.0;
		}

		m_ContourSphere->SetRadius(m_Radius);
		m_ContourSphere->Update();

		// Calculate intersection
		m_IntersectionPolyDataFilter->SetInputConnection( 1, m_ContourSphere->GetOutputPort() );
		m_IntersectionPolyDataFilter->Update();

		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		// change radius slider
		UpdateRadiusSlider(m_Radius);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::ToolExtension::SetRadiusInteractorOn()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RadiusInteractorOn = true;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
void mitk::ToolExtension::SetRadiusInteractorOff()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RadiusInteractorOn = false;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

vtkSmartPointer<vtkPolyData> mitk::ToolExtension::CreateTestContour()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
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
		vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
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

	//double pt1[3], pt2[3], pt3[3], pt4[3];
	//pt1[0] = 0.0;pt1[2] = 0.0;pt1[3] = 0.0;
	//pt2[0] = 1.0;pt2[2] = 0.0;pt2[3] = 0.0;
	//pt3[0] = 1.0;pt3[2] = 1.0;pt3[3] = 0.0;
	//pt4[0] = 100.0;pt4[2] = 100.0;pt4[3] = 100.0;
	//vtkIdType t1, t2, t3, t4;
	//t1 = points->InsertNextPoint(pt1);
	//t2 = points->InsertNextPoint(pt2);
	//t3 = points->InsertNextPoint(pt3);
	//t4 = points->InsertNextPoint(pt4);
	//newSurface->SetPoints(points);			
	//vtkIdList* pts4 = vtkIdList::New();
	//vtkIdList* pts5 = vtkIdList::New();
	//pts4->InsertNextId(t1);
	//pts4->InsertNextId(t2);
	//pts4->InsertNextId(t3);
	//pts5->InsertNextId(t2);
	//pts5->InsertNextId(t3);
	//pts5->InsertNextId(t4);
	////vtkIdType 	InsertNextPoint (const float x[3])
	////vtkIdType 	InsertNextPoint (const double x[3])
	////vtkIdType 	InsertNextPoint (double x, double y, double z)
	//newSurface->InsertNextCell(VTK_TRIANGLE, pts4);
	//newSurface->InsertNextCell(VTK_TRIANGLE, pts5);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return linesPolyData;
}