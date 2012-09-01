// "pdp"
#include "mitkRegistrationTool.h"
#include "mitkRegistrationTool.xpm"
#include <../../../../RenderingManagerProperty.h>

// MITK
#include "mitkToolManager.h"
#include "mitkPointSet.h"
#include <mitkStateEvent.h>

#include <mitkMoveSurfaceInteractor.h>
#include <mitkGlobalInteraction.h>
#include "mitkInteractionConst.h"

#include "mitkDisplayCoordinateOperation.h"

// VTK
#include <vtkClipPolyData.h>
#include "vtkPlane.h" 
#include <vtkSphere.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkCleanPolyData.h>

#include <vtkThreshold.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkSelectionNode.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>

#include <vtkShrinkPolyData.h>

#include <mitkSurfaceOperation.h>
#include <mitkOperationEvent.h>
#include <mitkDiffImageApplier.h>
#include <mitkUndoController.h>

#include <mitkRotationOperation.h>
#include <mitkPointOperation.h>

#include "vtkTransform.h"

mitk::RegistrationTool::RegistrationTool()
:Tool("MySelectAndMoveObjectWithArrowKeys")
{
	// great magic numbers
	CONNECT_ACTION( 92, Move );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();
	m_Ext->SetCalcIntersection(false);	
	m_MoveSurfaceSpeed = 1;

	this->EnableUndo(true);
	//this->EnableUndo(false);
	
	m_AutoReinit = false;
	m_Factor = 1.0;
	m_LastTool = -1;

	mitk::UndoModel* model = m_UndoController->GetCurrentUndoModel();
	//model->Print(std::cout);
}

mitk::RegistrationTool::~RegistrationTool()
{}

const char** mitk::RegistrationTool::GetXPM() const
{
	return mitkRegistrationTool_xpm;
}

const char* mitk::RegistrationTool::GetName() const
{
	return "Registration Tool";
}

const char* mitk::RegistrationTool::GetGroup() const
{
	return "default";
}

void mitk::RegistrationTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));

	m_Ext->GetMitkView()->activateWindow();

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::RegistrationTool, int>(this, &mitk::RegistrationTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::RegistrationTool, int>(this, &mitk::RegistrationTool::UpdateRadiusSlider);

	m_LastTool = -1;
}

void mitk::RegistrationTool::Deactivated()
{
	std::cout << "Deactivated.\n";

	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";


	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::RegistrationTool, int>(this, &mitk::RegistrationTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::RegistrationTool, int>(this, &mitk::RegistrationTool::UpdateRadiusSlider);
	Superclass::Deactivated();

	std::cout << "Deactivated 2.\n";
}

void mitk::RegistrationTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}
			
void mitk::RegistrationTool::MoveSurfaceSpeed(int speed)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Speed: " << speed << std::endl;
	m_MoveSurfaceSpeed = speed / 20.0;
	std::cout << "Speed: " << m_MoveSurfaceSpeed << std::endl;

	// todo avtivate or set focused window 4. interactor should always work
	//m_Ext->GetMitkView()->activateWindow();
	//m_Ext->GetMitkView()->


	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

//void mitk::RegistrationTool::ShrinkSurface(int size)
//{
//	std::cout << "hi from " << __FUNCSIG__ << std::endl;
//
//	std::cout << "Size: " << size << std::endl;
//
//	vtkSmartPointer<vtkShrinkPolyData> shrinkFilter =
//	vtkSmartPointer<vtkShrinkPolyData>::New();
//	//shrinkFilter->SetInputConnection(sphereSource->GetOutputPort());
//	shrinkFilter->SetInput(m_Ext->GetSurfacePolyData());
//	
//	shrinkFilter->Update();
//
//	m_Ext->SetSurfacePolyData(shrinkFilter->GetOutput());
//
//	//update rendering
//	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//
//	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
//}

