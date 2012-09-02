#include <iostream>
// Qt
#include <QToolBar>
#include <QFile>
#include <QPalette>
#include <QColor>
// MITK
#include "mitkStandaloneDataStorage.h"
#include <mitkDataNodeFactory.h>
#include <QmitkDataStorageTreeModel.h>
#include <QmitkExt/QmitkApplicationBase/QmitkCommonFunctionality.h>
#include "QmitkStdMultiWidget.h"
#include <mitkGlobalInteraction.h>
#include <QmitkToolSelectionBox.h>
#include "mitkEnumerationProperty.h"
#include <mitkLookupTableProperty.h>
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
//VTK
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkRendererCollection.h>
// PDP
#include "../../mitkpdp/gui/single_load.h"
#include "../../mitkpdp/app/lung_dataset.h"
#include "../../mitkpdp/gui/mainwindow.h"
#include "global.h"

#include "ThreeDEditing.h"
#include "DataManager.h"
#include "Toolbar.h"


pdp::Toolbar::Toolbar(ThreeDEditing* editing)
    : m_ThreeDEditing(editing)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_OneColor = true;
	m_ToggleOrientationWidget = 0;
	m_InitOrientationWidget = true;
	m_ShowAnnotation = false;
	m_WheelInteractor = new WheelScrollInteractor("MyWheelInteraction");

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::Toolbar::~Toolbar()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::AddToolbar()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_Toolbar = m_ThreeDEditing->GetMainWindow()->addToolBar("tools");

	QToolButton* btn_OpenFile = new QToolButton(m_Toolbar);
	QIcon icn_OpenFile;
	icn_OpenFile.addFile(":/threeDEditing/res/threeDEditing/Open-32.png");
	btn_OpenFile->setIcon(icn_OpenFile);
	btn_OpenFile->setToolTip("Open new file.");
	btn_OpenFile->setShortcut(QKeySequence("CTRL+O"));
	m_Toolbar->addWidget(btn_OpenFile);
	QObject::connect(btn_OpenFile, SIGNAL(clicked()), this, SLOT(OpenFile()));
	
	QToolButton* btn_SaveSelectedNode = new QToolButton(m_Toolbar);
	QIcon icn_SaveSelectedNode;
	icn_SaveSelectedNode.addFile(":/threeDEditing/res/threeDEditing/Save-32.png");
	btn_SaveSelectedNode->setIcon(icn_SaveSelectedNode);
	btn_SaveSelectedNode->setToolTip("Save selected file.");
	btn_SaveSelectedNode->setShortcut(QKeySequence("CTRL+S"));
	m_Toolbar->addWidget(btn_SaveSelectedNode);
	QObject::connect(btn_SaveSelectedNode, SIGNAL(clicked()), this, SLOT(SaveSelectedNode()));

	QToolButton* btn_GetCurrentDataStorage = new QToolButton(m_Toolbar);
	QIcon icn_GetCurrentDataStorage;
	icn_GetCurrentDataStorage.addFile(":/threeDEditing/res/threeDEditing/Refresh-32.png");
	btn_GetCurrentDataStorage->setIcon(icn_GetCurrentDataStorage);
	btn_GetCurrentDataStorage->setToolTip("Load thickenings created by Pleuradat Plus.");
	m_Toolbar->addWidget(btn_GetCurrentDataStorage);
	QObject::connect(btn_GetCurrentDataStorage, SIGNAL(clicked()), this, SLOT(GetCurrentDataStorage()));

	QToolButton* btn_RemoveSelectedNode = new QToolButton(m_Toolbar);
	QIcon icn_RemoveSelectedNode;
	icn_RemoveSelectedNode.addFile(":/threeDEditing/res/threeDEditing/Delete-32.png");
	btn_RemoveSelectedNode->setIcon(icn_RemoveSelectedNode);
	btn_RemoveSelectedNode->setToolTip("Remove selected data node.");
	btn_RemoveSelectedNode->setShortcut(QKeySequence("Del"));
	m_Toolbar->addWidget(btn_RemoveSelectedNode);
	QObject::connect(btn_RemoveSelectedNode, SIGNAL(clicked()), this, SLOT(RemoveSelectedNode()));
	
	m_Toolbar->addSeparator();
	
	QToolButton* btn_Undo = new QToolButton(m_Toolbar);
	QIcon icn_Undo;
	icn_Undo.addFile(":/threeDEditing/res/threeDEditing/Undo_48.png");
	btn_Undo->setIcon(icn_Undo);
	btn_Undo->setToolTip("Undo the last action (not supported by all modules).");
	btn_Undo->setShortcut(QKeySequence("CTRL+Z"));
	m_Toolbar->addWidget(btn_Undo);
	QObject::connect(btn_Undo, SIGNAL(clicked()), this, SLOT(Undo()));

	QToolButton* btn_Redo = new QToolButton(m_Toolbar);
	QIcon icn_Redo;
	icn_Redo.addFile(":/threeDEditing/res/threeDEditing/Redo_48.png");
	btn_Redo->setIcon(icn_Redo);
	btn_Redo->setToolTip("Execute the last action that was undone again (not supported by all modules).");
	btn_Redo->setShortcut(QKeySequence("CTRL+Y"));
	m_Toolbar->addWidget(btn_Redo);
	QObject::connect(btn_Redo, SIGNAL(clicked()), this, SLOT(Redo()));

	m_Toolbar->addSeparator();

	QToolButton* btn_ToggleDataManager = new QToolButton(m_Toolbar);
	QIcon icn_ToggleDataManager;
	icn_ToggleDataManager.addFile(":/threeDEditing/res/threeDEditing/DataManager.png");
	btn_ToggleDataManager->setIcon(icn_ToggleDataManager);
	btn_ToggleDataManager->setToolTip("Toggle data manager.");
	m_Toolbar->addWidget(btn_ToggleDataManager);
	m_DataManagerToggled = true;
	btn_ToggleDataManager->setDown(true);
	QObject::connect(btn_ToggleDataManager, SIGNAL(clicked()), this, SLOT(ToggleDataManager()));


	QToolButton* btn_UseMITKSegmentationTools = new QToolButton(m_Toolbar);
	QIcon icn_UseMITKSegmentationTools;
	icn_UseMITKSegmentationTools.addFile(":/threeDEditing/res/threeDEditing/Segmentation.png");
	btn_UseMITKSegmentationTools->setIcon(icn_UseMITKSegmentationTools);
	btn_UseMITKSegmentationTools->setToolTip("Toggle tool manager.");
	m_Toolbar->addWidget(btn_UseMITKSegmentationTools);
	m_ToolManagerInitialize = true;
	m_ToolManagerToggled = false;
	btn_UseMITKSegmentationTools->setDown(false);
	QObject::connect(btn_UseMITKSegmentationTools, SIGNAL(clicked()), this, SLOT(ToggleToolManager()));

	m_Toolbar->addSeparator();

	// Surface Representation part
	QToolButton* btn_ToggleView = new QToolButton(m_Toolbar);
	QIcon icn_ToggleView;
	icn_ToggleView.addFile(":/threeDEditing/res/threeDEditing/View.png");
	btn_ToggleView->setIcon(icn_ToggleView);
	btn_ToggleView->setToolTip("Toggle surface representation: Points -> Surface -> Wireframe.");
	m_Toolbar->addWidget(btn_ToggleView);
	QObject::connect(btn_ToggleView, SIGNAL(clicked()), this, SLOT(ToggleView()));

	QToolButton* btn_ToggleSurfaceColor = new QToolButton(m_Toolbar);
	QIcon icn_ToggleSurfaceColor;
	icn_ToggleSurfaceColor.addFile(":/threeDEditing/res/threeDEditing/Color.png");
	btn_ToggleSurfaceColor->setIcon(icn_ToggleSurfaceColor);
	btn_ToggleSurfaceColor->setToolTip("Toggle color of surfaces. Show one or multiple colors.");
	m_Toolbar->addWidget(btn_ToggleSurfaceColor);
	QObject::connect(btn_ToggleSurfaceColor, SIGNAL(clicked()), this, SLOT(ToggleSurfaceColor()));

	QToolButton* btn_ToggleOrientationWidget = new QToolButton(m_Toolbar);
	QIcon icn_ToggleOrientationWidget;
	icn_ToggleOrientationWidget.addFile(":/threeDEditing/res/threeDEditing/Orientation.png");
	btn_ToggleOrientationWidget->setIcon(icn_ToggleOrientationWidget);
	btn_ToggleOrientationWidget->setToolTip("Toggle Orientation Widget.");
	m_Toolbar->addWidget(btn_ToggleOrientationWidget);
	QObject::connect(btn_ToggleOrientationWidget, SIGNAL(clicked()), this, SLOT(ToggleOrientationWidget()));

	QToolButton* btn_ToggleAnnotation = new QToolButton(m_Toolbar);
	QIcon icn_ToggleAnnotation;
	icn_ToggleAnnotation.addFile(":/threeDEditing/res/threeDEditing/Annotation.png");
	btn_ToggleAnnotation->setIcon(icn_ToggleAnnotation);
	btn_ToggleAnnotation->setToolTip("Toggle renderer window informations.");
	m_Toolbar->addWidget(btn_ToggleAnnotation);
	QObject::connect(btn_ToggleAnnotation, SIGNAL(clicked()), this, SLOT(ToggleAnnotation()));

	m_Toolbar->addSeparator();
	
	QColor color(214,247,202);
	QPalette colorPalette(color);
	m_Toolbar->setAutoFillBackground(true);
	m_Toolbar->setPalette(colorPalette);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::Toolbar::RemoveToolbar()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_ThreeDEditing->GetMainWindow()->removeToolBar(m_Toolbar);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


