// "pdp"
#include "mitkDragTool.h"
#include "mitkDragTool.xpm"
#include "QmitkDragToolGUI.h"

// MITK
#include "mitkToolManager.h"
#include <mitkStateEvent.h>
#include <mitkApplicationCursor.h>
#include <mitkLookupTableProperty.h>

// VTK
//#include <vtkCellLocator.h>
#include <vtkPointLocator.h>
#include "itkVTKPolyDataWriter.h"
#include <vtkMath.h>
#include <vtkIdList.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>

#include <vtkCleanPolyData.h>

#include "cursor_hand_closed.xpm"
#include "cursor_hand_open.xpm"


mitk::DragTool::DragTool()
:Tool("MyPositionTrackerWithLeftDrag")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );
	CONNECT_ACTION( 1003, LeftDrag );
	CONNECT_ACTION( 1004, LeftClick );
	CONNECT_ACTION( 1005, LeftClickRelease );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	//m_Ut = new pdp::Utilities(m_Ext);
	//m_CursorOnSurface = false;

	m_ROIRadius = 9.0;
	m_DragOnce = false;
	m_Color = false;
	m_SelectWeight = 0;

	m_ChangeRadius = new RadiusInteractor("MyRadiusInteractor");
	m_ChangeRadius->UpdateRadius += mitk::MessageDelegate1<mitk::DragTool, int>(this, &mitk::DragTool::UpdateRadiusViaInteractor);
}

mitk::DragTool::~DragTool()
{}

const char** mitk::DragTool::GetXPM() const
{
	return mitkDragTool_xpm;
}

const char* mitk::DragTool::GetName() const
{
	return "Drag Tool";
}

const char* mitk::DragTool::GetGroup() const
{
	return "default";
}

void mitk::DragTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::DragTool, int>(this, &mitk::DragTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::DragTool, int>(this, &mitk::DragTool::UpdateRadiusSlider);

	m_Ext->GetIntersectionNode()->SetVisibility(false);
	m_Ext->GetBallNode()->SetVisibility(false);

	m_Ext->SetRadiusInteractorOff();
	m_ROIRadius = (m_Ext->GetRadius())*(m_Ext->GetRadius());
	m_Ext->SetRadius(0.7);

	// ROI
	m_Ball2 = mitk::Surface::New();
	m_BallNode2 = mitk::DataNode::New();
	m_BallNode2->SetName("ROI 2D");
	m_BallNode2->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	// Set ball visible in renderer 1 to 3 and visible=false in renderer 4
	m_BallNode2->SetVisibility(false, m_Ext->GetMitkView()->GetRenderWindow4()->GetRenderer());
	m_BallNode2->SetData(m_Ball2);
	// Add unique identifier for data nodes
	int newId = m_Ext->GetUniqueId();
	m_BallNode2->SetIntProperty("UniqueID", newId);
	m_Ext->GetDataStorage()->Add(m_BallNode2);
	m_ContourSphere2 = vtkSphereSource::New();
	m_ContourSphere2->SetThetaResolution(20/*m_Ext->GetContourResolution()*/);
	m_ContourSphere2->SetPhiResolution(20/*m_Ext->GetContourResolution()*/);
	m_ContourSphere2->SetRadius(sqrt((double)m_ROIRadius));
	m_ContourSphere2->SetCenter(m_Ext->GetContourSphere()->GetCenter());
	m_ContourSphere2->Update();
	m_Ball2->SetVtkPolyData(m_ContourSphere2->GetOutput());
	
	m_Ext->SetImagesToBottom();

	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->AddListener(m_ChangeRadius);

	m_Weights.clear();
	m_Weights.push_back("Constant");
	m_Weights.push_back("Distance");
	m_Weights.push_back("Tangential");

	GetCurrentWeights(m_Weights);
}

void mitk::DragTool::Deactivated()
{
	m_Ext->SetRadius(sqrt((double)m_ROIRadius));
	mitk::Point3D origin;
	origin[0] = m_ContourSphere2->GetCenter()[0];
	origin[1] = m_ContourSphere2->GetCenter()[1];
	origin[2] = m_ContourSphere2->GetCenter()[2];
	m_Ext->GetBall()->SetOrigin(origin);

	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::DragTool, int>(this, &mitk::DragTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::DragTool, int>(this, &mitk::DragTool::UpdateRadiusSlider);

	m_Ext->GetDataStorage()->Remove(m_BallNode2);

	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->RemoveListener(m_ChangeRadius);

	// sichern?
	mitk::ApplicationCursor::GetInstance()->PopCursor();
}

void mitk::DragTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::DragTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// sichern?
	mitk::ApplicationCursor::GetInstance()->PopCursor();

	m_Ext->GetIntersectionPolyDataFilter()->SetInput(0, m_Ext->GetSurfacePolyData());
	m_Ext->GetIntersectionPolyDataFilter()->Update();

	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	
	mitk::Surface* intersectionData = m_Ext->GetIntersectionData();
	vtkPolyData* intersectionPolyData = intersectionData->GetVtkPolyData();
	if(intersectionPolyData->GetNumberOfCells() > 0)
	{
		// cursor hit surface -> change corsor to open hand
		mitk::ApplicationCursor::GetInstance()->PushCursor(cursor_hand_open_xpm);
		m_CursorOnSurface = true;
	}
	else
	{
		mitk::ApplicationCursor::GetInstance()->PopCursor();
		m_CursorOnSurface = false;
	}

	m_ContourSphere2->SetRadius(sqrt((double)m_ROIRadius)); 
	m_ContourSphere2->SetCenter(m_Ext->GetContourSphere()->GetCenter());
	m_ContourSphere2->Update();

	return ok;
}

