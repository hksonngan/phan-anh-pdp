// "pdp"
#include "mitkSmoothTool.h"
#include "mitkSmoothTool.xpm"
#include <../../../../RenderingManagerProperty.h>
#include "QmitkSmoothToolGUI.h"

// MITK
#include "mitkToolManager.h"
#include <mitkSurfaceOperation.h>
#include "mitkInteractionConst.h"
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>

// VTK
#include <vtkAppendPolyData.h>
#include <vtkClipPolyData.h>
#include <vtkSphere.h>
#include <vtkTriangleFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkDecimatePro.h>

mitk::SmoothTool::SmoothTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

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

	m_SmoothIteration = 5;
}

mitk::SmoothTool::~SmoothTool()
{}

const char** mitk::SmoothTool::GetXPM() const
{
	return mitkSmoothTool_xpm;
}

const char* mitk::SmoothTool::GetName() const
{
	return "Smooth Tool";
}

const char* mitk::SmoothTool::GetGroup() const
{
	return "default";
}

void mitk::SmoothTool::Activated()
{
	Superclass::Activated();
	std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::SmoothTool, int>(this, &mitk::SmoothTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::SmoothTool, int>(this, &mitk::SmoothTool::UpdateRadiusSlider);
}

void mitk::SmoothTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::SmoothTool, int>(this, &mitk::SmoothTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::SmoothTool, int>(this, &mitk::SmoothTool::UpdateRadiusSlider);
}

void mitk::SmoothTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::SmoothTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);

	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::SmoothVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// Smooth ROI: 1) Cut out VOI from Surface, 2) Smooth extraction with preserve edges on, 3) Append smoothed extraction to surface
	vtkClipPolyData *clip = vtkClipPolyData::New();
	clip->AddInput(m_Ext->GetSurfacePolyData());

	vtkSphere *sphere1 = vtkSphere::New();
	sphere1->SetRadius(m_Ext->GetContourSphere()->GetRadius());
	sphere1->SetCenter(m_Ext->GetContourSphere()->GetCenter());

	clip->SetClipFunction(sphere1);
	clip->GenerateClippedOutputOn();
	clip->Update();

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
	
	m_Ext->SetSurfacePolyData(app->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Smooth VOI");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::DecimateVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// Decimate ROI: 1) Cut out VOI from Surface, 2) Decimate extraction with preserve edges on, 3) Append extraction to surface
	vtkClipPolyData *clip = vtkClipPolyData::New();
	clip->AddInput(m_Ext->GetSurfacePolyData());

	vtkSphere *sphere1 = vtkSphere::New();
	sphere1->SetRadius(m_Ext->GetContourSphere()->GetRadius());
	sphere1->SetCenter(m_Ext->GetContourSphere()->GetCenter());

	clip->SetClipFunction(sphere1);
	clip->GenerateClippedOutputOn();
	clip->Update();

	// decimate
	vtkDecimatePro *decimate = vtkDecimatePro::New();
	decimate->SplittingOff();
	//decimate->SplittingOn();
	decimate->SetErrorIsAbsolute(5);
	decimate->SetFeatureAngle(30);
	decimate->PreserveTopologyOn();
	//decimate->PreserveTopologyOff();
	decimate->BoundaryVertexDeletionOff();
	//decimate->BoundaryVertexDeletionOn();
	decimate->SetDegree(10); //std-value is 25!

	//decimate->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());//RC++
	decimate->SetInput(clip->GetOutput(1));//RC++
	//decimate->SetTargetReduction(/*m_TargetReduction*/0.95f);
	decimate->SetTargetReduction(/*m_TargetReduction*/0.70f);
	//decimate->SetMaximumError(0.002);
	decimate->SetMaximumError(0.02);
	//polydata->Delete();//RC--

	// vtkAppendPolyData
	vtkAppendPolyData *app = vtkAppendPolyData::New();
	app->AddInput(clip->GetOutput(0));
	app->AddInput(decimate->GetOutput());
	app->Update();
	
	m_Ext->SetSurfacePolyData(app->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Decimate VOI");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::TriangulateVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// Triangulate ROI: 1) Cut out VOI from Surface, 2) Triangulate extraction with preserve edges on, 3) Append extraction to surface
	vtkClipPolyData *clip = vtkClipPolyData::New();
	clip->AddInput(m_Ext->GetSurfacePolyData());

	vtkSphere *sphere1 = vtkSphere::New();
	sphere1->SetRadius(m_Ext->GetContourSphere()->GetRadius());
	sphere1->SetCenter(m_Ext->GetContourSphere()->GetCenter());

	clip->SetClipFunction(sphere1);
	clip->GenerateClippedOutputOn();
	clip->Update();

	// Triangulate
	vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
    triangleFilter->SetInput(clip->GetOutput(1));
    triangleFilter->Update();

	// vtkAppendPolyData
	vtkAppendPolyData *app = vtkAppendPolyData::New();
	app->AddInput(clip->GetOutput(0));
	app->AddInput(triangleFilter->GetOutput());
	app->Update();
	
	m_Ext->SetSurfacePolyData(app->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Triangulate VOI");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::SmoothSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// Smooth
	vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
	//read poly1 (poly1 can be the original polygon, or the decimated polygon)
	smoother->SetInput(m_Ext->GetSurfacePolyData());
	smoother->SetNumberOfIterations(m_SmoothIteration);
	smoother->SetRelaxationFactor( /*m_SmoothRelaxation*/ 0.1);
	smoother->SetFeatureAngle( 60 );
	smoother->FeatureEdgeSmoothingOff();
	//smoother->BoundarySmoothingOff();
	smoother->BoundarySmoothingOn();
	smoother->SetConvergence( 0 );
	smoother->Update();

	m_Ext->SetSurfacePolyData(smoother->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Smooth surface");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::DecimateSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// decimate
	vtkDecimatePro *decimate = vtkDecimatePro::New();
	decimate->SplittingOff();
	//decimate->SplittingOn();
	decimate->SetErrorIsAbsolute(5);
	decimate->SetFeatureAngle(30);
	decimate->PreserveTopologyOn();
	//decimate->PreserveTopologyOff();
	decimate->BoundaryVertexDeletionOff();
	//decimate->BoundaryVertexDeletionOn();
	decimate->SetDegree(10); //std-value is 25!

	//decimate->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());//RC++
	decimate->SetInput(m_Ext->GetSurfacePolyData());//RC++
	//decimate->SetTargetReduction(/*m_TargetReduction*/0.95f);
	decimate->SetTargetReduction(/*m_TargetReduction*/0.70f);
	//decimate->SetMaximumError(0.002);
	decimate->SetMaximumError(0.02);
	//polydata->Delete();//RC--
	m_Ext->SetSurfacePolyData(decimate->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Decimate surface");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::TriangulateSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	// Triangulate
	vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
    triangleFilter->SetInput(m_Ext->GetSurfacePolyData());
    triangleFilter->Update();
    m_Ext->SetSurfacePolyData(triangleFilter->GetOutput());

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Triangulate surface");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SmoothTool::SmoothChanged(int value)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SmoothIteration = value;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}