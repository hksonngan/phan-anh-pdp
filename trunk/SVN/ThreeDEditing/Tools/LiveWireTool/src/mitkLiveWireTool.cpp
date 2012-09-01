// "pdp"
#include "mitkLiveWireTool.h"
#include "mitkLiveWireTool.xpm"
#include "QmitkLiveWireToolGUI.h"

// MITK
#include "mitkToolManager.h"
#include <mitkSegTool2D.h>
#include <mitkExtractImageFilter.h>
#include <mitkContour.h>
#include <mitkContourUtils.h>
#include <mitkOverwriteSliceImageFilter.h>
#include <mitkOverwriteDirectedPlaneImageFilter.h>

#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"

#include "mitkBoundingObjectCutter.h"
#include <mitkCuboid.h>
//#include <mitkPointSetInteractor.h>

#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkApplicationCursor.h>

#include <mitkLookupTableProperty.h>

#include "mitkSurfaceToImageFilter.h"

// VTK
#include "vtkContourWidget.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkDijkstraImageContourLineInterpolator.h"
//#include "vtkDijkstraImageGeodesicPath.h"
#include "vtkImageActor.h"
#include "vtkImageActorPointPlacer.h"
#include "vtkImageAnisotropicDiffusion2D.h"
#include "vtkImageData.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageShiftScale.h"
#include <vtkImageCast.h>
#include <vtkImageFlip.h>
#include <mitkVtkWidgetRendering.h>
#include "vtkProperty.h"

#include <vtkMath.h>
#include <vtkCellArray.h>

#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkCutter.h>
#include <vtkLine.h>

#include <vtkPointLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkMergePoints.h>

#include <vtkLookupTable.h>
#include <vtkPointData.h>

#include <vtkCubeSource.h>
#include <vtkTriangleFilter.h>
//#include <vtkVoxelModeller.h>

#include "vtkVoxelizePolyData.h"
#include "vtkImageViewer.h"
#include "vtkImageActor.h"


// ITK
//#include "itkRegionOfInterestImageFilter.h"

#include "cursor_hand_closed.xpm"
#include "cursor_hand_open.xpm"


mitk::LiveWireTool::LiveWireTool()
:Tool("MyPositionTrackerWithLeftDrag")
{
	// great magic numbers
	CONNECT_ACTION( 92, OnMouseMoved );
	CONNECT_ACTION( 1003, LeftDrag );
	CONNECT_ACTION( 1004, LeftClick );
	CONNECT_ACTION( 1005, LeftClickRelease );

	m_Ext = new mitk::ToolExtension();
	m_Ext->InitTool();

	m_PointId2 = 0;
	m_LastBoundingBox[0] = -1; 
	m_LastBoundingBox[1] = -1; 
	m_LastBoundingBox[2] = -1; 
	m_LastBoundingBox[3] = -1; 
	m_LastBoundingBox[4] = -1; 
	m_LastBoundingBox[5] = -1; 

	m_LastPoint2D[0] = 0.0;
	m_LastPoint2D[1] = 0.0;

	m_FirstPoint = true;

	m_ROIRadius = 9.0;
	m_Color = false;
	m_FirstDrag = true;

	m_ChangeRadius = new RadiusInteractor("MyRadiusInteractor");
	m_ChangeRadius->UpdateRadius += mitk::MessageDelegate1<mitk::LiveWireTool, int>(this, &mitk::LiveWireTool::UpdateRadiusViaInteractor);
}

mitk::LiveWireTool::~LiveWireTool()
{}

const char** mitk::LiveWireTool::GetXPM() const
{
	return mitkLiveWireTool_xpm;
}

const char* mitk::LiveWireTool::GetName() const
{
	return "Live Wire Tool";
}

const char* mitk::LiveWireTool::GetGroup() const
{
	return "default";
}

void mitk::LiveWireTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";

	m_Ext->SetDataStorage(m_ToolManager->GetDataStorage());
	m_Ext->ActivateTool();
	GetCurrentSurfaces.Send(m_Ext->GetCurrentSurfaceVector());

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	m_Ext->UpdateSelectedSurface += mitk::MessageDelegate1<mitk::LiveWireTool, int>(this, &mitk::LiveWireTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider += mitk::MessageDelegate1<mitk::LiveWireTool, int>(this, &mitk::LiveWireTool::UpdateRadiusSlider);

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
	
	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->AddListener(m_ChangeRadius);

	// 		
	mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();
	m_PointSetNode = mitk::DataNode::New();
	m_PointSetNode->SetName("Contour Dijkstra");
	m_PointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
	m_PointSetNode->SetProperty("close contour", mitk::BoolProperty::New(true));
	m_PointSetNode->SetProperty("contoursize", mitk::FloatProperty::New(0.1));
	m_PointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(0.3));
	m_PointSetNode->SetProperty("show angle", mitk::BoolProperty::New(true));
	m_PointSetNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
	int newId3 = m_Ext->GetUniqueId();
	m_PointSetNode->SetIntProperty("UniqueID", newId3);
	m_PointSetNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_PointSetNode->SetData(pointSet2);
	m_Ext->GetDataStorage()->Add(m_PointSetNode);
	
	mitk::PointSet::Pointer leftContour = mitk::PointSet::New();
	m_LeftContourNode = mitk::DataNode::New();
	m_LeftContourNode->SetName("Left Contour");
	m_LeftContourNode->SetProperty("show contour", mitk::BoolProperty::New(true));
	m_LeftContourNode->SetProperty("close contour", mitk::BoolProperty::New(true));
	m_LeftContourNode->SetProperty("contoursize", mitk::FloatProperty::New(0.1));
	m_LeftContourNode->SetProperty("pointsize", mitk::FloatProperty::New(0.3));
	m_LeftContourNode->SetProperty("show angle", mitk::BoolProperty::New(true));
	m_LeftContourNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
	int newId4 = m_Ext->GetUniqueId();
	m_LeftContourNode->SetIntProperty("UniqueID", newId4);
	m_LeftContourNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_LeftContourNode->SetData(leftContour);
	//m_Ext->GetDataStorage()->Add(m_LeftContourNode);

	mitk::PointSet::Pointer rightContour = mitk::PointSet::New();
	m_RightContourNode = mitk::DataNode::New();
	m_RightContourNode->SetName("Right Contour");
	m_RightContourNode->SetProperty("show contour", mitk::BoolProperty::New(true));
	m_RightContourNode->SetProperty("close contour", mitk::BoolProperty::New(true));
	m_RightContourNode->SetProperty("contoursize", mitk::FloatProperty::New(0.1));
	m_RightContourNode->SetProperty("pointsize", mitk::FloatProperty::New(0.3));
	m_RightContourNode->SetProperty("show angle", mitk::BoolProperty::New(true));
	m_RightContourNode->SetProperty("point 2D size", mitk::FloatProperty::New(0.1));
	int newId5 = m_Ext->GetUniqueId();
	m_RightContourNode->SetIntProperty("UniqueID", newId5);
	m_RightContourNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_RightContourNode->SetData(rightContour);
	//m_Ext->GetDataStorage()->Add(m_RightContourNode);

	m_Ext->SetImagesToBottom();

	m_PathCreator = vtkDijkstraImageGeodesicPath::New();

	//Zoom(150);
}

void mitk::LiveWireTool::Deactivated()
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

	m_Ext->UpdateSelectedSurface -= mitk::MessageDelegate1<mitk::LiveWireTool, int>(this, &mitk::LiveWireTool::UpdateSelectedSurfaceBox);
	m_Ext->UpdateRadiusSlider -= mitk::MessageDelegate1<mitk::LiveWireTool, int>(this, &mitk::LiveWireTool::UpdateRadiusSlider);

	m_Ext->GetDataStorage()->Remove(m_BallNode2);
	m_Ext->GetDataStorage()->Remove(m_PointSetNode);
	m_Ext->GetDataStorage()->Remove(m_IntersectionContourNode);

	// Change radius interactor
	mitk::GlobalInteraction::GetInstance()->RemoveListener(m_ChangeRadius);

	// sichern?
	mitk::ApplicationCursor::GetInstance()->PopCursor();
}

void mitk::LiveWireTool::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Ext->SelectSurface(selection);

	// Set Old Sphere Radius
	SetSliderToLastRadius.Send((int)(m_Ext->GetRadius()* m_Ext->GetRadiusFactor()-1));	

	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::LiveWireTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
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

	m_ContourSphere2->SetRadius(sqrt((double)m_ROIRadius)); // todo
	m_ContourSphere2->SetCenter(m_Ext->GetContourSphere()->GetCenter());
	m_ContourSphere2->Update();
	
	return ok;
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

bool mitk::LiveWireTool::LeftClick(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	bool ok = true;
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
	}
 
	//std::cout << "Done\n";
	return ok;
}

bool mitk::LiveWireTool::LeftClickRelease(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	if(m_CursorOnSurface)
	{
		mitk::ApplicationCursor::GetInstance()->PopCursor();
	}

	m_FirstDrag = true;
	
	return ok;
}