bool mitk::DragTool::LeftDrag(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
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

	if(m_CursorOnSurface)
	{
		//if(m_DragOnce)
		//	return true;

		pdp::Pt pt(displayPositionEvent->GetWorldPosition());

		// fahrplan:
		// bestimme differential coordinates (deltas) aus global coordinates
		// solve linear equation system: Lx = delta in least squares sense and reconstruct cartesian surface coordinates

		// zusatz: Spalte berechung in factorization and solving of the system

		// berechne vector delta
		int numberOfAllPoints = m_AllPoints.size();	
		int numberOfAnchorPointsOutsideROI = m_AnchorPointsOutsideROI.size();
		double** X = new double*[numberOfAllPoints];
		double** delta = new double*[numberOfAllPoints];
		int row = 0;
		for(std::vector<vtkIdType>::iterator it = m_AllPoints.begin(); it != m_AllPoints.end(); it++, row++)
		{
				double* newRow = new double[3];
				//X[row] = newRow;
				delta[row] = newRow;
				pdp::Pt pt2;
				m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, pt2.toVtk());	
				X[row] = pt2.toVtk();
		}

		//for(int i = 0; i < numberOfAllPoints; i++)
		//{
		//	std::cout << "X: " << X[i][0] << " " << X[i][1] << " " << X[i][2] << "\n";
		//}

		//vtkMath::MultiplyMatrix(const double **	A,const double ** B,unsigned int rowA,unsigned int colA,unsigned int rowB,unsigned int colB,double **C) 
		vtkMath::MultiplyMatrix((const double**) m_LMatrix, (const double**) X, numberOfAllPoints, numberOfAllPoints, numberOfAllPoints, 3, delta); 

		/*for(int i = 0; i < numberOfAllPoints; i++)
		{
			std::cout << "Delta: " << delta[i][0] << " " << delta[i][1] << " " << delta[i][2] << "\n";
		}*/

		// set different weights for relation to neighbours
		double w = 1.0;


		// vector b consists of (delta, w*vi)
		double** B = new double*[numberOfAllPoints+numberOfAnchorPointsOutsideROI+1];
		// add delta to b
		for(int row = 0; row < numberOfAllPoints; row++)
		{
				double* newRow = new double[3];
				B[row] = newRow;
				
				B[row][0] = delta[row][0] * w;
				B[row][1] = delta[row][1] * w;
				B[row][2] = delta[row][2] * w;
		}
		// add w*vi to b 
		std::vector<vtkIdType>::iterator anchorIt = m_AnchorPointsOutsideROI.begin();
		for(int row = numberOfAllPoints; row < numberOfAllPoints + numberOfAnchorPointsOutsideROI; row++, anchorIt++)
		{
				double* newRow = new double[3];
				B[row] = newRow;

				pdp::Pt pt2;
				m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*anchorIt, pt2.toVtk());	
				
				B[row][0] = pt2[0];
				B[row][1] = pt2[1];
				B[row][2] = pt2[2];
		}
		// add handle with new coordinates?, pt = handlePoint
		// calc delta coordinates for new handle point position
		// Chris Tralie: and set the corresponding "delta coordinate" of this row to be the absolute position of that vertex:
		double* newRow = new double[3];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI] = newRow;
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][0] = pt[0];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][1] = pt[1];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][2] = pt[2];

		// test: old position: mesh should not move at all	
		//pdp::Pt handle;
		//m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_HandlePointID, handle.toVtk());
		//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][0] = handle[0];
		//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][1] = handle[1];
		//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][2] = handle[2];

		// solve linear equation system: Lx = delta in least squares sense and reconstruct cartesian surface coordinates
		//static int 	SolveLeastSquares (int numberOfSamples, double **xt, int xOrder, double **yt, int yOrder, double **mt, int checkHomogeneous=1)
		//static int 	SolveLeastSquares (#Zeilen, double **xt, int xOrder, double **yt, int yOrder, double **mt, int checkHomogeneous=1)
		// allocate X_OUT
		double** X_OUT = new double*[numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1];
		for(int i = 0; i < numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1; i++)
		{
			double* newRow = new double[3];
			X_OUT[i] = newRow;
			for(int j = 0; j < 3; j++)
			{
				X_OUT[i][j] = 0.0;
			}
		}

		int numberOfRows = numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1;
		/*std::cout << "numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1: " << numberOfRows << "\n";
		std::cout << "numberOfAllPoints: " << numberOfAllPoints << "\n";
		std::cout << "numberOfAnchorPointsOutsideROI: " << numberOfAnchorPointsOutsideROI << "\n";*/

		//std::cout << "Number of rows: " << numberOfRows << "\n";
		//std::cout << "numberOfAllPoints: " << numberOfAllPoints << "\n";

		//for(int row = 0; row < numberOfRows; row++)
		//{
		//	for(int col = 0; col < numberOfAllPoints; col++)
		//	{
		//		double dummy = m_LMatrixWithAnchors[row][col];
		//	}
		//}

		if(vtkMath::SolveLeastSquares(numberOfRows, m_LMatrixWithAnchors, numberOfAllPoints, B, 3/*numberOfAllPoints*/, X_OUT, 0 /*known to be non homogeneous*/) == 0 )
		{
			std::cout << "Error solving matrix!\n";
			return true;
		}
		else
		{
			//std::cout << "Matrix solved!\n";
		}

		// largest difference between old and new handle point, calc difference
		/*pdp::Pt handle2;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_HandlePointID, handle2.toVtk());
		std::cout << "Handle point(old vs. new): " << handle2[0] << " " << handle2[1] << " " << handle2[2] << ", vs. " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";*/

		// move points to new position (without ROI edge anchors)
		vtkPolyData* newData2 = vtkPolyData::New();
		newData2->DeepCopy(m_Ext->GetSurfacePolyData());

		// only move points inside ROI 
		int numberOfPointsInROI = m_PointsInROI.size();
		std::vector<vtkIdType>::iterator originalIt2 = m_AllPoints.begin();
		for(int i = 0; i < numberOfPointsInROI; i++, originalIt2++)
		{
			pdp::Pt newPt;
			newPt[0] = X_OUT[i][0];
			newPt[1] = X_OUT[i][1];
			newPt[2] = X_OUT[i][2];

			newData2->GetPoints()->SetPoint(*originalIt2, newPt.toVtk());

			//std::cout << "X_OUT: " << X_OUT[i][0] << " " << X_OUT[i][1] << " " << X_OUT[i][2] << " vs: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << "\n";
		}
		// move handle point
		//pdp::Pt pt2;
		//m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint((m_AllPoints.back()), pt2.toVtk());	
		pdp::Pt newPt;
		newPt[0] = X_OUT[numberOfAllPoints-1][0];
		newPt[1] = X_OUT[numberOfAllPoints-1][1];
		newPt[2] = X_OUT[numberOfAllPoints-1][2];
		newData2->GetPoints()->SetPoint((m_AllPoints.back()), newPt.toVtk());

		m_Ext->GetSurface()->SetVtkPolyData(newData2);

		m_Ext->UpdateRenderer();
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		//std::cout << "Done!\n";
		m_DragOnce = true;
	}
	return ok;
}

