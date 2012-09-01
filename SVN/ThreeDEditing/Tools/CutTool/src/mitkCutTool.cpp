// "pdp"
#include "mitkCutTool.h"
#include "mitkCutTool.xpm"
#include <../../../../RenderingManagerProperty.h>

// MITK
#include "mitkToolManager.h"
#include "mitkPointSet.h"
#include <mitkStateEvent.h>
#include <mitkSurfaceOperation.h>
#include "mitkInteractionConst.h"
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>

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


mitk::CutTool::CutTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_SurfaceHadBeenHit = false;
	m_SelectCells = false;
	m_AddCells = true;

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

mitk::CutTool::~CutTool()
{}

const char** mitk::CutTool::GetXPM() const
{
	return mitkCutTool_xpm;
}

const char* mitk::CutTool::GetName() const
{
	return "Cut Tool";
}

const char* mitk::CutTool::GetGroup() const
{
	return "default";
}

void mitk::CutTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::CutTool, int>(this, &mitk::CutTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::CutTool, int>(this, &mitk::CutTool::UpdateRadiusSlider);
}

void mitk::CutTool::Deactivated()
{
	std::cout << "Deactivated.\n";

	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::CutTool, int>(this, &mitk::CutTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::CutTool, int>(this, &mitk::CutTool::UpdateRadiusSlider);
}

void mitk::CutTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::CutTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
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

void mitk::CutTool::CutVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Set first snapshot
	vtkPolyData* vtkdata = vtkPolyData::New();
	vtkdata->DeepCopy(m_Ext->GetSurfacePolyData());
	mitk::Surface::Pointer oldSurface = mitk::Surface::New();
	oldSurface->SetVtkPolyData(vtkdata);

	vtkClipPolyData *clip = vtkClipPolyData::New();
	clip->AddInput(m_Ext->GetSurfacePolyData());
	//clip->AddInput(m_Ball->GetVtkPolyData());
	
	vtkPlane *plane=vtkPlane::New(); 
	plane->SetOrigin(0.25, 0, 0); 
	plane->SetNormal(-1, 1, 0); 

	//clip->SetClipFunction(plane); 
	//clip->SetClipFunction(m_Ball->GetVtkPolyData());

	vtkSphere *sphere1 = vtkSphere::New();
	sphere1->SetRadius(m_Ext->GetContourSphere()->GetRadius());
	sphere1->SetCenter(m_Ext->GetContourSphere()->GetCenter());

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
		m_Ext->GetDataStorage()->Add(outNode);
	}

	if(0)
	{
		vtkPolyData* outputCopy2 = clip->GetOutput(1);
		mitk::Surface::Pointer outSurface2 = mitk::Surface::New();
		outSurface2->SetVtkPolyData(outputCopy2);
		mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
		outNode2->SetData(outSurface2);
		outNode2->SetName("Appended Surfaces 2");
		m_Ext->GetDataStorage()->Add(outNode2);
	}

	m_Ext->SetSurfacePolyData(clip->GetOutput(0));
	UpdateRenderer();

	// set second snapshot 
	mitk::SurfaceOperation* doOp = new SurfaceOperation( mitk::OpSURFACECHANGED, m_Ext->GetSurfacePolyData(), 0);
	if ( m_UndoEnabled )
	{
		mitk::SurfaceOperation* undoOp = new SurfaceOperation( mitk::OpSURFACECHANGED, oldSurface->GetVtkPolyData(), 0);
		mitk::OperationEvent* operationEvent = new OperationEvent(m_Ext->GetSurface(), doOp, undoOp, "VOI cut in surface");
		mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
		operationEvent->IncCurrObjectEventId();
	}
	m_Ext->GetSurface()->ExecuteOperation(doOp);

	m_Ext->GetSurface()->Update();
	m_Ext->GetSurfaceNode()->Modified();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
