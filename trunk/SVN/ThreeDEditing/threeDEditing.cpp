#include <iostream>
// QT
#include <QtGui/QToolButton>
#include <QFile>
#include <QtGui/QHBoxLayout>
#include <QMenu>
#include <QTabWidget>
#include <QSpinBox>

// VTK
#include <vtkMarchingCubes.h>
#include <vtkImageStencil.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkSmoothPolyDataFilter.h>
//#include <vtkRendererCollection.h>
#include <vtkCubeSource.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkMassProperties.h>
#include <vtkImageWriter.h>
#include <vtkRenderLargeImage.h>
#include <vtkPNGWriter.h>
#include <vtkClipPolyData.h>
#include <vtkBox.h>
#include <vtkFillHolesFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkSTLWriter.h>

// ITK
#include <itkRGBPixel.h>

// MITK
#include "mitkStandaloneDataStorage.h"
#include "QmitkStdMultiWidget.h"
#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>
#include <QmitkExt/QmitkApplicationBase/QmitkCommonFunctionality.h>
#include "mitkVtkRepresentationProperty.h"
#include <mitkMoveSurfaceInteractor.h>
#include <QmitkToolSelectionBox.h>
#include <QmitkDataStorageTreeModel.h>
#include <mitkMapper.h>
#include <mitkGPUVolumeMapper3D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include "mitkSurfaceToImageFilter.h"
#include <mitkPixelType.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkPropertyList.h>
#include <mitkCoreExtObjectFactory.h>
#include <mitkColorProperty.h>
#include "mitkDisplayInteractor.h"
#include <mitkSurfaceVtkWriter.h>
#include <mitkImageWriter.h>
#include "mitkShowSegmentationAsSurface.h"
#include "mitkManualSegmentationToSurfaceFilter.h"

// Sonst
#include <time.h>

// PDP
#include "../../mitkpdp/gui/mainwindow.h"
#include "global.h"
#include "toolbar.h"
#include "dataManager.h"
#include "RenderingManagerProperty.h"

#include "threeDEditing.h"

pdp::ThreeDEditing::ThreeDEditing(MainWindow* mainwin)
    : m_MainWindow(mainwin)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_Data = new LungDataset();
	RegisterCoreExtObjectFactory();
	m_ConvertSegmentationData = false;
	m_HasWorkingDataSet = false;
	m_HasReferenceDataSet = false;
	m_UseTriangle = false;
	m_UseClean = false;
	m_UseSmooth = false;

	// set up statemachine
	if(AT_HOME == 1)
	{
		mitk::GlobalInteraction::GetInstance()->GetStateMachineFactory()->LoadBehavior("C:\\DA\\SVN\\ThreeDEditing\\StateMachine.xml");
		//mitk::GlobalInteraction::GetInstance()->GetStateMachineFactory()->LoadBehavior("LocalStateMachine.xml");
	}
	else
	{
		mitk::GlobalInteraction::GetInstance()->GetStateMachineFactory()->LoadBehavior("E:\\Media Informatics\\thesis\\Hendrik_PhanAnh_pdp\\PA_Hendrik\\PA_Hendrik\\pdp\\Hendrik_pdp\\SVN\\ThreeDEditing\\StateMachine.xml");
		//mitk::GlobalInteraction::GetInstance()->GetStateMachineFactory()->LoadBehavior("LocalStateMachine.xml");
	}

	QColor color(214,247,202);
	QPalette colorPalette(color);
	this->setAutoFillBackground(true);
	this->setPalette(colorPalette);

	m_LastColorIndex = 0;
	//float lastColor[3] = {255, 0, 0};
	//m_Colors.push_back(lastColor);
	//float lastColor1[3] = {0, 255, 0};
	//m_Colors.push_back(lastColor1);
	//float lastColor2[3] = {0, 0, 255};
	//m_Colors.push_back(lastColor2);

	/*float lastColor[3] = {255/256, 0/256, 0/256};
	m_Colors.push_back(lastColor);
	float lastColor1[3] = {0/256, 255/256, 0/256};
	m_Colors.push_back(lastColor1);
	float lastColor2[3] = {0/256, 0/256, 255/256};
	m_Colors.push_back(lastColor2);*/

	m_Colors.push_back(mitk::ColorProperty::New(0.4,0.0,0.0));
	m_Colors.push_back(mitk::ColorProperty::New(0.0,0.4,0.0));
	m_Colors.push_back(mitk::ColorProperty::New(0.0,0.0,0.4));
	m_Colors.push_back(mitk::ColorProperty::New(0.4,0.4,0.0));
	m_Colors.push_back(mitk::ColorProperty::New(0.0,0.4,0.4));
	m_Colors.push_back(mitk::ColorProperty::New(0.4,0.0,0.4));
	m_Colors.push_back(mitk::ColorProperty::New(0.4,0.4,0.4));

	m_Play = true;
	m_ElapsedTime = 0;
	m_DurationOfTest = 120; // in seconds
	m_SaveDataEveryXSeconds = 5;

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::ThreeDEditing::~ThreeDEditing()
{
	m_Toolbar->RemoveToolbar();
	*m_MainWindow->GetNumberOfInstancesOfThreeDEditing() = 0;
}