bool mitk::LiveWireTool::LeftDrag(Action* action, const StateEvent* stateEvent)
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
		m_WorldPoint = displayPositionEvent->GetWorldPosition();
		
		//std::cout << "New point at position: " << m_WorldPoint.toItk() << "\n";

		if(m_FirstDrag)
		{
			// Clean up everything old
			m_StartStopContour.clear();
			m_SurfaceIds.clear();
			m_FirstContourPointsToBeDeleted.clear();
			m_SecondContourPointsToBeDeleted.clear();
			m_PointsInROI.clear();
			m_AnchorPointsOutsideROI.clear();
			m_AllPoints.clear();	
			m_FurtherHandlePoints.clear();

			// Konstruckt ueber m_FirstDrag ist besser als left click abfangen
			LeftDragInit(action, stateEvent);
			m_FirstDrag = false;

			// Color surface according to values in pointsToBeDeleted2
			// ColorSurface();

			// Initialize Laplacian Framework
			//InitializeLaplacianFramework(action, stateEvent);
		}

		// Dijstra runs two times: from start to handle point and from handle point to end point
		// Both paths result in the new contour
	
		// Koordinatentransformation für handle point
		pdp::Pt shiftedHandlePoint(m_WorldPoint[0] - m_Origin[0] - (m_Start[0] * m_Spacings[0]), 
								   m_WorldPoint[1] - m_Origin[1] - (m_Start[1] * m_Spacings[1]),
								   m_WorldPoint[2] - m_Origin[2] /*- (m_Start[2] * m_Spacings[2])*/);
		vtkIdType handleVertId = m_CostImage->FindPoint( shiftedHandlePoint[0], shiftedHandlePoint[1], 0 );
		//std::cout << "Shifted handle point: " << handleVertId << ", pos: " << shiftedHandlePoint.toItk() << "\n";
		//std::cout << "IDs: " << m_BeginVertId << " " << handleVertId << " " << m_EndVertId << "\n";

		if(m_BeginVertId == -1 || handleVertId == -1 || m_EndVertId == -1)
		{
			std::cout << "One of the dijkstra start/stop points ran out of the ROI -> Error.\n";
			return true;
		}

		m_PathCreator->SetStartVertex(m_BeginVertId);
		m_PathCreator->SetEndVertex(handleVertId);
		m_PathCreator->Update();
		vtkPolyData* out1 = m_PathCreator->GetOutput(0);
		//std::cout << "Number of points in dijstra output: " << out1->GetNumberOfPoints() << "\n";

		// store dijkstral in contour	
		// test: add three points on the way to next point
		//std::cout << "Number of points in dijstra output: " << out1->GetNumberOfPoints() << "\n";
				
		mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
		mitk::PointSet::Pointer leftPointSet = mitk::PointSet::New();
		mitk::PointSet::Pointer rightPointSet = mitk::PointSet::New();

		for(int i = out1->GetNumberOfPoints()-1; i >= 0; i--)
		{
			double* outPt = out1->GetPoints()->GetPoint(i);
			// Calculate new point's position
			pdp::Pt pt;
			pt[0] = outPt[0] + m_Origin[0] + m_Start[0] * m_Spacings[0];
			pt[1] = outPt[1] + m_Origin[1] + m_Start[1] * m_Spacings[1];
			pt[2] = m_StartPoint[2];
			//std::cout << "Contour point: " << pt[0] << " "  << pt[1] << " " << pt[2] << "\n";
			
			//mitk::PointOperation* doOp2 = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), m_PointId2);
			//leftPointSet->ExecuteOperation(doOp2);

			mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), m_PointId2);
			m_PointId2++;
			pointSet->ExecuteOperation(doOp);
		}
		// Without start and stop point
		int leftCounter = 0;
		for(int i = out1->GetNumberOfPoints()-2; i >= 0; i--) // > 0 // Changed
		//for(int i = out1->GetNumberOfPoints()-1; i >= 0; i--) // > 0
		{
			double* outPt = out1->GetPoints()->GetPoint(i);
			// Calculate new point's position
			pdp::Pt pt;
			pt[0] = outPt[0] + m_Origin[0] + m_Start[0] * m_Spacings[0];
			pt[1] = outPt[1] + m_Origin[1] + m_Start[1] * m_Spacings[1];
			pt[2] = m_StartPoint[2];
			//std::cout << "Contour point: " << pt[0] << " "  << pt[1] << " " << pt[2] << "\n";
			//m_LeftContour->InsertPoint(leftCounter, pt.toItk());
			mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), leftCounter);
			leftPointSet->ExecuteOperation(doOp);
			//m_LeftContour->ExecuteOperation(doOp);
			leftCounter++;
		}

		m_PathCreator->SetStartVertex(handleVertId);
		m_PathCreator->SetEndVertex(m_EndVertId);
		m_PathCreator->Update();
		vtkPolyData* out2 = m_PathCreator->GetOutput(0);
		//std::cout << "Number of points in dijstra output: " << out2->GetNumberOfPoints() << "\n";

		mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();
		for(int i = out2->GetNumberOfPoints()-2; i >= 0; i--) // -2: because we don't want to add the handle point twice
		{
			double* outPt = out2->GetPoints()->GetPoint(i);
			// Calculate new point's position
			pdp::Pt pt;
			pt[0] = outPt[0] + m_Origin[0] + m_Start[0] * m_Spacings[0];
			pt[1] = outPt[1] + m_Origin[1] + m_Start[1] * m_Spacings[1];
			pt[2] = m_StartPoint[2];
			//std::cout << "Contour point: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";

			//mitk::PointOperation* doOp2 = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), m_PointId2);
			//rightPointSet->ExecuteOperation(doOp2);

			mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), m_PointId2);
			m_PointId2++;
			pointSet->ExecuteOperation(doOp);
		}
		// Without start and stop point
		int rightCounter = 0;
		for(int i = out2->GetNumberOfPoints()-2; i > 0; i--)
		{
			double* outPt = out2->GetPoints()->GetPoint(i);
			// Calculate new point's position
			pdp::Pt pt;
			pt[0] = outPt[0] + m_Origin[0] + m_Start[0] * m_Spacings[0];
			pt[1] = outPt[1] + m_Origin[1] + m_Start[1] * m_Spacings[1];
			pt[2] = m_StartPoint[2];
			//std::cout << "Contour point: " << pt[0] << " "  << pt[1] << " " << pt[2] << "\n";

			//m_RightContour->InsertPoint(rightCounter, pt.toItk());
			mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, pt.toItk(), rightCounter);
			rightCounter++;
			rightPointSet->ExecuteOperation(doOp);
			//m_RightContour->ExecuteOperation(doOp);
		}

		m_PointSetNode->SetData(pointSet);
		m_PointId2 = 0;

		m_LeftContourNode->SetData(leftPointSet);
		m_RightContourNode->SetData(rightPointSet);

		//std::cout << "Number of points in surface part vs. dijkstra: " << m_SurfaceIds.size() << " " << pointSet->GetSize() - 2 << "\n"; // -2: We don't need start and end point -> those did not move
		//std::cout << "Number of points in left and right contour: " << m_LeftContour->GetSize() << " " << m_RightContour->GetSize() << "\n";
		//std::cout << "Number of points in first and second points to be deleted: " << m_FirstContourPointsToBeDeleted.size() << " " << m_SecondContourPointsToBeDeleted.size() << "\n";
	}
	return ok;
}