void pdp::Toolbar::OpenFile()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
	QStringList fileNames;
	
	if(AT_HOME == 1)
	{
		fileNames = QFileDialog::getOpenFileNames(NULL, "Open", "C:\\DA\\Data\\", "*" );
	}
	else
	{
		fileNames = QFileDialog::getOpenFileNames(NULL, "Open", "E:\\Media Informatics\\thesis\\Hendrik_PhanAnh_pdp\\PA_Hendrik\\PA_Hendrik\\pdp\\Hendrik_pdp\\data", "*" );
		
	}

	if(!(fileNames.isEmpty()))
	{
		for(QStringList::Iterator fileName = fileNames.begin(); fileName != fileNames.end(); fileName++)
		{
			if(QFile::exists(fileName->toLocal8Bit().data()))
			{
				nodeReader->SetFileName(fileName->toLocal8Bit().data());
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
				
				// Add unique id
				int newId = m_ThreeDEditing->GetUniqueId();
				node->SetIntProperty("UniqueID", newId);
				node->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));

				// Surface visualisation
				//node->SetFloatProperty("material.wireframeLineWidth", 8.0);
				node->ReplaceProperty("line width", mitk::IntProperty::New(3));

				m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(node);
				m_ThreeDEditing->SetUpMitkView();
			}
			else
			{
				std::cout << "Selected file does not exist!\n";
			}
		}
	}
	else
	{
		std::cout << "No file given to the program!\n";
	}

	// Segmentations should be in the front, the reference image in the back
	SetImagesToBottom();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::SaveSelectedNode()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Test save selected nodes
	QModelIndexList indexesOfSelectedRows = m_ThreeDEditing->GetDataManager()->GetTreeView()->selectionModel()->selectedRows();

	mitk::DataNode* node = 0;
	unsigned int indexesOfSelectedRowsSize = indexesOfSelectedRows.size();
	for (unsigned int i = 0; i<indexesOfSelectedRowsSize; ++i)
	{
		node = m_ThreeDEditing->GetDataManager()->GetTreeModel()->GetNode(indexesOfSelectedRows.at(i));
		// if node is not defined or if the node contains geometry data do not remove it
		if ( node != 0 )
		{
		  mitk::BaseData::Pointer data = node->GetData();
		  if (data.IsNotNull())
		  {
			QString error;
			try
			{
			  CommonFunctionality::SaveBaseData( data.GetPointer(), node->GetName().c_str() );
			}
			catch(std::exception& e)
			{
			  error = e.what();
			}
			catch(...)
			{
			  error = "Unknown error occured";
			}
			if( !error.isEmpty() )
				std::cout << error.toStdString() << "\n";
		  }
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::GetCurrentDataStorage()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Retrieve data storage -> search for widget
	std::stack<QWidget*>* mainWinTabs = m_ThreeDEditing->GetMainWindow()->GetTabs();
	std::stack<QWidget*> tmpTabs;
	QWidget* currentTab;
	SingleLoad* singleLoadWidget;
	bool tabFound = false;
	std::cout << "tmp tabs size: " << tmpTabs.size() << "\n";

	std::cout << "Current number of Main Window Tabs: " << mainWinTabs->size() << "\n";
	// search in the stack
	int originalMainWinTabsSize = mainWinTabs->size();
	for(int i = 0; i < originalMainWinTabsSize; i++)
	{
		currentTab = mainWinTabs->top();
		if(currentTab->windowTitle().toStdString().compare("Messung laden") == 0)
		{
			singleLoadWidget = (SingleLoad*)currentTab;
			tabFound = true;
			break;
		}
		tmpTabs.push(currentTab);
		mainWinTabs->pop();
	}
	// Reset the stack
	int originalTmpTabsSize = tmpTabs.size();
	for(int i = 0; i < originalTmpTabsSize; i++)
	{
		mainWinTabs->push(tmpTabs.top());
		tmpTabs.pop();
	}
	std::cout << "Current number of Main Window Tabs: " << mainWinTabs->size() << "\n";

	if(tabFound)
	{
		std::cout << "Found instance of Single Load\n";

		// Get all stored nodes in the common data storage
		mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = singleLoadWidget->GetDataSet()->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allImageNodes->size(); i++)
		{
			// Add unique id
			int newId = m_ThreeDEditing->GetUniqueId();
			allImageNodes->at(i)->SetIntProperty("UniqueID", newId);
			allImageNodes->at(i)->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));

			// Add each node stored in the common data storage to our lokal threeDEditing data storage
			m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(allImageNodes->at(i));
		}
		//m_ThreeDEditing->GetLungDataset()->getDataStore()->Print(std::cout);
	}
	else
	{
		std::cout << "No instance of Single Load found\n";
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::RemoveSelectedNode()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//m_ThreeDEditing->GetLungDataset()->getDataStore()->Print(std::cout);

	// Test save selected nodes
	QModelIndexList indexesOfSelectedRows = m_ThreeDEditing->GetDataManager()->GetTreeView()->selectionModel()->selectedRows();

	mitk::DataNode* node = 0;
	unsigned int indexesOfSelectedRowsSize = indexesOfSelectedRows.size();
	for (unsigned int i = 0; i<indexesOfSelectedRowsSize; ++i)
	{
		node = m_ThreeDEditing->GetDataManager()->GetTreeModel()->GetNode(indexesOfSelectedRows.at(i));
		// if node is not defined or if the node contains geometry data do not remove it
		if ( node != 0 )
		{
			mitk::BaseData::Pointer data = node->GetData();
			if (data.IsNotNull())
			{
				m_ThreeDEditing->GetLungDataset()->getDataStore()->Remove(node);
			}
		}
	}
	
	//m_ThreeDEditing->GetLungDataset()->getDataStore()->Print(std::cout);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::SetImagesToBottom()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
		{
			// binary? -> segmentation
			bool isBinary = false;
			allNodes->at(i)->GetBoolProperty("binary", isBinary);
			if(isBinary)
				continue;

			// Image found?
			//std::cout << "Reference Image found? : " << allNodes->at(i)->GetName() << "\n";

			// Remove image from dataStorage
			m_ThreeDEditing->GetLungDataset()->getDataStore()->Remove(allNodes->at(i));
			// "Re-"Add image to dataStorage
			m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(allNodes->at(i));
		}
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::Undo()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
	if (model)
	{
		if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
		{
			mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
			verboseundo->GetUndoDescriptions();
			if (descriptions.size() >= 1)
			{
				MITK_INFO << "Undo " << descriptions.front().second;
			}
		}
		model->Undo();
	}
	else
	{
		MITK_ERROR << "No undo model instantiated";
	}

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::Redo()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
	if (model)
	{
		if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
		{
			mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
			verboseundo->GetRedoDescriptions();
			if (descriptions.size() >= 1)
			{
				MITK_INFO << "Redo " << descriptions.front().second;
			}
		}
		model->Redo();
	}
	else
	{
		MITK_ERROR << "No undo model instantiated";
	}

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleDataManager()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	QToolButton* btn_tmp = (QToolButton*) QObject::sender();
	
	if(m_DataManagerToggled)
	{
		std::cout << "Button is set up\n";	
		m_DataManagerToggled = false;
		btn_tmp->setDown(false);

		// Hide DataManager
		m_ThreeDEditing->GetDataManager()->GetTreeView()->setVisible(false);
	}
	else if(!m_DataManagerToggled)
	{
		std::cout << "Button is set down\n";
		m_DataManagerToggled = true;
		btn_tmp->setDown(true);

		// Show DataManager
		m_ThreeDEditing->GetDataManager()->GetTreeView()->setVisible(true);
	}
	else
		std::cout <<"Toggle Button Error\n";

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleToolManager()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	QToolButton* btn_tmp = (QToolButton*) QObject::sender();
	
	if(m_ToolManagerInitialize)
	{
		// Initialize ToolManager
		m_ThreeDEditing->UseMITKSegmentationTools();
		m_ToolManagerInitialize = false;
	}

	if(m_ToolManagerToggled)
	{
		std::cout << "Button is set up\n";	
		m_ToolManagerToggled = false;
		btn_tmp->setDown(false);

		// Hide Tool Manager
		m_ThreeDEditing->GetToolTabs()->setVisible(false);
	}
	else if(!m_ToolManagerToggled)
	{
		std::cout << "Button is set down\n";
		m_ToolManagerToggled = true;
		btn_tmp->setDown(true);

		// Show Tool Manager
		m_ThreeDEditing->GetToolTabs()->setVisible(true);
	}
	else
		std::cout <<"Toggle Button Error\n";

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleView()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	unsigned int currentRepresentationId;
	mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			mitk::EnumerationProperty* representationProp = dynamic_cast<mitk::EnumerationProperty*> (allNodes->at(i)->GetProperty("material.representation"));
			if(!representationProp)
				return;
			currentRepresentationId = representationProp->GetValueAsId();
			break;
		}
	}

	//std::cout << "Current material representation: " << currentRepresentationId << "\n";

	// increase index modulo 3
	currentRepresentationId++;
	currentRepresentationId %= 3;

	//std::cout << "New material representation: " << currentRepresentationId << "\n";

	for(unsigned int i = 0; i < allNodes->size(); i++)
	{
		if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			mitk::EnumerationProperty* representationProp = dynamic_cast<mitk::EnumerationProperty*> (allNodes->at(i)->GetProperty("material.representation"));
			if(!representationProp)
				return;
			representationProp->SetValue(currentRepresentationId);
		}
	}
	
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleSurfaceColor()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	if(m_OneColor)
	{
		// take a look at colors as defined in data node
		std::cout << "Use one surface color.\n"; 
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
			{
				if(strcmp(allNodes->at(i)->GetName().c_str(), "IntersectionOutput 2D") == 0)
					continue;
				if(strcmp(allNodes->at(i)->GetName().c_str(), "IntersectionOutput 3D") == 0)
					continue;
				std::cout << "Surface Name: " << allNodes->at(i)->GetName() << "\n";
				allNodes->at(i)->SetBoolProperty("scalar visibility", false);
				allNodes->at(i)->SetBoolProperty("color mode", false);
			}
		}
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		m_OneColor = false;
	}
	else
	{
		// many colors
		std::cout << "Highlight surfaces with many colors.\n"; 

		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
			{
				if(strcmp(allNodes->at(i)->GetName().c_str(), "IntersectionOutput 2D") == 0)
					continue;
				if(strcmp(allNodes->at(i)->GetName().c_str(), "IntersectionOutput 3D") == 0)
					continue;

				std::cout << "Surface Name: " << allNodes->at(i)->GetName() << "\n";

				allNodes->at(i)->SetBoolProperty("scalar visibility", true);
				allNodes->at(i)->SetBoolProperty("color mode", true);

				float minimum = 0.0;
				float maximum = 255.0;
				allNodes->at(i)->SetFloatProperty("ScalarsRangeMinimum", minimum);
				allNodes->at(i)->SetFloatProperty("ScalarsRangeMaximum", maximum);

				mitk::Color color;
				mitk::ColorProperty::Pointer colorProp;
				allNodes->at(i)->GetProperty(colorProp,"color");
				color = colorProp->GetValue();

				int j = 0;
				int inputNumCells = ((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData()->GetNumberOfCells();
				std::cout << "Number of Cells: " << inputNumCells << "\n";
				vtkUnsignedCharArray *faceColors = vtkUnsignedCharArray::New();
				faceColors->SetNumberOfComponents(3);
				for(; j < inputNumCells; j += 3)
				{
					/*faceColors->InsertNextTuple3(0, 0, 0);
					faceColors->InsertNextTuple3(24, 24, 24);*/

					unsigned char red[3] = {255, 0, 0};
					unsigned char green[3] = {0, 255, 0};
					unsigned char blue[3] = {0, 0, 255};
					
					faceColors->SetName("Colors");

					faceColors->InsertNextTupleValue(red);
					faceColors->InsertNextTupleValue(green);
					faceColors->InsertNextTupleValue(blue);
 

					/*faceColors->InsertNextTuple3(254, 0, 0);
					faceColors->InsertNextTuple3(0, 254, 0);
					faceColors->InsertNextTuple3(254, 254, 0);
					faceColors->InsertNextTuple3(0, 0, 254);
					faceColors->InsertNextTuple3(254, 0, 254);
					faceColors->InsertNextTuple3(0, 254, 254);
					faceColors->InsertNextTuple3(254, 254, 254);*/
				}
				//for(;j < inputNumCells; j++)
				//{
				//	faceColors->InsertNextTuple3(254, 254, 254);
				//}

				//faceColors->Print(std::cout);

				((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData()->GetCellData()->SetScalars(faceColors);		
				//((mitk::Surface*)allNodes->at(i)->GetData())->GetVtkPolyData()->GetPointData()->SetScalars(faceColors);		

				vtkSmartPointer<vtkLookupTable> vtkLUT = vtkSmartPointer<vtkLookupTable>::New();
				vtkLUT->SetNumberOfTableValues( 3 );
				//vtkLUT->SetRange(0.0, 3.0);
				vtkLUT->SetRange(0.0, 255.0);
				vtkLUT->SetTableValue( 0, 0.9, 0.1, 0.1 );
				vtkLUT->SetTableValue( 1, 0.1, 0.9, 0.1 );
				vtkLUT->SetTableValue( 2, 0.1, 0.1, 0.9 );
				vtkLUT->Build();

				mitk::LookupTable::Pointer lookupTable = mitk::LookupTable::New();
				lookupTable->SetVtkLookupTable(vtkLUT);
				mitk::LookupTableProperty::Pointer lookupTableProperty = mitk::LookupTableProperty::New(lookupTable);    
				allNodes->at(i)->SetProperty( "LookupTable", lookupTableProperty );
	
				allNodes->at(i)->SetBoolProperty("scalar visibility", true);
				allNodes->at(i)->SetFloatProperty("ScalarsRangeMaximum", 3);
				allNodes->at(i)->SetBoolProperty("color mode", true);
				//allNodes->at(i)->SetBoolProperty("use color", false);
				allNodes->at(i)->SetBoolProperty("use color", true);
				allNodes->at(i)->Update();


			}
		}

		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		m_OneColor = true;
	}
	
	// set color TODO: only two colors?

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleOrientationWidget()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Toogle Orientation Widget. Case: " << m_ToggleOrientationWidget << "\n";
	
	if(m_InitOrientationWidget)
	{
		//std::cout << "Initialize Orientation Widget.\n";

		m_Axes = vtkSmartPointer<vtkAxesActor>::New();
		m_AnnotatedCube = vtkSmartPointer<vtkAnnotatedCubeActor>::New();
		m_AnnotatedCube->SetXPlusFaceText("R");
		m_AnnotatedCube->SetXMinusFaceText("L");
		m_AnnotatedCube->SetYMinusFaceText("I");
		m_AnnotatedCube->SetYPlusFaceText("S");
		m_AnnotatedCube->SetZMinusFaceText("P");
		m_AnnotatedCube->SetZPlusFaceText("A");

		//TODO
		m_Widget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
		m_Widget->SetOutlineColor( 0.9300, 0.5700, 0.1300 );
		m_Widget->SetViewport( 0.0, 0.0, 0.4, 0.4 );

		m_InitOrientationWidget = false;
	}

	if(m_ToggleOrientationWidget == 0)
	{
		//std::cout << "Orientation Widget, Case: 0.\n";
		
		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());
		m_Widget->SetEnabled(0);

		m_Widget->SetOrientationMarker(m_Axes);
		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());
		m_Widget->SetEnabled(1);
		//m_Widget->SetInteractive(1);
		m_Widget->SetInteractive(0);
		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->Render();

		m_ToggleOrientationWidget++;	
	}
	else if(m_ToggleOrientationWidget == 1)
	{
		//std::cout << "Orientation Widget, Case: 1.\n";

		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());
		m_Widget->SetEnabled(0);

		m_Widget->SetOrientationMarker(m_AnnotatedCube);
		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());

		m_Widget->SetEnabled(1);
		//m_Widget->SetInteractive(1);
		m_Widget->SetInteractive(0);
		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->Render();

		m_ToggleOrientationWidget++;
	}
	else if(m_ToggleOrientationWidget == 2)
	{
		//std::cout << "Orientation Widget, Case: 2.\n";

		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());
		m_Widget->SetEnabled(0);

		vtkSmartPointer<vtkAxesActor> Axes = vtkSmartPointer<vtkAxesActor>::New();
		vtkSmartPointer<vtkAnnotatedCubeActor> AnnotatedCube = vtkSmartPointer<vtkAnnotatedCubeActor>::New();
		AnnotatedCube->SetXPlusFaceText("R");
		AnnotatedCube->SetXMinusFaceText("L");
		AnnotatedCube->SetYMinusFaceText("I");
		AnnotatedCube->SetYPlusFaceText("S");
		AnnotatedCube->SetZMinusFaceText("P");
		AnnotatedCube->SetZPlusFaceText("A");

		vtkAssembly* Assemble = vtkAssembly::New();
		Assemble->AddPart(Axes);
		Assemble->AddPart(AnnotatedCube);
		m_Widget->SetOrientationMarker(Assemble);

		m_Widget->SetInteractor(m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->GetInteractor());

		m_Widget->SetEnabled(1);
		//m_Widget->SetInteractive(1);
		m_Widget->SetInteractive(0);
		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow()->Render();

		m_ToggleOrientationWidget++;	
	}
	else if(m_ToggleOrientationWidget == 3)
	{
		//std::cout << "Orientation Widget, Case: 3.\n";
		m_Widget->SetEnabled(0);			
		m_ToggleOrientationWidget = 0;
	}
	else 
	{
		std::cout << "Orientation Widget, Case: ERROR.\n";
	}

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Toolbar::ToggleAnnotation()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	if(m_ShowAnnotation)
	{
		m_ShowAnnotation = false;
		mitk::GlobalInteraction::GetInstance()->RemoveListener(m_WheelInteractor);
		m_WheelInteractor->HideAnnotations();
	}
	else
	{
		m_ShowAnnotation = true;
		mitk::GlobalInteraction::GetInstance()->AddListener(m_WheelInteractor);
		m_WheelInteractor->ShowAnnotations();
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}