bool mitk::DragTool::LeftClick(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	// test: remove doublicated points
	vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
	removeDuplicatedPoints->SetInput(m_Ext->GetSurfacePolyData());
	removeDuplicatedPoints->Update();
	m_Ext->SetSurfacePolyData(removeDuplicatedPoints->GetOutput());
	// todo: eliminate doublicated points


	// Check if click is in 3d renderer
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

	if(m_CursorOnSurface)
	{
		// sichern?
		mitk::ApplicationCursor::GetInstance()->PopCursor();
		mitk::ApplicationCursor::GetInstance()->PushCursor(cursor_hand_closed_xpm);

		m_PointsInROI.clear();
		m_AnchorPointsOutsideROI.clear();
		m_AllPoints.clear();
		//delete m_LMatrix;
		//delete m_LMatrixWithAnchors;

		// get point to be moved
		const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
		if (displayPositionEvent == NULL)
			return false;
		pdp::Pt pt(displayPositionEvent->GetWorldPosition());

		vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
		pointLocator->SetDataSet(m_Ext->GetSurfacePolyData());
		pointLocator->BuildLocator();
		m_HandlePointID = pointLocator->FindClosestPoint(pt.toVtk());
		std::cout << "Closest point pointId: " << m_HandlePointID << "\n";	
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_HandlePointID, m_HandlePoint.toVtk());
		std::cout << "Closest point position: " << m_HandlePoint.toItk() << "\n";	

		// define region of interest around old point position
		vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
		for(vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
		{
			pdp::Pt pt;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());	

			if(ptId == m_HandlePointID)
			{
				//std::cout << "Don't tough handle point.\n";
				continue;
			}
			// calculate distance to handle point
			double currentDistance =/*sqrt(*/vtkMath::Distance2BetweenPoints(m_HandlePoint.toVtk(), pt.toVtk())/*)*/;
			//std::cout << "Distance to other points: " << currentDistance << "\n";
			if(currentDistance < m_ROIRadius)
			{
				// point is inside of ROI -> add to vector
				m_PointsInROI.push_back(ptId);
				//m_InitialDistance.push_back(currentDistance);
			}
		}

		if(0) // print
		{
			/*for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
			{
				std::cout << "Point in ROI: " << *it << "\n";
			}*/
			std::cout << "Number of Points in ROI: " << m_PointsInROI.size() << "\n";
		}
	

		//m_LastPoint = m_HandlePoint;

		// calculate all points with an edge pointing inside the ROI
		for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
		{
			// get cells to point
			vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
			m_Ext->GetSurfacePolyData()->GetPointCells(*it, cellIds);

			for(vtkIdType i = 0;  i < cellIds->GetNumberOfIds(); i++)
			{
				// Get points to cells
				vtkSmartPointer<vtkIdList> ptIds = vtkSmartPointer<vtkIdList>::New();
				m_Ext->GetSurfacePolyData()->GetCellPoints(cellIds->GetId(i), ptIds); 

				for(vtkIdType j = 0; j < ptIds->GetNumberOfIds(); j++)
				{
					// Compare ptIds to points in ROI
					bool pointFoundInROI = false;
					for(std::vector<vtkIdType>::iterator it2 = m_PointsInROI.begin(); it2 != m_PointsInROI.end(); it2++)
					{
						if(*it2 == ptIds->GetId(j))
						{
							// Point in ROI
							pointFoundInROI = true;
						}
					}	
					if(!pointFoundInROI)
					{
						// Is point handle point?
						if(ptIds->GetId(j) == m_HandlePointID)
						{
							; // skip
						}
						else
						{
							// Point found that has an edge inside the ROI -> add to vector
							m_AnchorPointsOutsideROI.push_back(ptIds->GetId(j));
						}
					}
				}
			}
		}

		// sort and make unique
		std::sort<std::vector<vtkIdType>::iterator>(m_AnchorPointsOutsideROI.begin(), m_AnchorPointsOutsideROI.end());
		std::vector<vtkIdType>::iterator lastElement = std::unique<std::vector<vtkIdType>::iterator>(m_AnchorPointsOutsideROI.begin(), m_AnchorPointsOutsideROI.end());
		m_AnchorPointsOutsideROI.resize(lastElement - m_AnchorPointsOutsideROI.begin()); // works

		//std::cout << "Number of anchor points outside the ROI: " << m_AnchorPointsOutsideROI.size() << "\n";
		//for(std::vector<vtkIdType>::iterator it = m_AnchorPointsOutsideROI.begin(); it != m_AnchorPointsOutsideROI.end(); it++)
		//{
		//	std::cout << *it << " ";
		//}
		

		//todo: check if all points are part of the frontside of the mesh

		//// sanity check:
		//// add all points from total mesh
		//vtkIdType inputNumPoints2 = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
		//for(vtkIdType ptId = 0; ptId < inputNumPoints2; ptId++)
		//{
		//	m_AllPoints.push_back(ptId);
		//}
		//// add handle anchor point
		//m_AllPoints.push_back(m_HandlePointID);

		// add points from ROI
		for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
		{
			m_AllPoints.push_back(*it);
		}
		// add anchor points at the edge of the ROI
		for(std::vector<vtkIdType>::iterator it = m_AnchorPointsOutsideROI.begin(); it != m_AnchorPointsOutsideROI.end(); it++)
		{
			m_AllPoints.push_back(*it);
		}
		// add handle anchor point
		m_AllPoints.push_back(m_HandlePointID);

		// print all points
		//for(std::vector<vtkIdType>::iterator it = m_AllPoints.begin(); it != m_AllPoints.end(); it++)
		//{
		//	pdp::Pt pt;
		//	m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, pt.toVtk());	

		//	std::cout << "Print AllPoints: id: " << *it << ", position: " << pt[0] << " "  << pt[1] << " "  << pt[2] << "\n"; 
		//}


		// create L-matrix 
		m_Ext->GetSurfacePolyData()->BuildLinks();
		int numberOfAllPoints = m_AllPoints.size();
		int numberOfAnchorPointsOutsideROI = m_AnchorPointsOutsideROI.size();
		m_LMatrix = new double*[numberOfAllPoints];	// square matrix
		m_LMatrixWithAnchors = new double*[numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1];	// number of cols in matrix
		
		if(m_SelectWeight == 0)
		{
			CreateLMatrixWithW1();
		}
		else if(m_SelectWeight == 1)
		{
			CreateLMatrixWithW2();
		}
		else if(m_SelectWeight == 2)
		{
			CreateLMatrixWithW3();
		}
		else
		{
			std::cout << "Error: Selected weight does not exist!\n";
		}

		//// Test Factorize Matrix
		//int* p = new int[numberOfAllPoints];
		//for(int i = 0; i < numberOfAllPoints; i++)
		//{
		//	p[i] = 0;
		//}
		//// vtkMath::LUFactorLinearSystem(double **A,int *index,int size) 	
		//if(vtkMath::LUFactorLinearSystem(m_LMatrix,p,numberOfAllPoints)  == 0)		// todo: es muessten beide matrizen factorisiert werden
		//{
		//	std::cout << "Error factor linear System!\n";
		//}


		// Add anchorOutsideROI rows to L-Matrix
		std::vector<vtkIdType>::iterator it3 = m_AnchorPointsOutsideROI.begin();
		for(int row = numberOfAllPoints; row < numberOfAllPoints+numberOfAnchorPointsOutsideROI; row++, it3++)
		{
			double* newCol = new double[numberOfAllPoints];
			m_LMatrixWithAnchors[row] = newCol;
			
			std::vector<vtkIdType>::iterator it4 = m_AllPoints.begin();
			for(int col = 0; col < numberOfAllPoints; col++, it4++)
			{
				m_LMatrixWithAnchors[row][col] = 0.0;
				if(*it3 == *it4)
				{
					m_LMatrixWithAnchors[row][col] = 1.0; 	
				}
			}
		}

		// Add handle anchor to L-Matrix
		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI] = newCol;
		for(int col = 0; col < numberOfAllPoints; col++)
		{
			m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI][col] = 0.0;
		}
		m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI][numberOfAllPoints-1] = 1.0;

		// print LMatrix
		if(0)
		{
			std::cout << "L-Matrix:\n";
			for(int row = 0; row < numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1; row++)
			{
				std::cout << "\n";
				for(int col = 0; col < numberOfAllPoints; col++)
				{		
					std::cout << m_LMatrixWithAnchors[row][col];
				}
			}
			std::cout << "Number of all Points: " << numberOfAllPoints << "\n";
		}
		
		if(m_Color)
			ColorNodes();
	}
	return ok;
}

