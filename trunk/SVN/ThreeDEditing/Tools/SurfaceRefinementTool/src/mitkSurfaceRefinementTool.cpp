// "pdp"
#include "mitkSurfaceRefinementTool.h"
#include "mitkSurfaceRefinementTool.xpm"
#include <../../../../RenderingManagerProperty.h>

// MITK
#include "mitkToolManager.h"
#include "mitkPointSet.h"
#include <mitkStateEvent.h>

// VTK
#include <vtkAppendPolyData.h>
#include <vtkClipPolyData.h>
#include <vtkSphere.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkCellLocator.h>
#include <vtkCellData.h>
#include <vtkPolyDataNormals.h>

#include <vtkButterflySubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkLinearSubdivisionFilter.h>

mitk::SurfaceRefinementTool::SurfaceRefinementTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_SurfaceHadBeenHit = false;
	m_SubdivisionMethode = 0; // linear subdivision
	m_SubdivisionMethodeROI = 0; // by point
}

mitk::SurfaceRefinementTool::~SurfaceRefinementTool()
{}

const char** mitk::SurfaceRefinementTool::GetXPM() const
{
	return mitkSurfaceRefinementTool_xpm;
}

const char* mitk::SurfaceRefinementTool::GetName() const
{
	return "Refinement Tool";
}

const char* mitk::SurfaceRefinementTool::GetGroup() const
{
	return "default";
}

void mitk::SurfaceRefinementTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::SurfaceRefinementTool, int>(this, &mitk::SurfaceRefinementTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::SurfaceRefinementTool, int>(this, &mitk::SurfaceRefinementTool::UpdateRadiusSlider);
}

void mitk::SurfaceRefinementTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::SurfaceRefinementTool, int>(this, &mitk::SurfaceRefinementTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::SurfaceRefinementTool, int>(this, &mitk::SurfaceRefinementTool::UpdateRadiusSlider);
}

