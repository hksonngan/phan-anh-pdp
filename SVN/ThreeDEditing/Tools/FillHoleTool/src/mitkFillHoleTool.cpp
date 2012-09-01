// "pdp"
#include "mitkFillHoleTool.h"
#include "mitkFillHoleTool.xpm"
#include "QmitkFillHoleToolGUI.h"

// MITK
#include "mitkToolManager.h"

// VTK
#include <vtkAppendPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkFillHolesFilter.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkLine.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkExtractEdges.h>
#include <vtkStripper.h>


#include "itkVTKPolyDataWriter.h"


mitk::FillHoleTool::FillHoleTool()
:Tool("PositionTracker")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_Ut = new pdp::Utilities(m_Ext);
}

mitk::FillHoleTool::~FillHoleTool()
{}

const char** mitk::FillHoleTool::GetXPM() const
{
	return mitkFillHoleTool_xpm;
}

const char* mitk::FillHoleTool::GetName() const
{
	return "Fill Hole Tool";
}

const char* mitk::FillHoleTool::GetGroup() const
{
	return "default";
}

void mitk::FillHoleTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::FillHoleTool, int>(this, &mitk::FillHoleTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::FillHoleTool, int>(this, &mitk::FillHoleTool::UpdateRadiusSlider);
}

void mitk::FillHoleTool::Deactivated()
{
	m_Ext->DeactivateTool();

	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::FillHoleTool, int>(this, &mitk::FillHoleTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::FillHoleTool, int>(this, &mitk::FillHoleTool::UpdateRadiusSlider);
}

void mitk::FillHoleTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::FillHoleTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = m_Ext->OnMouseMoved(action, stateEvent);
	
	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void mitk::FillHoleTool::DetectHoles()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	vtkSmartPointer<vtkFeatureEdges> featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
	featureEdges->SetInput(m_Ext->GetSurface()->GetVtkPolyData());
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
	outNode->SetName("Surface-Holes");
	m_Ext->GetDataStorage()->Add(outNode);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::FillHoleTool::FillHoles()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

if(0){
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
	triangleFilter->SetInput(m_Ext->GetSurface()->GetVtkPolyData());
	triangleFilter->Update();
	m_Ext->GetSurface()->SetVtkPolyData(triangleFilter->GetOutput(0));



	
	vtkIdType inputNumCells = m_Ext->GetSurface()->GetVtkPolyData()->GetNumberOfCells();
	//std::cout << "Number of Cells in Surface: " << inputNumCells << "\n";

	vtkPolyData* input = m_Ext->GetSurface()->GetVtkPolyData();
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
	vtkIdType *pts, npts, cellId, numNei, p1, p2;
	
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
	if(0)
	{
		vtkSmartPointer<vtkPolyData> outputCopy2 = vtkSmartPointer<vtkPolyData>::New();

		vtkSmartPointer<vtkFeatureEdges> featureEdges =
		vtkSmartPointer<vtkFeatureEdges>::New();
		//featureEdges->SetInputConnection(diskSource->GetOutputPort());
		featureEdges->SetInput(m_Ext->GetSurfacePolyData());
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
		m_Ext->GetDataStorage()->Add(outNode);

		//std::cout << "There are " << outputCopy->GetNumberOfLines() << " lines." << std::endl;

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
			m_Ext->GetDataStorage()->Add(outNode3);

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
		m_Ext->GetDataStorage()->Add(outNode2);

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
}

	// try VTK functions
	if(1)
	{
		//vtkPolyData* outputCopy;
		if(1)
		{
			vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter = vtkSmartPointer<vtkFillHolesFilter>::New();

			std::cout << "Fill Holes Filter Input Data: " << m_Ext->GetSurfaceNode()->GetName() << "\n";

			fillHolesFilter->SetInput(m_Ext->GetSurface()->GetVtkPolyData());

			double maxHoleSize = 10000.0;
			//double maxHoleSize = 20.0;
			fillHolesFilter->SetHoleSize(maxHoleSize);

			fillHolesFilter->Update();

			std::cout << "Hole Size: " << fillHolesFilter->GetHoleSize() << "\n";
			std::cout << "Number of Output Ports: " << fillHolesFilter->GetNumberOfOutputPorts() << "\n";
			
			m_Ext->GetSurface()->SetVtkPolyData(fillHolesFilter->GetOutput());

			// Remove duplicated points
			//vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
			//removeDuplicatedPoints->SetInput(m_Surface->GetVtkPolyData());
			//removeDuplicatedPoints->Update();
			//m_Surface->SetVtkPolyData(removeDuplicatedPoints->GetOutput());

			// Clean Poly Data
			//vtkSmartPointer<vtkCleanPolyData> filledContours = vtkSmartPointer<vtkCleanPolyData>::New(); // Filter tuts! 
			//filledContours->SetInput(m_Surface->GetVtkPolyData());
			//filledContours->Update();
			//m_Surface->SetVtkPolyData(filledContours->GetOutput());

			// Triangulate
			//vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
			////triangleFilter->SetInput(removeDuplicatedPoints->GetOutput());
			//triangleFilter->SetInput(m_Surface->GetVtkPolyData());
			//triangleFilter->Update();
			//m_Surface->SetVtkPolyData(triangleFilter->GetOutput(0));


			/*mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(outputCopy);
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Surface with patched hole.");
			m_DataStorage->Add(outNode);*/

			//m_Surface->Update();
			//m_SurfaceNode->SetProperty("color",mitk::ColorProperty::New(1.0/255.0, 254.0/255.0, 1.0/255.0));
			m_Ext->GetSurfacePolyData()->BuildCells();
			m_Ext->GetSurfacePolyData()->BuildLinks();
			UpdateRenderer();
		}
		if(0)
		{
			// TODO
			vtkSmartPointer<vtkSurfaceReconstructionFilter> surf = 
			vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();
			surf->SetInput(m_Ext->GetSurfacePolyData());
			vtkImageData* tmp = surf->GetOutput();
			mitk::Image::Pointer outImage = mitk::Image::New();
			outImage->Initialize(tmp);
			
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outImage);
			outNode->SetName("Surface with patched hole.");
			m_Ext->GetDataStorage()->Add(outNode);
			//outputCopy = surf->GetOutput();
		}

		// TODO
		if(0)
		{
			vtkSmartPointer<vtkExtractEdges> extractEdges = vtkSmartPointer<vtkExtractEdges>::New();
			extractEdges->SetInput(m_Ext->GetSurfacePolyData());
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
			append->AddInput(m_Ext->GetSurfacePolyData());
			append->AddInput(triangleFilter->GetOutput());
			append->Update();

			mitk::Surface::Pointer outSurface = mitk::Surface::New();
			outSurface->SetVtkPolyData(append->GetOutput());
			mitk::DataNode::Pointer outNode = mitk::DataNode::New();
			outNode->SetData(outSurface);
			outNode->SetName("Surface with patched hole.");
			m_Ext->GetDataStorage()->Add(outNode);
		}



		//vtkPolyData* outputCopy = fillHolesFilter->GetOutput(0);
		
		//mitk::Surface::Pointer outSurface = mitk::Surface::New();
		//outSurface->SetVtkPolyData(outputCopy);


		//mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		//outNode->SetData(outSurface);
		//outNode->SetName("Surface with patched hole.");
		//m_DataStorage->Add(outNode);

	}

	// Try to change show normal behaviour
	//m_Surface->GetVtkPolyData()->GetCellData()->

	// Pick nearest Surface Point from position event -> calc normal
	vtkSmartPointer<vtkPolyDataNormals> skinNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	skinNormals->SetInput(m_Ext->GetSurfacePolyData());
	//skinNormals->SetInputConnection(m_Surface->GetVtkPolyData());
	skinNormals->ComputePointNormalsOn();
	skinNormals->ComputeCellNormalsOff();
	skinNormals->SetFeatureAngle(60.0);
	skinNormals->Update();
	m_Ext->SetSurfacePolyData(skinNormals->GetOutput(0));


	//m_MitkView->showNormal();
	//m_MitkView->setAutoFillBackground(true);
	//m_MitkView->changeLayoutToBig3D();
	//m_MitkView->GetRenderWindow4()->
	
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	//UpdateRenderer();

	m_Ext->GetIntersectionPolyDataFilter()->SetInput(0,  m_Ext->GetSurfacePolyData());

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::FillHoleTool::FillHolesVOI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Triangulate
	vtkSmartPointer<vtkTriangleFilter> triangleFilter =
    vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInput(m_Ext->GetSurfacePolyData());
	triangleFilter->Update();
	m_Ext->GetSurface()->SetVtkPolyData(triangleFilter->GetOutput(0));

	// MESH 1
	if(1)
	{
		pdp::Utilities::MeshType::Pointer mesh;
		mesh = m_Ut->VtkSurfaceToItk(m_Ext->GetSurfacePolyData());

		//mesh->Print(std::cout);
		
		std::cout << "Mesh  " << std::endl;
		std::cout << "Number of Points =   " << mesh->GetNumberOfPoints() << std::endl;
		std::cout << "Number of Cells  =   " << mesh->GetNumberOfCells()  << std::endl;

		vtkPolyData* newSurface;
		newSurface = m_Ut->ItkSurfaceToVtk(mesh);

	/*	typedef itk::VTKPolyDataWriter<pdp::Utilities::MeshType> WriterType;	
		WriterType::Pointer writer = WriterType::New();
		writer->SetInput(mesh2);
		writer->SetFileName("test.vtk");
		writer->Update();*/
		
		// add new mesh to dataStorage
		m_Ut->AddPolyDataToDataStorage(newSurface, "NewSurface");
	}

	// MESH 2	
	if(0)
	{
		pdp::Utilities::TriangleMeshType::Pointer mesh2;
		mesh2 = m_Ut->VtkSurfaceToItk2(m_Ext->GetSurfacePolyData());

		//mesh2->Print(std::cout);
		
		std::cout << "Mesh  " << std::endl;
		std::cout << "Number of Points =   " << mesh2->GetNumberOfPoints() << std::endl;
		std::cout << "Number of Cells  =   " << mesh2->GetNumberOfCells()  << std::endl;

		typedef itk::VTKPolyDataWriter<pdp::Utilities::TriangleMeshType> WriterType;
		WriterType::Pointer writer = WriterType::New();
		writer->SetInput(mesh2);
		writer->SetFileName("test.vtk");
		writer->Update();		
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}