void mitk::DragTool::CreateLMatrixWithW1()
{
	// set different weights for relation to neighbours
	double w = 1.0;	// slider for constanst weights (or factor: influence by neighbours), (w > 1: ueberschwinger?)

	int numberOfAllPoints = m_AllPoints.size();
	
	// Add points in ROI rows to L-Matrix
	std::vector<vtkIdType>::iterator it1 = m_AllPoints.begin();
	for(int row = 0; row < numberOfAllPoints; row++, it1++)
	{
		double totalDistanceInRow = 0.0;

		pdp::Pt pt1;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it1, pt1.toVtk());

		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[row] = newCol;
		double* newCol1 = new double[numberOfAllPoints];
		m_LMatrix[row] = newCol1;
		
		int neighbourCounter = 0;
		std::vector<vtkIdType>::iterator it2 = m_AllPoints.begin();
		for(int col = 0; col < numberOfAllPoints; col++, it2++)
		{		
			pdp::Pt pt2;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it2, pt2.toVtk());

			if(*it1 == *it2)
			{
				; // skip	
			}
			else if(m_Ext->GetSurfacePolyData()->IsEdge(*it1,*it2) > 0)
			{
				// Edge found -> insert -1 in LMatrix
				m_LMatrixWithAnchors[row][col] = -w;
				m_LMatrix[row][col] = -w;
				neighbourCounter++;
			}
			else
			{
				m_LMatrixWithAnchors[row][col] = 0;
				m_LMatrix[row][col] = 0;
			}
		}

		// Add diagonal entry
		m_LMatrixWithAnchors[row][row] = neighbourCounter;
		m_LMatrix[row][row] = neighbourCounter;
	}
}