bool mitk::LiveWireTool::LeftDragInit(Action* action, const StateEvent* stateEvent)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	bool ok = true;

	// Clip surface with plane
	// Create plane according to renderer
	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	pdp::Pt rendererPlaneOrigin(planeGeometry->GetOrigin());

	vtkSmartPointer<vtkPlane> plane = vtkPlane::New();;
	plane->SetOrigin(rendererPlaneOrigin.toVtk());
	plane->SetNormal( 0, 0, 1 );

	vtkSmartPointer<vtkCutter> cutter2 = vtkCutter::New();
	cutter2->SetCutFunction(plane);
	cutter2->SetInput(m_Ext->GetSurfacePolyData());
	cutter2->Update();

	// merge dublicate points
	vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
	removeDuplicatedPoints->SetInput(cutter2->GetOutput());
	removeDuplicatedPoints->SetTolerance(0.01);
	removeDuplicatedPoints->ConvertLinesToPointsOn();
	removeDuplicatedPoints->PointMergingOn();
	removeDuplicatedPoints->ConvertPolysToLinesOn();
	removeDuplicatedPoints->PieceInvariantOn();
	removeDuplicatedPoints->ToleranceIsAbsoluteOn();
	removeDuplicatedPoints->SetAbsoluteTolerance(0.01);
	removeDuplicatedPoints->ConvertPolysToLinesOn();
	removeDuplicatedPoints->ConvertStripsToPolysOn();
	removeDuplicatedPoints->Update();

	// The output contour contains the same points as the input surface (guess: construction of the thickening surface os pixel based)
	m_IntersectionContour = mitk::Surface::New();
	m_IntersectionContour->SetVtkPolyData(removeDuplicatedPoints->GetOutput());

	vtkSmartPointer<vtkCellArray> lines2 = m_IntersectionContour->GetVtkPolyData()->GetLines();
	vtkSmartPointer<vtkPoints> points2 = m_IntersectionContour->GetVtkPolyData()->GetPoints();
	vtkSmartPointer<vtkPolyData> poly = vtkPolyData::New();
	poly->SetPoints(points2);
	poly->SetLines(lines2);
	m_IntersectionContour->SetVtkPolyData(poly);
	m_IntersectionContourNode = mitk::DataNode::New();
	m_IntersectionContourNode->SetName("Intersection Contour");	
	int newId = m_Ext->GetUniqueId();
	m_IntersectionContourNode->SetIntProperty("UniqueID", newId);
	m_IntersectionContourNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_IntersectionContourNode->SetData(m_IntersectionContour);
	//m_Ext->GetDataStorage()->Add(m_IntersectionContourNode);

	//std::cout << "Contour number of points: " << m_IntersectionContour->GetVtkPolyData()->GetNumberOfPoints() << "\n";
	//std::cout << "Contour, number of cells: " << m_IntersectionContour->GetVtkPolyData()->GetNumberOfCells() << "\n";
	//std::cout << "Contour, number of lines: " << m_IntersectionContour->GetVtkPolyData()->GetNumberOfLines() << "\n";

	//test: cut hole in contour according to ball 
	//vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	std::vector<vtkIdType> pointsToBeDeleted;
	std::vector<vtkIdType> pointsToBeDeleted2;
	std::map<vtkIdType,int> searchForStartEndPoint;

	pdp::Pt ballPosition(0.0);
	m_Ext->GetContourSphere()->GetCenter(ballPosition.toVtk());
	int inputNumPoints = m_IntersectionContour->GetVtkPolyData()->GetNumberOfPoints();
	for (vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());
		
		double distance = vtkMath::Distance2BetweenPoints(pt.toVtk(), ballPosition.toVtk());
		if(distance < m_ROIRadius)
		{
			// Contour points are inside ball
			pointsToBeDeleted.push_back(ptId);
		}
	}

	// Teste ob punkte eine oder mehrere contourabschnitte enthält
	// Wir suchen das contourstueck das in zwei richtungen vom ballmittelpunkt ausgehend bis zum ballrand reicht
	// -> alles andere sind gewollte contourstücke und sie werden nicht herrausgeschnitten 

	// Get closest pointsToBeDeletedpoint to the handlepoint	
	vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
	pointLocator->SetDataSet(m_IntersectionContour->GetVtkPolyData());
	pointLocator->BuildLocator();
	m_HandlePointID = pointLocator->FindClosestPoint(m_WorldPoint.toVtk());
	std::cout << "Closest point pointId: " << m_HandlePointID << "\n";	
	m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(m_HandlePointID, m_HandlePoint.toVtk());
	std::cout << "Closest point position: " << m_HandlePoint.toItk() << "\n";	

	vtkSmartPointer<vtkPointLocator> pointLocator2 = vtkSmartPointer<vtkPointLocator>::New();
	pointLocator2->SetDataSet(m_Ext->GetSurfacePolyData());
	pointLocator2->BuildLocator();
	m_SurfaceHandlePointID = pointLocator2->FindClosestPoint(m_WorldPoint.toVtk());
	std::cout << "Surface handle point pointId: " << m_SurfaceHandlePointID << "\n";	

	// Teste ob punkte einen oder mehrere contourabschnitte enthält
	// Get both neighbours of handle point and add all three to vector
	vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
	m_IntersectionContour->GetVtkPolyData()->GetPointCells(m_HandlePointID, cellIds);
	pointsToBeDeleted2.push_back(m_HandlePointID);
	std::sort(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end());
	std::sort(pointsToBeDeleted.begin(), pointsToBeDeleted.end());

	vtkIdType firstNeighbour = -1;
	vtkIdType secondNeighbour = -1;

	// Search for first two neigbours to world point
	for(vtkIdType i = 0;  i < cellIds->GetNumberOfIds(); i++)
	{
		// get both neighbours
		vtkIdType neighbourCell = cellIds->GetId(i);
		//std::cout << "Handle neighbour Cell: " << neighbourCell << "\n";

		vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
		m_IntersectionContour->GetVtkPolyData()->GetCellPoints(neighbourCell, pointIds);

		if(pointIds->GetNumberOfIds() == 2)
		{
			for(vtkIdType j = 0;  j < pointIds->GetNumberOfIds(); j++)
			{
				vtkIdType neighbour = pointIds->GetId(j);
				pdp::Pt pt;
				m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(neighbour, pt.toVtk());
				//std::cout << "Neighbour, id: " << neighbour << ", position: " << pt.toItk() << "\n";  

				// Is already in pointsToBeDeleted2 vector?
				if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), neighbour))
				{
					//std::cout << "Neighbour, id: " << neighbour << ", position: " << pt.toItk() << " is already in pointsToBeDeleted2.\n";
					continue;
				}
				
				// Is inside pointsToBeDeleted vector?
				for(std::vector<vtkIdType>::iterator it = pointsToBeDeleted.begin(); it != pointsToBeDeleted.end(); it++)
				{
					if(neighbour == *it)
					{
						// new neighbouring point in contour found
						pointsToBeDeleted2.push_back(neighbour);
						std::sort(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end());

						if(firstNeighbour == -1)
						{
							firstNeighbour = neighbour;
						}
						else
						{
							secondNeighbour = neighbour;
						}
					}
				}
			}
		}
		else
		{
			std::cout << "Wrong number of neighbours = " << pointIds->GetNumberOfIds() << "\n";
		}
	}

	/*std::cout << "Neighbours: " << firstNeighbour << ", " << secondNeighbour << "\n";
	for(std::vector<vtkIdType>::iterator it = pointsToBeDeleted2.begin(); it != pointsToBeDeleted2.end(); it++)
	{
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
		std::cout << "Point to be deleted 2: " << *it << ", at position: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
	}*/

	// At this moment I got the two direct neighbours in the mesh if both exist
	if(firstNeighbour != -1)
	{
		// run on contour starting by the first neighbour
		m_FirstContourPointsToBeDeleted.push_back(firstNeighbour);
		bool loop = true;
		vtkIdType runningID = firstNeighbour;
		while(loop)
		{
			vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
			m_IntersectionContour->GetVtkPolyData()->GetPointCells(runningID, cellIds);
			for(vtkIdType i = 0;  i < cellIds->GetNumberOfIds(); i++)
			{
				//std::cout << "i: " << i << "\n";
				// get both neighbours
				vtkIdType neighbourCell = cellIds->GetId(i);

				vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
				m_IntersectionContour->GetVtkPolyData()->GetCellPoints(neighbourCell, pointIds);

				if(pointIds->GetNumberOfIds() == 2)
				{
					for(vtkIdType j = 0;  j < pointIds->GetNumberOfIds(); j++)
					{
						vtkIdType neighbourPoint = pointIds->GetId(j);

						// Is neighbour already in list? -> wrong direction!
						if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), neighbourPoint))
						{
							//std::cout << "Point: " << neighbourPoint << " is already in pointsToBeDeleted2\n"; // skip
						}
						else if(std::binary_search(pointsToBeDeleted.begin(), pointsToBeDeleted.end(), neighbourPoint))
						{
							//std::cout << "Point: " << neighbourPoint << " is in pointsToBeDeleted -> new neighbour in ROI found.\n";
							// New neighbour found that is part of the hole
							pointsToBeDeleted2.push_back(neighbourPoint);
							std::sort(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end());
							m_FirstContourPointsToBeDeleted.push_back(neighbourPoint);
							//std::sort(m_FirstContourPointsToBeDeleted.begin(), m_FirstContourPointsToBeDeleted.end());
							runningID = neighbourPoint;
						}
						else
						{
							// end point found -> current runningID is endpoint in this direction
							//std::cout << "Point: " << neighbourPoint << " is not in pointsToBeDeleted -> end found.\n";
							m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(runningID, m_FirstEndPoint.toVtk());
							loop = false;
						}
					}							
				}
				else
				{
					std::cout << "Wrong number of neighbours = " << pointIds->GetNumberOfIds() << "\n";
				}
			}					
		}
	}

	// Second direction
	if(secondNeighbour != -1)
	{
		// run on contour starting by the first neighbour
		m_SecondContourPointsToBeDeleted.push_back(secondNeighbour);
		bool loop = true;
		vtkIdType runningID = secondNeighbour;
		while(loop)
		{
			vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
			m_IntersectionContour->GetVtkPolyData()->GetPointCells(runningID, cellIds);
			for(vtkIdType i = 0;  i < cellIds->GetNumberOfIds(); i++)
			{
				//std::cout << "i: " << i << "\n";
				// get both neighbours
				vtkIdType neighbourCell = cellIds->GetId(i);

				vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
				m_IntersectionContour->GetVtkPolyData()->GetCellPoints(neighbourCell, pointIds);

				if(pointIds->GetNumberOfIds() == 2)
				{
					for(vtkIdType j = 0;  j < pointIds->GetNumberOfIds(); j++)
					{
						vtkIdType neighbourPoint = pointIds->GetId(j);

						// Is neighbour already in list? -> wrong direction!
						if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), neighbourPoint))
						{
							//std::cout << "Point: " << neighbourPoint << " is already in pointsToBeDeleted2\n"; // skip
						}
						else if(std::binary_search(pointsToBeDeleted.begin(), pointsToBeDeleted.end(), neighbourPoint))
						{
							//std::cout << "Point: " << neighbourPoint << " is in pointsToBeDeleted -> new neighbour in ROI found.\n";
							// New neighbour found that is part of the hole
							pointsToBeDeleted2.push_back(neighbourPoint);
							std::sort(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end());
							m_SecondContourPointsToBeDeleted.push_back(neighbourPoint);
							//std::sort(m_SecondContourPointsToBeDeleted.begin(), m_SecondContourPointsToBeDeleted.end());
							runningID = neighbourPoint;
						}
						else
						{
							// end point found -> current runningID is endpoint in this direction
							//std::cout << "Point: " << neighbourPoint << " is not in pointsToBeDeleted -> end found.\n";
							m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(runningID, m_SecondEndPoint.toVtk());
							loop = false;
						}
					}
				}
				else
				{
					std::cout << "Wrong number of neighbours = " << pointIds->GetNumberOfIds() << "\n";
				}
			}					
		}
	}

	std::cout << "Compare sizes: pointsToBeDeleted2: " << pointsToBeDeleted2.size() << ", FirstContourPointsToBeDeleted: " << m_FirstContourPointsToBeDeleted.size() << ", SecondContourPointsToBeDeleted: " << m_SecondContourPointsToBeDeleted.size() << "\n";

	if(0) // print
	{
		std::cout << "Point to be deleted size: " << pointsToBeDeleted.size() << "\n";
		for(std::vector<vtkIdType>::iterator it = pointsToBeDeleted.begin(); it != pointsToBeDeleted.end(); it++)
		{
			pdp::Pt pt;
			m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
			std::cout << "Point to be deleted: " << *it << ", at position: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
		}
		std::cout << "Point to be deleted 2 size: " << pointsToBeDeleted2.size() << "\n";
		for(std::vector<vtkIdType>::iterator it = pointsToBeDeleted2.begin(); it != pointsToBeDeleted2.end(); it++)
		{
			pdp::Pt pt;
			m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
			std::cout << "Point to be deleted 2: " << *it << ", at position: " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
		}
	}

	m_IntersectionContour->GetVtkPolyData()->GetLines()->InitTraversal();
	vtkIdType npts, *pts;
	std::sort(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end());
	
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	for(vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());
		
		if(!std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), ptId))
		{
			//points->InsertNextPoint(pt.toVtk());		
			points->InsertPoint(ptId, pt.toVtk());
			// Careful: vtkIdType ist nicht mehr stetig
		}
	}

	vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();			
	while(m_IntersectionContour->GetVtkPolyData()->GetLines()->GetNextCell(npts,pts))
	{
		//std::cout << "Number of points in line: " << npts << "\n";
		if(npts != 2)
		{
			std::cout << "Error: Line with more or less than two points found\n";
		}

		if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), pts[0]))
		{
			//std::cout << "Delete line: " << pts[0] << " " << pts[1] << "\n";
			continue;
		}

		if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), pts[1]))
		{
			//std::cout << "Delete line: " << pts[0] << " " << pts[1] << "\n";
			continue;
		}

		//std::cout << "Add line: " << pts[0] << " " << pts[1] << "\n";
		vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
		line->GetPointIds()->SetId(0,pts[0]);
		line->GetPointIds()->SetId(1,pts[1]);
		lines->InsertNextCell(line);

		// Calculate start and end point of the contour
		std::map<vtkIdType,int>::iterator it = searchForStartEndPoint.find(pts[0]);
		if(it != searchForStartEndPoint.end())
		{
			searchForStartEndPoint[pts[0]]++;
		}
		else
		{
			searchForStartEndPoint.insert(std::pair<vtkIdType,int>(pts[0], 1));
		}

		std::map<vtkIdType,int>::iterator it2 = searchForStartEndPoint.find(pts[1]);
		if(it2 != searchForStartEndPoint.end())
		{
			searchForStartEndPoint[pts[1]]++;
		}
		else
		{
			searchForStartEndPoint.insert(std::pair<vtkIdType,int>(pts[1], 1));
		}
	}
	//std::cout << "2\n";
	
	// tanslate points to be deleted in vtkIdTypes of the surface
	for(std::vector<vtkIdType>::iterator it =  pointsToBeDeleted2.begin(); it != pointsToBeDeleted2.end(); it++)
	{
		// Get point from id
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
		//std::cout << "Point in contour: " << *it << ", at pos: " << pt.toItk() << "\n";
		// Try to find next point in original surface
		vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
		pointLocator->SetDataSet(m_Ext->GetSurfacePolyData());
		pointLocator->BuildLocator();
		vtkIdType surfaceId = pointLocator->FindClosestPoint(pt.toVtk());
		m_SurfaceIds.push_back(surfaceId);
	}
	
	// first and second contour points to be deleted
	m_FirstContourPointsToBeDeletedSurfaceId.clear();
	m_SecondContourPointsToBeDeletedSurfaceId.clear();
	
	for(std::vector<vtkIdType>::iterator it =  m_FirstContourPointsToBeDeleted.begin(); it != m_FirstContourPointsToBeDeleted.end(); it++)
	{
		// Get point from id
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
		//std::cout << "Point in first contour: " << *it << ", at pos: " << pt.toItk() << "\n";
		// Try to find next point in original surface
		vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
		pointLocator->SetDataSet(m_Ext->GetSurfacePolyData());
		pointLocator->BuildLocator();
		vtkIdType surfaceId = pointLocator->FindClosestPoint(pt.toVtk());

		pdp::Pt pt2;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(surfaceId, pt2.toVtk());

		// Make m_FirstContourPointsToBeDeletedSurfaceId -> only add new points
		bool alreadyIn = false;
		for(std::vector<vtkIdType>::iterator it2 = m_FirstContourPointsToBeDeletedSurfaceId.begin(); it2 != m_FirstContourPointsToBeDeletedSurfaceId.end(); it2++)
		{
			pdp::Pt newPt;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it2, newPt.toVtk());

			//std::cout << "Compare: " << pt2.toItk() << " with " << newPt.toItk() << "\n";

			if(pt2[0] == newPt[0] && pt2[1] == newPt[1] && pt2[2] == newPt[2])
			{
				//std::cout << "Point is already in vector\n";
				alreadyIn = true;
			}
		}
		if(!alreadyIn)
			m_FirstContourPointsToBeDeletedSurfaceId.push_back(surfaceId);
	}

	for(std::vector<vtkIdType>::iterator it =  m_SecondContourPointsToBeDeleted.begin(); it != m_SecondContourPointsToBeDeleted.end(); it++)
	{
		// Get point from id
		pdp::Pt pt;
		m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, pt.toVtk());
		//std::cout << "Point in second contour: " << *it << ", at pos: " << pt.toItk() << "\n";
		// Try to find next point in original surface
		vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
		pointLocator->SetDataSet(m_Ext->GetSurfacePolyData());
		pointLocator->BuildLocator();
		vtkIdType surfaceId = pointLocator->FindClosestPoint(pt.toVtk());

		pdp::Pt pt2;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(surfaceId, pt2.toVtk());

		// Make m_SecondContourPointsToBeDeletedSurfaceId unique -> only add new points
		bool alreadyIn = false;
		for(std::vector<vtkIdType>::iterator it2 = m_SecondContourPointsToBeDeletedSurfaceId.begin(); it2 != m_SecondContourPointsToBeDeletedSurfaceId.end(); it2++)
		{
			pdp::Pt newPt;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it2, newPt.toVtk());

			//std::cout << "Compare: " << pt2.toItk() << " with " << newPt.toItk() << "\n";

			if(pt2[0] == newPt[0] && pt2[1] == newPt[1] && pt2[2] == newPt[2])
			{
				//std::cout << "Point is already in vector\n";
				alreadyIn = true;
			}
		}
		if(!alreadyIn)
			m_SecondContourPointsToBeDeletedSurfaceId.push_back(surfaceId);
	}

	if(0)
	{
		std::cout << "First Contour Intersection Contour: \n";
		for(std::vector<vtkIdType>::iterator it = m_FirstContourPointsToBeDeleted.begin(); it != m_FirstContourPointsToBeDeleted.end(); it++)
		{
			pdp::Pt newPt;
			m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, newPt.toVtk());
			std::cout << newPt.toItk() << "\n";
		}
		std::cout << "Second Contour Intersection Contour: \n";
		for(std::vector<vtkIdType>::iterator it = m_SecondContourPointsToBeDeleted.begin(); it != m_SecondContourPointsToBeDeleted.end(); it++)
		{
			pdp::Pt newPt;
			m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(*it, newPt.toVtk());
			std::cout << newPt.toItk() << "\n";
		}
	}

	if(0) // print
	{
		std::cout << "Compare sizes between pointsToBeDeleted2 and surfaceId: " << pointsToBeDeleted2.size() << " " << m_SurfaceIds.size() << "\n";
		for(std::vector<vtkIdType>::iterator it =  m_SurfaceIds.begin(); it != m_SurfaceIds.end(); it++)
		{
			std::cout << "Surface Ids: " << *it << "\n";
		}
		for(std::vector<vtkIdType>::iterator it =  m_FirstContourPointsToBeDeletedSurfaceId.begin(); it != m_FirstContourPointsToBeDeletedSurfaceId.end(); it++)
		{
			std::cout << "First contour surface Ids: " << *it << "\n";
		}
		for(std::vector<vtkIdType>::iterator it =  m_SecondContourPointsToBeDeletedSurfaceId.begin(); it != m_SecondContourPointsToBeDeletedSurfaceId.end(); it++)
		{
			std::cout << "Second contour surface Ids: " << *it << "\n";
		}
	}

	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();
	// Add the points to the dataset
	linesPolyData->SetPoints(points);
	// Add the lines to the dataset
	linesPolyData->SetLines(lines);

	m_IntersectionContour->SetVtkPolyData(linesPolyData);
	m_Ext->UpdateRenderer();

	// Search map for start and end point
	for(std::map<vtkIdType,int>::iterator it = searchForStartEndPoint.begin(); it != searchForStartEndPoint.end(); it++)
	{
		if(it->second == 1)
		{
			m_StartStopContour.push_back(it->first);
		}
	}

	//std::cout << "Number of points in map: " << searchForStartEndPoint.size() << "\n";
	//for(std::map<vtkIdType,int>::iterator it = searchForStartEndPoint.begin(); it != searchForStartEndPoint.end(); it++)
	//{
	//	std::cout << "Map: " << it->first << ", " << it->second << "\n";
	//}
	//std::cout << "Start/Stop point: " << m_StartStopContour[0] << ", " << m_StartStopContour[1] << "\n";

	// Get point position to start and stop point
	double dummy[3];	// Get Point return a pointer. I need to copy values.
	//double startPoint[3];
	m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(m_StartStopContour[0], dummy);
	m_StartPoint[0] = dummy[0];
	m_StartPoint[1] = dummy[1];
	m_StartPoint[2] = dummy[2];
	//std::cout << "Start Point: " << startPoint[0] << " " << startPoint[1] << " " << startPoint[2] << "\n";
	
	//double stopPoint[3]; 
	m_IntersectionContour->GetVtkPolyData()->GetPoints()->GetPoint(m_StartStopContour[1], dummy);
	m_StopPoint[0] = dummy[0];
	m_StopPoint[1] = dummy[1];
	m_StopPoint[2] = dummy[2];
	//std::cout << "Stop Point: " << stopPoint[0] << " " << stopPoint[1] << " " << stopPoint[2] << "\n";

	// If surface boundingbox has changed, than we need a new cost image
	mitk::Geometry3D::Pointer surfaceGeo = m_Ext->GetSurface()->GetGeometry();	
	mitk::BoundingBox* surfaceBb = const_cast<mitk::BoundingBox*>(surfaceGeo->GetBoundingBox());
	mitk::BoundingBox::BoundsArrayType surfaceBounds = surfaceBb->GetBounds();

	if(surfaceBounds[0] != m_LastBoundingBox[0] || 
	   surfaceBounds[1] != m_LastBoundingBox[1] || 
	   surfaceBounds[2] != m_LastBoundingBox[2] || 
	   surfaceBounds[3] != m_LastBoundingBox[3] || 
	   surfaceBounds[4] != m_LastBoundingBox[4] || 
	   surfaceBounds[5] != m_LastBoundingBox[5])
	{
		std::cout << "New Region of Intrest. We need a new cost image.\n";
		m_CostImage = GetCostImage();

		m_LastBoundingBox[0] = surfaceBounds[0];
		m_LastBoundingBox[1] = surfaceBounds[1];
		m_LastBoundingBox[2] = surfaceBounds[2];
		m_LastBoundingBox[3] = surfaceBounds[3];
		m_LastBoundingBox[4] = surfaceBounds[4];
		m_LastBoundingBox[5] = surfaceBounds[5];
	}

	m_PathCreator->SetInput(m_CostImage);

	//mitk::Point3D origin = imageNode->GetData()->GetGeometry()->GetOrigin();
	//std::cout << "Origin: " << origin << "\n";

	// Koordinatentransformation vereinfachen
	pdp::Pt shiftedStartPoint(m_StartPoint[0] - m_Origin[0] - (m_Start[0] * m_Spacings[0]), 
							  m_StartPoint[1] - m_Origin[1] - (m_Start[1] * m_Spacings[1]),
							  m_StartPoint[2] - m_Origin[2] /*- (m_Start[2] * m_Spacings[2])*/);
	pdp::Pt shiftedStopPoint(m_StopPoint[0] - m_Origin[0] - (m_Start[0] * m_Spacings[0]), 
							 m_StopPoint[1] - m_Origin[1] - (m_Start[1] * m_Spacings[1]), 
							 m_StopPoint[2] - m_Origin[2] /*- (m_Start[2] * m_Spacings[2])*/);


	//std::cout << "Shifted start point: " << shiftedStartPoint.toItk() <<  "\n";
	//std::cout << "Shifted stop point: " << shiftedStopPoint.toItk() << "\n";

	m_BeginVertId = m_CostImage->FindPoint( shiftedStartPoint[0], shiftedStartPoint[1], 0 );
	m_EndVertId = m_CostImage->FindPoint( shiftedStopPoint[0], shiftedStopPoint[1], 0 );
	std::cout << "Begin Vertex: " << m_BeginVertId << ", end: " << m_EndVertId << "\n";
	
	// Shortest path problem
	m_PathCreator->StopWhenEndReachedOn();
	// prevent contour segments from overlapping
	m_PathCreator->RepelPathFromVerticesOn();
	// weights are scaled from 0 to 1 as are associated cost
	// components
	m_PathCreator->SetCurvatureWeight( 0.15 );
	m_PathCreator->SetEdgeLengthWeight( 0.8 );
	m_PathCreator->SetImageWeight( 1.0 );

	return ok;
}