void pdp::ThreeDEditing::Run()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	this->setWindowIconText(QApplication::translate("SingleThickenings", "Form", 0, QApplication::UnicodeUTF8));
    m_MainWindow->addTab(this);
	SetUpGui();

	// AutoRun ******************************************************
	if(AUTO_IMPORT == 1)
		AutoLoadFile();

	UseMITKSegmentationTools();
	
	// Initialize ToolManager
	if(1)
	{
		//UseMITKSegmentationTools();

		m_Tabs->setCurrentIndex(1);
		mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
		//std::cout << "Number of registered tools: " << manager->GetTools().size() << "\n";

		for(int i = 0; i < manager->GetTools().size(); i++)
		{
			mitk::Tool* currentTool = manager->GetToolById(i);
			//std::cout << "Tool found: " << currentTool->GetName() << "\n";
			//std::string toolName = "Bulge Tool";
			std::string toolName = "Test Tool";
			//std::string toolName = "Registration Tool";
			//std::string toolName = "Refinement Tool";
			//std::string toolName = "Live Wire Tool";
			//std::string toolName = "Active Surface Tool";
			//std::string toolName = "Drag Tool";
			if(toolName.compare(currentTool->GetName()) == 0)
			{
				//std::cout << "Bulge Tool found.\n";
				manager->ActivateTool(i);
				break;
			}
		}
	}

	/****************************************************************/

	//m_Data->getDataStore()->ChangedNodeEvent.AddListener(mitk::MessageDelegate1<ThreeDEditing, const mitk::DataNode *>(this, &pdp::ThreeDEditing::CatchChangedNodesEvents));

	// catch active tool changed message
	mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
	manager->ActiveToolChanged += mitk::MessageDelegate<pdp::ThreeDEditing>(this, &pdp::ThreeDEditing::ActiveToolChangedChangeTabs);

	//m_Toolbar->ToggleDataManager();
	//m_Toolbar->ToggleToolManager();

	//FirstScenario();

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::ActiveToolChangedChangeTabs()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
	int currentToolId = manager->GetActiveToolID();
	int currentTab = m_Tabs->currentIndex();

	if(currentToolId == -1)
	{
		// Tools deactivated -> allow tab changes
		m_Tabs->setTabEnabled(0,true);
		m_Tabs->setTabEnabled(1,true);
		m_Tabs->setTabEnabled(2,true);
		m_Tabs->setTabToolTip(0,"Switch to 2D tools.");
		m_Tabs->setTabToolTip(1,"Switch to 3D tools.");
		m_Tabs->setTabToolTip(2,"Switch to Options.");
	}
	else
	{
		// Tool activated -> disable tab changes
		if(currentTab == 0)
		{
			// 2D tool 
			m_Tabs->setTabEnabled(1,false);
			m_Tabs->setTabEnabled(2,false);
			m_Tabs->setTabToolTip(0,"Deactivate selected tool before switching tabs.");
			m_Tabs->setTabToolTip(1,"Deactivate selected tool before switching tabs.");
			m_Tabs->setTabToolTip(2,"Deactivate selected tool before switching tabs.");

		}
		if(currentTab == 1)
		{
			// 3D tool
			m_Tabs->setTabEnabled(0,false);
			m_Tabs->setTabEnabled(2,false);
			m_Tabs->setTabToolTip(0,"Deactivate selected tool before switching tabs.");
			m_Tabs->setTabToolTip(1,"Deactivate selected tool before switching tabs.");
			m_Tabs->setTabToolTip(2,"Deactivate selected tool before switching tabs.");		
		}	
	}

	//std::cout << "Current Tool Id: " << currentToolId << "\n";
	//std::cout << "Current Tab: " << currentTab << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::SetUpGui()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_Layout= new QHBoxLayout(this);
	m_Layout->setObjectName(QString::fromUtf8("layout_main"));
	
	QVBoxLayout* verticalLayout1 = new QVBoxLayout();
	verticalLayout1->setObjectName(QString::fromUtf8("layout_left"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
		
	QVBoxLayout* verticalLayout2 = new QVBoxLayout();
	verticalLayout2->setObjectName(QString::fromUtf8("layout_right"));
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
   	
	m_Layout->addLayout(verticalLayout1);
	m_Layout->addLayout(verticalLayout2);

	m_MitkView = new QmitkStdMultiWidget(this);
	m_MitkView->setObjectName(QString::fromUtf8("mitkView"));
	//sizePolicy1.setHeightForWidth(m_MitkView->sizePolicy().hasHeightForWidth());
	//sizePolicy1.setHorizontalPolicy(QSizePolicy::Fixed);
	//m_MitkView->setSizePolicy(sizePolicy1);
	const char* propertyKey = "Properties"; 
	RenderingManagerProperty* properties = new RenderingManagerProperty();
	properties->SetStdMultiWidget(m_MitkView);
	properties->SetReinitSurrounding(0);
	mitk::RenderingManager::GetInstance()->SetProperty(propertyKey, properties);
	
	m_DataManager = new DataManager(this);
	sizePolicy.setHeightForWidth(m_DataManager->GetTreeView()->sizePolicy().hasHeightForWidth());
	//sizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
	m_DataManager->GetTreeView()->setSizePolicy(sizePolicy);
	m_DataManager->GetTreeView()->setObjectName ("datamanager");
	verticalLayout1->addWidget(m_DataManager->GetTreeView());
	verticalLayout2->addWidget(m_MitkView);

	// horizontal toolbar
	m_Toolbar = new Toolbar(this);
	m_Toolbar->AddToolbar();
	AddTools();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::SetUpMitkView()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//m_MitkView->InitializeWidget();
	m_MitkView->SetDataStorage(m_Data->getDataStore());

    //// Initialize views as transversal, sagittal, coronar to all data objects in DataStorage
    //// (from top-left to bottom)
    //mitk::TimeSlicedGeometry::Pointer geo = m_Data->getDataStore()->ComputeBoundingGeometry3D(m_Data->getDataStore()->GetAll());
    //mitk::RenderingManager::GetInstance()->InitializeViews(geo);

    //// Initialize bottom-right view as 3D view
    //m_MitkView->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

    //// Enable standard handler for levelwindow-slider
    m_MitkView->EnableStandardLevelWindow();

    //// Add the displayed views to the tree to see their positions
    //// in 2D and 3D
   // m_MitkView->AddDisplayPlaneSubTree();

	// Renderer window context menu, rotation mode, swivel etc
    // m_MitkView->EnableNavigationControllerEventListening();

	// crashes, why?
	//m_MitkView->EnablePositionTracking();

	m_MitkView->DisableDepartmentLogo();

	//vtkSmartPointer<vtkRendererCollection> rendererCollection = m_MitkView->GetRenderWindow4()->GetVtkRenderWindow()->GetRenderers();
	//vtkSmartPointer<vtkRenderer> renderer = rendererCollection->GetFirstRenderer();
	//renderer->RemoveAllLights();

	// Zooming and panning for all 2D planes
    mitk::GlobalInteraction::GetInstance()->AddListener(m_MitkView->GetMoveAndZoomInteractor());

	// Scrolling via mouse movement
	//m_Scroll = mitk::DisplayVectorInteractorScroll::New("LeftClickScroll", new mitk::DisplayInteractor());
	//mitk::GlobalInteraction::GetInstance()->AddListener(m_Scroll);

	m_DataManager->UpdateTreeModel();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::AddTools() // to toolbar
{
	QToolButton* btn_PrintDataStorage = new QToolButton(m_Toolbar->GetToolbar());
	QIcon icn_PrintDataStorage;
	icn_PrintDataStorage.addFile(":/threeDEditing/res/threeDEditing/print.png");
	btn_PrintDataStorage->setIcon(icn_PrintDataStorage);
	btn_PrintDataStorage->setToolTip("Print information on data nodes to the console.");
	m_Toolbar->GetToolbar()->addWidget(btn_PrintDataStorage);
	QObject::connect(btn_PrintDataStorage, SIGNAL(clicked()), this, SLOT(PrintDataStorage()));

	QToolButton* btn_CreateSurface = new QToolButton(m_Toolbar->GetToolbar());
	QIcon icn_CreateSurface;
	icn_CreateSurface.addFile(":/threeDEditing/res/threeDEditing/CreateSurface-32.png");
	btn_CreateSurface->setIcon(icn_CreateSurface);
	btn_CreateSurface->setToolTip("Create surface from image.");
	m_Toolbar->GetToolbar()->addWidget(btn_CreateSurface);
	QObject::connect(btn_CreateSurface, SIGNAL(clicked()), this, SLOT(CreateSurface()));

	QToolButton* btn_TakeScreenshot = new QToolButton(m_Toolbar);
	QIcon icn_TakeScreenshot;
	icn_TakeScreenshot.addFile(":/threeDEditing/res/threeDEditing/fotoapparat.png");
	btn_TakeScreenshot->setIcon(icn_TakeScreenshot);
	btn_TakeScreenshot->setToolTip("Take Screenshot.");
	m_Toolbar->GetToolbar()->addWidget(btn_TakeScreenshot);
	QObject::connect(btn_TakeScreenshot, SIGNAL(clicked()), this, SLOT(TakeScreenshot()));

	QToolButton* btn_CalculateVolume = new QToolButton(m_Toolbar);
	QIcon icn_CalculateVolume;
	icn_CalculateVolume.addFile(":/threeDEditing/res/threeDEditing/volume.png");
	btn_CalculateVolume->setIcon(icn_CalculateVolume);
	btn_CalculateVolume->setToolTip("Calculate Volume.");
	m_Toolbar->GetToolbar()->addWidget(btn_CalculateVolume);
	QObject::connect(btn_CalculateVolume, SIGNAL(clicked()), this, SLOT(CalculateVolume()));

	QToolButton* btn_CurrentVolume = new QToolButton(m_Toolbar);
	QIcon icn_CurrentVolume;
	icn_CurrentVolume.addFile(":/threeDEditing/res/threeDEditing/vol2.png");
	btn_CurrentVolume->setIcon(icn_CurrentVolume);
	btn_CurrentVolume->setToolTip("Current Volume.");
	m_Toolbar->GetToolbar()->addWidget(btn_CurrentVolume);
	QObject::connect(btn_CurrentVolume, SIGNAL(clicked()), this, SLOT(CurrentVolume()));

	m_Toolbar->GetToolbar()->addSeparator();

	QToolButton* btn_FirstScenario = new QToolButton(m_Toolbar);
	QIcon icn_FirstScenario;
	icn_FirstScenario.addFile(":/threeDEditing/res/threeDEditing/1.png");
	btn_FirstScenario->setIcon(icn_FirstScenario);
	btn_FirstScenario->setToolTip("First Scenario.");
	m_Toolbar->GetToolbar()->addWidget(btn_FirstScenario);
	QObject::connect(btn_FirstScenario, SIGNAL(clicked()), this, SLOT(FirstScenario()));

	QToolButton* btn_SecondScenario = new QToolButton(m_Toolbar);
	QIcon icn_SecondScenario;
	icn_SecondScenario.addFile(":/threeDEditing/res/threeDEditing/2.png");
	btn_SecondScenario->setIcon(icn_SecondScenario);
	btn_SecondScenario->setToolTip("Second Scenario.");
	m_Toolbar->GetToolbar()->addWidget(btn_SecondScenario);
	QObject::connect(btn_SecondScenario, SIGNAL(clicked()), this, SLOT(SecondScenario()));

	m_Btn_StartSegmentation = new QToolButton(m_Toolbar);
	QIcon icn_StartSegmentation;
	icn_StartSegmentation.addFile(":/threeDEditing/res/threeDEditing/Forward.png");
	m_Btn_StartSegmentation->setIcon(icn_StartSegmentation);
	m_Btn_StartSegmentation->setToolTip("Start Segmentation.");
	m_Toolbar->GetToolbar()->addWidget(m_Btn_StartSegmentation);
	QObject::connect(m_Btn_StartSegmentation, SIGNAL(clicked()), this, SLOT(StartSegmentation()));

	QLabel* label5 = new QLabel( "  Elapsed time:  ", this );
	QFont f5 = label5->font();
	f5.setBold(false);
	label5->setFont( f5 );
	m_Toolbar->GetToolbar()->addWidget(label5);

	m_Time = new QLineEdit(this);
	m_Time->setText("0 seconds");
	m_Time->setReadOnly(true);
	m_Time->setFixedWidth(80);
	m_Time->setAlignment(Qt::AlignRight);
	m_Toolbar->GetToolbar()->addWidget(m_Time);
}

void pdp::ThreeDEditing::CurrentTab(int tabNumber)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Current Tab: " << tabNumber << "\n";

	// deactivate current tool befor tab switches
	//mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
	//mitk::Tool* activeTool = manager->GetActiveTool();
	//manager->ActivateTool(-1);
	
	if(tabNumber == 0)
	{
		// 2D Tools
		// Convert surface to binary image
		if(m_ConvertSegmentationData)
		{
			std::cout << "Create new binary segmentations from surfaces.\n";

			// get all surfaces
			mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
			for(unsigned int i = 0; i < allNodes->size(); i++)
			{
				if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
				{
					bool convert = false;
					if(!allNodes->at(i)->GetBoolProperty("ModifiedThusConvert", convert))
					{
						// There is no property Convert -> add
						allNodes->at(i)->SetBoolProperty("ModifiedThusConvert", false);
					}
					if(convert)
					{
						std::cout << "Node: " << allNodes->at(i)->GetName() << " has been modified. It will be converted.\n";

						//std::cout << "Surface found: " << allNodes->at(i)->GetName() << "\n";

						// TODO if there is a binary for the current surface
						// delete old binary and substitute with new created binary image 

						if(m_UseTriangle)
						{
							// Triangulate first
							vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
							triangleFilter->SetInput(((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData());
							triangleFilter->Update();
							((mitk::Surface*)allNodes->at(i)->GetData())->SetVtkPolyData(triangleFilter->GetOutput());
						}
						if(m_UseClean)
						{
							vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
							removeDuplicatedPoints->SetInput(((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData());
							removeDuplicatedPoints->Update();
							((mitk::Surface*)allNodes->at(i)->GetData())->SetVtkPolyData(removeDuplicatedPoints->GetOutput());
						}
						if(m_UseSmooth)
						{
							// Smooth
							vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
							smoother->SetInput(((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData());//RC++
							smoother->SetNumberOfIterations( /*m_SmoothIteration*/ 50);
							smoother->SetRelaxationFactor( /*m_SmoothRelaxation*/ 0.1);
							smoother->SetFeatureAngle( 60 );
							smoother->FeatureEdgeSmoothingOff();
							smoother->BoundarySmoothingOn();
							smoother->SetConvergence( 0 );
							smoother->Update();
							((mitk::Surface*)allNodes->at(i)->GetData())->SetVtkPolyData(smoother->GetOutput());
						}
		
						// Create reference image for geometric information
						mitk::DataNode::Pointer node = mitk::DataNode::New();
						mitk::PixelType pixelType(typeid(short));
						mitk::Image::Pointer out = mitk::Image::New();
						mitk::Geometry3D::Pointer geo = mitk::Geometry3D::New();					
						geo = ((mitk::Surface*)allNodes->at(i)->GetData())->GetGeometry();					
						geo->ChangeImageGeometryConsideringOriginOffset(true);
						geo->SetBounds( ((mitk::Surface*)allNodes->at(i)->GetData())->GetGeometry()->GetBounds() );
						
						// Change resolution TODO
						if(0)
						{
							//float floatSpacing[3] = {0.5, 0.5, 0.5};
							//geo->SetFloatSpacing(floatSpacing);						
							//geo->SetEvenlySpaced(false);
							mitk::Vector3D spacing;
							spacing[0] = 0.1;
							spacing[1] = 0.1;
							spacing[2] = 0.1;
							geo->SetSpacing(spacing);
						}

						out->Initialize(pixelType, *geo);

						mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
						filter->SetInput((mitk::Surface*)allNodes->at(i)->GetData());						
						filter->SetImage(out);
						filter->SetMakeOutputBinary(true);
						filter->Update();

						// Does surface already have binary child? -> substitude
						mitk::DataNode* child = m_Data->getDataStore()->GetNamedDerivedNode(node->GetName().c_str(), node);
						if(m_Data->getDataStore()->Exists(child))
						{
							// remove old child
							m_Data->getDataStore()->Remove(child);
							
							// delete child ?? todo
						}


						// Add binary image to data storage as a child of the surface
						node->SetData((mitk::BaseData*)filter->GetOutput(0));
						node->SetName(allNodes->at(i)->GetName());
						node->SetBoolProperty("binary", true);

						// Add unique identifier for data nodes
						int newId = GetUniqueId();
						node->SetIntProperty("UniqueID", newId);

						m_Data->getDataStore()->Add(node, allNodes->at(i));

						allNodes->at(i)->SetBoolProperty("ModifiedThusConvert", false);
					}
				}
			}	
		}

		// Collect and display binary images
		int countBinary = 0;
		m_SelectSurfaceBox->clear();
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			bool isBinary = false;
			allNodes->at(i)->GetBoolProperty("binary", isBinary);
			if(isBinary)
			{
				std::string name = allNodes->at(i)->GetName();
				std::cout << "Binary found: " << name << "\n";
				m_CurrentSurfaces.push_back(name);
				m_SelectSurfaceBox->addItem(name.c_str());
				// set unique name
				allNodes->at(i)->SetIntProperty("UniqueNameBinary", countBinary);
				countBinary++;
			}
		}

		
		// Tool Selection Box
		m_Page1Layout->addWidget(GetManualToolSelectionBox());
		m_Page1Layout->addWidget(GetManualToolGUIContainer());

		m_ManualToolSelectionBox->SetDisplayedToolGroups("Add Subtract Paint Wipe Fill Erase 'Region Growing' Correction");/*Thresholding ThresholdingUL Statistics 'RegionGrower 3D'*/

		QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
		m_Tabs->setSizePolicy(sizePolicy2);
		m_ManualToolSelectionBox->setSizePolicy(sizePolicy2);
		//m_ManualToolGUIContainer->setSizePolicy(sizePolicy2);

		// Todo vertical sizePolicy

		//todo
		//if(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Scroll))
		//{
		//	mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Scroll);
		//}
	}
	else if(tabNumber == 1)
	{
		// 3D Tools
		// Convert binary image to surface
		if(m_ConvertSegmentationData)
		{
			std::cout << "Create surfaces from binary segmentations.\n";

			// Use vtkMarchingCubes to create Surface from binary image
			mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
			for(unsigned int i = 0; i < allNodes->size(); i++)
			{
				bool isBinary = false;
				allNodes->at(i)->GetBoolProperty("binary", isBinary);
				if(isBinary)
				{
					bool convert = false;
					if(!allNodes->at(i)->GetBoolProperty("ModifiedThusConvert", convert))
					{
						// There is no property Convert -> add
						allNodes->at(i)->SetBoolProperty("ModifiedThusConvert", false);
					}
					if(convert)
					{
						std::cout << "Node: " << allNodes->at(i)->GetName() << " has been modified. It will be converted.\n";

						/*vtkMarchingCubes* surfaceCreator = vtkMarchingCubes::New();
						surfaceCreator->SetInput((vtkDataObject*)((mitk::Image*)allNodes->at(i)->GetData())->GetVtkImageData());
						surfaceCreator->SetValue(0, 1);

						mitk::Surface::Pointer surface = mitk::Surface::New();
						surface->SetVtkPolyData(surfaceCreator->GetOutput());

						mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
						surfaceNode->SetData(surface);*/

						// Try ImageToSurfaceFilter
						mitk::ImageToSurfaceFilter::Pointer surfaceCreator = mitk::ImageToSurfaceFilter::New();
						surfaceCreator->SetInput((mitk::Image*)allNodes->at(i)->GetData());
						surfaceCreator->Update();
						mitk::Surface::Pointer surface = surfaceCreator->GetOutput();
						mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
						surfaceNode->SetData(surface);
						surfaceNode->SetName(allNodes->at(i)->GetName());

						// Get Parent node (surface to current binary)
						mitk::DataNode* parentToCurrentBinary = NULL;
						int currentId = -1;
						if(!allNodes->at(i)->GetIntProperty("UniqueID", currentId))
							std::cout << "Data without UniqueID found (1)!\n";
						
						mitk::DataStorage::SetOfObjects::ConstPointer allParentNodes = m_Data->getDataStore()->GetAll();
						for(unsigned int j = 0; j < allParentNodes->size(); j++)
						{
							// Get all derivation (childs) of the current node
							mitk::DataStorage::SetOfObjects::ConstPointer allChildToCurrentNode = m_Data->getDataStore()->GetDerivations(allParentNodes->at(j));	

							// Search for current binary
							for(unsigned int k = 0; k < allChildToCurrentNode->size(); k++)
							{
								int childId = -2;
								if(!allChildToCurrentNode->at(k)->GetIntProperty("UniqueID", childId))
									std::cout << "Data without UniqueID found (2)!\n";

								if(childId == currentId)
								{
									// Parent to binary found -> add to list
									if(allChildToCurrentNode->size() > 1)
										std::cout << "ERROR: THERE ARE MORE THAN ONE CHILD TO CURRENT SEGMENTATION!\n";
									parentToCurrentBinary = allParentNodes->at(j);
								}
							}
						}
						
						std::cout << "Komm ich bis hier?\n";

						// Remove binary image from data storage (child)
						m_Data->getDataStore()->Remove(allNodes->at(i));						
						// Remove old parent from data storage (parent) if there is one
						if(!parentToCurrentBinary == NULL)
						{
							std::cout << "Was soll das?\n";
							if(m_Data->getDataStore()->Exists(parentToCurrentBinary))
								m_Data->getDataStore()->Remove(parentToCurrentBinary);
						}

						// Add unique identifier for data nodes
						int newId = GetUniqueId();
						surfaceNode->SetIntProperty("UniqueID", newId);
						
						// Add surface to data storage
						m_Data->getDataStore()->Add(surfaceNode);

						// Add binary image as surface's child
						m_Data->getDataStore()->Add(allNodes->at(i), surfaceNode);

						mitk::RenderingManager::GetInstance()->RequestUpdateAll();
						surfaceCreator->Delete();

						allNodes->at(i)->SetBoolProperty("ModifiedThusConvert", false);

						m_Toolbar->SetImagesToBottom();
						
					}
				}
			}	
		}
		m_Page2Layout->addWidget(GetManualToolSelectionBox());
		m_Page2Layout->addWidget(GetManualToolGUIContainer());

		m_ManualToolSelectionBox->SetDisplayedToolGroups("'Bulge Tool' 'Drag Tool' 'Active Surface Tool' 'Live Wire Tool' 'Registration Tool' 'Cut Tool' 'Fill Hole Tool' 'Smooth Tool' 'Refinement Tool' 'Test Tool'"); /*'Refinement Tool''Active Contour Tool' 'Region Growing Tool' 'Test Tool' 'Move ROI'*/

		QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
		m_Tabs->setSizePolicy(sizePolicy2);
		m_ManualToolSelectionBox->setSizePolicy(sizePolicy2);
		//m_ManualToolGUIContainer->setSizePolicy(sizePolicy2);

		//todo
		//if(!mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Scroll))
		//{
		//	mitk::GlobalInteraction::GetInstance()->AddListener(m_Scroll);
		//}
	}

	else
	{
		// Options

	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::SelectBinaryImage(int binarySelection)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// get corresponding binary image
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		int currentIntProperty = -1;
		bool propertyFound = allNodes->at(i)->GetIntProperty("UniqueNameBinary", currentIntProperty);
		if(!propertyFound)
			continue;
		if(currentIntProperty == binarySelection)
		{
			// show found node
			std::string name = allNodes->at(i)->GetName();
			std::cout << "Binary found: " << name << ", at position: " << binarySelection << "\n";

			// set binary image as 2D segmentation
			mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
			toolManager->SetWorkingData(allNodes->at(i));
		}
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::CreateEmptySegmentation()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Select reference image -> change current layout
	m_SelectReferenceImageBoxWithNewSegmentation = new QComboBox();
	m_SelectSurfaceLayout2->addWidget(m_SelectReferenceImageBoxWithNewSegmentation);
	connect(m_SelectReferenceImageBoxWithNewSegmentation, SIGNAL(activated(int)), this, SLOT(ReferenceImageWithNewSegmentation(int)));

	m_SelectReferenceImageBoxWithNewSegmentation->addItem("Segment which Reference Image?");
	// List all images	
	int countReferenceImages = 0;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
		{
			// Sort out binary images
			int dummy;
			bool propertyFound = allNodes->at(i)->GetIntProperty("UniqueNameBinary", dummy);
			if(propertyFound)
				continue;

			// add images to vector
			m_CurrentReferenceImages.push_back(allNodes->at(i)->GetName());
			m_SelectReferenceImageBoxWithNewSegmentation->addItem(allNodes->at(i)->GetName().c_str());
			allNodes->at(i)->SetIntProperty("UniqueNameReferenceImages", countReferenceImages);
			countReferenceImages++;
		}		
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::ReferenceImageWithNewSegmentation(int referenceImageSelection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Reference image: " << referenceImageSelection << "\n";

	// Get image
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		int currentIntProperty = -1;
		bool propertyFound = allNodes->at(i)->GetIntProperty("UniqueNameReferenceImages", currentIntProperty);
		if(!propertyFound)
			continue;
		if(currentIntProperty == referenceImageSelection-1)
		{
			// Create empty segmentation to this image
			mitk::ColorProperty::Pointer color = mitk::ColorProperty::New();
			color->SetColor(255,0,0);
			mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( allNodes->at(i)->GetData() );
			mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
			mitk::Tool* firstTool = toolManager->GetToolById(0);
			mitk::DataNode::Pointer emptySegmentation = firstTool->CreateEmptySegmentationNode(image, allNodes->at(i)->GetName() + "Segmentation", color->GetColor());
			emptySegmentation->SetBoolProperty("binary", true);
			
			// Show Volume tuts todo, togglebutton
			emptySegmentation->SetBoolProperty("showVolume", true);
			//emptySegmentation->SetBoolProperty("showVolume", false);

			// Add unique identifier for data nodes
			int newId = GetUniqueId();
			emptySegmentation->SetIntProperty("UniqueID", newId);

			m_Data->getDataStore()->Add(emptySegmentation); 

			toolManager->SetWorkingData(emptySegmentation);
			toolManager->SetReferenceData(allNodes->at(i));
		}
	}

	m_SelectSurfaceLayout2->removeWidget(m_SelectReferenceImageBoxWithNewSegmentation);
	m_SelectReferenceImageBoxWithNewSegmentation->hide();

	// Update 2D binary images list (add new segmentation)
	CurrentTab(0);
	m_Toolbar->SetImagesToBottom();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::ReferenceImage(int referenceImageSelection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Reference image: " << referenceImageSelection << "\n";

	// Get image
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		int currentIntProperty = -1;
		bool propertyFound = allNodes->at(i)->GetIntProperty("UniqueNameReferenceImages", currentIntProperty);
		if(!propertyFound)
			continue;
		if(currentIntProperty == referenceImageSelection-1)
		{
			// Create empty segmentation to this image
			mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
			toolManager->SetReferenceData(allNodes->at(i));
		}
	}

	m_SelectReferenceImageLayout->removeWidget(m_SelectReferenceImageBox);
	m_SelectReferenceImageBox->hide();
	m_WorkingDatasetLabel->hide();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::ConvertData()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_ConvertSegmentationData)
		m_ConvertSegmentationData = false;
	else
		m_ConvertSegmentationData = true;

	std::cout << "Convert Segmentation set to: " << m_ConvertSegmentationData << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::UseTriangle()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_UseTriangle)
		m_UseTriangle = false;
	else
		m_UseTriangle = true;

	std::cout << "Use Triange set to: " << m_UseTriangle << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::UseClean()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_UseClean)
		m_UseClean = false;
	else
		m_UseClean = true;

	std::cout << "Use Clean Data set to: " << m_UseClean << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::UseSmooth()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_UseSmooth)
		m_UseSmooth = false;
	else
		m_UseSmooth = true;

	std::cout << "Use Smooth set to: " << m_UseSmooth << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::PrintDataStorage()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		int id = -1;
		if(!(allNodes->at(i)->GetIntProperty("UniqueID", id)))
		{
			std::cout << "Error: Data found without unique id: " << allNodes->at(i)->GetName() << "\n";
		}
		else
		{
			std::cout << "Data found with ID: " << id << ", Name: " << allNodes->at(i)->GetName() << "\n";
		}
	}

	// Print modified nodes
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		bool modified;
		if(!(allNodes->at(i)->GetBoolProperty("ModifiedThusConvert", modified)))
		{
			std::cout << "Error: Data found without modified property: " << allNodes->at(i)->GetName() << "\n";
		}
		else
		{
			if(modified)
				std::cout << allNodes->at(i)->GetName() << " has been modified.\n";
		}
	}

	m_Data->getDataStore()->Print(std::cout);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::CreateSurface()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allImageNodes->size(); i++)
	{
		if(strcmp(allImageNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
		{
			vtkMarchingCubes* surfaceCreator = vtkMarchingCubes::New();
			surfaceCreator->SetInput((vtkDataObject*)((mitk::Image*)allImageNodes->at(i)->GetData())->GetVtkImageData());
			surfaceCreator->SetValue(0, 1);

			mitk::Surface::Pointer surface = mitk::Surface::New();
			surface->SetVtkPolyData(surfaceCreator->GetOutput());

			mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
			surfaceNode->SetName(allImageNodes->at(i)->GetName() + "Surface");
			surfaceNode->SetData(surface);
			
			// Add unique identifier for data nodes
			int newId = GetUniqueId();
			surfaceNode->SetIntProperty("UniqueID", newId);

			m_Data->getDataStore()->Add(surfaceNode);

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
			surfaceCreator->Delete();
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::TakeScreenshot()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;	

	// Get renderer
	std::vector<vtkRenderer*> renderer;
	renderer.push_back(m_MitkView->GetRenderWindow1()->GetRenderer()->GetVtkRenderer());
	renderer.push_back(m_MitkView->GetRenderWindow2()->GetRenderer()->GetVtkRenderer());
	renderer.push_back(m_MitkView->GetRenderWindow3()->GetRenderer()->GetVtkRenderer());
	renderer.push_back(m_MitkView->GetRenderWindow4()->GetRenderer()->GetVtkRenderer());

	std::vector<std::string> suffix;
	suffix.push_back("1");
	suffix.push_back("2");
	suffix.push_back("3");
	suffix.push_back("4");

	for(int i = 0; i < 4 ; i++)
	{
		
		std::string filename = "RendererWindow";
		filename += suffix[i];
		filename += "Image.png";
		unsigned int magnificationFactor = 1;

		bool doubleBuffering( renderer[i]->GetRenderWindow()->GetDoubleBuffer() );
		renderer[i]->GetRenderWindow()->DoubleBufferOff();

		vtkImageWriter* fileWriter;
		fileWriter = vtkPNGWriter::New();
		
		vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
		magnifier->SetInput(renderer[i]);
		magnifier->SetMagnification(magnificationFactor);
		//magnifier->Update();
		fileWriter->SetInput(magnifier->GetOutput());
		fileWriter->SetFileName(filename.c_str());

		// vtkRenderLargeImage has problems with different layers, therefore we have to 
		// temporarily deactivate all other layers.
		// we set the background to white, because it is nicer than black...
		double oldBackground[3];
		renderer[i]->GetBackground(oldBackground);
		double white[] = {1.0, 1.0, 1.0};
		double black[] = {0.0, 0.0, 0.0};
		//renderer[i]->SetBackground(white);
		renderer[i]->SetBackground(black);
		//m_MitkView->DisableColoredRectangles();
		m_MitkView->DisableDepartmentLogo();
		m_MitkView->DisableGradientBackground();

		fileWriter->Write();  
		fileWriter->Delete();  

		//m_MitkView->EnableColoredRectangles();
		m_MitkView->EnableDepartmentLogo();
		m_MitkView->EnableGradientBackground();
		renderer[i]->SetBackground(oldBackground);

		renderer[i]->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
	}
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::FirstScenario()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;	

	mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
	mitk::DataNode::Pointer refNode = toolManager->GetReferenceData(0);
	mitk::DataNode::Pointer workNode = toolManager->GetWorkingData(0);
	std::string refName = "";
	std::string workName = "";
	if(refNode.IsNotNull())
		refNode->GetName(refName);
	if(workNode.IsNotNull())
		workNode->GetName(workName);
	std::cout << "Ref name: " << refName << ", work name: " << workName << "\n";

	// disable current tool
	mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
	manager->ActivateTool(-1);

	// empty current datastorage
	m_Data->getDataStore()->Remove(m_Data->getDataStore()->GetAll());

	// load or create files
	// Full Box
	//mitk::Surface::Pointer box = mitk::Surface::New();
	//mitk::DataNode::Pointer boxNode = mitk::DataNode::New();
	//boxNode->SetName("Box");
	//boxNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	//boxNode->SetData(box);
	//boxNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
	//// Add unique identifier for data nodes
	//int newId = GetUniqueId();
	//boxNode->SetIntProperty("UniqueID", newId);
	//m_Data->getDataStore()->Add(boxNode);	
	//vtkCubeSource* vBox = vtkCubeSource::New();
	//vBox->SetBounds(10,50,10,50,5,25);
	//box->SetVtkPolyData(vBox->GetOutput());

	// Half Box
	mitk::Surface::Pointer halfBox = mitk::Surface::New();
	mitk::DataNode::Pointer halfBoxNode = mitk::DataNode::New();
	halfBoxNode->SetName("Scenario_1_Segmentation_3D");
	halfBoxNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	halfBoxNode->SetData(halfBox);
	halfBoxNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
	// Add unique identifier for data nodes
	int newId = GetUniqueId();
	halfBoxNode->SetIntProperty("UniqueID", newId);
	m_Data->getDataStore()->Add(halfBoxNode);	
	vtkCubeSource* vHalfBox = vtkCubeSource::New();
	vHalfBox->SetBounds(10,35,10,35,5,25);
	halfBox->SetVtkPolyData(vHalfBox->GetOutput());

	// Surrounding
	mitk::Surface::Pointer surroundingBox = mitk::Surface::New();
	mitk::DataNode::Pointer surroundingBoxNode = mitk::DataNode::New();
	surroundingBoxNode->SetName("Surrounding Box");
	surroundingBoxNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
	surroundingBoxNode->SetData(surroundingBox);
	surroundingBoxNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
	// Add unique identifier for data nodes
	newId = GetUniqueId();
	surroundingBoxNode->SetIntProperty("UniqueID", newId);
	//m_Data->getDataStore()->Add(surroundingBoxNode);	
	vtkCubeSource* vSurroundingBox = vtkCubeSource::New();
	vSurroundingBox->SetBounds(0,60,0,60,0,30);
	surroundingBox->SetVtkPolyData(vSurroundingBox->GetOutput());

	// Show as Wireframe
	mitk::EnumerationProperty* representationProp1 = dynamic_cast<mitk::EnumerationProperty*> (halfBoxNode->GetProperty("material.representation"));
	representationProp1->SetValue(1);
	mitk::EnumerationProperty* representationProp2 = dynamic_cast<mitk::EnumerationProperty*> (surroundingBoxNode->GetProperty("material.representation"));
	representationProp2->SetValue(1);

	// Triangulate 
	/*vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
	triangleFilter->SetInput(box->GetVtkPolyData());
	triangleFilter->Update();
	box->SetVtkPolyData(triangleFilter->GetOutput());*/

	vtkTriangleFilter* triangleFilter2 = vtkTriangleFilter::New();
	triangleFilter2->SetInput(halfBox->GetVtkPolyData());
	triangleFilter2->Update();
	halfBox->SetVtkPolyData(triangleFilter2->GetOutput());

	vtkTriangleFilter* triangleFilter3 = vtkTriangleFilter::New();
	triangleFilter3->SetInput(surroundingBox->GetVtkPolyData());
	triangleFilter3->Update();
	surroundingBox->SetVtkPolyData(triangleFilter3->GetOutput());

	// Subdivide half box
	vtkSmartPointer<vtkLinearSubdivisionFilter> subdivisionFilter;
    subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
	subdivisionFilter->SetInput(halfBox->GetVtkPolyData());
    subdivisionFilter->SetNumberOfSubdivisions(4);
    subdivisionFilter->Update();
	halfBox->SetVtkPolyData(subdivisionFilter->GetOutput());

	// Create binary segmentation
	vtkSmartPointer<vtkImageData> binarySegmentation = vtkSmartPointer<vtkImageData>::New();
	// Specify the size of the image data
	binarySegmentation->SetDimensions(60,60,30);
	binarySegmentation->SetNumberOfScalarComponents(1);
	//imageData->SetScalarTypeToShort();
	binarySegmentation->SetScalarTypeToUnsignedChar();
	binarySegmentation->AllocateScalars();
	int* dims = binarySegmentation->GetDimensions();
	// int dims[3]; // can't do this

	//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;
	//std::cout << "Number of points: " << imageData->GetNumberOfPoints() << std::endl;
	//std::cout << "Number of cells: " << imageData->GetNumberOfCells() << std::endl;

	// Fill every entry of the image data with "2.0"
	for (int z = 0; z < dims[2]; z++)
	{
		for (int y = 0; y < dims[1]; y++)
		{
			for (int x = 0; x < dims[0]; x++)
			{
				if(x >= 10 && x < 35)
				{
					if(y >= 10 && y < 35)
					{
						if(z >= 5 && z < 25)
						{
							//short* pixel = static_cast<short*>(binarySegmentation->GetScalarPointer(x,y,z));
							unsigned char* pixel = static_cast<unsigned char*>(binarySegmentation->GetScalarPointer(x,y,z));
							pixel[0] = 1;
							//std::cout << "Pixel = 0\n";
						}
						else
						{
							//short* pixel = static_cast<short*>(binarySegmentation->GetScalarPointer(x,y,z));
							unsigned char* pixel = static_cast<unsigned char*>(binarySegmentation->GetScalarPointer(x,y,z));
							pixel[0] = 0;
						}
					}
					else
					{
						//short* pixel = static_cast<short*>(binarySegmentation->GetScalarPointer(x,y,z));
						unsigned char* pixel = static_cast<unsigned char*>(binarySegmentation->GetScalarPointer(x,y,z));
						pixel[0] = 0;
					}
				}
				else
				{
					//short* pixel = static_cast<short*>(binarySegmentation->GetScalarPointer(x,y,z));
					unsigned char* pixel = static_cast<unsigned char*>(binarySegmentation->GetScalarPointer(x,y,z));
					pixel[0] = 0;
				}
			}
		}
	}
	mitk::Image::Pointer binarySegmentationImage = mitk::Image::New();
	binarySegmentationImage->Initialize(binarySegmentation);
	binarySegmentationImage->SetVolume(binarySegmentation->GetScalarPointer());
	mitk::Point3D origin;
	origin[0] = 0.5;
	origin[1] = 0.5;
	origin[2] = 0.5;
	binarySegmentationImage->SetOrigin(origin);

	mitk::DataNode::Pointer binarySegmentationNode = mitk::DataNode::New();
	binarySegmentationNode->SetData(binarySegmentationImage);
	binarySegmentationNode->SetName("Scenario_1_Segmentation_2D");
	binarySegmentationNode->SetBoolProperty("binary", true);
	binarySegmentationNode->SetOpacity(0.8);
	binarySegmentationNode->SetColor(255,0,0);
	// Add unique identifier for data nodes
	newId = GetUniqueId();
	binarySegmentationNode->SetIntProperty("UniqueID", newId);
	m_Data->getDataStore()->Add(binarySegmentationNode);


	// Create an image data
	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	// Specify the size of the image data
	imageData->SetDimensions(60,60,30);
	imageData->SetNumberOfScalarComponents(1);
	//imageData->SetScalarTypeToShort();
	imageData->SetScalarTypeToUnsignedChar();
	imageData->AllocateScalars();
	int* dims2 = imageData->GetDimensions();
	// int dims[3]; // can't do this

	//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;
	//std::cout << "Number of points: " << imageData->GetNumberOfPoints() << std::endl;
	//std::cout << "Number of cells: " << imageData->GetNumberOfCells() << std::endl;

	// Fill every entry of the image data with "2.0"
	for (int z = 0; z < dims2[2]; z++)
	{
		for (int y = 0; y < dims2[1]; y++)
		{
			for (int x = 0; x < dims2[0]; x++)
			{
				if(x >= 10 && x < 50)
				{
					if(y >= 10 && y < 50)
					{
						if(z >= 5 && z < 25)
						{
							//short* pixel = static_cast<short*>(imageData->GetScalarPointer(x,y,z));
							unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
							pixel[0] = 10;
							//std::cout << "Pixel = 0\n";
						}
						else
						{
							//short* pixel = static_cast<short*>(imageData->GetScalarPointer(x,y,z));
							unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
							pixel[0] = 200;
						}
					}
					else
					{
						//short* pixel = static_cast<short*>(imageData->GetScalarPointer(x,y,z));
						unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
						pixel[0] = 200;
					}
				}
				else
				{
					//short* pixel = static_cast<short*>(imageData->GetScalarPointer(x,y,z));
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					pixel[0] = 200;
				}
			}
		}
	}
	mitk::Image::Pointer surroundingImage = mitk::Image::New();
	surroundingImage->Initialize(imageData);
	surroundingImage->SetVolume(imageData->GetScalarPointer());
	mitk::Point3D origin2;
	origin2[0] = 0.5;
	origin2[1] = 0.5;
	origin2[2] = 0.5;
	surroundingImage->SetOrigin(origin2);

	mitk::DataNode::Pointer node = mitk::DataNode::New();
	node->SetData(surroundingImage);
	node->SetName("Scenario_1_ReferenceImage");
	node->SetBoolProperty("binary", false);
	node->SetOpacity(1.0);
	node->SetColor(200,200,200);
	// Add unique identifier for data nodes
	newId = GetUniqueId();
	node->SetIntProperty("UniqueID", newId);
	m_Data->getDataStore()->Add(node);

	// Get Surrounding
	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	int surrounding = 0;

	mitk::Geometry3D::Pointer geo = surroundingImage->GetGeometry();	
	mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geo->GetBoundingBox());
	mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

	surfBounds[0] -= surrounding;
	surfBounds[1] += surrounding;
	surfBounds[2] -= surrounding;
	surfBounds[3] += surrounding;
	surfBounds[4] -= surrounding;
	surfBounds[5] += surrounding;
	geo->SetBounds(surfBounds);

	mitk::RenderingManager::GetInstance()->InitializeViews(geo, mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	// Update ToolManager
	if(/*refName.compare("Scenario_1_Image") == 0 &&*/ workName.compare("Segmentation_1_2D") == 0)
	{
		std::cout << "Can reuse old data in toolmanager.\n";
		toolManager->SetReferenceData(node);
		toolManager->SetWorkingData(binarySegmentationNode);
	}	
	else
	{
		std::cout << "Reinit toolmanager\n";
		m_HasWorkingDataSet = false;
		m_HasReferenceDataSet = false;
		CurrentTab(1);
		CurrentTab(0);
	}

	//CurrentTab(1);
	CurrentTab(0);

	SelectBinaryImage(0);
	m_Tabs->setCurrentIndex(0);
	//ReferenceImage(1);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::SecondScenario()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;	

	// disable current tool
	mitk::ToolManager* manager = m_ManualToolSelectionBox->GetToolManager();
	manager->ActivateTool(-1);

	// empty current datastorage
	m_Data->getDataStore()->Remove(m_Data->getDataStore()->GetAll());

	// Load data
	std::vector<std::string> files;
	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
  
	// Adding file/image to the DataNodeFactory
	if(AT_HOME == 1)
	{
		files.push_back("C:\\DA\\Data\\Scenario2\\Scenario_2_ReferenceImage.nrrd");
		files.push_back("C:\\DA\\Data\\Scenario2\\Scenario_2_Segmentation_3D.stl");
		files.push_back("C:\\DA\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd");
		files.push_back("C:\\DA\\Data\\Scenario2\\GoldStandard.stl");
	}
	else
	{
		files.push_back("D:\\hachmann\\Data\\Scenario2\\Scenario_2_ReferenceImage.nrrd");
		files.push_back("D:\\hachmann\\Data\\Scenario2\\Scenario_2_Segmentation_3D.stl");
		files.push_back("D:\\hachmann\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd");
		files.push_back("D:\\hachmann\\Data\\Scenario2\\GoldStandard.stl");
	}

	for(int i = 0; i < files.size(); i++)
	{
		nodeReader->SetFileName(files[i]);

		try
		{
			nodeReader->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
			std::cout << ex << std::endl;
		}
		catch (std::exception &ex1)
		{
			ex1.what();
		}

		// Adding file/image to the DataNode
		mitk::DataNode::Pointer node;
		node = nodeReader->GetOutput(0);

		// Add unique identifier for data nodes
		int newId = GetUniqueId();
		node->SetIntProperty("UniqueID", newId);
		node->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));

		m_Data->getDataStore()->Add(node);
		SetUpMitkView();

		mitk::BaseData::Pointer basedata = node->GetData();
		if (basedata.IsNotNull())
		{
			mitk::RenderingManager::GetInstance()->InitializeViews(basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}

	// set segmentation boundingbox
	mitk::DataNode::Pointer surfaceNode = m_Data->getDataStore()->GetNamedNode("Scenario_2_Segmentation_3D");
	if(m_Data->getDataStore()->Exists(surfaceNode))
	{
		std::cout << "Scenario_2_Segmentation_3D found.\n";

		double surrounding = 15;

		mitk::Geometry3D::Pointer geo = surfaceNode->GetData()->GetGeometry();	
		mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geo->GetBoundingBox());
		mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

		surfBounds[0] -= surrounding;
		surfBounds[1] += surrounding;
		surfBounds[2] -= surrounding;
		surfBounds[3] += surrounding;
		surfBounds[4] -= surrounding;
		surfBounds[5] += surrounding;
		geo->SetBounds(surfBounds);
				
		mitk::RenderingManager::GetInstance()->InitializeViews(geo, mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
	mitk::DataNode::Pointer surfaceNode2 = m_Data->getDataStore()->GetNamedNode("GoldStandard");
	if(m_Data->getDataStore()->Exists(surfaceNode2))
	{
		std::cout << "GoldStandard found.\n";
		surfaceNode2->SetOpacity(0.5);

		double surrounding = 15;

		mitk::Geometry3D::Pointer geo = surfaceNode2->GetData()->GetGeometry();	
		mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geo->GetBoundingBox());
		mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

		surfBounds[0] -= surrounding;
		surfBounds[1] += surrounding;
		surfBounds[2] -= surrounding;
		surfBounds[3] += surrounding;
		surfBounds[4] -= surrounding;
		surfBounds[5] += surrounding;
		geo->SetBounds(surfBounds);

		mitk::RenderingManager::GetInstance()->InitializeViews(geo, mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	// Set Reference Image to bottom
	m_Toolbar->SetImagesToBottom();

	//CurrentTab(1);
	CurrentTab(0);

	SelectBinaryImage(0);
	//ReferenceImage(1);
	m_Tabs->setCurrentIndex(0);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::CalculateVolume()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Get Surface
	// Get image
	bool nodeFound = false;
	mitk::DataNode::Pointer node;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		std::string name;
		allNodes->at(i)->GetName(name);
		if(strcmp(name.c_str(), "Segmentation") == 0)
		{
			node = allNodes->at(i);
			nodeFound = true;
		}
		if(strcmp(name.c_str(), "Surface 6_MissingTop") == 0)
		{
			node = allNodes->at(i);
			nodeFound = true;
		}
	}

	if(nodeFound)
	{
		std::cout << "Calculate Volume of node: " << node->GetName() << "\n";

		// Calculate Properties
		vtkSmartPointer<vtkMassProperties> vol = vtkMassProperties::New();
		vol->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
		std::cout << "Volume: " << vol->GetVolume() << "\n";
	
		// Cut surface by boundingbox
		std::string name;
		node->GetName(name);
		if(strcmp(name.c_str(), "Segmentation") == 0)
		{
			if(0)
			{
				std::cout << "Clip via full box.\n";
				double maxHoleSize = 10000.0;
				std::vector<double> volumes;
				int counter = 0;
				//vBox->SetBounds(10,50,10,50,5,25);

				std::vector<vtkBox*> boxes;
				vtkBox* vBox1 = vtkBox::New();				
				vBox1->SetBounds(0,10,0,60,5,25);
				boxes.push_back(vBox1);
				vtkBox* vBox2 = vtkBox::New();				
				vBox2->SetBounds(50,60,0,60,5,25);
				boxes.push_back(vBox2);
				vtkBox* vBox3 = vtkBox::New();				
				vBox3->SetBounds(10,50,0,10,5,25);
				boxes.push_back(vBox3);
				vtkBox* vBox4 = vtkBox::New();				
				vBox4->SetBounds(10,50,50,60,5,25);
				boxes.push_back(vBox4);
				vtkBox* vBox5 = vtkBox::New();				
				vBox5->SetBounds(0,60,0,60,0,5);
				boxes.push_back(vBox5);
				vtkBox* vBox6 = vtkBox::New();				
				vBox6->SetBounds(0,60,0,60,25,30);
				boxes.push_back(vBox6);

				for(std::vector<vtkBox*>::iterator it = boxes.begin(); it != boxes.end(); it++)
				{
					std::cout << "Box Contour: " << counter << "\n";
					counter++;

					vtkClipPolyData *clip = vtkClipPolyData::New();
					clip->AddInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
					clip->SetClipFunction(*it);
					clip->GenerateClippedOutputOn();	
					clip->Update();

					// fill hole
					vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter = vtkSmartPointer<vtkFillHolesFilter>::New();
					fillHolesFilter->SetInput(clip->GetOutput(1));				
					fillHolesFilter->SetHoleSize(maxHoleSize);
					fillHolesFilter->Update();
					if(fillHolesFilter->GetOutput()->GetNumberOfPoints() > 0)
					{
						// Save
						mitk::Surface::Pointer clipped = mitk::Surface::New();
						clipped->SetVtkPolyData(fillHolesFilter->GetOutput());
						mitk::DataNode::Pointer clippedNode = mitk::DataNode::New();
						clippedNode->SetName("Clipped by Full Box");
						clippedNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
						clippedNode->SetData(clipped);
						clippedNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
						// Add unique identifier for data nodes
						int newId = GetUniqueId();
						clippedNode->SetIntProperty("UniqueID", newId);
						m_Data->getDataStore()->Add(clippedNode);	

						// Calculate Properties
						vtkSmartPointer<vtkMassProperties> vol = vtkMassProperties::New();
						vol->SetInput(clipped->GetVtkPolyData());
						volumes.push_back(vol->GetVolume());
						std::cout << "Volume: " << vol->GetVolume() << "\n";
					}
				}
				
				double totalVolume = 0.0;
				for(std::vector<double>::iterator it = volumes.begin(); it != volumes.end(); it++)
				{
					totalVolume += *it;
				}
				std::cout << "Total volume: " << totalVolume << "\n";

				
			}
			if(0)
			{
				// Append to full box
				// Full Box
				//mitk::Surface::Pointer box = mitk::Surface::New();
				//mitk::DataNode::Pointer boxNode = mitk::DataNode::New();
				//boxNode->SetName("Box");
				//boxNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
				//boxNode->SetData(box);
				//boxNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
				//// Add unique identifier for data nodes
				//int newId = GetUniqueId();
				//boxNode->SetIntProperty("UniqueID", newId);
				//m_Data->getDataStore()->Add(boxNode);	
			
				vtkBox* vBox = vtkBox::New();
				vBox->SetBounds(10,50,10,50,5,25);

				vtkClipPolyData *clip = vtkClipPolyData::New();
				clip->AddInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
				clip->SetClipFunction(vBox);
				clip->GenerateClippedOutputOn();	
				clip->Update();

				vtkCubeSource* vBoxSource = vtkCubeSource::New();
				vBoxSource->SetBounds(10,50,10,50,5,25);

				vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
				append->AddInput(clip->GetOutput());
				append->AddInput(vBoxSource->GetOutput());
				append->Update();

				vtkSmartPointer<vtkCleanPolyData> cleanFilter =
				vtkSmartPointer<vtkCleanPolyData>::New();
				cleanFilter->SetInput(append->GetOutput());
				cleanFilter->Update();
 

				mitk::Surface::Pointer clipped = mitk::Surface::New();
				clipped->SetVtkPolyData(cleanFilter->GetOutput());
				mitk::DataNode::Pointer clippedNode = mitk::DataNode::New();
				clippedNode->SetName("Clipped by Full Box");
				clippedNode->SetProperty("color",mitk::ColorProperty::New(254.0/255.0, 1.0/255.0, 1.0/255.0));
				clippedNode->SetData(clipped);
				clippedNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
				// Add unique identifier for data nodes
				int newId = GetUniqueId();
				clippedNode->SetIntProperty("UniqueID", newId);
				//m_Data->getDataStore()->Add(clippedNode);	

				// Calculate Properties
				vtkSmartPointer<vtkMassProperties> vol2 = vtkMassProperties::New();
				vol2->SetInput(clipped->GetVtkPolyData());
				std::cout << "Volume: " << vol2->GetVolume() << "\n";			
			}
			if(1)
			{
				// Get reference image for geometric information
				mitk::DataNode::Pointer imageNode;
				mitk::DataStorage::SetOfObjects::ConstPointer allNodes2 = m_Data->getDataStore()->GetAll();
				for(unsigned int i = 0; i < allNodes2->size(); i++)
				{
					std::string name2;
					allNodes2->at(i)->GetName(name2);
					if(strcmp(name2.c_str(), "Surrounding Binary") == 0)
					{
						std::cout << "Surrounding Binary found.\n";
						imageNode = allNodes->at(i);
					}
				}
				mitk::Image::Pointer image = ((mitk::Image*)imageNode->GetData());

				// Create an image data
				((mitk::Surface*)node->GetData())->CalculateBoundingBox();
				((mitk::Surface*)node->GetData())->Update();

				mitk::Geometry3D::Pointer surfGeo = ((mitk::Surface*)node->GetData())->GetGeometry();		
				mitk::BoundingBox* sbb = const_cast<mitk::BoundingBox*>(surfGeo->GetBoundingBox());
				mitk::BoundingBox::BoundsArrayType surfBounds = sbb->GetBounds();

				vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
				// Specify the size of the image data
				imageData->SetDimensions(surfBounds[1]-surfBounds[0],surfBounds[3]-surfBounds[2],surfBounds[5]-surfBounds[4]);
				imageData->SetNumberOfScalarComponents(1);
				//imageData->SetScalarTypeToShort();
				imageData->SetScalarTypeToUnsignedChar();
				imageData->AllocateScalars();
				int* dims = imageData->GetDimensions();
				// int dims[3]; // can't do this

				//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;
				//std::cout << "Number of points: " << imageData->GetNumberOfPoints() << std::endl;
				//std::cout << "Number of cells: " << imageData->GetNumberOfCells() << std::endl;

				// Fill every entry of the image data with "2.0"
				for (int z = 0; z < dims[2]; z++)
				{
					for (int y = 0; y < dims[1]; y++)
					{
						for (int x = 0; x < dims[0]; x++)
						{
							//short* pixel = static_cast<short*>(imageData->GetScalarPointer(x,y,z));
							unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
							pixel[0] = 1;
						}
					}
				}

				mitk::Image::Pointer surroundingImage = mitk::Image::New();
				surroundingImage->Initialize(imageData);
				surroundingImage->SetVolume(imageData->GetScalarPointer());
				mitk::Point3D origin;
				origin[0] = surfBounds[0]+0.5;
				origin[1] = surfBounds[2]+0.5;
				origin[2] = surfBounds[4]+0.5;
				surroundingImage->SetOrigin(origin);
				mitk::DataNode::Pointer node3 = mitk::DataNode::New();
				node3->SetData(surroundingImage);
				node3->SetName("Volume Calculation Image");
				node3->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
				//node2->SetBoolProperty("binary", true);
				// Add unique identifier for data nodes
				int newId = GetUniqueId();
				node3->SetIntProperty("UniqueID", newId);
				//m_Data->getDataStore()->Add(node3);

				mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
				filter->SetInput(((mitk::Surface*)node->GetData())/*->Clone()*/);						
				filter->SetImage(surroundingImage/*->Clone()*/);
				filter->SetMakeOutputBinary(true);
				filter->Update();

				// Add binary image to data storage as a child of the surface
				mitk::DataNode::Pointer node2 = mitk::DataNode::New();
				node2->SetName(node->GetName());
				node2->SetData((mitk::Image*)filter->GetOutput(0));
				//node2->SetBoolProperty("binary", true);
				// Add unique identifier for data nodes
				newId = GetUniqueId();
				node2->SetIntProperty("UniqueID", newId);
				//m_Data->getDataStore()->Add(node2);

				// Calculate volume via counting pixels	
				mitk::Image::Pointer mitkImageData2 = (mitk::Image*)filter->GetOutput(0)/*->Clone()*/;
				vtkSmartPointer<vtkImageData> imageData2 = mitkImageData2->GetVtkImageData();
				
				int* dims2 = imageData2->GetDimensions();  // TODO: Warum stürzt der hier ab?
				// int dims[3]; // can't do this

				//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;
				//std::cout << "Number of points: " << imageData->GetNumberOfPoints() << std::endl;
				//std::cout << "Number of cells: " << imageData->GetNumberOfCells() << std::endl;
				
				int pixelCounterInsideFullBox = 0;
				int pixelCounterOutsideFullBox = 0;
				double dimShift[3];
				mitk::Point3D origin2 = mitkImageData2->GetGeometry()->GetOrigin();
				dimShift[0] = origin2[0] - 0.5;
				dimShift[1] = origin2[1] - 0.5;
				dimShift[2] = origin2[2] - 0.5;
				std::cout << "Dim shift: " << dimShift[0] << " " << dimShift[1] << " " << dimShift[2] << " " << "\n";

				for (int z = 0; z < dims2[2]; z++)
				{
					for (int y = 0; y < dims2[1]; y++)
					{
						for (int x = 0; x < dims2[0]; x++)
						{
							unsigned char* pixel = static_cast<unsigned char*>(imageData2->GetScalarPointer(x,y,z));
							if(pixel[0] > 0)
							{
								if((x+dimShift[0]) >= 9.5 && (x+dimShift[0]) < 50)
								{
									if((y+dimShift[1]) >= 9.5 && (y+dimShift[1]) < 50)
									{
										if((z+dimShift[2]) >= 5 && (z+dimShift[2]) < 25)
										{
											pixelCounterInsideFullBox++;
										}
										else
										{
											pixelCounterOutsideFullBox++;
											//std::cout << x+dimShift[0] << " " << y+dimShift[1] << " " << z+dimShift[2] << "\n";
										}
									}
									else
									{
										pixelCounterOutsideFullBox++;
										//std::cout << x+dimShift[0] << " " << y+dimShift[1] << " " << z+dimShift[2] << "\n";
									}
								}
								else
								{
									pixelCounterOutsideFullBox++;
									//std::cout << x+dimShift[0] << " " << y+dimShift[1] << " " << z+dimShift[2] << "\n";
								}
							}
						}
					}
				}
				std::cout << "Number of pixels inside Full Box: " << pixelCounterInsideFullBox << "\n";
				std::cout << "Number of pixels outside Full Box: " << pixelCounterOutsideFullBox << "\n";
				std::cout << "Total number of pixels: " << pixelCounterInsideFullBox+pixelCounterOutsideFullBox << "\n";							
			}
		}
	}
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::CurrentVolume()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Test save selected nodes
	QModelIndexList indexesOfSelectedRows = m_DataManager->GetTreeView()->selectionModel()->selectedRows();

	mitk::DataNode* node = 0;
	unsigned int indexesOfSelectedRowsSize = indexesOfSelectedRows.size();
	for (unsigned int i = 0; i<indexesOfSelectedRowsSize; ++i)
	{
		node = m_DataManager->GetTreeModel()->GetNode(indexesOfSelectedRows.at(i));
		// if node is not defined or if the node contains geometry data do not remove it
		if ( node != 0 )
		{
			mitk::BaseData::Pointer data = node->GetData();

			std::cout << "Name of Class: " << data->GetNameOfClass() << "\n";
			std::string ref = "Surface";
			if(ref.compare(data->GetNameOfClass()) == 0)
			{
				// Surface found
				std::cout << "Surface found\n";

				// Calculate Properties
				vtkSmartPointer<vtkMassProperties> vol = vtkMassProperties::New();
				vol->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
				vol->Update();
				std::cout << "Volume: " << vol->GetVolume() << "\n";

				// Convert surface
				mitk::PixelType pixelType(typeid(short));
				mitk::Image::Pointer out = mitk::Image::New();
				mitk::Geometry3D::Pointer geo = mitk::Geometry3D::New();					
				geo = ((mitk::Surface*)node->GetData())->GetGeometry();					
				//geo->ChangeImageGeometryConsideringOriginOffset(true);
				geo->SetBounds( ((mitk::Surface*)node->GetData())->GetGeometry()->GetBounds() );
				out->Initialize(pixelType, *geo);

				mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
				filter->SetInput((mitk::Surface*)node->GetData());						
				filter->SetImage(out);
				filter->SetMakeOutputBinary(true);
				filter->Update();
				mitk::Image::Pointer image = (mitk::Image*)filter->GetOutput(0);

				// count pixels
				vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
				int* dims = imageData->GetDimensions();  // TODO: Warum stürzt der hier ab?
				// int dims[3]; // can't do this
				//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
				int pixelCounter = 0;
				for (int z = 0; z < dims[2]; z++)
				{
					for (int y = 0; y < dims[1]; y++)
					{
						for (int x = 0; x < dims[0]; x++)
						{
							unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
							if(pixel[0] > 0)
							{
								pixelCounter++;
							}
						}
					}
				}
				std::cout << "Number of pixels in Image: " << pixelCounter << "\n";
			}
			std::string ref2 = "Image";
			if(ref2.compare(data->GetNameOfClass()) == 0)
			{
				// Image found
				std::cout << "Image found\n";

				// count pixels
				vtkSmartPointer<vtkImageData> imageData = ((mitk::Image*)node->GetData())->GetVtkImageData();
				int* dims = imageData->GetDimensions();  // TODO: Warum stürzt der hier ab?
				// int dims[3]; // can't do this
				//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
				int pixelCounter = 0;
				for (int z = 0; z < dims[2]; z++)
				{
					for (int y = 0; y < dims[1]; y++)
					{
						for (int x = 0; x < dims[0]; x++)
						{
							unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
							if(pixel[0] > 0)
							{
								pixelCounter++;
							}
						}
					}
				}
				std::cout << "Number of pixels in Image: " << pixelCounter << "\n";

				// Convert image to surface
				if (node)
				{
					mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
					if (image.IsNull()) return;

					bool smooth(true);
					bool applyMedian(false);
					bool decimateMesh(true);
					unsigned int medianKernelSize(3u);
					float gaussianSD(2.5f);
					float reductionRate(0.80f);

					mitk::ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = mitk::ManualSegmentationToSurfaceFilter::New();
					surfaceFilter->SetInput( image );
					surfaceFilter->SetThreshold( 1 ); //expects binary image with zeros and ones

					surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
					if (smooth)
					{
						surfaceFilter->InterpolationOn();
						surfaceFilter->SetGaussianStandardDeviation( gaussianSD );
					}

					surfaceFilter->SetMedianFilter3D(applyMedian); // apply median to segmentation before marching cubes ?
					if (applyMedian)
					{
						surfaceFilter->SetMedianKernelSize(medianKernelSize, medianKernelSize, medianKernelSize); // apply median to segmentation before marching cubes
					}

					//fix to avoid vtk warnings see bug #5390
					if ( image->GetDimension() > 3 )
						decimateMesh = false;

					if (decimateMesh)
					{
						surfaceFilter->SetDecimate( mitk::ImageToSurfaceFilter::QuadricDecimation );
						surfaceFilter->SetTargetReduction( reductionRate );
					}
					else
					{
						surfaceFilter->SetDecimate( mitk::ImageToSurfaceFilter::NoDecimation );
					}

					surfaceFilter->UpdateLargestPossibleRegion();

					// calculate normals for nicer display
					mitk::Surface::Pointer m_Surface = surfaceFilter->GetOutput();

					vtkPolyData* polyData = m_Surface->GetVtkPolyData();

					if (!polyData) throw std::logic_error("Could not create polygon model");

					polyData->SetVerts(0);
					polyData->SetLines(0);

					if ( smooth || applyMedian || decimateMesh)
					{
						vtkPolyDataNormals* normalsGen = vtkPolyDataNormals::New();
						normalsGen->SetInput( polyData );
						normalsGen->Update();
						m_Surface->SetVtkPolyData( normalsGen->GetOutput() );
						normalsGen->Delete();
					}
					else
					{
						m_Surface->SetVtkPolyData( polyData );
					}
 
					mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
					surfaceNode->SetData(m_Surface);
					surfaceNode->SetName("Surface");
					m_Data->getDataStore()->Add(surfaceNode);
				}
			}
		}
	}
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::StartSegmentation()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	if(m_Play)
	{
		QIcon icn_StartSegmentation;
		icn_StartSegmentation.addFile(":/threeDEditing/res/threeDEditing/stop.png");
		m_Btn_StartSegmentation->setIcon(icn_StartSegmentation);

		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
		m_Timer->start(1000);

		m_StartSeconds = time(NULL);

		m_Play = false;
	}
	else
	{
		QIcon icn_StartSegmentation;
		icn_StartSegmentation.addFile(":/threeDEditing/res/threeDEditing/Forward.png");
		m_Btn_StartSegmentation->setIcon(icn_StartSegmentation);

		m_Timer->stop();
		m_ElapsedTime = 0;
	
		m_Play = true;
	}

	//TimerThread* timer = new TimerThread(/*this*/);
	//timer->start();
	//timer->run();

	/*MyTimer* timer = new MyTimer();
	QThreadEx timerThread;
	timer->moveToThread(&timerThread);
	timerThread.start();
	timer->start();
	QThread* thread = this->thread();
	std::cout << "Ideal thread count on this machine: " << thread->idealThreadCount() << "\n";*/

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::TimerEvent()
{
	std::string addString = "Add";
	std::string substractString = "Subtract";
	std::string paintString = "Paint";
	std::string wipeString = "Wipe";
	std::string bulgeString = "Bulge Tool";
	std::string dragString = "Drag Tool";
	std::string liveWireString = "Live Wire Tool";
	std::string activeSurfaceString = "Active Surface Tool";

	// Calc elapsed seconds
	int elapsedSeconds = time(NULL) - m_StartSeconds;
	std::cout << "Elapsed time (ctime): " << elapsedSeconds << "\n";
	//std::cout << "Elapsed time (save?): " << m_ElapsedTime << "\n";

	//std::cout << "Timer Event.\n";
	char buffer [33];
	std::string text = _itoa(elapsedSeconds,buffer,10);
	text += " seconds";
	m_Time->setText(text.c_str());

	if(m_ElapsedTime <= elapsedSeconds)
	{
		// alle paar sekunden surface bzw. binary rausschreiben
		m_ElapsedTime += m_SaveDataEveryXSeconds;	

		// Save segmentation
		mitk::DataNode::Pointer node;
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(m_Tabs->currentIndex() == 0)
			{	
				// 2D Tool
				if((allNodes->at(i)->GetName()).compare("Scenario_1_Segmentation_2D") == 0 ||   
			       (allNodes->at(i)->GetName()).compare("Scenario_2_Segmentation_2D") == 0)
				{
					node = allNodes->at(i);
					std::cout << "2D Tool: " << node->GetName() << "\n";

					mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
					std::cout << "Active Tool: " << toolManager->GetActiveToolID() << "\n";

					std::string dir;
					std::string text2 = node->GetName();
					text2 += " ";
					text2 += text;
					mitk::Image::Pointer data = (mitk::Image*)node->GetData();

					//if(toolManager->GetActiveToolID() == -1)
					//	break;
					if(toolManager->GetActiveTool() == NULL)
						break;

					if(addString.compare(toolManager->GetActiveTool()->GetName()) == 0 || substractString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					//if(toolManager->GetActiveToolID() == 12 || toolManager->GetActiveToolID() == 21)
					{
						std::cout << "AddSubstract\n";
						// Add oder substract
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\AddSubstract\\";
							dir += text2;
							//dir += ".nrrd";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\AddSubstract\\";
							dir += text2;
							//dir += ".nrrd";
						}
					}
					else if(paintString.compare(toolManager->GetActiveTool()->GetName()) == 0 || wipeString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					//else if(toolManager->GetActiveToolID() == 22 || toolManager->GetActiveToolID() == 23)
					{
						std::cout << "PaintWipe\n";
						// Paint oder wipe
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\PaintWipe\\";
							dir += text2;
							//dir += ".nrrd";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\PaintWipe\\";
							dir += text2;
							//dir += ".nrrd";
						}
					}
					else
					{
						std::cout << "Forbidden Tool!\n";
						continue;
					}
					mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
					imageWriter->SetInput(data);
					imageWriter->SetFileName(dir.c_str());
					imageWriter->SetExtension(".nrrd");
					imageWriter->Write();				
				}
			}
			else if(m_Tabs->currentIndex() == 1)
			{
				// 3D Tool
				if((allNodes->at(i)->GetName()).compare("Scenario_1_Segmentation_3D") == 0)	
				{
					node = allNodes->at(i);
					std::cout << "3D Tool: " << node->GetName() << "\n";

					mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
					std::cout << "Active Tool: " << toolManager->GetActiveToolID() << "\n";
					
					if(toolManager->GetActiveTool() == NULL)
						break;

					std::string dir;
					std::string text2 = node->GetName();
					text2 += " ";
					text2 += text;
					mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();

					//if(toolManager->GetActiveToolID() == 2)
					if(bulgeString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\Bulge\\";
							dir += text2;
							dir += ".stl";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\Bulge\\";
							dir += text2;
							dir += ".stl";
						}
					}
					//else if(toolManager->GetActiveToolID() == 4)
					else if(dragString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\Drag\\";
							dir += text2;
							dir += ".stl";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\Drag\\";
							dir += text2;
							dir += ".stl";
						}
					}
					else if(toolManager->GetActiveToolID() == 7)
					{
						continue;
					}
					else if(toolManager->GetActiveToolID() == 1)
					{
						continue;
					}
					else
					{
						std::cout << "Forbidden Tool!\n";
						continue;
					}

					mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
					surfaceWriter->SetInput( data );
					surfaceWriter->SetFileName(dir.c_str());
					surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
					surfaceWriter->Write();
				}
				else if((allNodes->at(i)->GetName()).compare("Scenario_2_Segmentation_3D") == 0)	
				{
					node = allNodes->at(i);
					std::cout << "3D Tool: " << node->GetName() << "\n";

					mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
					std::cout << "Active Tool: " << toolManager->GetActiveToolID() << "\n";
					
					//if(toolManager->GetActiveToolID() == -1)
					//	break;
					if(toolManager->GetActiveTool() == NULL)
						break;

					std::string dir;
					std::string text2 = node->GetName();
					text2 += " ";
					text2 += text;
					mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();

					//if(toolManager->GetActiveToolID() == 2)
					if(bulgeString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						continue;
					}
					//else if(toolManager->GetActiveToolID() == 4)
					else if(dragString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						continue;
					}
					//else if(toolManager->GetActiveToolID() == 6)
					else if(liveWireString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\LiveWire\\";
							dir += text2;
							dir += ".stl";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\LiveWire\\";
							dir += text2;
							dir += ".stl";
						}
					}
					//else if(toolManager->GetActiveToolID() == 1)
					else if(activeSurfaceString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						continue;
					}
					else
					{
						std::cout << "Forbidden Tool!\n";
						continue;
					}

					mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
					surfaceWriter->SetInput( data );
					surfaceWriter->SetFileName(dir.c_str());
					surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
					surfaceWriter->Write();
				}
				else if((allNodes->at(i)->GetName()).compare("Output") == 0)	
				{
					node = allNodes->at(i);
					std::cout << "3D Tool: " << node->GetName() << "\n";

					mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
					std::cout << "Active Tool: " << toolManager->GetActiveToolID() << "\n";

					//if(toolManager->GetActiveToolID() == -1)
					//	break;
					if(toolManager->GetActiveTool() == NULL)
						break;

					std::string dir;
					std::string text2 = node->GetName();
					text2 += " ";
					text2 += text;
					mitk::Image::Pointer data = (mitk::Image*)node->GetData();

					//if(toolManager->GetActiveToolID() == 1)
					if(activeSurfaceString.compare(toolManager->GetActiveTool()->GetName()) == 0)
					{
						// Add oder substract
						if(AT_HOME == 1)
						{
							dir += "C:\\DA\\Data\\Segmentations\\ActiveSurface\\";
							dir += text2;
							//dir += ".nrrd";
						}
						else
						{
							dir += "D:\\hachmann\\Data\\Segmentations\\ActiveSurface\\";
							dir += text2;
							//dir += ".nrrd";
						}
					}
					else
					{
						std::cout << "Forbidden Tool!\n";
						continue;
					}

					mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
					imageWriter->SetInput(data);
					imageWriter->SetFileName(dir.c_str());
					imageWriter->SetExtension(".nrrd");
					imageWriter->Write();
				}
			}
			else if(m_Tabs->currentIndex() == 2)
			{
				// Options
				std::cout << "Options: " << node->GetName() << "\n";
				break;
			}
			else
			{
				std::cout << "Error: Unknown tab index!\n";
			}
		}
	}
	if(elapsedSeconds >= m_DurationOfTest)
	{
		// time is up
		std::cout << "Time is up: " << elapsedSeconds << " seconds worked!\n";
		m_ManualToolSelectionBox->OnGeneralToolMessage("Time is up!");
		m_ManualToolSelectionBox->OnToolGUIProcessEventsMessage();

		QIcon icn_StartSegmentation;
		icn_StartSegmentation.addFile(":/threeDEditing/res/threeDEditing/Forward.png");
		m_Btn_StartSegmentation->setIcon(icn_StartSegmentation);

		m_Timer->stop();
		m_ElapsedTime = 0;

		m_Play = true;
	}
}

void pdp::ThreeDEditing::UseMITKSegmentationTools()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// see QmitkSegmentationView.cpp and ui_QmitkSegmentationControls.h
	// see QmitkToolWorkingDataSelectionBox Class 

	// Tabs fuer Toolmanager
	m_Tabs = new QTabWidget(this);
	QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
	m_Tabs->setSizePolicy(sizePolicy2);
	QWidget *page1 = new QWidget();
	QWidget *page2 = new QWidget();
	QWidget *page3 = new QWidget();
	m_Page1Layout = new QVBoxLayout();
	m_Page2Layout = new QVBoxLayout();
	m_Page3Layout = new QVBoxLayout();
	
	m_Tabs->addTab(page1, "&2D Tools");
	m_Tabs->addTab(page2, "&3D Tools");
	m_Tabs->addTab(page3, "&Options");
	page1->setLayout(m_Page1Layout);
	page2->setLayout(m_Page2Layout);
	page3->setLayout(m_Page3Layout);
	m_Layout->itemAt(0)->layout()->addWidget(m_Tabs);
	QObject::connect(m_Tabs, SIGNAL(currentChanged(int)), this, SLOT(CurrentTab(int)));

	m_Tabs->setTabToolTip(0, "Switch to 2D tools.");
	m_Tabs->setTabToolTip(2, "Switch to 3D tools.");
	m_Tabs->setTabToolTip(2, "Switch to Options.");
	
	m_SelectReferenceImageLayout = new QHBoxLayout();
	m_Page1Layout->addLayout(m_SelectReferenceImageLayout);

	m_SelectSurfaceLayout = new QVBoxLayout();
	m_Page1Layout->addLayout(m_SelectSurfaceLayout);
	m_SelectSurfaceLayout1 = new QHBoxLayout();
	m_SelectSurfaceLayout->addLayout(m_SelectSurfaceLayout1);
	m_SelectSurfaceLayout2 = new QHBoxLayout();
	m_SelectSurfaceLayout->addLayout(m_SelectSurfaceLayout2);

	if(!m_HasReferenceDataSet)
	{
		// Reference Data Set not set yet -> ask for dataset
		m_WorkingDatasetLabel = new QLabel( "Working Dataset:", this );
		QFont f = m_WorkingDatasetLabel->font();
		f.setBold(false);
		m_WorkingDatasetLabel->setFont( f );
		m_SelectReferenceImageLayout->addWidget(m_WorkingDatasetLabel);

		m_SelectReferenceImageBox = new QComboBox();
		m_SelectReferenceImageLayout->addWidget(m_SelectReferenceImageBox);
		connect(m_SelectReferenceImageBox, SIGNAL(activated(int)), this, SLOT(ReferenceImage(int)));

		m_SelectReferenceImageBox->addItem("Select Reference Image");
		// List all images	
		int countReferenceImages = 0;
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
			{
				// Sort out binary images
				int dummy;
				bool propertyFound = allNodes->at(i)->GetIntProperty("UniqueNameBinary", dummy);
				if(propertyFound)
					continue;

				// add images to vector
				m_CurrentReferenceImages.push_back(allNodes->at(i)->GetName());
				m_SelectReferenceImageBox->addItem(allNodes->at(i)->GetName().c_str());
				allNodes->at(i)->SetIntProperty("UniqueNameReferenceImages", countReferenceImages);
				countReferenceImages++;
			}		
		}

		m_HasReferenceDataSet = true;
	}

	// Select Segmentation Surface Box
	m_SelectSurfaceBox = new QComboBox();
	m_SelectSurfaceLayout1->addWidget(m_SelectSurfaceBox);
	connect(m_SelectSurfaceBox, SIGNAL(activated(int)), this, SLOT(SelectBinaryImage(int)));

	// Create new empty surface
	m_NewSurfaceButton = new QPushButton();
	m_NewSurfaceButton->setText("New");
	m_SelectSurfaceLayout1->addWidget(m_NewSurfaceButton);
	connect(m_NewSurfaceButton, SIGNAL(clicked()), this, SLOT(CreateEmptySegmentation()));

	// Get first datastorage entry
	mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = m_Data->getDataStore()->GetAll();
	mitk::DataNode* node = allImageNodes->at(0);
	mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
	
	// Toolmanager
    m_ManualToolSelectionBox = new QmitkToolSelectionBox(this);
    m_ManualToolSelectionBox->setObjectName(QString::fromUtf8("m_ManualToolSelectionBox"));
	m_ManualToolGUIContainer = new QmitkToolGUIArea(this);
	m_ManualToolGUIContainer->setObjectName(QString::fromUtf8("m_ManualToolGUIContainer"));
    
	// Add toolbox to layout
	//m_ManualToolSelectionBox->setFixedWidth(250);
	//m_ManualToolGUIContainer->setFixedWidth(250);
	m_ManualToolSelectionBox->setFixedWidth(300);
	m_ManualToolGUIContainer->setFixedWidth(300);
	
	m_ManualToolSelectionBox->setEnabled( true );
	mitk::ToolManager* toolManager = m_ManualToolSelectionBox->GetToolManager();
	toolManager->SetDataStorage(*(m_Data->getDataStore()));

	m_ManualToolSelectionBox->SetGenerateAccelerators(true);
	m_ManualToolSelectionBox->SetToolGUIArea(m_ManualToolGUIContainer);
	m_ManualToolSelectionBox->SetEnabledMode(QmitkToolSelectionBox::AlwaysEnabled );
	m_ManualToolSelectionBox->setEnabled( true );

	// Options:
	// Convert when changing tool tabs
	QPushButton* convertDataButton = new QPushButton();
	convertDataButton->setText("Convert Data (2D <-> 3D)");
	convertDataButton->setCheckable(true);
	m_Page3Layout->addWidget(convertDataButton);
	connect(convertDataButton, SIGNAL(clicked()), this, SLOT(ConvertData()));
	
	// Convert when changing tool tabs	bool triangle = false;	
	QPushButton* useTriangleButton = new QPushButton();
	useTriangleButton->setText("Triangulate Surface");
	useTriangleButton->setCheckable(true);
	m_Page3Layout->addWidget(useTriangleButton);
	connect(useTriangleButton, SIGNAL(clicked()), this, SLOT(UseTriangle()));
	// Convert when changing tool tabsbool clean = false;
	QPushButton* useCleanButton = new QPushButton();
	useCleanButton->setText("Clean Surface");
	useCleanButton->setCheckable(true);
	m_Page3Layout->addWidget(useCleanButton);
	connect(useCleanButton, SIGNAL(clicked()), this, SLOT(UseClean()));
	// Convert when changing tool tabs	bool smooth = false;
	QPushButton* useSmoothButton = new QPushButton();
	useSmoothButton->setText("Smooth Surface");
	useSmoothButton->setCheckable(true);
	m_Page3Layout->addWidget(useSmoothButton);
	connect(useSmoothButton, SIGNAL(clicked()), this, SLOT(UseSmooth()));

	// Reinit Surrounding
	m_SelectReinitSurroundingLayout = new QHBoxLayout();
	m_Page3Layout->addLayout(m_SelectReinitSurroundingLayout);

	QLabel* surroundingLabel = new QLabel( "Reinit Surrounding", this );
	QFont f2 = surroundingLabel->font();
	f2.setBold(false);
	surroundingLabel->setFont( f2 );
	m_SelectReinitSurroundingLayout->addWidget(surroundingLabel);

	QSpinBox* selectSurroundingBox = new QSpinBox();
	selectSurroundingBox->setRange(0, 50);
	selectSurroundingBox->setSuffix(" px");
	selectSurroundingBox->setValue(0);
	m_SelectReinitSurroundingLayout->addWidget(selectSurroundingBox);
	connect(selectSurroundingBox, SIGNAL(valueChanged(int)), this, SLOT(ChangeReinitSurrounding(int)));
	
	if(0)
	{
		// disable crosshair, for the other two rendering windows
		m_MitkView->DisableNavigationControllerEventListening();
		m_MitkView->SetWidgetPlaneMode(0);
		//mitk::RenderingManager::GetInstance()->SetDataStorage(m_Data->getDataStore());
		m_MitkView->AddPlanesToDataStorage();
		m_MitkView->DisablePositionTracking();
	}

	CurrentTab(0);
	m_Toolbar->SetToolManagerToggled(true);
	m_Toolbar->SetToolManagerInitialize(false);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::ThreeDEditing::CatchChangedNodesEvents(const mitk::DataNode* dataNode)
{
	//std::cout << "Changed Node: " << dataNode->GetName() << "\n";

	// Get Unique Node Identifier of this node
	int currentId = -2;
	dataNode->GetIntProperty("UniqueID", currentId);
	// Get non-const pointer to this node
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		int id = -1;
		if(!allNodes->at(i)->GetIntProperty("UniqueID", id))
		{
			std::cout << "Error: Data found without unique id: " << allNodes->at(i)->GetName() << "\n";
			continue;
		}
		if(currentId == id)
		{
			// Do for binary images only (2d Tools). For Surfaces (3d Tools) the ModifiedThusConvert property is send at the tools.
			bool isBinary = false;
			allNodes->at(i)->GetBoolProperty("binary", isBinary);
			if(isBinary)
			{
				// pointer to node found
				allNodes->at(i)->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(true));
				std::cout << "Changed Node: " << dataNode->GetName() << "\n";
			}
		}
	}
}

 int pdp::ThreeDEditing::GetUniqueId()
 {
	// Search for free index
	std::list<int> ids;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
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

void pdp::ThreeDEditing::ChangeReinitSurrounding(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	std::cout << "Change reinit surrounding to: " << selection << "\n";	

	const char* propertyKey = "Properties"; 
	RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	properties->SetReinitSurrounding(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::ThreeDEditing::AutoLoadFile()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::vector<std::string> files;
	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
  
	// Adding file/image to the DataNodeFactory
	if(AT_HOME == 1)
	{
		//files.push_back("C:\\DA\\Data\\LungSegmentationSurface.stl");
		//files.push_back("C:\\DA\\Data\\peternode.nrrd");
		//files.push_back("C:\\DA\\Data\\LungSegmentation2.stl");
		//files.push_back("C:\\DA\\Data\\binary.stl");
		//files.push_back("C:\\DA\\Data\\ball.stl");
		//files.push_back("C:\\DA\\Data\\BallWithHole.stl");
		//files.push_back("C:\\DA\\Data\\BallWithSmallHole.stl");
		//files.push_back("C:\\DA\\Data\\SmallBallSmallHole.stl");
		//files.push_back("C:\\DA\\Data\\BallWithSmallHole.stl");
		//files.push_back("C:\\DA\\Data\\BallWithTwoSmallHoles.stl");
		//files.push_back("C:\\DA\\Data\\ClippedPartOfBallWithHole.stl");

		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\OriginalImageThickenings.nrrd");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 1.stl");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 2.stl");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 3.stl");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 4.stl");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 5.stl");
		//files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 6.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 6_MissingTop.stl");

		/*files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 8.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 10.stl");		
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 13.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 14.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 15.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 16.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 17.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 18.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 21.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 22.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 23.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 27.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 28.stl");
		files.push_back("C:\\DA\\Data\\DetectedThickenings\\Surface 31.stl");*/
	}
	else
	{
		//files.push_back("D:\\hachmann\\Data\\peternode.nrrd");
		//files.push_back("D:\\hachmann\\Data\\LungSegmentationSurface.stl");
		//files.push_back("D:\\hachmann\\Data\\peternode.nrrd");
		//files.push_back("D:\\hachmann\\Data\\binary.stl");
		//files.push_back("D:\\hachmann\\Data\\ball.stl");
		//files.push_back("D:\\hachmann\\Data\\BallWithHole.stl");
		//files.push_back("D:\\hachmann\\Data\\BallWithSmallHole.stl");
		//files.push_back("D:\\hachmann\\Data\\BallWithTwoSmallHoles.stl");
		//files.push_back("D:\\hachmann\\Data\\HalfBinary.stl");
		//files.push_back("D:\\hachmann\\Data\\HalfBinary2.stl");
		//files.push_back("D:\\hachmann\\Data\\ClippedPartOfBallWithHole.stl");
		//files.push_back("D:\\hachmann\\Data\\ClippedPartOfBallWithHole2.stl");

		//files.push_back("D:\\hachmann\\Data\\EvaluationScenario\\Box.stl");
		//files.push_back("D:\\hachmann\\Data\\EvaluationScenario\\Half Box 4.stl");
		//files.push_back("D:\\hachmann\\Data\\EvaluationScenario\\Surrounding Box.stl");		
		//files.push_back("D:\\hachmann\\Data\\EvaluationScenario\\Surrounding Binary.nrrd");

		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\OriginalImageThickenings.nrrd");
		//files.push_back("D:\\hachmann\\Data\\klein.nrrd");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 1.stl");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 2.stl");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 3.stl");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 4.stl");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 5.stl");
		//files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 6.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 6_MissingTop.stl");

		/*files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 8.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 10.stl");		
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 13.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 14.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 15.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 16.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 17.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 18.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 21.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 22.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 23.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 27.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 28.stl");
		files.push_back("D:\\hachmann\\Data\\DetectedThickenings\\Surface 31.stl");*/
	}

	for(int i = 0; i < files.size(); i++)
	{
		nodeReader->SetFileName(files[i]);

		try
		{
			nodeReader->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
			std::cout << ex << std::endl;
		}
		catch (std::exception &ex1)
		{
			ex1.what();
		}

		// Adding file/image to the DataNode
		mitk::DataNode::Pointer node;
		node = nodeReader->GetOutput(0);

		// Add unique identifier for data nodes
		int newId = GetUniqueId();
		node->SetIntProperty("UniqueID", newId);
		node->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
		
		// Surface visualisation
		node->SetFloatProperty("material.wireframeLineWidth", 5.0);

		m_Data->getDataStore()->Add(node);
		//SetUpMitkView();

		mitk::BaseData::Pointer basedata = node->GetData();
		if (basedata.IsNotNull())
		{
			mitk::RenderingManager::GetInstance()->InitializeViews(
			basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}

	// Set Colors to surfaces
	//std::cout << "Set Colors.\n";
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_Data->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			//allNodes->at(i)->SetProperty("color",mitk::ColorProperty::New(0.9,0.1,0.1/*m_Colors[m_LastColorIndex]*/));
			//std::cout << "Color index: " << m_LastColorIndex << ", max: " << m_Colors.size() << "\n";

			//mitk::ColorProperty::P
			allNodes->at(i)->SetProperty("color",m_Colors[m_LastColorIndex]);
			//std::cout << "Set color to : " << allNodes->at(i)->GetName() << "\n";

			m_LastColorIndex++;
			m_LastColorIndex %= 7;
		}
	}

	// Set Reference Image to bottom
	SetUpMitkView();
	m_Toolbar->SetImagesToBottom();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

/***************************************************************************************************/
// Tab header

QIcon pdp::ThreeDEditing::windowIcon()
{
	QIcon icn_tab;
	icn_tab.addFile(":/threeDEditing/res/threeDEditing/Rubber-32.png");
	return icn_tab;
}

QString pdp::ThreeDEditing::windowTitle()
{
	QString title = "3D Editing";
	return title;
}