void mitk::DragTool::CreateLMatrixWithW2()
{
	// set different weights for relation to neighbours
	// set distance to neighbours as weight
	// negative normalzied weight (divided by the sum of all weights from i)

	int numberOfAllPoints = m_AllPoints.size();

	// Add points in ROI rows to L-Matrix
	std::vector<vtkIdType>::iterator it1 = m_AllPoints.begin();
	for(int row = 0; row < numberOfAllPoints; row++, it1++)
	{
		double totalDistanceInRow = 0.0;

		pdp::Pt pt1;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it1, pt1.toVtk());

		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[row] = newCol;
		double* newCol1 = new double[numberOfAllPoints];
		m_LMatrix[row] = newCol1;
		
		int neighbourCounter = 0;
		std::vector<vtkIdType>::iterator it2 = m_AllPoints.begin();
		for(int col = 0; col < numberOfAllPoints; col++, it2++)
		{		
			pdp::Pt pt2;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it2, pt2.toVtk());

			if(*it1 == *it2)
			{
				; // skip	
			}
			else if(m_Ext->GetSurfacePolyData()->IsEdge(*it1,*it2) > 0)
			{
				// use distance as weight
				// it2 -> pt2, it1 -> pt1
				double distance = sqrt(vtkMath::Distance2BetweenPoints(pt1.toVtk(), pt2.toVtk())); 	
				totalDistanceInRow += distance;

				//std::cout << "Distance to neighbour: " << distance << "\n";

				m_LMatrixWithAnchors[row][col] = -distance;
				m_LMatrix[row][col] = -distance;

				neighbourCounter++;
			}
			else
			{
				m_LMatrixWithAnchors[row][col] = 0;
				m_LMatrix[row][col] = 0;
			}
		}

		// Add diagonal entry
		m_LMatrixWithAnchors[row][row] = neighbourCounter;
		m_LMatrix[row][row] = neighbourCounter;
				
		// if use distance as weight -> normalize weight to neighbours
		//std::cout << "Distance: Sum to all neighbours in this row: " << totalDistanceInRow << "\n";
		std::vector<vtkIdType>::iterator it3 = m_AllPoints.begin();
		for(int col = 0; col < numberOfAllPoints; col++, it3++)
		{		
			pdp::Pt pt3;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it3, pt3.toVtk());

			if(*it1 == *it3)
			{
				; // skip	
			}
			else if(m_Ext->GetSurfacePolyData()->IsEdge(*it1,*it3) > 0)
			{
				// normalize
				double normFactor = (neighbourCounter/totalDistanceInRow);

				m_LMatrixWithAnchors[row][col] *= normFactor;
				m_LMatrix[row][col] = m_LMatrix[row][col] *normFactor;

				//std::cout << "Normalization factor: " << normFactor << "\n";
				//std::cout << "Normalized: " << m_LMatrix[row][col] << "\n";							
			}
		}
	}
}

