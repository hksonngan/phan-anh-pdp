// "pdp"
#include "mitkBulgeTool.h"
#include "mitkBulgeTool.xpm"
#include <../../../../RenderingManagerProperty.h>

// MITK
#include "mitkToolManager.h"
#include "mitkPointSet.h"

#include <mitkSurfaceOperation.h>
#include <mitkOperationEvent.h>
#include <mitkSurface.h>
#include "mitkInteractionConst.h"
#include <mitkUndoController.h>
#include <mitkStateEvent.h>

// VTK
#include <vtkAppendPolyData.h>
#include <vtkClipPolyData.h>
#include <vtkSphere.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkCellLocator.h>
#include <vtkPolyDataNormals.h>


mitk::BulgeTool::BulgeTool()
//:Tool("PositionTracker")
:Tool("MyPositionTrackerWithLeftDrag")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );
	CONNECT_ACTION( 1003, OnMouseDrag );
	CONNECT_ACTION( 1004, OnMouseLeftClick );
	CONNECT_ACTION( 1005, OnMouseLeftClickRelease );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();
	
	m_SurfaceHadBeenHit = false;
	m_RadialDirection = false;
	m_ShowBall = true;
	m_SurfaceChanged = false;

	m_RefinementTechnique = 0;

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

	// umso groesser desto eher insert points und umso weniger by edge
	m_AutoInsertPointThreshold = 1.5;
	// compare length: 3 side length of the triangle to the radius*m_RadiusThresholdFactor
 	m_RadiusTresholdFactor = 0.5;
}

mitk::BulgeTool::~BulgeTool()
{}

const char** mitk::BulgeTool::GetXPM() const
{
	return mitkBulgeTool_xpm;
}

const char* mitk::BulgeTool::GetName() const
{
	return "Bulge Tool";
}

const char* mitk::BulgeTool::GetGroup() const
{
	return "default";
}

void mitk::BulgeTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::BulgeTool, int>(this, &mitk::BulgeTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::BulgeTool, int>(this, &mitk::BulgeTool::UpdateRadiusSlider);
}

void mitk::BulgeTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::BulgeTool, int>(this, &mitk::BulgeTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::BulgeTool, int>(this, &mitk::BulgeTool::UpdateRadiusSlider);
}

void mitk::BulgeTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::BulgeTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);

	return ok;
}

bool mitk::BulgeTool::OnMouseLeftClick(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	//m_Ext->SetCalcIntersection(false);
	return ok;
}

bool mitk::BulgeTool::OnMouseDrag(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	//bool ok = true;

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

	//m_Ext->GetIntersectionPolyDataFilter()->SetInput(0, m_Ext->GetSurfacePolyData());

	// set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);
	
	if(m_RadialDirection)
		MovePointsInRadialDirection();
	else
		MovePointsInMouseMovementDirection();	

	// set second snapshot 
	if(m_SurfaceChanged)
	{
		mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
		if ( m_UndoEnabled )
		{
			mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
			mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "Surface changed");
			mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
			operationEvent->IncCurrObjectEventId();
			//operationEvent->IncCurrGroupEventId();
		}
		m_Ext->GetSurface()->ExecuteOperation(doOp);

		m_Ext->GetSurface()->Update();
		m_Ext->GetSurfaceNode()->Modified();

		m_Ext->GetSurfaceNode()->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
		m_SurfaceChanged = false;
	}
	
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//QCoreApplication::processEvents();

	//std::cout << "Move ROI Mouse Moved\n";
	return ok;
}

bool mitk::BulgeTool::OnMouseLeftClickRelease(Action* action, const StateEvent* stateEvent)
{
	//m_Ext->SetCalcIntersection(true);
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	m_Ext->GetIntersectionPolyDataFilter()->SetInput(0, m_Ext->GetSurfacePolyData());
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	return ok;
}