void mitk::RegistrationTool::ScaleChangedByCross(int scale)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	if(scale == 0)
	{
		return;
	}
	else if(scale > 0)
	{
		m_Factor = 1.01;
	
	}
	else
	{
		m_Factor = 0.99;
	}
	//std::cout << "Scale factor: " << m_Factor << "\n"; 

	mitk::Geometry3D::Pointer geometry = m_Ext->GetSurface()->GetGeometry();
	// from mitk::Geometry3D:
	/* calculate new scale: newscale = oldscale * (oldscale + scaletoadd)/oldscale */
	mitk::Point3D newScale;
	newScale.Fill(m_Factor);
	mitk::Point3D newScale2;
	newScale2[0] = (newScale[0] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(0).magnitude());
	newScale2[1] = (newScale[1] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(1).magnitude());
	newScale2[2] = (newScale[2] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(2).magnitude());

	mitk::Point3D reverseScale;
	reverseScale[0] = -newScale2[0];
	reverseScale[1] = -newScale2[1];
	reverseScale[2] = -newScale2[2];

	m_CenterBeforeScale = m_Ext->GetSurface()->GetGeometry()->GetCenter();
		
	/* generate Operation and send it to the receiving geometry */
	PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale2, 0); // Index is not used here

	if (m_UndoEnabled)  //write to UndoMechanism
	{
		mitk::Point3D oldScaleData;
		oldScaleData[0] = -newScale[0];
		oldScaleData[1] = -newScale[1];
		oldScaleData[2] = -newScale[2];

		PointOperation* undoOp = new mitk::PointOperation(OpSCALE, reverseScale/*oldScaleData*/, 0);
		OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp, "Scale surface");
		m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 0)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 0.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 0;
		}
	}
	/* execute the Operation */
	geometry->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	//update rendering
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();

	// translate to keep object at place
	mitk::Point3D forwardPoint;
	mitk::Point3D reversePoint;

	m_CenterAfterScale = m_Ext->GetSurface()->GetGeometry()->GetCenter();

	forwardPoint[0] = m_CenterBeforeScale[0] - m_CenterAfterScale[0];
	forwardPoint[1] = m_CenterBeforeScale[1] - m_CenterAfterScale[1];
	forwardPoint[2] = m_CenterBeforeScale[2] - m_CenterAfterScale[2];

	reversePoint[0] = -forwardPoint[0];
	reversePoint[1] = -forwardPoint[1];
	reversePoint[2] = -forwardPoint[2];

	if(0) // print
	{
		std::cout << "Scale: " << newScale[0] << ", " << newScale[1] << ", " << newScale[2] << "\n";
		std::cout << "Translation: " << forwardPoint[0] << ", " << forwardPoint[1] << ", " << forwardPoint[2] << "\n";
		std::cout << "Center of Surface: " << m_Ext->GetSurface()->GetGeometry()->GetCenter() << "\n";
	}

	mitk::PointOperation* doOp2 = new mitk::PointOperation(OpMOVE, forwardPoint);
	if ( m_UndoEnabled )
	{
		mitk::PointOperation* undoOp2 = new mitk::PointOperation(OpMOVE, reversePoint);
		OperationEvent *operationEvent2 = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp2, undoOp2, "Back transformation (translate after scale)");
		m_UndoController->SetOperationEvent(operationEvent2);
	}
	//execute the Operation
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp2);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	//update rendering
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossRotateX(int x)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0)
	{
		return;
	}

	mitk::Point3D newCenter;
	newCenter = m_Ext->GetSurface()->GetGeometry()->GetCenter();
	mitk::Vector3D rotationAxis;
	rotationAxis[0] = 0.0; rotationAxis[1] = 0.0; rotationAxis[2] = 20.0;
	//(sagittal, coronal, transversal)
	mitk::RotationOperation *doOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, x);

	if(m_UndoEnabled)
	{
		mitk::RotationOperation* undoOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, -x);
	    OperationEvent *operationEvent = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Rotate Transversal (X)");
	    m_UndoController->SetOperationEvent(operationEvent);
		
		if(m_LastTool != 1)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 1.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 1;
		}
	}
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossRotateY(int x)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0)
	{
		return;
	}

	mitk::Point3D newCenter;
	newCenter = m_Ext->GetSurface()->GetGeometry()->GetCenter();
	mitk::Vector3D rotationAxis;
	rotationAxis[0] = 20.0; rotationAxis[1] = 0.0; rotationAxis[2] = 0.0;
	//(sagittal, coronal, transversal)
	mitk::RotationOperation *doOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, -x);

	if(m_UndoEnabled)
	{
		mitk::RotationOperation* undoOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, x);
	    OperationEvent *operationEvent = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Rotate Sagittal (Y)");
	    m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 2)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 2.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 2;
		}
	}
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossRotateZ(int x)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0)
	{
		return;
	}

	mitk::Point3D newCenter;
	newCenter = m_Ext->GetSurface()->GetGeometry()->GetCenter();
	mitk::Vector3D rotationAxis;
	rotationAxis[0] = 0.0; rotationAxis[1] = 20.0; rotationAxis[2] = 0.0;
	//(sagittal, coronal, transversal)
	mitk::RotationOperation *doOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, x);

	if(m_UndoEnabled)
	{
		mitk::RotationOperation* undoOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, -x);
	    OperationEvent *operationEvent = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Rotate Coronal (Z)");
	    m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 3)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 3.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 3;
		}
	}
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossTranslateTransversal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0 && y == 0)
	{
		return;
	}

	// Undo / Redo controller
	mitk::Point3D forwardPoint;
	mitk::Point3D reversePoint;

	forwardPoint[0] = (double)x / 10.0;
	forwardPoint[1] = (double)y / 10.0;
	forwardPoint[2] = 0.0;

	reversePoint[0] = -forwardPoint[0];
	reversePoint[1] = -forwardPoint[1];
	reversePoint[2] = -forwardPoint[2];

	mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, forwardPoint);
	if ( m_UndoEnabled )
	{
		mitk::PointOperation* undoOp = new mitk::PointOperation(OpMOVE, reversePoint);
	    OperationEvent *operationEvent = new OperationEvent( m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Move surface transversal");
	    m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 4)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 4.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 4;
		}
	}
	//execute the Operation
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossTranslateSagittal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0 && y == 0)
	{
		return;
	}

	// Undo / Redo controller
	mitk::Point3D forwardPoint;
	mitk::Point3D reversePoint;

	forwardPoint[0] = 0.0;
	forwardPoint[1] = (double)x / 10.0;
	forwardPoint[2] = -(double)y / 10.0;

	reversePoint[0] = -forwardPoint[0];
	reversePoint[1] = -forwardPoint[1];
	reversePoint[2] = -forwardPoint[2];

	mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, forwardPoint);
	if ( m_UndoEnabled )
	{
		mitk::PointOperation* undoOp = new mitk::PointOperation(OpMOVE, reversePoint);
	    OperationEvent *operationEvent = new OperationEvent( m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Move surface sagittal");
	    m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 5)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 5.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 5;
		}
	}
	//execute the Operation
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::CrossTranslateCoronal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(x == 0 && y == 0)
	{
		return;
	}

	// Undo / Redo controller
	mitk::Point3D forwardPoint;
	mitk::Point3D reversePoint;

	forwardPoint[0] = (double)x / 10.0;
	forwardPoint[1] = 0.0;
	forwardPoint[2] = -(double)y / 10.0;

	reversePoint[0] = -forwardPoint[0];
	reversePoint[1] = -forwardPoint[1];
	reversePoint[2] = -forwardPoint[2];

	mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, forwardPoint);
	if ( m_UndoEnabled )
	{
		mitk::PointOperation* undoOp = new mitk::PointOperation(OpMOVE, reversePoint);
	    OperationEvent *operationEvent = new OperationEvent( m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Move surface coronal");
	    m_UndoController->SetOperationEvent(operationEvent);

		if(m_LastTool != 6)
		{
			std::cout << "Change last tool from: " << m_LastTool << ", to: 6.\n"; 
			operationEvent->IncCurrObjectEventId();
			operationEvent->IncCurrGroupEventId();
			m_LastTool = 6;
		}
	}
	//execute the Operation
	m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::RegistrationTool::ReinitAutomatical()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_AutoReinit)
	{
		//std::cout << "Auto reinit = false.\n";
		m_AutoReinit = false;
	}
	else
	{
		//std::cout << "Auto reinit = true.\n";
		m_AutoReinit = true;
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
void mitk::RegistrationTool::ReinitNow()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->Reinit();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool mitk::RegistrationTool::Move(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	if(0)
	{
		// Try undo redo funktionality
		// rotate
		if (m_UndoEnabled)  //write to UndoMechanism
		{
			std::cout << "m_UndoEnabled == true\n";
		}

		mitk::Point3D newCenter;
		newCenter = m_Ext->GetSurface()->GetGeometry()->GetCenter();
		mitk::Vector3D rotationAxis;
		rotationAxis[0] = 0.0; rotationAxis[1] = 20.0; rotationAxis[2] = 0.0;
		mitk::RotationOperation *doOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, 90.0);

		if(m_UndoEnabled)
		{
			mitk::RotationOperation* undoOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, -90.0);
		    OperationEvent *operationEvent = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Rotate");
		    m_UndoController->SetOperationEvent(operationEvent);
		}
		m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
		m_Ext->GetSurface()->Update();
		m_Ext->GetSurfaceNode()->Modified();
	}

	if(0)
	{
		// move points
		if (m_UndoEnabled)  //write to UndoMechanism
		{
			std::cout << "m_UndoEnabled == true\n";
		}
		// Point movement direction
		mitk::Point3D forwardPoint;
		mitk::Point3D reversePoint;
		forwardPoint.Fill(2);
		reversePoint.Fill(-2);
		mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, forwardPoint);
		if ( m_UndoEnabled )
		{
			mitk::PointOperation* undoOp = new mitk::PointOperation(OpMOVE, reversePoint);
		    OperationEvent *operationEvent = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Move point");
		    m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
		m_Ext->GetSurface()->Update();
		m_Ext->GetSurfaceNode()->Modified();
	}

	if(0)
	{
		// scale

		mitk::Geometry3D::Pointer geometry = m_Ext->GetSurface()->GetGeometry();
		// from mitk::Geometry3D:
		/* calculate new scale: newscale = oldscale * (oldscale + scaletoadd)/oldscale */
		mitk::Point3D newScale;
		newScale.Fill(m_Factor);
		mitk::Point3D newScale2;
		newScale2[0] = (newScale[0] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(0).magnitude());
		newScale2[1] = (newScale[1] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(1).magnitude());
		newScale2[2] = (newScale[2] - 1) * (m_Ext->GetSurface()->GetGeometry()->GetMatrixColumn(2).magnitude());

		// data[2] = 1 + (newScale[2] / GetMatrixColumn(2).magnitude());

		// check if we scale positive or negative
		if(0)
		{
			// projection onto the axis
			/* convert movement to local object coordinate system and mirror it to the positive quadrant */

			//Vector3D start;
			//Vector3D end;
			//mitk::ScalarType convert[3];
			//itk2vtk(m_LastMousePosition, convert);
			//geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform start point to local object coordinates
			//start[0] = fabs(convert[0]);  start[1] = fabs(convert[1]);  start[2] = fabs(convert[2]);  // mirror it to the positive quadrant
			//itk2vtk(p, convert);
			//geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform end point to local object coordinates
			//end[0] = fabs(convert[0]);  end[1] = fabs(convert[1]);  end[2] = fabs(convert[2]);  // mirror it to the positive quadrant

			///* check if mouse movement is towards or away from the objects axes and adjust scale factors accordingly */
			//Vector3D vLocal = start - end;
			//newScale[0] = (vLocal[0] > 0.0) ? -fabs(newScale[0]) : +fabs(newScale[0]);
			//newScale[1] = (vLocal[1] > 0.0) ? -fabs(newScale[1]) : +fabs(newScale[1]);
			//newScale[2] = (vLocal[2] > 0.0) ? -fabs(newScale[2]) : +fabs(newScale[2]);
		}

		//m_LastMousePosition = p;  // update lastPosition for next mouse move

		m_CenterBeforeScale = m_Ext->GetSurface()->GetGeometry()->GetCenter();
			
		/* generate Operation and send it to the receiving geometry */
		//PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0); // Index is not used here
		PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale2, 0); // Index is not used here

		if (m_UndoEnabled)  //write to UndoMechanism
		{
			mitk::Point3D oldScaleData;
			oldScaleData[0] = -newScale[0];
			oldScaleData[1] = -newScale[1];
			oldScaleData[2] = -newScale[2];

			PointOperation* undoOp = new mitk::PointOperation(OpSCALE, oldScaleData, 0);
			OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp, "Scale");
			m_UndoController->SetOperationEvent(operationEvent);
		}
		/* execute the Operation */
		geometry->ExecuteOperation(doOp);

		if(0)
		{
			// translate to keep object at place
			mitk::Point3D forwardPoint;
			mitk::Point3D reversePoint;

			m_CenterAfterScale = m_Ext->GetSurface()->GetGeometry()->GetCenter();

			forwardPoint[0] = m_CenterBeforeScale[0] - m_CenterAfterScale[0];
			forwardPoint[1] = m_CenterBeforeScale[1] - m_CenterAfterScale[1];
			forwardPoint[2] = m_CenterBeforeScale[2] - m_CenterAfterScale[2];

			reversePoint[0] = -forwardPoint[0];
			reversePoint[1] = -forwardPoint[1];
			reversePoint[2] = -forwardPoint[2];

			if(0) // print
			{
				std::cout << "Scale: " << newScale[0] << ", " << newScale[1] << ", " << newScale[2] << "\n";
				std::cout << "Translation: " << forwardPoint[0] << ", " << forwardPoint[1] << ", " << forwardPoint[2] << "\n";
				std::cout << "Center of Surface: " << m_Ext->GetSurface()->GetGeometry()->GetCenter() << "\n";
			}

			mitk::PointOperation* doOp2 = new mitk::PointOperation(OpMOVE, forwardPoint);
			if ( m_UndoEnabled )
			{
				mitk::PointOperation* undoOp2 = new mitk::PointOperation(OpMOVE, reversePoint);
				OperationEvent *operationEvent2 = new OperationEvent(m_Ext->GetSurface()->GetGeometry(), doOp2, undoOp2, "Move point");
				m_UndoController->SetOperationEvent(operationEvent2);
			}
			//execute the Operation
			m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp2);
			m_Ext->GetSurface()->Update();
			m_Ext->GetSurfaceNode()->Modified();
		}
	}

	if(0)
	{
		// surface operation
		/*mitk::Point3D newCenter;
		newCenter[0] = 0.0; newCenter[1] = 0.0; newCenter[2] = 0.0;
		mitk::Vector3D rotationAxis;
		rotationAxis[0] = 0.0; rotationAxis[1] = 20.0; rotationAxis[2] = 0.0;
		mitk::RotationOperation *doOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, 90.0);
		mitk::RotationOperation *undoOp = new mitk::RotationOperation(mitk::OpROTATE, newCenter, rotationAxis, -90.0);*/

		mitk::Surface::Pointer oldSurface = m_Ext->GetSurface()->Clone();

		// move a point of the old surface
		vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
		double* center = m_Ext->GetBall()->GetVtkPolyData()->GetCenter();
		double translateFactor;
		for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
		{
			double pt[3];
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt);
			
			translateFactor = 0.5;
			pt[0] = (translateFactor  + center[0]);
			pt[1] = (translateFactor  + center[1]);
			pt[2] = (translateFactor  + center[2]);
			m_Ext->GetSurfacePolyData()->GetPoints()->SetPoint(ptId, pt);
		}
		m_Ext->UpdateRenderer();

		mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* undoStackItem = new OperationEvent( m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Surface changed") ;
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );
		m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
		delete doOp;
		delete undoOp;
		m_Ext->GetSurface()->Update();
		m_Ext->GetSurfaceNode()->Modified();
	}

	if(1)
	{
		//modify Geometry from data as given in parameters or in event
		mitk::IntProperty* xP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_X"));
		mitk::IntProperty* yP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_Y"));
		mitk::IntProperty* zP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_Z"));
		if (xP == NULL || yP == NULL || zP == NULL)
		{
			MITK_WARN<<"No properties returned\n!";
			return false;
		}
		mitk::Vector3D movementVector;
		movementVector.SetElement(0, (float) xP->GetValue() * m_MoveSurfaceSpeed);
		movementVector.SetElement(1, (float) yP->GetValue() * m_MoveSurfaceSpeed);
		movementVector.SetElement(2, (float) zP->GetValue() * m_MoveSurfaceSpeed);

		//std::cout << "Displacement Vector: " << (float) xP->GetValue() << " " << (float) yP->GetValue()  << " " <<  (float) zP->GetValue() << "\n";
		//std::cout << "Displacement Vector float: " << (float) xP->GetValue()* m_MoveSurfaceSpeed << " " << (float) yP->GetValue()* m_MoveSurfaceSpeed  << " " <<  (float) zP->GetValue()* m_MoveSurfaceSpeed << "\n";

		//checking corresponding Data; has to be a surface or a subclass
		mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_Ext->GetSurfaceNode()->GetData());
		if ( surface == NULL )
		{
			MITK_WARN<<"MoveSurfaceInteractor got wrong type of data! Aborting interaction!\n";
			return false;
		}
		//Geometry3D* geometry = surface->GetUpdatedTimeSlicedGeometry()->GetGeometry3D( m_TimeStep );
		//geometry->Translate(movementVector);

		
		// Undo / Redo controller
		mitk::Point3D forwardPoint;
		mitk::Point3D reversePoint;

		forwardPoint[0] = movementVector[0];
		forwardPoint[1] = movementVector[1];
		forwardPoint[2] = movementVector[2];

		reversePoint[0] = -movementVector[0];
		reversePoint[1] = -movementVector[1];
		reversePoint[2] = -movementVector[2];

		mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, forwardPoint);
		if ( m_UndoEnabled )
		{
			mitk::PointOperation* undoOp = new mitk::PointOperation(OpMOVE, reversePoint);
		    OperationEvent *operationEvent = new OperationEvent( m_Ext->GetSurface()->GetGeometry(), doOp, undoOp, "Move surface by keyboard");
		    m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		m_Ext->GetSurface()->GetGeometry()->ExecuteOperation(doOp);
		m_Ext->GetSurface()->Update();
		m_Ext->GetSurfaceNode()->Modified();

		//update rendering
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	// for IncCurrObjectEventId of other tools
	if(m_LastTool != 6)
	{
		std::cout << "Change last tool from: " << m_LastTool << ", to: 6.\n"; 
		m_LastTool = 6;
	}

	std::cout << "\n";

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	if(m_AutoReinit)
	{
		m_Ext->Reinit();
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return ok;
}