void mitk::LiveWireTool::ROIRadiusChanged(int value)
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

void mitk::LiveWireTool::UpdateRadiusViaInteractor(int value)
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
	UpdateRadiusSlider2(sqrt(m_ROIRadius)*5-1);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

vtkSmartPointer<vtkImageData> mitk::LiveWireTool::GetCostImage()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// get image
	mitk::Image::Pointer image;
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("Scenario_2_ReferenceImage");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("Scenario_2_ReferenceImage");
		if(m_Ext->GetDataStorage()->Exists(imageNode))
		{
			std::cout << "Scenario_2_ReferenceImage found!\n";
		}
		 image = (mitk::Image*)imageNode->GetData();
		std::cout << "Reference image name: " << imageNode->GetName() << "\n";
	}
	else
	{
		bool first = true;
		mitk::DataNode::Pointer imageNode;
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Ext->GetDataStorage()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
			{
				if(first)
				{
					imageNode = allNodes->at(i);
					first = false;

					// TODO: set opacity
					//m_ReferenceImageNode->SetOpacity(0.5);
				}
			}
		}
		 image = (mitk::Image*)imageNode->GetData();
	}
	
	m_Origin = image->GetGeometry()->GetOrigin();

	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	int affectedDimension( -1 );
	int affectedSlice( -1 );
	SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

	std::cout << "Affected dimension: " << affectedDimension << ", affected slice: " << affectedSlice << "\n";

	// 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice;
	try
	{
		// now we extract the correct slice from the volume, resulting in a 2D image
		mitk::ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
		extractor->SetInput( image );
		extractor->SetSliceDimension( affectedDimension );
		extractor->SetSliceIndex( affectedSlice );
		extractor->SetTimeStep( 0 );
		extractor->Update();

		// here we have a single slice that can be modified
		slice = extractor->GetOutput();
	}
	catch(...)
	{
		// not working
    }

	std::cout << "1\n";

	// New: Calculate ROI
	// Create VOI feature input image according to the current bounding box of the thickening surface
	mitk::Geometry3D::Pointer surfaceGeo = m_Ext->GetSurface()->GetGeometry();	
	mitk::BoundingBox* surfaceBb = const_cast<mitk::BoundingBox*>(surfaceGeo->GetBoundingBox());
	mitk::BoundingBox::BoundsArrayType surfaceBounds = surfaceBb->GetBounds();
	
	// Cut ROI 
	/*mitk::Point3D fakeOrigin;
	fakeOrigin.Fill(0.0);
	slice->SetOrigin(fakeOrigin);*/
	ImageType::Pointer itkSlice;
	mitk::CastToItkImage( slice, itkSlice );

	typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > FilterType;
	FilterType::Pointer filter = FilterType::New();

	std::cout << "2\n";

	m_Spacings = image->GetGeometry()->GetSpacing();
	//std::cout << "Spacings: " << m_Spacings << "\n";

	// todo: set ROI larger than bounding box
	//ImageType::IndexType start;
	m_Start[0] = surfaceBounds[0]/m_Spacings[0]-m_Origin[0]/m_Spacings[0] -1; 
	m_Start[1] = surfaceBounds[2]/m_Spacings[1]-m_Origin[1]/m_Spacings[1]; 
	//ImageType::SizeType size;
	m_Size[0] = surfaceBounds[1]/m_Spacings[0]-surfaceBounds[0]/m_Spacings[0] + 3; 
	m_Size[1] = surfaceBounds[3]/m_Spacings[1]-surfaceBounds[2]/m_Spacings[1] + 4; 
	std::cout << "Start point: " << m_Start[0] << " " << m_Start[1] << ", size: " << m_Size[0] << " " << m_Size[1] << "\n";
	
	if(m_Start[0] < 0)
	{
		std::cout << "Start point: " << m_Start[0] << " kleiner 0, -> calculate full size image.\n";
		return GetCostImageWithOutROI();
	}
	if(m_Start[1] < 0)
	{
		std::cout << "Start point: " << m_Start[1] << " kleiner 0, -> calculate full size image.\n";
		return GetCostImageWithOutROI();
	}
	if((m_Start[0] + m_Size[0]) > slice->GetGeometry()->GetBounds()[1])
	{
		std::cout << "Start point + size: " << (m_Start[0] + m_Size[0]) << " larger than image: " << slice->GetGeometry()->GetBounds()[1] << ", -> calculate full size image.\n";
		return GetCostImageWithOutROI();
	}
	if((m_Start[1] + m_Size[1]) > slice->GetGeometry()->GetBounds()[3])
	{
		std::cout << "Start point + size: " << (m_Start[1] + m_Size[2]) << " larger than image, " << slice->GetGeometry()->GetBounds()[3] << ", -> calculate full size image.\n";
		return GetCostImageWithOutROI();
	}

	ImageType::RegionType desiredRegion;
	desiredRegion.SetSize(m_Size);
	desiredRegion.SetIndex(m_Start);

	filter->SetRegionOfInterest(desiredRegion);
	filter->SetInput(itkSlice);

	mitk::DataNode::Pointer inputNode = mitk::DataNode::New();
	mitk::Image::Pointer mitkSliceROI = mitk::ImportItkImage(filter->GetOutput());
	inputNode->SetData(mitkSliceROI);
	inputNode->SetName("ROI Image");
	int newId2 = m_Ext->GetUniqueId();
	inputNode->SetIntProperty("UniqueID", newId2);
	inputNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(inputNode);

	// modify slice
	vtkSmartPointer<vtkImageData> vImage;// = vtkSmartPointer<vtkImageData>::New();
	//vImage = slice->GetVtkImageData();
	vImage = mitkSliceROI->GetVtkImageData();

	//vImage->Print(std::cout);

	bool followCursor = false;

	// Smooth the image
	vtkImageAnisotropicDiffusion2D *diffusion = vtkImageAnisotropicDiffusion2D::New();
	diffusion->SetInput( vImage );
	diffusion->SetDiffusionFactor( 1.0 );
	diffusion->SetDiffusionThreshold( 200.0 );
	diffusion->SetNumberOfIterations( 5 );

	std::cout << "5\n";

	// Gradient magnitude for edges
	vtkImageGradientMagnitude *grad = vtkImageGradientMagnitude::New();
	grad->SetDimensionality( 2 );
	grad->HandleBoundariesOn();
	grad->SetInputConnection( diffusion->GetOutputPort() );
	grad->Update();

	double* range = grad->GetOutput()->GetScalarRange();

	// Invert the gradient magnitude so that low costs are
	// associated with strong edges and scale from 0 to 1
	vtkImageShiftScale *gradInvert = vtkImageShiftScale::New();
	gradInvert->SetShift( -1.0*range[ 1 ] );
	gradInvert->SetScale( 1.0 /( range[ 0 ] - range[ 1 ] ) );
	gradInvert->SetOutputScalarTypeToFloat();
	gradInvert->SetInputConnection( grad->GetOutputPort() );
	gradInvert->Update();

	// The color map will accept any scalar image type and convert to
	// unsigned char for the image actor
	vtkImageMapToWindowLevelColors *colorMap = vtkImageMapToWindowLevelColors::New();
	colorMap->SetInputConnection( gradInvert->GetOutputPort() );
	range = gradInvert->GetOutput()->GetScalarRange();
	colorMap->SetWindow(1.0);
	colorMap->SetLevel(0.5);

	// flip x axis			Kann ich nicht löschen, warum nicht?
	vtkSmartPointer<vtkImageFlip> flipXFilter =  vtkSmartPointer<vtkImageFlip>::New();
	flipXFilter->SetFilteredAxis(1); // flip x axis -> 0
	flipXFilter->SetInput(colorMap->GetOutput());
	flipXFilter->Update();
	vtkSmartPointer<vtkImageCast> castXFilter = vtkSmartPointer<vtkImageCast>::New();
	castXFilter->SetOutputScalarTypeToUnsignedChar();
	castXFilter->SetInputConnection(flipXFilter->GetOutputPort());
	castXFilter->Update();

	// add image to data storage to see if already messed up
	mitk::Image::Pointer newcostimage2 = mitk::Image::New();
	newcostimage2->Initialize(colorMap->GetOutput());
	newcostimage2->SetVolume(colorMap->GetOutput()->GetScalarPointer());
	mitk::DataNode::Pointer newImageNode3 = mitk::DataNode::New();
	newImageNode3->SetData(newcostimage2);
	//newImageNode->SetData(slice);
	newImageNode3->SetColor(200.0,0,0);
	newImageNode3->SetName("Cost Image");
	int newId = m_Ext->GetUniqueId();
	newImageNode3->SetIntProperty("UniqueID", newId);
	newImageNode3->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(newImageNode3);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return colorMap->GetOutput();
}

