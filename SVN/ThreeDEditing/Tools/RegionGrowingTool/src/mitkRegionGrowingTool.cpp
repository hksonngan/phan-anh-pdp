// "pdp"
#include "mitkRegionGrowingTool.h"
#include "mitkRegionGrowingTool.xpm"
#include "QmitkRegionGrowingToolGUI.h"

// MITK
#include "mitkToolManager.h"

// VTK

#include "itkVTKPolyDataWriter.h"


mitk::RegionGrowingTool::RegionGrowingTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_Ut = new pdp::Utilities(m_Ext);
}

mitk::RegionGrowingTool::~RegionGrowingTool()
{}

const char** mitk::RegionGrowingTool::GetXPM() const
{
	return mitkRegionGrowingTool_xpm;
}

const char* mitk::RegionGrowingTool::GetName() const
{
	return "Region Growing Tool";
}

const char* mitk::RegionGrowingTool::GetGroup() const
{
	return "default";
}

void mitk::RegionGrowingTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::RegionGrowingTool, int>(this, &mitk::RegionGrowingTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::RegionGrowingTool, int>(this, &mitk::RegionGrowingTool::UpdateRadiusSlider);
}

void mitk::RegionGrowingTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::RegionGrowingTool, int>(this, &mitk::RegionGrowingTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::RegionGrowingTool, int>(this, &mitk::RegionGrowingTool::UpdateRadiusSlider);
}

void mitk::RegionGrowingTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::RegionGrowingTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	
	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::RegionGrowingTool::DetectHoles()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegionGrowingTool::RegionGrowings()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegionGrowingTool::RegionGrowingsVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}