void mitk::DragTool::CreateLMatrixWithW3()
{
	//cotangent weights
	int numberOfAllPoints = m_AllPoints.size();

	// Add points in ROI rows to L-Matrix
	std::vector<vtkIdType>::iterator it1 = m_AllPoints.begin();
	for(int row = 0; row < numberOfAllPoints; row++, it1++)
	{
		double totalDistanceInRow = 0.0;

		pdp::Pt pt1;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it1, pt1.toVtk());

		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[row] = newCol;
		double* newCol1 = new double[numberOfAllPoints];
		m_LMatrix[row] = newCol1;
		
		int neighbourCounter = 0;
		std::vector<vtkIdType>::iterator it2 = m_AllPoints.begin();
		for(int col = 0; col < numberOfAllPoints; col++, it2++)
		{		
			pdp::Pt pt2;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it2, pt2.toVtk());

			if(*it1 == *it2)
			{
				; // skip	
			}
			else if(m_Ext->GetSurfacePolyData()->IsEdge(*it1,*it2) > 0)
			{
				// Edge found -> get all four points

				// Get both neighbouring cells to edge
				vtkSmartPointer<vtkIdList> cellIds1 = vtkSmartPointer<vtkIdList>::New();
				m_Ext->GetSurfacePolyData()->GetPointCells(*it1, cellIds1);
				vtkSmartPointer<vtkIdList> cellIds2 = vtkSmartPointer<vtkIdList>::New();
				m_Ext->GetSurfacePolyData()->GetPointCells(*it2, cellIds2);

				vtkSmartPointer<vtkIdList> cellsToEdge = vtkSmartPointer<vtkIdList>::New();	
				for(vtkIdType i = 0;  i < cellIds1->GetNumberOfIds(); i++)
				{
					for(vtkIdType j = 0;  j < cellIds2->GetNumberOfIds(); j++)
					{
						if(cellIds1->GetId(i) == cellIds2->GetId(j))
						{
							// found cell to both points
							cellsToEdge->InsertNextId(cellIds1->GetId(i)); 
						}
					}
				}
				//std::cout << "Number of cells next to edge: " << cellsToEdge->GetNumberOfIds() << "\n";
				if(cellsToEdge->GetNumberOfIds() != 2)
				{
					std::cout << "Error: Edge has more then two neighbouring cells.\n";
				}

				// Get third and fourth point
				vtkSmartPointer<vtkIdList> cellsPoints3 = vtkSmartPointer<vtkIdList>::New();	
				m_Ext->GetSurfacePolyData()->GetCellPoints(cellsToEdge->GetId(0),cellsPoints3);
				//std::cout << "Cell points 3: " << cellsPoints3->GetId(0) << " " << cellsPoints3->GetId(1) << " " << cellsPoints3->GetId(2) << "\n";
				vtkIdType point3Id;
				for(vtkIdType i = 0;  i < cellsPoints3->GetNumberOfIds(); i++)
				{
					if(cellsPoints3->GetId(i) != *it1 && cellsPoints3->GetId(i) != *it2)
					{
						point3Id = cellsPoints3->GetId(i);
					}
				}
				pdp::Pt pt3;
				m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(point3Id, pt3.toVtk());
								
				vtkSmartPointer<vtkIdList> cellsPoints4 = vtkSmartPointer<vtkIdList>::New();	
				m_Ext->GetSurfacePolyData()->GetCellPoints(cellsToEdge->GetId(1),cellsPoints4);
				//std::cout << "Cell points 3: " << cellsPoints4->GetId(0) << " " << cellsPoints4->GetId(1) << " " << cellsPoints4->GetId(2) << "\n";
				vtkIdType point4Id;
				for(vtkIdType i = 0;  i < cellsPoints4->GetNumberOfIds(); i++)
				{
					if(cellsPoints4->GetId(i) != *it1 && cellsPoints4->GetId(i) != *it2)
					{
						point4Id = cellsPoints4->GetId(i);
					}
				}
				pdp::Pt pt4;
				m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(point4Id, pt4.toVtk());

				// We want to calculate the tangens(theta) at point 2. 
				// Therefore we calculate the vector (point1 to point2) and the vectors (point 2 to point 3 (or 4))
				// We calculate the cos(theta) via dot product and can transfer the cos(theta) to the tan(theta/2)
				pdp::Pt ptIJ = pt1 - pt2;
				double ptIJLength = ptIJ.Length();
				pdp::Pt ptI3 = pt3 - pt2;
				pdp::Pt ptI4 = pt4 - pt2;

				// Calculate Cos(theta1) and cos(theta2)
				double cosTheta1 = vtkMath::Dot(ptIJ.toVtk(), ptI3.toVtk()) / (ptIJLength * ptI3.Length());
				double cosTheta2 = vtkMath::Dot(ptIJ.toVtk(), ptI4.toVtk()) / (ptIJLength * ptI4.Length());

				double tanTheta1Half = sqrt((1-cosTheta1) / (1+cosTheta1));
				double tanTheta2Half = sqrt((1-cosTheta2) / (1+cosTheta2));

				// Calculate weigth 
				double wIJ = (tanTheta1Half + tanTheta2Half) / ptIJLength;

				if(0)
				{

					//std::cout << "Four points: (1) id: " << *it1 << ", pos: " << pt1.toItk() << "\n";
					//std::cout << "Four points: (2) id: " << *it2 << ", pos: " << pt2.toItk() << "\n";
					//std::cout << "Four points: (3) id: " << point3Id << ", pos: " << pt3.toItk() << "\n";
					//std::cout << "Four points: (4) id: " << point4Id << ", pos: " << pt4.toItk() << "\n";

					std::cout << "Three vectors: (1), pos: " << ptIJ.toItk() << "\n";
					std::cout << "Three vectors: (2), pos: " << ptI3.toItk() << "\n";
					std::cout << "Three vectors: (3), pos: " << ptI4.toItk() << "\n";

					std::cout << "Cos(Theta1): " << cosTheta1 << "\n";
					std::cout << "Cos(Theta2): " << cosTheta2 << "\n";

					std::cout << "Vector ij length: " << ptIJLength << "\n";
					std::cout << "Vector i3 length: " << ptI3.Length() << "\n";
					std::cout << "Vector i4 length: " << ptI4.Length() << "\n";

					std::cout << "Weight: " << wIJ << "\n";	
					std::cout << "\n";
				}

				
				// -> set weight
				m_LMatrixWithAnchors[row][col] = -wIJ;
				m_LMatrix[row][col] = -wIJ;
				neighbourCounter++;
			}
			else
			{
				m_LMatrixWithAnchors[row][col] = 0;
				m_LMatrix[row][col] = 0;
			}
		}

		// Add diagonal entry
		m_LMatrixWithAnchors[row][row] = neighbourCounter;
		m_LMatrix[row][row] = neighbourCounter;

		std::cout << "\n";
	}

}