void mitk::SurfaceRefinementTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::SurfaceRefinementTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
		
	const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
	mitk::Point3D worldPoint = displayPositionEvent->GetWorldPosition();
	if(m_SelectCells)
	{
		ProcessSelectCells(worldPoint);
	}

	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SelectRefinementMethode(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Subdivision Methode: " << selection << "\n";
	m_SubdivisionMethode = selection;

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SelectRefinementMethodeROI(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Subdivision Methode: " << selection << "\n";
	m_SubdivisionMethodeROI = selection;

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SelectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == true && m_AddCells == false)
	{
		// change cell selection to select
		m_AddCells = true;

		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		ColorCells(m_SelectedCells);
	}
	else if (m_SelectCells == true && m_AddCells == true)
	{
		// exit cell selection
		m_Ext->GetIntersectionNode()->SetVisibility(true);
		m_Ext->GetBallNode()->SetVisibility(true);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		m_SelectCells = false;
		m_Ext->SetCalcIntersection(true);

		UnColorSurface();
	}
	else
	{
		// start cell selection with select-mode
		m_SelectCells = true;
		m_Ext->SetCalcIntersection(false);
		m_AddCells = true;

		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		ColorCells(m_SelectedCells);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::DeselectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == true && m_AddCells == true)
	{
		// change cell selection to deselect
		m_AddCells = false;

		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		ColorCells(m_SelectedCells);
	}
	else if(m_SelectCells == true && m_AddCells == false)
	{
		// exit cell selection
		m_SelectCells = false;
		m_Ext->SetCalcIntersection(true);

		m_Ext->GetIntersectionNode()->SetVisibility(true);
		m_Ext->GetBallNode()->SetVisibility(true);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		UnColorSurface();
	}
	else 
	{
		// start cell selection with deselect-mode
		m_SelectCells = true;
		m_Ext->SetCalcIntersection(false);
		m_AddCells = false;

		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

		ColorCells(m_SelectedCells);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SelectAllCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == false)
	{
		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);		
		m_Ext->SetCalcIntersection(false);
		m_SelectCells = true;
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();		
	}

	m_SelectedCells.clear();
	int numberOfCellsInSurface = m_Ext->GetSurfacePolyData()->GetNumberOfCells();

	for(int i = 0; i < numberOfCellsInSurface; i++)
	{
		m_SelectedCells.push_back(i);
	}
	
	m_AddCells = false;
	ToogleDeselectCellsButton();

	ColorCells(m_SelectedCells);

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::DeselectAllCells()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == false)
	{
		m_Ext->GetIntersectionNode()->SetVisibility(false);
		m_Ext->GetBallNode()->SetVisibility(false);
		m_Ext->SetCalcIntersection(false);
		m_SelectCells = true;		
		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();		
	}

	m_SelectedCells.clear();
	ColorCells(m_SelectedCells);
	m_AddCells = true;
	ToogleSelectCellsButton();

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::ProcessSelectCells(mitk::Point3D worldPoint)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "World point: " << worldPoint << "\n";

	double worldPointDouble [3];
	worldPointDouble[0] = worldPoint[0];
	worldPointDouble[1] = worldPoint[1];
	worldPointDouble[2] = worldPoint[2];
	
	// Get surface cell to world point
	vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
	cellLocator->SetDataSet(m_Ext->GetSurfacePolyData());
	cellLocator->BuildLocator();

	//Find the closest points to surfaceCentralHitPoint
	double closestPoint[3];//the coordinates of the closest point will be returned here
	double closestPointDist2; //the squared distance to the closest point will be returned here
	vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
	int subId; //this is rarely used (in triangle strips only, I believe)
	cellLocator->FindClosestPoint(worldPointDouble, closestPoint, cellId, subId, closestPointDist2);
	// std::cout << "Distance to closest point in surface: " << closestPointDist2 << "\n";

	int tmp = (int) cellId;
	std::sort(m_SelectedCells.begin(),m_SelectedCells.end());

	//std::cout << "Vector size before (de)selection: " << m_SelectedCells.size() << "\n";

	if(m_AddCells)
	{
		// add new cells to list
		if(!std::binary_search(m_SelectedCells.begin(),m_SelectedCells.end(), cellId))
		{
			
			//std::cout << "Current CellID: " << cellId << "\n";
			for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
			{
				//std::cout << "Vector Cells: " << *it << "\n";
			}
			m_SelectedCells.push_back(cellId);
		}
	}
	else
	{
		// deselect cells -> remove from vector
		if(std::binary_search(m_SelectedCells.begin(),m_SelectedCells.end(), cellId))
		{
			for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
			{
				if(*it == tmp)
				{
					// entry found -> erase
					m_SelectedCells.erase(it);
					break;
				}				
			}
		}
	}

	ColorCells(m_SelectedCells);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SubdivideSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Try VTK subdivision filters
	vtkSmartPointer<vtkPolyDataAlgorithm> subdivisionFilter;
    switch(m_SubdivisionMethode)
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

	subdivisionFilter->SetInput(m_Ext->GetSurfacePolyData());
    //subdivisionFilter->SetNumberOfSubdivisions(2);
    subdivisionFilter->Update();
	m_Ext->SetSurfacePolyData(subdivisionFilter->GetOutput());
	UpdateRenderer();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SubdivideSurfaceVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Extend subdivide VOI abilities:
	// Problem: All three subdivision filters need an initial point in the mesh. They cannot work on single cells without a point inside.
	// Solution:	1) Check if there is one or more point(s) in the VOI
	//				2) Insert point if necessary
	//				3) Use subdivision algorithm if wanted 

	if(m_SelectCells == false) // Use MoveROI interactor
	{
		m_SelectedCells.clear();

		// Check for point(s) inside the VOI
		bool pointsAreInVOI = false;
		vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
		double* center = m_Ext->GetBall()->GetVtkPolyData()->GetCenter();
		for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
		{
			double pt[3];
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt);			
			// Use distance			
			double distance = sqrt((center[0]-pt[0])*(center[0]-pt[0])+(center[1]-pt[1])*(center[1]-pt[1])+(center[2]-pt[2])*(center[2]-pt[2]));
			if(distance < m_Ext->GetRadius())
			{
				pointsAreInVOI = true;
				m_SelectedCells.push_back(ptId);
			}
		}

		if(pointsAreInVOI == true)
		{
			std::cout << "There are one or more point(s) inside the VOI.\n";
			switch(m_SubdivisionMethodeROI)
			{
			  case 0:
				{
					// TODO: Subdivide by point 
					SubdivideByPoint();		
				}
				
				break;
			  case 1:
				{
					// Subdivide edge
					SubdivideByEdge();					
				}
				break;
			  default:
				break;
			}
		}		
		else
		{
			// Insert point(s)
			std::cout << "There is no point inside the VOI. Subdivision filter will fail. Insert new point first.\n";

			// Test: Construct unsymmetric contour to test centroid function
			//vtkSmartPointer<vtkPolyData> linesPolyData = m_Ext->CreateTestContour();

			double weightSum = 0.0;
			//int numberOfPoints = m_IntersectionData->GetVtkPolyData()->GetNumberOfLines() * 2;
			double currentWeight;
			double centroid[3] = {0.0, 0.0, 0.0};

			vtkIdType ntps, *pts;
			//linesPolyData->GetLines()->InitTraversal();
			//while(linesPolyData->GetLines()->GetNextCell(ntps, pts))
			m_Ext->GetIntersectionData()->GetVtkPolyData()->GetLines()->InitTraversal();
			while(m_Ext->GetIntersectionData()->GetVtkPolyData()->GetLines()->GetNextCell(ntps, pts))
			{
				if(ntps != 2)
					std::cout << "Error: Number of points in intersection contour line != 2.\n";

				double pt1[3], pt2[3];
				m_Ext->GetIntersectionData()->GetVtkPolyData()->GetPoints()->GetPoint(pts[0], pt1);
				m_Ext->GetIntersectionData()->GetVtkPolyData()->GetPoints()->GetPoint(pts[1], pt2);
				//linesPolyData->GetPoints()->GetPoint(pts[0], pt1);
				//linesPolyData->GetPoints()->GetPoint(pts[1], pt2);

				currentWeight = sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2));
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
					std::cout << "Distance between two points: " << sqrt(vtkMath::Distance2BetweenPoints(pt1, pt2)) << "\n";
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
				m_Ext->GetDataStorage()->Add(pointSetNode);
			}

			// Add centroid point to surface
			// Get Cell to surfaceCentralHitPoint
			vtkSmartPointer<vtkCellLocator> cellLocator = 
			vtkSmartPointer<vtkCellLocator>::New();
			cellLocator->SetDataSet(m_Ext->GetSurfacePolyData());
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
			m_Ext->GetSurfacePolyData()->GetCellPoints(cellId, ntps, pts);
			std::cout << "Number of Points in Cell: " << ntps << "\n";
			if(ntps != 3)
				std::cout << "Error -> Triangulate!\n"; // TODO
			double pt1[3], pt2[3], pt3[3];
			m_Ext->GetSurfacePolyData()->GetPoint(pts[0], pt1);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[1], pt2);
			m_Ext->GetSurfacePolyData()->GetPoint(pts[2], pt3);
			
			// Delete hit cell
			m_Ext->GetSurfacePolyData()->DeleteCell(cellId);
			m_Ext->GetSurfacePolyData()->RemoveDeletedCells(); 	
			m_Ext->GetSurfacePolyData()->Update();

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
		}		
	}

	if(m_SelectCells == true)
	{
		if(!m_SelectedCells.empty())
		{
			switch(m_SubdivisionMethodeROI)
			{
			  case 0:
				{
					// TODO: Subdivide by point 
					SubdivideByPoint();		
				}
				
				break;
			  case 1:
				{
					// Subdivide edge
					SubdivideByEdge();					
				}
				break;
			  default:
				break;
			}
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SubdivideByEdge()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::vector<vtkIdType> cellsToBeRemoved;
	// For all selected cells:
	for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
	{
		if(!std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), *it))
		{
			std::cout << "Subdivide cell: " << *it << "\n";

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
				std::cout << "Edge 1.\n";
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
				std::cout << "Edge 2.\n";
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
				std::cout << "Edge 3.\n";
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

			std::cout << "Current Cell: " << *it << "\n";
			std::cout << "first: " << first << "\n";			

			// If one of the cells has already been modified don't modify
			std::cout << "Cells to be removed: ";
			for(std::vector<vtkIdType>::iterator it2 = cellsToBeRemoved.begin(); it2 != cellsToBeRemoved.end(); it2++)
			{
				std::cout << *it2 << " ";
			}
			std::cout << "\n";

			// if(std::binary_search(cellsToBeRemoved.begin(),cellsToBeRemoved.end(), *it)) -> tuts nicht
			bool skipLoop = false; 
			for(std::vector<vtkIdType>::iterator it2 = cellsToBeRemoved.begin(); it2 != cellsToBeRemoved.end(); it2++)
			{
				if(*it == *it2)
				{
					std::cout << "Cell: " << *it << " has already been modified.\n";
					skipLoop = true;
					break;
				}
				if(first == *it2)
				{						
					std::cout << "Cell: " << first << " has already been modified.\n";
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
				std::cout << "pt4 = tmp1\n";
			}
			if(!m_Ext->UniquePoint(pt1, tmp2) && !m_Ext->UniquePoint(pt2, tmp2) && !m_Ext->UniquePoint(pt3, tmp2))
			{
				pt4[0] = tmp2[0];
				pt4[1] = tmp2[1];
				pt4[2] = tmp2[2];
				std::cout << "pt4 = tmp2\n";
			}
			if(!m_Ext->UniquePoint(pt1, tmp3) && !m_Ext->UniquePoint(pt2, tmp3) && !m_Ext->UniquePoint(pt3, tmp3))
			{
				pt4[0] = tmp3[0];
				pt4[1] = tmp3[1];
				pt4[2] = tmp3[2];
				std::cout << "pt4 = tmp3\n";
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
				std::cout << "Point distribution: 1\n";
				pt1Id = points->InsertNextPoint(pt1);
				pt2Id = points->InsertNextPoint(pt2);
				pt3Id = points->InsertNextPoint(pt3);
			}
			else if(pointDistribution == 2)
			{
				std::cout << "Point distribution: 2\n";
				pt1Id = points->InsertNextPoint(pt1);						
				pt2Id = points->InsertNextPoint(pt3);
				pt3Id = points->InsertNextPoint(pt2);
			}
			else
			{
				std::cout << "Point distribution: 3\n";
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
			std::cout << "Cell is already subdivided, due to neighbour cells!\n";
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

	vtkCellArray* cells = m_Ext->GetSurfacePolyData()->GetPolys();
	vtkIdType *pts, npts, cellId;

	for(cellId=0, cells->InitTraversal(); cells->GetNextCell(npts,pts); cellId++)
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

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::SubdivideByPoint()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::vector<vtkIdType> cellsToBeRemoved;
	// For all selected cells:
	for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
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
		std::cout << "Centroid: " << centroid[0] << " " << centroid[0] << " " << centroid[0] << "\n";

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

	vtkCellArray* cells = m_Ext->GetSurfacePolyData()->GetPolys();
	vtkIdType *pts, npts, cellId;

	for(cellId=0, cells->InitTraversal(); cells->GetNextCell(npts,pts); cellId++)
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

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::ColorCells(std::vector<vtkIdType> cells)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_Ext->GetSurfaceNode()->SetBoolProperty("scalar visibility", true);
	m_Ext->GetSurfaceNode()->SetBoolProperty("color mode", true);

	vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
	faceColors->SetNumberOfComponents(3);
	vtkIdType inputNumCells = m_Ext->GetSurfacePolyData()->GetNumberOfCells();
	for(int i = 0; i < inputNumCells; i++)
	{
		faceColors->InsertNextTuple3(255, 0, 0);
	}
	for(std::vector<vtkIdType>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		faceColors->InsertTuple3(*it,0, 255, 0);
	}
	m_Ext->GetSurfacePolyData()->GetCellData()->SetScalars(faceColors);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::UnColorSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_Ext->GetSurfaceNode()->SetBoolProperty("scalar visibility", false);
	m_Ext->GetSurfaceNode()->SetBoolProperty("color mode", false);
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::SurfaceRefinementTool::ResetNormals()
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