vtkSmartPointer<vtkImageData> mitk::LiveWireTool::GetCostImageWithOutROI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// get image
	mitk::Image::Pointer image;
	mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("Scenario_2_ReferenceImage");
	if(m_Ext->GetDataStorage()->Exists(imageNode))
	{
		mitk::DataNode::Pointer imageNode = m_Ext->GetDataStorage()->GetNamedNode("Scenario_2_ReferenceImage");
		if(m_Ext->GetDataStorage()->Exists(imageNode))
		{
			std::cout << "Scenario_2_ReferenceImage found!\n";
		}
		 image = (mitk::Image*)imageNode->GetData();
		std::cout << "Reference image name: " << imageNode->GetName() << "\n";
	}
	else
	{
		bool first = true;
		mitk::DataNode::Pointer imageNode;
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Ext->GetDataStorage()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
			{
				if(first)
				{
					imageNode = allNodes->at(i);
					first = false;

					// TODO: set opacity
					//m_ReferenceImageNode->SetOpacity(0.5);
				}
			}
		}
		 image = (mitk::Image*)imageNode->GetData();
	}
	
	m_Origin = image->GetGeometry()->GetOrigin();

	const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (m_Ext->GetMitkView()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D()) );
	int affectedDimension( -1 );
	int affectedSlice( -1 );
	SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

	std::cout << "Affected dimension: " << affectedDimension << ", affected slice: " << affectedSlice << "\n";

	// 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice;
	try
	{
		// now we extract the correct slice from the volume, resulting in a 2D image
		mitk::ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
		extractor->SetInput( image );
		extractor->SetSliceDimension( affectedDimension );
		extractor->SetSliceIndex( affectedSlice );
		extractor->SetTimeStep( 0 );
		extractor->Update();

		// here we have a single slice that can be modified
		slice = extractor->GetOutput();
	}
	catch(...)
	{
		// not working
    }

	// modify slice
	vtkSmartPointer<vtkImageData> vImage;// = vtkSmartPointer<vtkImageData>::New();
	vImage = slice->GetVtkImageData();
	//vImage = mitkSliceROI->GetVtkImageData();

	// Dummies 
	m_Start[0] = 0.0;
	m_Start[1] = 0.0;

	bool followCursor = false;

	// Smooth the image
	vtkImageAnisotropicDiffusion2D *diffusion = vtkImageAnisotropicDiffusion2D::New();
	diffusion->SetInput( vImage );
	diffusion->SetDiffusionFactor( 1.0 );
	diffusion->SetDiffusionThreshold( 200.0 );
	diffusion->SetNumberOfIterations( 5 );

	// Gradient magnitude for edges
	vtkImageGradientMagnitude *grad = vtkImageGradientMagnitude::New();
	grad->SetDimensionality( 2 );
	grad->HandleBoundariesOn();
	grad->SetInputConnection( diffusion->GetOutputPort() );
	grad->Update();

	double* range = grad->GetOutput()->GetScalarRange();

	// Invert the gradient magnitude so that low costs are
	// associated with strong edges and scale from 0 to 1
	vtkImageShiftScale *gradInvert = vtkImageShiftScale::New();
	gradInvert->SetShift( -1.0*range[ 1 ] );
	gradInvert->SetScale( 1.0 /( range[ 0 ] - range[ 1 ] ) );
	gradInvert->SetOutputScalarTypeToFloat();
	gradInvert->SetInputConnection( grad->GetOutputPort() );
	gradInvert->Update();

	// The color map will accept any scalar image type and convert to
	// unsigned char for the image actor
	vtkImageMapToWindowLevelColors *colorMap = vtkImageMapToWindowLevelColors::New();
	colorMap->SetInputConnection( gradInvert->GetOutputPort() );
	range = gradInvert->GetOutput()->GetScalarRange();
	colorMap->SetWindow(1.0);
	colorMap->SetLevel(0.5);

	// flip x axis			Kann ich nicht löschen, warum nicht?
	vtkSmartPointer<vtkImageFlip> flipXFilter =  vtkSmartPointer<vtkImageFlip>::New();
	flipXFilter->SetFilteredAxis(1); // flip x axis -> 0
	flipXFilter->SetInput(colorMap->GetOutput());
	flipXFilter->Update();
	vtkSmartPointer<vtkImageCast> castXFilter = vtkSmartPointer<vtkImageCast>::New();
	castXFilter->SetOutputScalarTypeToUnsignedChar();
	castXFilter->SetInputConnection(flipXFilter->GetOutputPort());
	castXFilter->Update();

	// add image to data storage to see if already messed up
	mitk::Image::Pointer newcostimage2 = mitk::Image::New();
	newcostimage2->Initialize(colorMap->GetOutput());
	newcostimage2->SetVolume(colorMap->GetOutput()->GetScalarPointer());
	mitk::DataNode::Pointer newImageNode3 = mitk::DataNode::New();
	newImageNode3->SetData(newcostimage2);
	//newImageNode->SetData(slice);
	newImageNode3->SetColor(200.0,0,0);
	newImageNode3->SetName("Cost Image");
	int newId = m_Ext->GetUniqueId();
	newImageNode3->SetIntProperty("UniqueID", newId);
	newImageNode3->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
	m_Ext->GetDataStorage()->Add(newImageNode3);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return colorMap->GetOutput();
}