void mitk::BulgeTool::MovePointsInMouseMovementDirection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Add Points to Mesh?
	if(m_RefinementTechnique != 0)
	{
		AddPoints();
	}

	// Create new vtk poly data that will substitutes the old
	vtkPolyData* newData = vtkPolyData::New();
	newData->DeepCopy(m_Ext->GetSurfacePolyData());

	// Use direction of the mouse movement
	vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
	
	// Get center of the ball interactor
	pdp::Pt center = m_Ext->GetBall()->GetVtkPolyData()->GetCenter();
	
	// Mouse Movement
	m_MouseMovementVector = center - m_OldVOICenterVector;

	// Mouse Movement with multiple vectors
	pdp::Pt oldVOICenterVectorSum(0.0);

	for(int i = 0; i < m_MouseMovementVectorSumLength; i++)
	{
		oldVOICenterVectorSum[0] += m_OldVOICenterVectors[0+i*3];
		oldVOICenterVectorSum[1] += m_OldVOICenterVectors[1+i*3];
		oldVOICenterVectorSum[2] += m_OldVOICenterVectors[2+i*3];
	}		
	m_MouseMovementVectorSum = center - oldVOICenterVectorSum / m_MouseMovementVectorSumLength;

	// Move Points
	for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		pdp::Pt pt;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());
				
		// Use distance			
		pdp::Pt centerToPointVector = pt - center;
		double centerToPointVectorLength = sqrt((centerToPointVector[0])*(centerToPointVector[0])+(centerToPointVector[1])*(centerToPointVector[1])+(centerToPointVector[2])*(centerToPointVector[2]));
		double radius = m_Ext->GetRadius();
		if(centerToPointVectorLength < radius)
		{

			// Calculate dot product between mouse movement vector and center to point vector. (Skalarprodukt)
			double dotProduct = m_MouseMovementVector[0]*centerToPointVector[0] + m_MouseMovementVector[1]*centerToPointVector[1] + m_MouseMovementVector[2]*centerToPointVector[2];
			//std::cout << "Dot Product: " << dotProduct << "\n";

			// Calculate length of Mouse Movement Vector
			double mouseMovementVectorLength = sqrt(m_MouseMovementVector[0]*m_MouseMovementVector[0]+m_MouseMovementVector[1]*m_MouseMovementVector[1]+m_MouseMovementVector[2]*m_MouseMovementVector[2]);

			// Calculate angle between mouse movement vector and center to point vector. (beta)
			double beta = asin(/*-*/(dotProduct / (centerToPointVectorLength*mouseMovementVectorLength)));   // FIXME: What about the minus? cos(a+90) = -sin(a) ???
			//std::cout << "Beta: " << beta << "\n";

			// Calculate angle between mouse movement vector and center to NEW point vector. (beta')
			double newBeta = acos((cos(beta) * centerToPointVectorLength) / radius);
			//std::cout << "Beta': " << newBeta << "\n";

			// Calculate translation length between old point and translated point.
			double oldPointToNewPointLength = (sin(newBeta) * radius) - (sin(beta) * centerToPointVectorLength);
			//test
			//double oldPointToNewPointLength = mouseMovementVectorLength;

			// Calculate translation vector.
			pdp::Pt translationVector;
			translationVector = m_MouseMovementVector * oldPointToNewPointLength / mouseMovementVectorLength;
		
			// Calculate translated point.			
			pt += translationVector;

			newData->GetPoints()->SetPoint(ptId, pt[0], pt[1], pt[2]);

			m_SurfaceChanged = true;
		}
	}
	// Mouse Movement
	m_OldVOICenterVector = center;

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

	m_Ext->GetSurface()->SetVtkPolyData(newData);
	m_Ext->UpdateRenderer();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::MovePointsInRadialDirection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Add Points to Mesh?
	if(m_RefinementTechnique != 0)
	{
		AddPoints();
	}

	vtkPolyData* newData = vtkPolyData::New();
	newData->DeepCopy(m_Ext->GetSurfacePolyData());

	// Use direction of the radius
	vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
	pdp::Pt center = m_Ext->GetBall()->GetVtkPolyData()->GetCenter();
	pdp::Pt translateXYZ;
	double translateFactor;

	for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		pdp::Pt pt;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());
				
		// Use distance			
		double distance = sqrt((center[0]-pt[0])*(center[0]-pt[0])+(center[1]-pt[1])*(center[1]-pt[1])+(center[2]-pt[2])*(center[2]-pt[2]));
		if(distance < m_Ext->GetRadius())
		{
			// move cells				
			translateXYZ = pt - center;
			translateFactor = m_Ext->GetRadius() / translateXYZ.Length();
			pt = translateXYZ * translateFactor  + center;
			newData->GetPoints()->SetPoint(ptId, pt.toVtk());
			m_SurfaceChanged = true;
		}
	}

	m_Ext->GetSurface()->SetVtkPolyData(newData);
	m_Ext->UpdateRenderer();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::AddPoints()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Calculate all cells hit by intersection ball
	vtkIdType inputNumPoints = m_Ext->GetIntersectionData()->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
	std::vector<vtkIdType> hitCells;
	std::vector<vtkIdType> hitCellsForSubdivision;
	for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		// get all intersection points
		pdp::Pt pt;
		m_Ext->GetIntersectionData()->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());

		// for all points in the intersection contour find closest surface cell
		vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
		cellLocator->SetDataSet(m_Ext->GetSurfacePolyData());
		cellLocator->BuildLocator();
		double closestPoint[3];//the coordinates of the closest point will be returned here
		double closestPointDist2; //the squared distance to the closest point will be returned here
		vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
		int subId; //this is rarely used (in triangle strips only, I believe)
		cellLocator->FindClosestPoint(pt.toVtk(), closestPoint, cellId, subId, closestPointDist2);

		// add to list if unique
		if(std::find(hitCells.begin(), hitCells.end(), cellId) == hitCells.end())
		{
			hitCells.push_back(cellId);
		}
	}

	// add all cells that include a point lying inside the ball
	vtkIdType inputNumPoints2 = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
	pdp::Pt center = m_Ext->GetBall()->GetVtkPolyData()->GetCenter();
	for (vtkIdType ptId = 0; ptId < inputNumPoints2; ptId++)
	{
		// get all intersection points
		pdp::Pt pt;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());

		// Use distance			
		double distance = sqrt((center[0]-pt[0])*(center[0]-pt[0])+(center[1]-pt[1])*(center[1]-pt[1])+(center[2]-pt[2])*(center[2]-pt[2]));
		if(distance < m_Ext->GetRadius())
		{
			// for all points in the intersection contour find closest surface cell
			vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
			cellLocator->SetDataSet(m_Ext->GetSurfacePolyData());
			cellLocator->BuildLocator();
			double closestPoint[3];//the coordinates of the closest point will be returned here
			double closestPointDist2; //the squared distance to the closest point will be returned here
			vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
			int subId; //this is rarely used (in triangle strips only, I believe)
			cellLocator->FindClosestPoint(pt.toVtk(), closestPoint, cellId, subId, closestPointDist2);

			// add to list if unique
			if(std::find(hitCells.begin(), hitCells.end(), cellId) == hitCells.end())
			{
				hitCells.push_back(cellId);
			}
		}
	}

	if(0)
	{
		std::cout << "First: \n";
		for(int i = 0; i < hitCells.size(); i++)
		{
			std::cout << " " << hitCells[i];
		}
	}

	if(1) // Color hit cell
	{
		vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
		faceColors->SetNumberOfComponents(3);
		vtkIdType inputNumCells = m_Ext->GetSurfacePolyData()->GetNumberOfCells();
		for(int i = 0; i < inputNumCells; i++)
		{
			faceColors->InsertNextTuple3(255, 0, 0);
		}
		std::vector<vtkIdType>::iterator it;
		for(it = hitCells.begin(); it != hitCells.end(); it++)
		{
			faceColors->InsertTuple3(*it,0, 255, 0);
		}
		m_Ext->GetSurfacePolyData()->GetCellData()->SetScalars(faceColors);
	}

	// Compare Cell point distances
	double radius = m_Ext->GetRadius();
	double m_ThresholdEdgeLength = radius * m_RadiusTresholdFactor;
	std::vector<vtkIdType>::iterator it;
	for(it = hitCells.begin(); it != hitCells.end(); it++)
	{
		// get all intersection points
		vtkIdType ntps, *pts;
		m_Ext->GetSurfacePolyData()->GetCellPoints(*it, ntps, pts);
		if(ntps != 3)
			std::cout << "Error -> Triangulate!\n"; // TODO
		double pt1[3], pt2[3], pt3[3];
		m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);

		// Calculate distance between cell points and compare to radius
		double dist1 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2));
		double dist2 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt3));
		double dist3 = sqrt(vtkMath::Distance2BetweenPoints(pt2, pt3));

		//std::cout << "Cell id: " << *it << ", dist1: " << dist1 << ", dist2: " << dist2 << ", dist3: " << dist3 << ", ThresholdEdgeLength: " << m_ThresholdEdgeLength << "\n";

		if(dist1 > m_ThresholdEdgeLength || dist2 > m_ThresholdEdgeLength || dist3 > m_ThresholdEdgeLength)
		{
			hitCellsForSubdivision.push_back(*it);
			//std::cout << "Hit cell for subdivision: " << *it << "\n";
		}		
	}
	
	if(0)
	{
		std::cout << "Second: \n";
		for(int i = 0; i < hitCellsForSubdivision.size(); i++)
		{
			std::cout << " " << hitCellsForSubdivision[i];
		}	
		std::cout << "\n\n\n";
	}

	// Drop down menu, select subdivision technique
	// m_RefinementTechnique == 0 -> Off
	if(m_RefinementTechnique == 1)
	{
		AutomaticInsertPoints(hitCellsForSubdivision);
	}
	else if(m_RefinementTechnique == 2)
	{
		SubdivideByInsertPoint(hitCellsForSubdivision);
	}
	else if (m_RefinementTechnique == 3)
	{
		SubdivideByLongestEdge(hitCellsForSubdivision);
	}

	std::cout << "Number of points in surface: " << m_Ext->GetSurfacePolyData()->GetNumberOfPoints() << "\n";
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::AutomaticInsertPoints(std::vector<vtkIdType> cells)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::vector<vtkIdType> subdivideByPointCells;
	std::vector<vtkIdType> subdivideByEdgeCells;
	
	// calculate shape of the triangle
	for(std::vector<vtkIdType>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		// get all intersection points
		
		vtkIdType ntps, *pts;
		m_Ext->GetSurfacePolyData()->GetCellPoints(*it, ntps, pts);
		if(ntps != 3)
			std::cout << "Error -> Triangulate!\n"; // TODO
		double pt1[3], pt2[3], pt3[3];
		m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);

		// Calculate distance between cell points and compare to radius
		double dist1 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2));
		double dist2 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt3));
		double dist3 = sqrt(vtkMath::Distance2BetweenPoints(pt2, pt3));

		// Calculate ratios
		double ratio12 = dist1 / dist2;
		double ratio13 = dist1 / dist3;
		double ratio23 = dist2 / dist3;

		if(abs(ratio12) > m_AutoInsertPointThreshold || abs(ratio13) > m_AutoInsertPointThreshold || abs(ratio23) > m_AutoInsertPointThreshold)
		{
			// laengliches dreieck -> use subdivide by edge
			subdivideByEdgeCells.push_back(*it);
		}
		else
		{
			subdivideByPointCells.push_back(*it);			
		}
	}

	if(0)
	{
		std::cout << "Edge: \n";
		for(std::vector<vtkIdType>::iterator it = subdivideByEdgeCells.begin(); it != subdivideByEdgeCells.end(); it++)
		{
			std::cout << *it << " ";
		}
		std::cout << "Insert Point: \n";
		for(std::vector<vtkIdType>::iterator it = subdivideByPointCells.begin(); it != subdivideByPointCells.end(); it++)
		{
			std::cout << *it << " ";
		}
		std::cout << "\n";
	}

	SubdivideByLongestEdge(subdivideByEdgeCells);
	SubdivideByInsertPoint(subdivideByPointCells);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::SubdivideByInsertPoint(std::vector<vtkIdType> cells)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::vector<vtkIdType> cellsToBeRemoved;
	// For all selected cells:
	for(std::vector<vtkIdType>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		// Get Points of hit cell
		vtkIdType ntps, *pts;
		m_Ext->GetSurfacePolyData()->GetCellPoints(*it, ntps, pts);
		//std::cout << "Number of Points in Cell: " << ntps << "\n";
		if(ntps != 3)
			std::cout << "Error -> Triangulate!\n"; // TODO
		double pt1[3], pt2[3], pt3[3];
		m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
		m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);
		
		// Mark old cell for deletion
		cellsToBeRemoved.push_back(*it);

		// Calculate centroid of the cell
		double weightSum = 0.0;
		double centroid[3] = {0.0, 0.0, 0.0};

		double currentWeight12 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2));
		double currentWeight13 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt3));
		double currentWeight23 = sqrt(vtkMath::Distance2BetweenPoints(pt2, pt3));

		centroid[0] += pt1[0] * (currentWeight12 + currentWeight13); // Produkt oder Summe der Gewichte?
		centroid[1] += pt1[1] * (currentWeight12 + currentWeight13);
		centroid[2] += pt1[2] * (currentWeight12 + currentWeight13);

		centroid[0] += pt2[0] * (currentWeight12 + currentWeight23);
		centroid[1] += pt2[1] * (currentWeight12 + currentWeight23);
		centroid[2] += pt2[2] * (currentWeight12 + currentWeight23);

		centroid[0] += pt3[0] * (currentWeight13 + currentWeight23);
		centroid[1] += pt3[1] * (currentWeight13 + currentWeight23);
		centroid[2] += pt3[2] * (currentWeight13 + currentWeight23);

		weightSum = currentWeight12 + currentWeight13 + currentWeight23;
		
		if(0)
		{
			std::cout << "Point 1: " << pt1[0] << " " << pt1[1] << " " << pt1[2] << "\n";
			std::cout << "Point 2: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << "\n";
			std::cout << "Distance between two points: " << sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2)) << "\n";
		}

		centroid[0] /= (weightSum * 2);
		centroid[1] /= (weightSum * 2);
		centroid[2] /= (weightSum * 2);

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

		vtkAppendPolyData *app = vtkAppendPolyData::New();
		app->AddInput(m_Ext->GetSurfacePolyData());
		app->AddInput(newTriangles);
		app->Update();
		m_Ext->SetSurfacePolyData(app->GetOutput());

		// Fill generated holes
		m_Ext->GetSurfacePolyData()->BuildCells();
		m_Ext->GetSurfacePolyData()->BuildCells();
	}

	// Forum Thread: easiest to rebuild polydata from scratch
	vtkSmartPointer<vtkPolyData> newSurface = vtkSmartPointer<vtkPolyData>::New();
	newSurface->Allocate(10000,10000); 
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->Allocate(10000);

	vtkCellArray* cells2 = m_Ext->GetSurfacePolyData()->GetPolys();
	vtkIdType *pts, npts, cellId;

	for(cellId=0, cells2->InitTraversal(); cells2->GetNextCell(npts,pts); cellId++)
	{
		if(!std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), cellId))
		{
			// CellId is not deleted -> add to new mesh
			double pt1[3], pt2[3], pt3[3];
			vtkIdType t1, t2, t3;
			m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);
			
			t1 = points->InsertNextPoint(pt1);
			t2 = points->InsertNextPoint(pt2);
			t3 = points->InsertNextPoint(pt3);

			vtkIdList* newPts = vtkIdList::New();
			newPts->InsertNextId(t1);
			newPts->InsertNextId(t2);
			newPts->InsertNextId(t3);

			newSurface->SetPoints(points);			
			newSurface->InsertNextCell(VTK_TRIANGLE, newPts);
		}
		else
		{
			//std::cout << "Vector includes cell.\n";
			// add clipped part
		}
	}

	vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
	removeDuplicatedPoints->SetInput(newSurface);
	removeDuplicatedPoints->Update();
	newSurface = removeDuplicatedPoints->GetOutput();

	m_Ext->SetSurfacePolyData(newSurface);

	ResetNormals();
	UpdateRenderer();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::SubdivideByLongestEdge(std::vector<vtkIdType> cells)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(0)
	{
		std::cout << "Cells: \n";
		for(int i = 0; i < cells.size(); i++)
		{
			std::cout << " " << cells[i];
		}	
		std::cout << "\n\n\n";
	}

	std::vector<vtkIdType> cellsToBeRemoved;
	// For all selected cells:
	for(std::vector<vtkIdType>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		if(!std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), *it))
		{
			//std::cout << "Subdivide cell: " << *it << "\n";

			// Get Points of hit cell
			int pointDistribution;
			vtkIdType ntps, *pts, *pts2;
			m_Ext->GetSurfacePolyData()->GetCellPoints(*it, ntps, pts);
			//std::cout << "Number of Points in Cell: " << ntps << "\n";
			if(ntps != 3)
				std::cout << "Error -> Triangulate!\n"; // TODO
			double pt1[3], pt2[3], pt3[3], tmp1[3], tmp2[3], tmp3[3], pt4[3], newPt[3];
			m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);
			
			// Calculate longest edge
			double edge12 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2));
			double edge13 = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt3));
			double edge23 = sqrt(vtkMath::Distance2BetweenPoints(pt2, pt3));
			int longestEdge = 0;
			vtkIdList *otherEdge = vtkIdList::New();
			if(edge12 > edge13 && edge12> edge23)
			{
				// 1
				//std::cout << "Edge 1.\n";
				pointDistribution = 1;
				newPt[0] = 0.5 * (pt1[0] + pt2[0]);
				newPt[1] = 0.5 * (pt1[1] + pt2[1]);
				newPt[2] = 0.5 * (pt1[2] + pt2[2]);
				// get other cell edge
				m_Ext->GetSurfacePolyData()->BuildLinks();	
				m_Ext->GetSurfacePolyData()->GetCellEdgeNeighbors(*it, pts[0], pts[1], otherEdge);	// get other cell
				//m_Ext->GetSurfacePolyData()->GetCellEdgeNeighbors(*it, pts[0], pts[1], otherEdge);	// get both cells
			}
			else if(edge13 > edge23)
			{
				// 2
				//std::cout << "Edge 2.\n";
				pointDistribution = 2;
				newPt[0] = 0.5 * (pt1[0] + pt3[0]);
				newPt[1] = 0.5 * (pt1[1] + pt3[1]);
				newPt[2] = 0.5 * (pt1[2] + pt3[2]);
				// get other cell edge
				m_Ext->GetSurfacePolyData()->BuildLinks();
				m_Ext->GetSurfacePolyData()->GetCellEdgeNeighbors(*it, pts[0], pts[2], otherEdge);
			}
			else 
			{
				// 3
				//std::cout << "Edge 3.\n";
				pointDistribution = 3;
				newPt[0] = 0.5 * (pt2[0] + pt3[0]);
				newPt[1] = 0.5 * (pt2[1] + pt3[1]);
				newPt[2] = 0.5 * (pt2[2] + pt3[2]);
				// get other cell edge
				m_Ext->GetSurfacePolyData()->BuildLinks();
				m_Ext->GetSurfacePolyData()->GetCellEdgeNeighbors(*it, pts[1], pts[2], otherEdge);
			}
				
			// Get first point
			//otherEdge->Print(std::cout);
			if(otherEdge->GetNumberOfIds() == 0)
			{
				std::cout << "Number of Edge Neighbours = 0 -> Boundary: Triangle is skipped!\n";
				continue;
			}
			if(otherEdge->GetNumberOfIds() != 1)
			{
				std::cout << "Number of Edge Neighbours = " << otherEdge->GetNumberOfIds() << " != 1 -> Error!\n";
				continue;
			}
			else
			{
				;//std::cout << "Number of Edge Neighbours = 1 -> ok!\n";
			}
			vtkIdType first = otherEdge->GetId(0);
			//vtkIdType second = otherEdge->GetId(1);

			//std::cout << "Current Cell: " << *it << "\n";
			//std::cout << "first: " << first << "\n";			

			// If one of the cells has already been modified don't modify
			if(0)
			{
				std::cout << "Cells to be removed: ";
				for(std::vector<vtkIdType>::iterator it2 = cellsToBeRemoved.begin(); it2 != cellsToBeRemoved.end(); it2++)
				{
					std::cout << *it2 << " ";
				}
				std::cout << "\n";
			}

			// if(std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), *it)) -> tuts nicht
			bool skipLoop = false; 
			for(std::vector<vtkIdType>::iterator it2 = cellsToBeRemoved.begin(); it2 != cellsToBeRemoved.end(); it2++)
			{
				if(*it == *it2)
				{
					//std::cout << "Cell: " << *it << " has already been modified.\n";
					skipLoop = true;
					break;
				}
				if(first == *it2)
				{						
					//std::cout << "Cell: " << first << " has already been modified.\n";
					skipLoop = true;
					break;
				}	
			}
			if(skipLoop)
				continue;

			m_Ext->GetSurfacePolyData()->GetCellPoints(first, ntps, pts2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts2[0], tmp1);
			m_Ext->GetSurfacePolyData()->GetPoint(pts2[1], tmp2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts2[2], tmp3);

			if(!m_Ext->UniquePoint(pt1, tmp1) && !m_Ext->UniquePoint(pt2, tmp1) && !m_Ext->UniquePoint(pt3, tmp1))
			{
				pt4[0] = tmp1[0];
				pt4[1] = tmp1[1];
				pt4[2] = tmp1[2];
				//std::cout << "pt4 = tmp1\n";
			}
			if(!m_Ext->UniquePoint(pt1, tmp2) && !m_Ext->UniquePoint(pt2, tmp2) && !m_Ext->UniquePoint(pt3, tmp2))
			{
				pt4[0] = tmp2[0];
				pt4[1] = tmp2[1];
				pt4[2] = tmp2[2];
				//std::cout << "pt4 = tmp2\n";
			}
			if(!m_Ext->UniquePoint(pt1, tmp3) && !m_Ext->UniquePoint(pt2, tmp3) && !m_Ext->UniquePoint(pt3, tmp3))
			{
				pt4[0] = tmp3[0];
				pt4[1] = tmp3[1];
				pt4[2] = tmp3[2];
				//std::cout << "pt4 = tmp3\n";
			}
			
			if(0) // print 4 points
			{
				std::cout << "P1: " << pt1[0] << " " << pt1[1] << " " << pt1[2] << "\n";
				std::cout << "P2: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << "\n";
				std::cout << "P3: " << pt3[0] << " " << pt3[1] << " " << pt3[2] << "\n";
				std::cout << "P4: " << pt4[0] << " " << pt4[1] << " " << pt4[2] << "\n";
			}

			// Build 4 new triangles
			vtkSmartPointer<vtkPolyData> newTriangles = vtkSmartPointer<vtkPolyData>::New();
			vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
			vtkIdType pt1Id, pt2Id, pt3Id, pt4Id, ptNewId;

			// pointDistribution == 1 -> subdivided edge has point 1 & point 2
			// pointDistribution == 2 -> subdivided edge has point 1 & point 3
			// pointDistribution == 3 -> subdivided edge has point 2 & point 3
			// arrange point 1, point 2 and point 3 according to subdivided edge (fixed scheme)
			if(pointDistribution == 1)
			{
				//std::cout << "Point distribution: 1\n";
				pt1Id = points->InsertNextPoint(pt1);
				pt2Id = points->InsertNextPoint(pt2);
				pt3Id = points->InsertNextPoint(pt3);
			}
			else if(pointDistribution == 2)
			{
				//std::cout << "Point distribution: 2\n";
				pt1Id = points->InsertNextPoint(pt1);						
				pt2Id = points->InsertNextPoint(pt3);
				pt3Id = points->InsertNextPoint(pt2);
			}
			else
			{
				//std::cout << "Point distribution: 3\n";
				pt1Id = points->InsertNextPoint(pt2);
				pt2Id = points->InsertNextPoint(pt3);
				pt3Id = points->InsertNextPoint(pt1);
			}

			pt4Id = points->InsertNextPoint(pt4);
			ptNewId =  points->InsertNextPoint(newPt);
			newTriangles->SetPoints(points);
			newTriangles->Allocate();

			vtkIdList* newPts1 = vtkIdList::New();
			newPts1->InsertNextId(pt1Id); newPts1->InsertNextId(pt3Id); newPts1->InsertNextId(ptNewId);			
			newTriangles->InsertNextCell(VTK_TRIANGLE, newPts1);

			vtkIdList* newPts2 = vtkIdList::New();
			newPts2->InsertNextId(pt3Id); newPts2->InsertNextId(pt2Id); newPts2->InsertNextId(ptNewId);			
			newTriangles->InsertNextCell(VTK_TRIANGLE, newPts2);

			vtkIdList* newPts3 = vtkIdList::New();
			newPts3->InsertNextId(pt2Id); newPts3->InsertNextId(pt4Id); newPts3->InsertNextId(ptNewId);			
			newTriangles->InsertNextCell(VTK_TRIANGLE, newPts3); 

			vtkIdList* newPts4 = vtkIdList::New();
			newPts4->InsertNextId(pt1Id); newPts4->InsertNextId(pt4Id); newPts4->InsertNextId(ptNewId);			
			newTriangles->InsertNextCell(VTK_TRIANGLE, newPts4);
			
			vtkAppendPolyData *app = vtkAppendPolyData::New();
			app->AddInput(m_Ext->GetSurfacePolyData());
			app->AddInput(newTriangles);
			app->Update();
			m_Ext->SetSurfacePolyData(app->GetOutput());
			
			cellsToBeRemoved.push_back(first);
			cellsToBeRemoved.push_back(*it);

			// Fill generated holes
			m_Ext->GetSurfacePolyData()->BuildCells();
			m_Ext->GetSurfacePolyData()->BuildLinks();
		}
		else
		{
			//std::cout << "Cell is already subdivided, due to neighbour cells!\n";
		}
	}

	// Rebuild mesh without cells to remove
	m_Ext->GetSurfacePolyData()->BuildCells();
	m_Ext->GetSurfacePolyData()->BuildLinks();

	// make cellsToBeRemoved vector unique, if there are dublicated entries 
	std::sort(cellsToBeRemoved.begin(), cellsToBeRemoved.end());
	std::vector<vtkIdType>::iterator it;
	it = std::unique(cellsToBeRemoved.begin(), cellsToBeRemoved.end()); 
	cellsToBeRemoved.resize(it - cellsToBeRemoved.begin());  
	
	if(0) // printing
	{
		std::cout << "Vector.size: (2) " << cellsToBeRemoved.size() << "\n";
		for(std::vector<vtkIdType>::iterator it = cellsToBeRemoved.begin(); it != cellsToBeRemoved.end(); it++)
		{
			std::cout << "Entries: " << *it << "\n";
		}
	}

	// Forum Thread: easiest to rebuild polydata from scratch
	vtkSmartPointer<vtkPolyData> newSurface = vtkSmartPointer<vtkPolyData>::New();
	newSurface->Allocate(10000,10000); 
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->Allocate(10000);

	vtkCellArray* cells2 = m_Ext->GetSurfacePolyData()->GetPolys();
	vtkIdType *pts, npts, cellId;

	for(cellId=0, cells2->InitTraversal(); cells2->GetNextCell(npts,pts); cellId++)
	{
		if(!std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), cellId))
		{
			// CellId is not deleted -> add to new mesh
			double pt1[3], pt2[3], pt3[3];
			vtkIdType t1, t2, t3;
			m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);
			
			t1 = points->InsertNextPoint(pt1);
			t2 = points->InsertNextPoint(pt2);
			t3 = points->InsertNextPoint(pt3);

			vtkIdList* newPts = vtkIdList::New();
			newPts->InsertNextId(t1);
			newPts->InsertNextId(t2);
			newPts->InsertNextId(t3);

			newSurface->SetPoints(points);			
			newSurface->InsertNextCell(VTK_TRIANGLE, newPts);
		}
		else
		{
			//std::cout << "Vector includes cell.\n";
			// add clipped part
		}
	}

	vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
	removeDuplicatedPoints->SetInput(newSurface);
	removeDuplicatedPoints->Update();
	newSurface = removeDuplicatedPoints->GetOutput();

	m_Ext->SetSurfacePolyData(newSurface);

	ResetNormals();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::ShowTool()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	if(m_ShowBall)
	{
		std::cout << "Show Tool in 3d view\n";
		m_ShowBall = false;

		m_Ext->GetBallNode()->SetVisibility(true, m_Ext->GetMitkView()->GetRenderWindow4()->GetRenderer());
	}
	else
	{
		std::cout << "Hide Tool in 3d view\n";
		m_ShowBall = true;

		m_Ext->GetBallNode()->SetVisibility(false, m_Ext->GetMitkView()->GetRenderWindow4()->GetRenderer());
	}
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::Collinear()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Move points in mouse movement direction.\n";
	m_RadialDirection = false;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::Radial()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Move points from origin radial to the surface.\n";
	m_RadialDirection = true;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::SetRefinementTechnique(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RefinementTechnique = selection;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::ResetNormals()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	vtkSmartPointer<vtkPolyDataNormals> skinNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	skinNormals->SetInput(m_Ext->GetSurfacePolyData());
	skinNormals->ComputePointNormalsOn();
	skinNormals->ComputeCellNormalsOff();
	skinNormals->SetFeatureAngle(60.0);
	skinNormals->Update();
	m_Ext->SetSurfacePolyData(skinNormals->GetOutput(0));
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::BulgeTool::ReinitNow()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->Reinit();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