bool mitk::DragTool::LeftClickRelease(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	if(m_CursorOnSurface)
	{
		mitk::ApplicationCursor::GetInstance()->PopCursor();
	}

	return ok;
}

void mitk::DragTool::ROIRadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_ROIRadius = (((double)value+1.0)/5.0) * (((double)value+1.0)/5.0);
	std::cout << "ROI radius: " << m_ROIRadius << "\n";
	m_ContourSphere2->SetRadius(sqrt((double)m_ROIRadius)); // todo
	m_ContourSphere2->SetCenter(m_Ext->GetContourSphere()->GetCenter());
	m_ContourSphere2->Update();
	m_Ext->UpdateRenderer();
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::DragTool::SelectWeight(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Select weight: " << selection << "\n";
	m_SelectWeight = selection;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::DragTool::SetColors()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	if(m_Color)
	{
		m_Color = false;
		m_Ext->GetSurfaceNode()->SetBoolProperty("scalar visibility", false);
		m_Ext->UpdateRenderer();
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}
	else
	{
		m_Color = true;
		ColorNodes(); // todo, click once?		
		m_Ext->UpdateRenderer();
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::DragTool::ColorNodes()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	float minimum = 0.0;
	float maximum = 255.0;
	m_Ext->GetSurfaceNode()->SetFloatProperty("ScalarsRangeMinimum", minimum);
	m_Ext->GetSurfaceNode()->SetFloatProperty("ScalarsRangeMaximum", maximum);

	mitk::Color color;
	mitk::ColorProperty::Pointer colorProp;
	m_Ext->GetSurfaceNode()->GetProperty(colorProp,"color");
	color = colorProp->GetValue();

	int inputNumPoints2 = m_Ext->GetSurfacePolyData()->GetNumberOfPoints();
	//std::cout << "Number of Points: " << inputNumPoints2 << "\n";
	vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
	faceColors->SetNumberOfComponents(3);
	for(vtkIdType j = 0; j < inputNumPoints2; j++)
	{
		unsigned char red[3] = {0, 0, 255};
		unsigned char green[3] = {0, 255, 0};
		unsigned char blue[3] = {255, 0, 0};
		faceColors->SetName("Colors");

		// point is handle
		if(j == m_HandlePointID)
		{
			faceColors->InsertNextTupleValue(red);
			continue;
		}

		// point is inside Roi
		//bool cont1 = true;
		//for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
		//{
		//	if(j == *it)
		//	{
		//		faceColors->InsertNextTupleValue(green);
		//		cont1 = false;
		//	}
		//}
		//if(!cont1)
		//	continue;

		// point is anchor outside the ROI
		bool cont2 = true;
		for(std::vector<vtkIdType>::iterator it2 = m_AnchorPointsOutsideROI.begin(); it2 != m_AnchorPointsOutsideROI.end(); it2++)
		{
			if(j == *it2)
			{
				faceColors->InsertNextTupleValue(red);
				cont2 = false;
			}
		}
		if(!cont2)
			continue;

		faceColors->InsertNextTupleValue(blue);
	}
	m_Ext->GetSurfacePolyData()->GetPointData()->SetScalars(faceColors);		

	vtkSmartPointer<vtkLookupTable> vtkLUT = vtkSmartPointer<vtkLookupTable>::New();
	vtkLUT->SetNumberOfTableValues( 3 );
	vtkLUT->SetRange(0.0, 255.0);
	vtkLUT->SetTableValue( 0, 0.9, 0.1, 0.1 );
	vtkLUT->SetTableValue( 1, 0.1, 0.9, 0.1 );
	vtkLUT->SetTableValue( 2, 0.1, 0.1, 0.9 );
	vtkLUT->Build();

	mitk::LookupTable::Pointer lookupTable = mitk::LookupTable::New();
	lookupTable->SetVtkLookupTable(vtkLUT);
	mitk::LookupTableProperty::Pointer lookupTableProperty = mitk::LookupTableProperty::New(lookupTable);    
	m_Ext->GetSurfaceNode()->SetProperty( "LookupTable", lookupTableProperty );

	m_Ext->GetSurfaceNode()->SetBoolProperty("scalar visibility", true);
	m_Ext->GetSurfaceNode()->SetFloatProperty("ScalarsRangeMaximum", 3);
	m_Ext->GetSurfaceNode()->SetBoolProperty("color mode", true);
	m_Ext->GetSurfaceNode()->SetBoolProperty("use color", true);
	m_Ext->GetSurfaceNode()->Update();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::DragTool::UpdateRadiusViaInteractor(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//double radiusAddend = 5.0;
	if(value > 0)
	{
		m_ROIRadius += m_ROIRadius/10.0;
	}
	if(value < 0)
	{
		m_ROIRadius -= m_ROIRadius/10.0;
	}

	m_ContourSphere2->SetRadius(sqrt((double)m_ROIRadius));
	m_ContourSphere2->Update();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	// change radius slider
	//m_ROIRadius = (((double)value+1.0)/5.0) * (((double)value+1.0)/5.0);
	UpdateRadiusSlider2(sqrt(m_ROIRadius)*5-1);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::DragTool::Zoom(int zoom)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//int currentSlice1 =  m_Ext->GetMitkView()->GetRenderWindow1()->GetSliceNavigationController()->GetSlice()->GetPos();
	//int currentSlice2 =  m_Ext->GetMitkView()->GetRenderWindow2()->GetSliceNavigationController()->GetSlice()->GetPos();
	//int currentSlice3 =  m_Ext->GetMitkView()->GetRenderWindow3()->GetSliceNavigationController()->GetSlice()->GetPos();

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

	// test: old slice position is inside new bounding box?
	//m_Ext->GetMitkView()->GetRenderWindow1()->GetSliceNavigationController()->GetSlice()->SetPos(currentSlice1);
	//m_Ext->GetMitkView()->GetRenderWindow2()->GetSliceNavigationController()->GetSlice()->SetPos(currentSlice2);
	//m_Ext->GetMitkView()->GetRenderWindow3()->GetSliceNavigationController()->GetSlice()->SetPos(currentSlice3);

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}