void mitk::LiveWireTool::ColorSurface()
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
	
	std::sort(m_SurfaceIds.begin(), m_SurfaceIds.end());

	int numberOfPointsInSurface = m_Ext->GetSurfacePolyData()->GetNumberOfPoints();
	for(vtkIdType j = 0; j < numberOfPointsInSurface; j++)
	{
		unsigned char red[3] = {0, 0, 255};
		unsigned char green[3] = {0, 255, 0};
		unsigned char blue[3] = {255, 0, 0};
		faceColors->SetName("Colors");

		if(std::binary_search(m_SurfaceIds.begin(), m_SurfaceIds.end(), j))
		{
			//std::cout << "Part of contour.\n";
			faceColors->InsertNextTupleValue(red);
			continue;
		}
		/*if(std::binary_search(pointsToBeDeleted2.begin(), pointsToBeDeleted2.end(), j))
		{
			faceColors->InsertNextTupleValue(red);
			continue;
		}*/
		
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

	m_Ext->GetSurfaceNode()->SetProperty("pointsize", mitk::FloatProperty::New(10.1));

	m_Ext->GetSurfaceNode()->Update();
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::LiveWireTool::Zoom(int zoom)
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

	// test
	//m_Ext->GetSurface()->CalculateBoundingBox();
	//m_Ext->GetSurface()->Update();
	

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::LiveWireTool::ColorNodesROI()
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
		if(j == m_SurfaceHandlePointID)
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

/* LAPLACIAN FRAMEWORK *******************************************************************************************************************/
void mitk::LiveWireTool::Interpolate3D()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	UpdateProgressBar(0);
	QCoreApplication::processEvents();
	InitializeLaplacianFramework();
	UpdateProgressBar(15);
	QCoreApplication::processEvents();
	MoveSurfacePoints();
	UpdateProgressBar(25);
	QCoreApplication::processEvents();
	// TODO convergence kriterium 
	int numberOfIterations = 3;
	for(int i = 0; i < numberOfIterations; i++)
	{
		CalculateLaplacianFramework();
		UpdateProgressBar(30 + i*15);
		QCoreApplication::processEvents();
	}
	UpdateProgressBar(100);
	QCoreApplication::processEvents();
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::LiveWireTool::InitializeLaplacianFramework()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_PointsInROI.clear();
	m_AnchorPointsOutsideROI.clear();
	m_AllPoints.clear();

	// define region of interest around old point position
	vtkIdType inputNumPoints = m_Ext->GetSurfacePolyData()->GetPoints()->GetNumberOfPoints();
	for(vtkIdType ptId = 0; ptId < inputNumPoints; ptId++)
	{
		pdp::Pt pt;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(ptId, pt.toVtk());	

		if(ptId == m_SurfaceHandlePointID)
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
		for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
		{
			std::cout << "Point in ROI: " << *it << "\n";
		}
		std::cout << "Number of Points in ROI: " << m_PointsInROI.size() << "\n";
	}

	// Calculate all points with an edge pointing inside the ROI
	for(std::vector<vtkIdType>::iterator it = m_PointsInROI.begin(); it != m_PointsInROI.end(); it++)
	{
		// Get cells to point
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
					if(ptIds->GetId(j) == m_SurfaceHandlePointID/*m_HandlePointID*/)
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
	// add handle anchor point	//todo add multiple anchor points -> dijkstra contour
	m_AllPoints.push_back(m_SurfaceHandlePointID);
	std::cout << "Handle point id laplace: " << m_SurfaceHandlePointID << "\n";

	// create L-matrix 
	m_Ext->GetSurfacePolyData()->BuildLinks();
	int numberOfAllPoints = m_AllPoints.size();
	int numberOfAnchorPointsOutsideROI = m_AnchorPointsOutsideROI.size();
	m_LMatrix = new double*[numberOfAllPoints];	// square matrix

	// Dijkstral handle points
	int numberOfHandlePoints = m_FirstContourPointsToBeDeletedSurfaceId.size() + m_SecondContourPointsToBeDeletedSurfaceId.size() + 1;

	m_LMatrixWithAnchors = new double*[numberOfAllPoints + numberOfAnchorPointsOutsideROI + numberOfHandlePoints];	// number of cols in matrix
	
	if(true/*m_SelectWeight == 0*/)
	{
		CreateLMatrixWithW1();
	}
	/*else if(m_SelectWeight == 1)
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
	}*/

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
	int handlePointCounter = 0;
	double* newCol = new double[numberOfAllPoints];
	m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI] = newCol;
	for(int col = 0; col < numberOfAllPoints; col++)
	{
		m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][col] = 0.0;
	}
	m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][numberOfAllPoints-1] = 1.0;
	handlePointCounter++;

	// Add further handle points to L-Matrix
	// We need a variable number of rows in the m_LMatrixWithAnchors according to points gained from Dijkstra algorithm
	// first contour part
	for(std::vector<vtkIdType>::iterator it = m_FirstContourPointsToBeDeletedSurfaceId.begin(); it != m_FirstContourPointsToBeDeletedSurfaceId.end(); it++)
	{
		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter] = newCol;

		for(int col = 0; col < numberOfAllPoints; col++)
		{
			m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][col] = 0.0;
		}

		// Search for matrix index to the handle point
		int counter = 0;
		for(std::vector<vtkIdType>::iterator it1 = m_AllPoints.begin(); it1 != m_AllPoints.end(); it1++)
		{
			if(*it1 == *it)
			{
				//std::cout << "Matrix index to handle point found\n";
				m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][counter] = 1.0;
				break;
			}
			counter++;
		}
		handlePointCounter++;
	}
	// second contour part
	for(std::vector<vtkIdType>::iterator it = m_SecondContourPointsToBeDeletedSurfaceId.begin(); it != m_SecondContourPointsToBeDeletedSurfaceId.end(); it++)
	{
		double* newCol = new double[numberOfAllPoints];
		m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter] = newCol;

		for(int col = 0; col < numberOfAllPoints; col++)
		{
			m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][col] = 0.0;
		}

		// Search for matrix index to the handle point
		int counter = 0;
		for(std::vector<vtkIdType>::iterator it1 = m_AllPoints.begin(); it1 != m_AllPoints.end(); it1++)
		{
			if(*it1 == *it)
			{
				//std::cout << "Matrix index to handle point found\n";
				m_LMatrixWithAnchors[numberOfAllPoints+numberOfAnchorPointsOutsideROI+handlePointCounter][counter] = 1.0;
				break;
			}
			counter++;
		}
		handlePointCounter++;
	}

	// print LMatrix
	if(0)
	{
		std::cout << "L-Matrix:\n";
		for(int row = 0; row < numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter - 1; row++)
		{
			std::cout << "\n";
			for(int col = 0; col < numberOfAllPoints; col++)
			{		
				std::cout << m_LMatrixWithAnchors[row][col];
			}
		}
		std::cout << "Number of all Points: " << numberOfAllPoints << "\n";
	}

	if(true/*m_Color*/)
		ColorNodesROI();
}