void mitk::CutTool::SelectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == true && m_AddCells == false)
	{
		// change cell selection to select
		m_AddCells = true;
	}
	else if (m_SelectCells == true && m_AddCells == true)
	{
		// exit cell selection
		m_SelectCells = false;
		m_Ext->SetCalcIntersection(true);

		// Warum geht das nicht?
		//m_DataStorage->Remove(m_IntersectionNode);
		//m_DataStorage->Remove(m_BallNode);
		//m_DataStorage->Remove(m_PositionTrackerNode);
	}
	else
	{
		// start cell selection with select-mode
		m_SelectCells = true;
		m_Ext->SetCalcIntersection(false);
		m_AddCells = true;

		//m_DataStorage->Add(m_IntersectionNode);
		//m_DataStorage->Add(m_BallNode);
		//m_DataStorage->Add(m_PositionTrackerNode);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::CutTool::DeselectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_SelectCells == true && m_AddCells == true)
	{
		// change cell selection to deselect
		m_AddCells = false;
	}
	else if(m_SelectCells == true && m_AddCells == false)
	{
		// exit cell selection
		m_SelectCells = false;
		m_Ext->SetCalcIntersection(true);

		// Warum geht das nicht?
		//m_DataStorage->Remove(m_IntersectionNode);
		//m_DataStorage->Remove(m_BallNode);
		//m_DataStorage->Remove(m_PositionTrackerNode);
	}
	else 
	{
		// start cell selection with deselect-mode
		m_SelectCells = true;
		m_Ext->SetCalcIntersection(false);
		m_AddCells = false;

		//m_DataStorage->Add(m_IntersectionNode);
		//m_DataStorage->Add(m_BallNode);
		//m_DataStorage->Add(m_PositionTrackerNode);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::CutTool::ProcessSelectCells(mitk::Point3D worldPoint)
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

	// Test: Display surfaceCentralHitPoint as point-node in renderer
	if(0)
	{
		mitk::PointSet::Pointer surfaceCentralHitPointSet = mitk::PointSet::New();
		mitk::PointSet::PointType point;
		point[0] = worldPoint[0];
		point[1] = worldPoint[1];
		point[2] = worldPoint[2];
		mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
		pointSetNode->SetData(surfaceCentralHitPointSet);
		pointSetNode->SetName("SurfaceHitPoint");
		// Add the node to the tree
		m_Ext->GetDataStorage()->Add(pointSetNode);
		// add closed point to pointset
		point[0] = closestPoint[0];
		point[1] = closestPoint[1];
		point[2] = closestPoint[2];
		surfaceCentralHitPointSet->InsertPoint(0,point);
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
		for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
		{
			faceColors->InsertTuple3(*it,0, 255, 0);
		}
		m_Ext->GetSurfacePolyData()->GetCellData()->SetScalars(faceColors);

		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}


	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::CutTool::CutSelection()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Forum Thread: easiest to rebuild polydata from scratch
	vtkSmartPointer<vtkPolyData> newSurface = vtkSmartPointer<vtkPolyData>::New();
	newSurface->Allocate(10000,10000); 
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->Allocate(10000);

	vtkCellArray* cells = m_Ext->GetSurfacePolyData()->GetPolys();
	vtkIdType *pts, npts, cellId;

	for(cellId=0, cells->InitTraversal(); cells->GetNextCell(npts,pts); cellId++)
	{
		if(!std::binary_search(m_SelectedCells.begin(),m_SelectedCells.end(), cellId))
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

	if(0) // print parameters
	{
		std::cout << "Vector Size: " << m_SelectedCells.size() << "\n";
		for(std::vector<vtkIdType>::iterator it = m_SelectedCells.begin(); it != m_SelectedCells.end(); it++)
		{
			std::cout << "Vector Cells: " << *it << "\n";
		}
		std::cout << "New surface number of cells: " << newSurface->GetNumberOfCells() << "\n";
	}

	// add new surface to datastorage
	if(0)
	{
		mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
		mitkSurface->SetVtkPolyData(newSurface);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(mitkSurface);
		outNode->SetName("New Surface");
		m_Ext->GetDataStorage()->Add(outNode);
	}

	m_Ext->SetSurfacePolyData(newSurface);

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();


	if(0)
	{
		vtkSmartPointer<vtkIdTypeArray> ids =
		vtkSmartPointer<vtkIdTypeArray>::New();
		ids->SetNumberOfComponents(1);
		vtkCellArray* cells = m_Ext->GetSurfacePolyData()->GetPolys();
		vtkIdType *pts, npts, cellId;
		for(cellId=0, cells->InitTraversal(); cells->GetNextCell(npts,pts); cellId++)
		{
			if(!std::binary_search(m_SelectedCells.begin(),m_SelectedCells.end(), cellId))
			{
				ids->InsertNextValue(cellId);
				std::cout << "Cell added to new surface!\n";
			}
		}

		std::cout << "Vector Size: " << m_SelectedCells.size() << "\n";
		std::cout << "Number of cells in surface: " << m_Ext->GetSurfacePolyData()->GetNumberOfCells() << "\n";

		//ids->InsertNextValue(4);
		//ids->InsertNextValue(5);
		//ids->InsertNextValue(6);
		//ids->InsertNextValue(7);
		
		vtkSmartPointer<vtkSelectionNode> selectionNode =
		vtkSmartPointer<vtkSelectionNode>::New();
		selectionNode->SetFieldType(vtkSelectionNode::CELL);
		//selectionNode->SetFieldType(vtkSelectionNode::POINT);
		selectionNode->SetContentType(vtkSelectionNode::INDICES);
		selectionNode->SetSelectionList(ids);

		vtkSmartPointer<vtkSelection> selection =
		vtkSmartPointer<vtkSelection>::New();
		selection->AddNode(selectionNode);

		vtkExtractSelection* ex = vtkExtractSelection::New();
		ex->SetInput(0, m_Ext->GetSurfacePolyData());
		ex->SetInput(1, selection);
		ex->Update();

		vtkSmartPointer<vtkUnstructuredGrid> selected =
		vtkSmartPointer<vtkUnstructuredGrid>::New();
		selected->ShallowCopy(ex->GetOutput());

		vtkPolyData* newSurface2poly = (vtkPolyData*)ex->GetOutput();
		vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
		removeDuplicatedPoints->SetInput(newSurface2poly);
		removeDuplicatedPoints->Update();
		newSurface2poly = removeDuplicatedPoints->GetOutput();
		
		mitk::Surface::Pointer mitkSurface2 = mitk::Surface::New();
		//mitkSurface2->SetVtkPolyData(vtkPolyData::SafeDownCast(ex->GetOutput()));
		//mitkSurface2->SetVtkPolyData((vtkPolyData*)(ex->GetOutput()));
		mitkSurface2->SetVtkPolyData(newSurface2poly);
		mitk::DataNode::Pointer outNode2 = mitk::DataNode::New();
		outNode2->SetData(mitkSurface2);
		outNode2->SetName("New Surface 2");
		m_Ext->GetDataStorage()->Add(outNode2);

		mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}


	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}