void mitk::LiveWireTool::MoveSurfacePoints()
{	
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Algorithm: Reduce number of points in dijkstra pointset to fit the size of suface points
	// 1) direktes zuweisen per einfaches laufen über die felder
	std::cout << "First and second end point: " << m_FirstEndPoint.toItk() << ", " << m_SecondEndPoint.toItk() << "\n";
	pdp::Pt leftEndPoint(m_StartPoint);
	pdp::Pt rightEndPoint(m_StopPoint);
	std::cout << "Left and right end point: " << leftEndPoint.toItk() << ", " << rightEndPoint.toItk() << "\n";

	// move points to new position (without ROI edge anchors)
	vtkPolyData* newData2 = vtkPolyData::New();
	newData2->DeepCopy(m_Ext->GetSurfacePolyData());

	// Calculate correct correspondance
	double distanceFirstLeft = sqrt(vtkMath::Distance2BetweenPoints(m_FirstEndPoint.toVtk(), leftEndPoint.toVtk()));
	double distanceFirstRight = sqrt(vtkMath::Distance2BetweenPoints(m_FirstEndPoint.toVtk(), rightEndPoint.toVtk()));

	double diff = 0.0;
	double quotient = 0.0;
	int gap = 0;
	double remainder = 0.0;
	int noHoles = 0;

	int rightContourSize = ((mitk::PointSet*)m_RightContourNode->GetData())->GetSize();
	int leftContourSize = ((mitk::PointSet*)m_LeftContourNode->GetData())->GetSize();
	int firstContourSize = m_FirstContourPointsToBeDeletedSurfaceId.size();
	int secondContourSize = m_SecondContourPointsToBeDeletedSurfaceId.size();
	m_FurtherHandlePoints.clear();

	if(rightContourSize == 0 || leftContourSize == 0 || firstContourSize == 0 || secondContourSize == 0)
	{
		std::cout << "Too few points in contour. Select larger radius.\n";
		return;
	}

	if(distanceFirstLeft > distanceFirstRight)
	{
		std::cout << "First corresponds to right.\n"; 

		// Move points for First and Right 
		int numberOfFirstPointsInROI = firstContourSize;
		if(rightContourSize/*m_RightContour->GetSize()*/ < numberOfFirstPointsInROI)
		{
			// Dijkstra kleiner als Surface
			std::cout << "Dijkstra kleiner als Surface.\n";
			numberOfFirstPointsInROI = rightContourSize;
			
			diff = firstContourSize - rightContourSize;
			quotient = diff / (rightContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP First: " << firstContourSize << ", NP Right: " << rightContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(rightContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
	
			// test forward or backward?
			// ich muss alle 4 möglichkeiten nehmen und nach dem kleinsten abstand suchen
			pdp::Pt ptFirstFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId[0],ptFirstFirst.toVtk());
			pdp::Pt ptFirstLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId.back(),ptFirstLast.toVtk());
			pdp::Pt ptRightFirst(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(0));
			pdp::Pt ptRightLast(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptFirstFirst.toItk() << ", " << ptFirstLast.toItk() << ", " << ptRightFirst.toItk() << ", " << ptRightLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptRightFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptRightLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptRightFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptRightLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "FirstFirst correstponds to RightFirst -> ok.\n"; 
				std::cout << "POSITION: 1\n";
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(i));
					newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
				}
			}
			else
			{
				std::cout << "FirstLast corresponds to RightFirst -> reverse.\n"; 
				std::cout << "POSITION: 2\n";
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(firstContourSize - (i+1)));
					newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
				}
			}
		}
		else
		{
			// Dijkstra größer als Surface
			std::cout << "Dijkstra größer als Surface.\n";
			diff = rightContourSize - firstContourSize;
			quotient = diff / (firstContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP First: " << firstContourSize << ", NP Right: " << rightContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(firstContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
		
			// test forward or backward?
			pdp::Pt ptFirstFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId[0],ptFirstFirst.toVtk());
			pdp::Pt ptFirstLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId.back(),ptFirstLast.toVtk());
			pdp::Pt ptRightFirst(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(0));
			pdp::Pt ptRightLast(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptFirstFirst.toItk() << ", " << ptFirstLast.toItk() << ", " << ptRightFirst.toItk() << ", " << ptRightLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptRightFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptRightLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptRightFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptRightLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "FirstFirst corresponds to RightFirst -> ok.\n"; 
				std::cout << "POSITION: 3\n";
				int counter = 1;
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(i));
						newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							// no Holes mit Gap = Gap-1
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((i+1) * gap));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((i+1) * gap + counter /*gap*/));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
			else
			{
				std::cout << "FirstLast corresponds to RightFirst -> reverse.\n"; 
				std::cout << "POSITION: 4\n";
				int counter = 1;
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(leftContourSize - (i+1)));
						newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							// no Holes mit Gap = Gap-1
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize - (i+1) * gap));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((rightContourSize-1) - (i+1) * gap + counter /*gap*/));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
		}

		// Move points for Second and Left
		int numberOfSecondPointsInROI = secondContourSize;
		if(leftContourSize < numberOfSecondPointsInROI)
		{
			// Dijkstra kleiner als Surface
			std::cout << "Dijkstra kleiner als Surface.\n";
			numberOfSecondPointsInROI = leftContourSize;

			diff = secondContourSize - leftContourSize;
			quotient = diff / (leftContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP Second: " << secondContourSize << ", NP Left: " << leftContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(leftContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
	
			// test forward or backward?
			pdp::Pt ptSecondFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId[0],ptSecondFirst.toVtk());
			pdp::Pt ptSecondLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId.back(),ptSecondLast.toVtk());
			pdp::Pt ptLeftFirst(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(0));
			pdp::Pt ptLeftLast(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptSecondFirst.toItk() << ", " << ptSecondLast.toItk() << ", " << ptLeftFirst.toItk() << ", " << ptLeftLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptLeftFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptLeftLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptLeftFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptLeftLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "SecondFirst corresponds to LeftFirst -> ok.\n"; 
				std::cout << "POSITION: 5\n";

				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(i)/*m_LeftContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
				}
			}
			else
			{
				std::cout << "SecondLast corresponds to LeftFirst -> reverse.\n"; 
				std::cout << "POSITION: 6\n";

				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - (i+1))/*m_LeftContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
				}
			}
		}
		else
		{
			// Dijkstra größer als Surface
			std::cout << "Dijkstra größer als Surface.\n";
			diff = leftContourSize - secondContourSize;
			quotient = diff / (secondContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP Second: " << secondContourSize << ", NP Left: " << leftContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(secondContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
			
			// test forward or backward?
			pdp::Pt ptSecondFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId[0],ptSecondFirst.toVtk());
			pdp::Pt ptSecondLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId.back(),ptSecondLast.toVtk());
			pdp::Pt ptLeftFirst(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(0));
			pdp::Pt ptLeftLast(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptSecondFirst.toItk() << ", " << ptSecondLast.toItk() << ", " << ptLeftFirst.toItk() << ", " << ptLeftLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptLeftFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptLeftLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptLeftFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptLeftLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "SecondFirst corresponds to LeftFirst -> ok.\n"; 
				std::cout << "POSITION: 7\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(i));
						newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((i+1) * gap));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((i+1) * gap + counter /*gap*/));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
			else
			{
				std::cout << "SecondLast corresponds to LeftFirst -> reverse.\n"; 
				std::cout << "POSITION: 8\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - (i+1)));
						newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - (i+1) * gap));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((leftContourSize-1) -((i+1) * gap + counter /*gap*/)));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}	
		}
	}
	else
	{
		std::cout << "First corresponds to left.\n";

		// Move points for First and Left 
		int numberOfFirstPointsInROI = firstContourSize;
		if(leftContourSize < numberOfFirstPointsInROI)
		{
			// Dijkstra kleiner als Surface
			std::cout << "Dijkstra kleiner als Surface.\n";
			numberOfFirstPointsInROI = leftContourSize;
		
			diff = firstContourSize - leftContourSize;
			quotient = diff / (leftContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP First: " << firstContourSize << ", NP Left: " << leftContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(leftContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
		
			// test forward or backward?
			pdp::Pt ptFirstFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId[0],ptFirstFirst.toVtk());
			pdp::Pt ptFirstLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId.back(),ptFirstLast.toVtk());
			pdp::Pt ptLeftFirst(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(0));
			pdp::Pt ptLeftLast(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize-1));
			std::cout << "First and Last Points: " << ptFirstFirst.toItk() << ", " << ptFirstLast.toItk() << ", " << ptLeftFirst.toItk() << ", " << ptLeftLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptLeftFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptLeftLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptLeftFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptLeftLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "FirstFirst corresponds to LeftFirst -> ok.\n"; 
				std::cout << "POSITION: 9\n";

				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(i)/*m_LeftContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
				}
			}
			else
			{
				std::cout << "FirstLast corresponds to LeftFirst -> reverse.\n"; 
				std::cout << "POSITION: 10\n";

				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - (i+1))/*m_LeftContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
				}	
			}
		}
		else
		{
			// Dijkstra größer als Surface
			std::cout << "Dijkstra größer als Surface.\n";
			diff = leftContourSize - firstContourSize;
			quotient = diff / (firstContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP First: " << firstContourSize << ", NP Left: " << leftContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(firstContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
		
			// test forward or backward?
			pdp::Pt ptFirstFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId[0],ptFirstFirst.toVtk());
			pdp::Pt ptFirstLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_FirstContourPointsToBeDeletedSurfaceId.back(),ptFirstLast.toVtk());
			pdp::Pt ptLeftFirst(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(0));
			pdp::Pt ptLeftLast(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptFirstFirst.toItk() << ", " << ptFirstLast.toItk() << ", " << ptLeftFirst.toItk() << ", " << ptLeftLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptLeftFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstFirst.toVtk(), ptLeftLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptLeftFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptFirstLast.toVtk(), ptLeftLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "FirstFirst corresponds to LeftFirst -> ok.\n"; 
				std::cout << "POSITION: 11\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(i));
						newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((i+1) * gap));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((i+1) * gap + counter));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
			else
			{
				std::cout << "FirstLast corresponds to LeftFirst -> reverse.\n"; 
				std::cout << "POSITION: 12\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator firstSurfaceIdIt = m_FirstContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfFirstPointsInROI; i++, firstSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - (i+1)));
						newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(leftContourSize - ((i+1) * gap)));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint((leftContourSize-1) - ((i+1) * gap + counter)));
							newData2->GetPoints()->SetPoint(*firstSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*firstSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
		}

		// Move points for Second and Right
		int numberOfSecondPointsInROI = secondContourSize;
		if(rightContourSize < numberOfSecondPointsInROI)
		{
			// Dijkstra kleiner als Surface
			std::cout << "Dijkstra kleiner als Surface.\n";
			numberOfSecondPointsInROI = rightContourSize;
					
			diff = secondContourSize - rightContourSize;
			quotient = diff / (rightContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP Second: " << secondContourSize << ", NP Right: " << rightContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(rightContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";

			// test forward or backward?
			pdp::Pt ptSecondFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId[0],ptSecondFirst.toVtk());
			pdp::Pt ptSecondLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId.back(),ptSecondLast.toVtk());
			pdp::Pt ptRightFirst(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(0));
			pdp::Pt ptRightLast(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptSecondFirst.toItk() << ", " << ptSecondLast.toItk() << ", " << ptRightFirst.toItk() << ", " << ptRightLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptRightFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptRightLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptRightFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptRightLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "SecondFirst corresponds to RightFirst -> ok.\n"; 
				std::cout << "POSITION: 13\n";
	
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(i)/*m_RightContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
				}
			}
			else
			{
				std::cout << "SecondLast correstponds to RightFirst -> reverse.\n"; 
				std::cout << "POSITION: 14\n";
	
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize - (i+1))/*m_RightContour->GetPoint(i)*/);
					newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
					m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
				}
			}
		}
		else
		{
			// Dijkstra größer als Surface
			std::cout << "Dijkstra größer als Surface.\n";
			diff = rightContourSize - secondContourSize;
			quotient = diff / (secondContourSize + 1); // differenz / anzahl an lücken
			std::cout << "NP Second: " << secondContourSize << ", NP Right: " << rightContourSize << ", diff: " << diff << "\n";
			gap = ceil(quotient);
			remainder = quotient - (int)quotient;
			noHoles = (1-remainder)*(secondContourSize + 1);
			std::cout << "Gap: " << gap << ", remainder: " << remainder <<  ", noholes: " << noHoles << "\n";
	
			// test forward or backward?
			pdp::Pt ptSecondFirst;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId[0],ptSecondFirst.toVtk());
			pdp::Pt ptSecondLast;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SecondContourPointsToBeDeletedSurfaceId.back(),ptSecondLast.toVtk());
			pdp::Pt ptRightFirst(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(0));
			pdp::Pt ptRightLast(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize-1));
			//std::cout << "First and Last Points: " << ptSecondFirst.toItk() << ", " << ptSecondLast.toItk() << ", " << ptRightFirst.toItk() << ", " << ptRightLast.toItk() << "\n";
			double distanceFirst2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptRightFirst.toVtk()));
			double distanceFirst2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondFirst.toVtk(), ptRightLast.toVtk()));
			double distanceLast2First = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptRightFirst.toVtk()));
			double distanceLast2Last = sqrt(vtkMath::Distance2BetweenPoints(ptSecondLast.toVtk(), ptRightLast.toVtk()));

			std::vector<double> distances;
			distances.push_back(distanceFirst2First);
			distances.push_back(distanceFirst2Last);
			distances.push_back(distanceLast2First);
			distances.push_back(distanceLast2Last);
			std::sort(distances.begin(), distances.end());
			double distanceMin = *std::min_element(distances.begin(), distances.end());
			//std::cout << "D1: " << distanceFirst2First << ", D2: " << distanceFirst2Last << ", D3: " << distanceLast2First << ", D3: " << distanceLast2Last << ", Min: " << distanceMin << "\n";

			if(distanceFirst2First == distanceMin || distanceLast2Last == distanceMin)
			{
				std::cout << "SecondFirst corresponds to RightFirst -> ok.\n"; 
				std::cout << "POSITION: 15\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(i));
						newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((i+1) * gap));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((i+1) * gap + counter /*gap*/));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
			else
			{
				std::cout << "SecondFirst corresponds to RightFirst -> reverse.\n"; 
				std::cout << "POSITION: 16\n";

				int counter = 1;
				std::vector<vtkIdType>::iterator secondSurfaceIdIt = m_SecondContourPointsToBeDeletedSurfaceId.begin();
				for(int i = 0; i < numberOfSecondPointsInROI; i++, secondSurfaceIdIt++)
				{
					if(gap == 0)
					{
						pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize - (i+1)));
						newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
						m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
					}
					else
					{
						if(i <= noHoles)
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(rightContourSize - (i+1) * gap));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
						}
						else
						{
							pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint((rightContourSize-1) - ((i+1) * gap + counter /*gap*/)));
							newData2->GetPoints()->SetPoint(*secondSurfaceIdIt, newPt.toVtk());
							m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(*secondSurfaceIdIt, newPt.toItk()));
							counter++;
						}
					}
				}
			}
		}
	}

	// Move handle point
	pdp::Pt newPt(m_WorldPoint);
	newData2->GetPoints()->SetPoint(m_SurfaceHandlePointID, newPt.toVtk());
	m_FurtherHandlePoints.insert(std::pair<vtkIdType,mitk::Point3D>(m_SurfaceHandlePointID, newPt.toItk()));

	// Show surface with moved points
	if(1)
	{
	mitk::Surface::Pointer surfaceWithMovedPoints = mitk::Surface::New();
	surfaceWithMovedPoints->SetVtkPolyData(newData2);
	mitk::DataNode::Pointer surfaceWithMovedPointsNode = mitk::DataNode::New();
	surfaceWithMovedPointsNode->SetProperty("pointsize", mitk::FloatProperty::New(0.3));
	surfaceWithMovedPointsNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	surfaceWithMovedPointsNode->SetData(surfaceWithMovedPoints);
	// Add unique identifier for data nodes
	int newId = m_Ext->GetUniqueId();
	surfaceWithMovedPointsNode->SetIntProperty("UniqueID", newId);
	//m_Ext->GetDataStorage()->Add(surfaceWithMovedPointsNode);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	}

	// Print Contours
	if(0)
	{
		std::cout << "Right Contour: \n";
		for(int i = 0; i < ((mitk::PointSet*)m_RightContourNode->GetData())->GetSize(); i++)
		{
			pdp::Pt newPt(((mitk::PointSet*)m_RightContourNode->GetData())->GetPoint(i));
			std::cout << newPt.toItk() << "\n";
		}
		std::cout << "Left Contour: \n";
		for(int i = 0; i < ((mitk::PointSet*)m_LeftContourNode->GetData())->GetSize(); i++)
		{
			pdp::Pt newPt(((mitk::PointSet*)m_LeftContourNode->GetData())->GetPoint(i));
			std::cout << newPt.toItk() << "\n";
		}
		std::cout << "First Contour: \n";
		for(std::vector<vtkIdType>::iterator it = m_FirstContourPointsToBeDeletedSurfaceId.begin(); it != m_FirstContourPointsToBeDeletedSurfaceId.end(); it++)
		{
			pdp::Pt newPt;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, newPt.toVtk());
			std::cout << newPt.toItk() << "\n";
		}
		std::cout << "Second Contour: \n";
		for(std::vector<vtkIdType>::iterator it = m_SecondContourPointsToBeDeletedSurfaceId.begin(); it != m_SecondContourPointsToBeDeletedSurfaceId.end(); it++)
		{
			pdp::Pt newPt;
			m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, newPt.toVtk());
			std::cout << newPt.toItk() << "\n";
		}
	}

	// todo: if dijkstra kleiner surface

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::LiveWireTool::CalculateLaplacianFramework()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
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

	// Dijkstral handle points
	int numberOfHandlePoints = m_FirstContourPointsToBeDeletedSurfaceId.size() + m_SecondContourPointsToBeDeletedSurfaceId.size() + 1;

	// vector b consists of (delta, w*vi)
	double** B = new double*[numberOfAllPoints+numberOfAnchorPointsOutsideROI+numberOfHandlePoints];
	//std::cout << "B size: " << numberOfAllPoints+numberOfAnchorPointsOutsideROI+numberOfHandlePoints << ", " << 3 << "\n";
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
	int handlePointCounter = 0;
	double* newRow = new double[3];
	B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter] = newRow;
	B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][0] = m_WorldPoint[0];	// current world position -> ok?
	B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][1] = m_WorldPoint[1];
	B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][2] = m_WorldPoint[2];

	//std::cout << "WorldPoint: " << m_WorldPoint.toItk() << "\n";

	// Add further handle points according to Dijkstal contour
	// first contour
	//std::cout << "HandlePointCounter: " << numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter << "\n";
	//std::cout << "First contour size: " << m_FirstContourPointsToBeDeletedSurfaceId.size() << ", second: " << m_SecondContourPointsToBeDeletedSurfaceId.size() << "\n";
	for(std::vector<vtkIdType>::iterator it = m_FirstContourPointsToBeDeletedSurfaceId.begin(); it != m_FirstContourPointsToBeDeletedSurfaceId.end(); it++)
	{
/*		pdp::Pt pt2;
		m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, pt2.toVtk());*/

		// dürfte eigentlich nie aufgerufen werden
		if(m_FurtherHandlePoints.find(*it) == m_FurtherHandlePoints.end())
		{
			//std::cout << "Element not found in map!\n";
			continue;
		}
		
		handlePointCounter++;

		mitk::Point3D pt2 = m_FurtherHandlePoints.find(*it)->second;

		//std::cout << "Pt2: " << pt2 << "\n";

		double* newRow = new double[3];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter] = newRow;
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][0] = pt2[0];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][1] = pt2[1];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][2] = pt2[2];		

		//std::cout << "HandlePointCounter: " << numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter << "\n";
	}
	//std::cout << "HandlePointCounter: " << numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter << "\n";
	// second contour
	for(std::vector<vtkIdType>::iterator it = m_SecondContourPointsToBeDeletedSurfaceId.begin(); it != m_SecondContourPointsToBeDeletedSurfaceId.end(); it++)
	{
		// dürfte eigentlich nie aufgerufen werden
		if(m_FurtherHandlePoints.find(*it) == m_FurtherHandlePoints.end())
		{
			//std::cout << "Element not found in map!\n";
			continue;
		}
		
		handlePointCounter++;
		
		//pdp::Pt pt2;
		//m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*it, pt2.toVtk());	

		mitk::Point3D pt2 = m_FurtherHandlePoints.find(*it)->second;

		double* newRow = new double[3];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter] = newRow;
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][0] = pt2[0];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][1] = pt2[1];
		B[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter][2] = pt2[2];
	}
	//std::cout << "HandlePointCounter: " << numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter << "\n";

	//std::cout << "Create X_OUT\n";

	// test: old position: mesh should not move at all	
	//pdp::Pt handle;
	//m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(m_SurfaceHandlePointID/*m_HandlePointID*/, handle.toVtk());
	//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][0] = handle[0];
	//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][1] = handle[1];
	//B[numberOfAllPoints + numberOfAnchorPointsOutsideROI][2] = handle[2];

	// solve linear equation system: Lx = delta in least squares sense and reconstruct cartesian surface coordinates
	//static int 	SolveLeastSquares (int numberOfSamples, double **xt, int xOrder, double **yt, int yOrder, double **mt, int checkHomogeneous=1)
	//static int 	SolveLeastSquares (#Zeilen, double **xt, int xOrder, double **yt, int yOrder, double **mt, int checkHomogeneous=1)
	// allocate X_OUT
	double** X_OUT = new double*[numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter + 1];
	for(int i = 0; i < numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter + 1; i++)
	{
		double* newRow = new double[3];
		X_OUT[i] = newRow;
		for(int j = 0; j < 3; j++)
		{
			X_OUT[i][j] = 0.0;
		}
	}

	//std::cout << "Create number of rows\n";

	int numberOfRows = numberOfAllPoints + numberOfAnchorPointsOutsideROI + handlePointCounter -1; // sinn

	//std::cout << "numberOfAllPoints + numberOfAnchorPointsOutsideROI + 1: " << numberOfRows << "\n";
	//std::cout << "numberOfAllPoints: " << numberOfAllPoints << "\n";
	//std::cout << "numberOfAnchorPointsOutsideROI: " << numberOfAnchorPointsOutsideROI << "\n";

	//std::cout << "Number of rows: " << numberOfRows << "\n";
	//std::cout << "numberOfAllPoints: " << numberOfAllPoints << "\n";

	for(int row = 0; row < numberOfRows; row++)
	{
		for(int col = 0; col < numberOfAllPoints; col++)
		{
			double dummy = m_LMatrixWithAnchors[row][col];
		}
	}
	//std::cout << "Dummy passed\n";

	if(vtkMath::SolveLeastSquares(numberOfRows, m_LMatrixWithAnchors, numberOfAllPoints, B, 3, X_OUT, 0 /*known to be non-homogeneous*/) == 0 )
	{
		std::cout << "Error solving matrix!\n";
		return;
	}
	else
	{
		std::cout << "Matrix solved!\n";
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
		//pdp::Pt pt2;
		//m_Ext->GetSurfacePolyData()->GetPoints()->GetPoint(*originalIt2, pt2.toVtk());	

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

	std::cout << "Done!\n";
}

void mitk::LiveWireTool::CreateLMatrixWithW1()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
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