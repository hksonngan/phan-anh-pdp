#include <iostream>

// QT
#include <QtGui/QTreeView>
#include <QSizePolicy>
#include <QtGui/QHBoxLayout>

// MITK
#include <QmitkDataStorageTreeModel.h>
#include "mitkEnumerationProperty.h"
#include <mitkRenderingManager.h>
#include <mitkMapper.h>
#include <mitkGPUVolumeMapper3D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include "QmitkStdMultiWidget.h"
#include "QmitkInfoDialog.h"
#include "mitkShowSegmentationAsSurface.h"
#include "mitkProgressBar.h"
#include "mitkReduceContourSetFilter.h"
#include "mitkSurfaceToImageFilter.h"
#include <mitkImageToSurfaceFilter.h>
#include "mitkManualSegmentationToSurfaceFilter.h"

// VTK
#include <vtkMarchingCubes.h>
#include <vtkDecimatePro.h>
#include <vtkTriangleFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkCleanPolyData.h>

// PDP
#include "ThreeDEditing.h"
#include "RenderingManagerProperty.h"
#include "toolbar.h"
#include "dataManager.h"

pdp::DataManager::DataManager(ThreeDEditing* editing)
    : m_ThreeDEditing(editing)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_NodeTreeView = new QTreeView(this);
	m_NodeTreeView->setObjectName(QString::fromUtf8("dicom_data_tree"));
	m_NodeTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_NodeTreeView->setAlternatingRowColors(true);
	m_NodeTreeView->setDragEnabled(true);
	m_NodeTreeView->setDropIndicatorShown(true);
	m_NodeTreeView->setAcceptDrops(true);

	m_NodeTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu(const QPoint&)));

	m_RenderingMode = "OFF";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::DataManager::~DataManager()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::OpenContextMenu(const QPoint&)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	//m_NodeMenu->clear();
	//# m_NodeMenu
	m_NodeMenu = new QMenu(m_NodeTreeView);
	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());

	// Reinit
	m_ReinitAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/Refresh-32.png"), "Reinit", this);
	QObject::connect(m_ReinitAction, SIGNAL(triggered(bool)), this, SLOT(ReinitSelectedNodes(bool)));
    m_NodeMenu->addAction(m_ReinitAction);

	// Volume Rendering
	m_ToggleVolumeRenderingAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/volumerendering32.png"), "Volume Rendering", this);
	m_ToggleVolumeRenderingAction->setMenu(new QMenu);
	QObject::connect(m_ToggleVolumeRenderingAction->menu(), SIGNAL(aboutToShow()), this, SLOT(VolumeRenderingMenuAboutToShow()));
	m_NodeMenu->addAction(m_ToggleVolumeRenderingAction);

	// Create Polygon Model
	m_CreatePolygonModelAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/CreateSurface-32.png"), "Create Polygon Model", this);
	m_CreatePolygonModelAction->setMenu(new QMenu);
	QObject::connect(m_CreatePolygonModelAction->menu(), SIGNAL(aboutToShow()), this, SLOT(CreatePolygonModelMenuAboutToShow()));
	m_NodeMenu->addAction(m_CreatePolygonModelAction);

	// Reduce Polygon Model
	m_ReduceContourSetAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/CreateSurface-32.png"), "Reduce Polygon Model", this);
	QObject::connect(m_ReduceContourSetAction, SIGNAL(triggered(bool)), this, SLOT(ReduceContourSet()));
	m_NodeMenu->addAction(m_ReduceContourSetAction);
	
	// Details
	m_DataNodePropertiesAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/propertylist.png"), "Details", this);
	QObject::connect(m_DataNodePropertiesAction, SIGNAL(triggered(bool)), this, SLOT(DataNodeProperties()));
	m_NodeMenu->addAction(m_DataNodePropertiesAction);

	// Surface Representation
	m_SurfaceRepresentation = new QAction(QIcon(":/threeDEditing/res/threeDEditing/data-type-mesh-24.png"), "Surface Representation", this);
	m_SurfaceRepresentation->setMenu(new QMenu);
	QObject::connect(m_SurfaceRepresentation->menu(), SIGNAL(aboutToShow()), this, SLOT(SurfaceRepresentationMenuAboutToShow()));
	m_NodeMenu->addAction(m_SurfaceRepresentation);

	// Opacity
	m_OpacitySlider = new QSlider;
	m_OpacitySlider->setMinimum(0);
	m_OpacitySlider->setMaximum(100);
	m_OpacitySlider->setOrientation(Qt::Horizontal);	
	float opacity = 0.0;
	node->GetFloatProperty("opacity", opacity);
	std::cout << "Current Opacity: " << opacity << "\n";
	m_OpacitySlider->setSliderPosition(opacity*100);	
	QObject::connect(m_OpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(OpacityChanged(int)));
	QLabel* OpacityLabel = new QLabel("Opacity: ");
	QHBoxLayout* OpacityWidgetLayout = new QHBoxLayout();
	OpacityWidgetLayout->setContentsMargins(4,4,4,4);
	OpacityWidgetLayout->addWidget(OpacityLabel);
	OpacityWidgetLayout->addWidget(m_OpacitySlider);
	QWidget* OpacityWidget = new QWidget;
	OpacityWidget->setLayout(OpacityWidgetLayout);
	m_OpacityAction = new QWidgetAction(this);
	m_OpacityAction->setDefaultWidget(OpacityWidget);
	QObject::connect(m_OpacityAction, SIGNAL(changed()), this, SLOT(OpacityActionChanged()));
	m_NodeMenu->addAction(m_OpacityAction);

	// Color
	m_ColorButton = new QPushButton;
	m_ColorButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
	mitk::Color color;
	mitk::ColorProperty::Pointer colorProp;
	node->GetProperty(colorProp,"color");
	if(!colorProp.IsNull())
	{
		color = colorProp->GetValue();
		QString styleSheet = "background-color:rgb(";
		styleSheet.append(QString::number(color[0]*255));
		styleSheet.append(",");
		styleSheet.append(QString::number(color[1]*255));
		styleSheet.append(",");
		styleSheet.append(QString::number(color[2]*255));
		styleSheet.append(")");
		m_ColorButton->setStyleSheet(styleSheet);
	}

	QObject::connect(m_ColorButton, SIGNAL(clicked()), this, SLOT(ColorChanged()));
	QLabel* ColorLabel = new QLabel("Color: ");
	ColorLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	QHBoxLayout* ColorWidgetLayout = new QHBoxLayout;
	ColorWidgetLayout->setContentsMargins(4,4,4,4);
	ColorWidgetLayout->addWidget(ColorLabel);
	ColorWidgetLayout->addWidget(m_ColorButton);
	QWidget* ColorWidget = new QWidget;
	ColorWidget->setLayout(ColorWidgetLayout);
	m_ColorAction = new QWidgetAction(this);
	m_ColorAction->setDefaultWidget(ColorWidget);
	QObject::connect(m_ColorAction, SIGNAL(changed()), this, SLOT(ColorActionChanged()));
	m_NodeMenu->addAction(m_ColorAction);

	m_ConvertAction = new QAction(QIcon(":/threeDEditing/res/threeDEditing/Convert.png"), "Convert", this);
	QObject::connect(m_ConvertAction, SIGNAL(triggered(bool)), this, SLOT(ConvertSelectedNodes(bool)));
    m_NodeMenu->addAction(m_ConvertAction);

	m_NodeMenu->popup(QCursor::pos());

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::ReinitSelectedNodes(bool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// Get Surrounding
	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	int surrounding =  properties->GetReinitSurrounding();

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(!node)
		return;
	mitk::BaseData::Pointer basedata = node->GetData();

	if (basedata.IsNotNull())
	{
		//mitk::Geometry3D::Pointer geo = basedata->GetTimeSlicedGeometry();
		mitk::Geometry3D::Pointer geo = basedata->GetGeometry();
		
		mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geo->GetBoundingBox());
		mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

		std::cout << "Bounding box before reinit: " << surfBounds << "\n";

		surfBounds[0] -= surrounding;
		surfBounds[1] += surrounding;
		surfBounds[2] -= surrounding;
		surfBounds[3] += surrounding;
		surfBounds[4] -= surrounding;
		surfBounds[5] += surrounding;
		geo->SetBounds(surfBounds);

		std::cout << "Bounding box after reinit: " << surfBounds << "\n";

		//mitk::Vector3D spacings;
		//spacings[0] = 1;
		//spacings[1] = 1;
		//spacings[2] = 0.5;
		//geo->SetSpacing(spacings);

		mitk::RenderingManager::GetInstance()->InitializeViews(
		geo/*basedata->GetTimeSlicedGeometry()*/, mitk::RenderingManager::REQUEST_UPDATE_ALL, false/*true*/ );
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::UpdateTreeModel()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_NodeTreeModel = new QmitkDataStorageTreeModel(m_ThreeDEditing->GetLungDataset()->getDataStore());
	m_NodeTreeModel->SetPlaceNewNodesOnTop(false); 
	m_NodeTreeView->setModel(m_NodeTreeModel);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::VolumeRenderingMenuAboutToShow()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_ToggleVolumeRenderingAction->menu()->clear();
	QAction* tmp;
	tmp = m_ToggleVolumeRenderingAction->menu()->addAction("OFF");
	tmp->setCheckable(true);
	if(m_RenderingMode.compare("OFF") == 0)
		tmp->setChecked(true);
	QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT(VolumeRenderingToggled(bool)));
	tmp = m_ToggleVolumeRenderingAction->menu()->addAction("CPU Rendering");
	tmp->setCheckable(true);
	if(m_RenderingMode.compare("CPU Rendering") == 0)
		tmp->setChecked(true);
	QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT(VolumeRenderingToggled(bool)));
	tmp = m_ToggleVolumeRenderingAction->menu()->addAction("GPU Rendering");
	tmp->setCheckable(true);
	if(m_RenderingMode.compare("GPU Rendering") == 0)
		tmp->setChecked(true);
	QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT(VolumeRenderingToggled(bool)));

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::VolumeRenderingToggled(bool checked)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	QAction* senderAction = qobject_cast<QAction*> (QObject::sender());
	std::string activatedItem = senderAction->text().toStdString();
	
	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(!node)
		return;

	if(activatedItem.compare("OFF") == 0)
	{
		std::cout << "OFF toggled\n";
		senderAction->setChecked(true);
		m_RenderingMode = "OFF";

		node->SetProperty("volumerendering", mitk::BoolProperty::New(false));

		//std::cout << "Image Mapper.\n";
		//unsigned int id = 1;
		//mitk::Mapper::Pointer newMapper = NULL;
		//newMapper = mitk::PlanarFigureMapper2D::New();

		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
		//m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->FullScreenMode(true);

		//TODO
		//node->SetMapper(id, newMapper);

		//ReinitSelectedNodes(true);
	}
	else if(activatedItem.compare("CPU Rendering") == 0)
	{
		std::cout << "CPU Rendering toggled\n";
		senderAction->setChecked(true);
		m_RenderingMode = "CPU Rendering";

		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

		node->SetProperty("volumerendering", mitk::BoolProperty::New(true));
		node->SetProperty("volumerendering.usegpu",mitk::BoolProperty::New(false));

		if(strcmp(node->GetData()->GetNameOfClass(), "Image") == 0)
		{
			std::cout << "Image Mapper.\n";
			unsigned int id = 1;
			mitk::Mapper::Pointer newMapper = NULL;
			newMapper = mitk::GPUVolumeMapper3D::New();
			node->SetMapper(id, newMapper);
		}
		else if(strcmp(node->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			std::cout << "Surface Mapper.\n";
			unsigned int id = 1;
			mitk::Mapper::Pointer newMapper = NULL;
			newMapper = mitk::SurfaceVtkMapper3D::New();
			node->SetMapper(id, newMapper);
		}
		else
		{
			std::cout << "Other kind of data\n";
		}

		//ReinitSelectedNodes(true);
	}
	else if(activatedItem.compare("GPU Rendering") == 0)
	{
		std::cout << "GPU Rendering toggled\n";
		senderAction->setChecked(true);
		m_RenderingMode = "GPU Rendering";

		m_ThreeDEditing->GetMultiWidget()->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

		node->SetProperty("volumerendering", mitk::BoolProperty::New(true));
		node->SetProperty("volumerendering.usegpu",mitk::BoolProperty::New(true));

		if(strcmp(node->GetData()->GetNameOfClass(), "Image") == 0)
		{
			std::cout << "Image Mapper.\n";
			unsigned int id = 1;
			mitk::Mapper::Pointer newMapper = NULL;
			newMapper = mitk::GPUVolumeMapper3D::New();
			node->SetMapper(id, newMapper);
		}
		else if(strcmp(node->GetData()->GetNameOfClass(), "Surface") == 0)
		{
			std::cout << "Surface Mapper.\n";
			unsigned int id = 1;
			mitk::Mapper::Pointer newMapper = NULL;
			newMapper = mitk::SurfaceVtkMapper3D::New();
			node->SetMapper(id, newMapper);
		}
		else
		{
			std::cout << "Other kind of data\n";
		}

		//ReinitSelectedNodes(true);
	}
	else 	
	{
		std::cout << "Error!\n";
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::CreatePolygonModelMenuAboutToShow()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_CreatePolygonModelAction->menu()->clear();
	QAction* tmp;
	tmp = m_CreatePolygonModelAction->menu()->addAction("Create Polygon Model");
	QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT(CreatePolygonModelActionToggled(bool)));
	tmp = m_CreatePolygonModelAction->menu()->addAction("Create Smoothed Polygon Model");
	QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT(CreatePolygonModelActionToggled(bool)));
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::CreatePolygonModelActionToggled(bool checked)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	QAction* senderAction = qobject_cast<QAction*> (QObject::sender());
	std::string activatedItem = senderAction->text().toStdString();
	
	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(!node)
		return;

	if(activatedItem.compare("Create Polygon Model") == 0)
	{
		if(strcmp(node->GetData()->GetNameOfClass(), "Image") == 0)
		{
			std::cout << "Create Polygon Model\n";
			/*vtkMarchingCubes* surfaceCreator = vtkMarchingCubes::New();
			surfaceCreator->SetInput((vtkDataObject*)((mitk::Image*)node->GetData())->GetVtkImageData());
			surfaceCreator->SetValue(0, 1);
			mitk::Surface::Pointer surface = mitk::Surface::New();
			surface->SetVtkPolyData(surfaceCreator->GetOutput());*/

			mitk::ImageToSurfaceFilter::Pointer surfaceCreator = mitk::ImageToSurfaceFilter::New();
			surfaceCreator->SetInput((mitk::Image*)node->GetData());
			surfaceCreator->Update();

			mitk::Surface::Pointer surface = surfaceCreator->GetOutput();
			mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
			surfaceNode->SetData(surface);

			std::string nodeName = node->GetName();
			nodeName += "Surface";
			surfaceNode->SetName(nodeName);

			m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(surfaceNode);

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
			surfaceCreator->Delete();
		}
	}
	else if(activatedItem.compare("Create Smoothed Polygon Model") == 0)
	{
		if(strcmp(node->GetData()->GetNameOfClass(), "Image") == 0)
		{
			std::cout << "Create Smoothed Polygon Model\n";
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

			std::string nodeName = node->GetName();
			nodeName += "Surface";
			mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
			surfaceNode->SetData(m_Surface);
			surfaceNode->SetName(nodeName);
			m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(surfaceNode);

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::SurfaceRepresentationMenuAboutToShow()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(!node)
		return;

	mitk::EnumerationProperty* representationProp =  dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
	if(!representationProp)
		return;

	// clear menu
	m_SurfaceRepresentation->menu()->clear();
	QAction* tmp;

	// create menu entries
	for(mitk::EnumerationProperty::EnumConstIterator it=representationProp->Begin(); it!=representationProp->End(); it++)
	{
		tmp = m_SurfaceRepresentation->menu()->addAction(QString::fromStdString(it->second));
		tmp->setCheckable(true);

		if(it->second == representationProp->GetValueAsString())
		{
			tmp->setChecked(true);
		}

		QObject::connect( tmp, SIGNAL(triggered(bool)), this, SLOT( SurfaceRepresentationActionToggled(bool) ) );
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::SurfaceRepresentationActionToggled( bool /*checked*/ )
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(!node)
		return;

	mitk::EnumerationProperty* representationProp = dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
	if(!representationProp)
		return;

	QAction* senderAction = qobject_cast<QAction*> ( QObject::sender() );

	if(!senderAction)
		return;

	std::string activatedItem = senderAction->text().toStdString();

	if ( activatedItem != representationProp->GetValueAsString() )
	{
		if ( representationProp->IsValidEnumerationValue( activatedItem ) )
		{
			representationProp->SetValue( activatedItem );
			representationProp->InvokeEvent( itk::ModifiedEvent() );
		    representationProp->Modified();

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}

	//ReinitSelectedNodes(true);
	
	//m_ThreeDEditing->GetMultiWidget()->GetRenderWindow1()->repaint();
	
	//node->UpdateOutputData();
	//mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	

	//m_ThreeDEditing->GetMultiWidget()->GetRenderWindow1()->GetRenderer()->SetDataStorage(m_ThreeDEditing->GetLungDataset()->getDataStore());
	//mitk::RenderingManager::GetInstance()->InitializeViews(m_ThreeDEditing->GetLungDataset()->getDataStore());

	//mitk::RenderingManager::GetInstance()->RequestUpdate(m_ThreeDEditing->GetMultiWidget()->mitkWidget1->GetRenderWindow()); 
	//mitk::RenderingManager::GetInstance()->RequestUpdate(m_ThreeDEditing->GetMultiWidget()->mitkWidget2->GetRenderWindow()); 

	//mitk::RenderingManager::GetInstance()->InitializeViews();
	//mitk::RenderingManager::GetInstance()->Print(std::cout);

	/*mitk::TimeSlicedGeometry::Pointer geo = m_ThreeDEditing->GetLungDataset()->getDataStore()->ComputeBoundingGeometry3D(m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews( geo );*/	

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;	
}

void pdp::DataManager::ReduceContourSet()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(node)
	{
		// Check if node is surface
		if(strcmp(node->GetData()->GetNameOfClass(), "Surface") != 0)
		{
			std::cout << "Filter works only with Surfaces!\n";
			return;
		}
		//node->Print(std::cout);
		//node->GetData()->Print(std::cout);


		//mitk::ReduceContourSetFilter::Pointer filter = mitk::ReduceContourSetFilter::New();
		////mitk::SurfaceToSurfaceFilter::Pointer filter = mitk::SurfaceToSurfaceFilter::New();
		//filter->SetInput((mitk::Surface*)node->GetData());
		//filter->SetReductionType(mitk::ReduceContourSetFilter::NTH_POINT);
		//filter->SetStepSize(1);
		//filter->SetMinSpacing(1);
		//filter->SetMaxSpacing(2);
		//std::cout << "Number of Filter Inputs: " << filter->GetNumberOfInputs() << "\n";
		//std::cout << "Number of Filter Outputs: " << filter->GetNumberOfOutputs() << "\n";
		////filter->SetReductionType(mitk::ReduceContourSetFilter::DOUGLAS_PEUCKER);
		//filter->SetTolerance(10);
		////filter->CreateOutputsForAllInputs(0);
		//filter->Update();
		//filter->CreateOutputsForAllInputs(0);
		//std::cout << "Number of Filter Outputs: " << filter->GetNumberOfOutputs() << "\n";


		// Triangulate first
		vtkPolyData *polydata;
		vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
        triangleFilter->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
        triangleFilter->Update();
        polydata = triangleFilter->GetOutput();
        //surface->SetVtkPolyData(polydata);

		// Smooth
		vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
		//read poly1 (poly1 can be the original polygon, or the decimated polygon)
		smoother->SetInput(polydata);//RC++
		smoother->SetNumberOfIterations( /*m_SmoothIteration*/ 50);
		smoother->SetRelaxationFactor( /*m_SmoothRelaxation*/ 0.1);
		smoother->SetFeatureAngle( 60 );
		smoother->FeatureEdgeSmoothingOff();
		//smoother->BoundarySmoothingOff();
		smoother->BoundarySmoothingOn();
		smoother->SetConvergence( 0 );

		smoother->Update();

		//polydata->Delete();//RC--
		vtkPolyData *polydata2;
		polydata2 = smoother->GetOutput();
		//polydata->Register(NULL);//RC++
		//smoother->Delete();

		
		// decimate
		vtkPolyData *polydata3;
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
		decimate->SetInput(polydata2);//RC++
		//decimate->SetTargetReduction(/*m_TargetReduction*/0.95f);
		decimate->SetTargetReduction(/*m_TargetReduction*/0.70f);
		//decimate->SetMaximumError(0.002);
		decimate->SetMaximumError(0.02);
		//polydata->Delete();//RC--
		polydata3 = decimate->GetOutput();
		//polydata->Register(NULL);//RC++
		//decimate->Delete();
				
		polydata3->Update();
		//polydata->SetSource(NULL);

		
		
		// vtkCleanPolyData
		//TODO: evtl. Grund fuer Sculptris Fehler



		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(polydata2);
		
		
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		//mitk::Surface::Pointer outSurface = mitk::Surface::New(); // TODO
		//outSurface = filter->GetOutput(0);

		if(outSurface.IsNull())
		{
			std::cout << "Out Surface is NULL!\n";
				return;
		}
		outNode->SetData(outSurface);
		outNode->SetName("ReducedSurface");

		// Add unique identifier for data nodes
		int newId = m_ThreeDEditing->GetUniqueId();
		outNode->SetIntProperty("UniqueID", newId);
		outNode->SetBoolProperty("ModifiedThusConvert", mitk::BoolProperty::New(false));
		m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(outNode);

		m_ThreeDEditing->GetToolbar()->SetImagesToBottom();		

		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::OpacityChanged(int value)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(node)
	{
		float opacity = static_cast<float>(value)/100.0f;
		node->SetFloatProperty("opacity", opacity);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::OpacityActionChanged()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(node)
	{
		float opacity = 0.0;
		if(node->GetFloatProperty("opacity", opacity))
		{
			m_OpacitySlider->setValue(static_cast<int>(opacity*100));
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::DataNodeProperties()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	std::vector<mitk::DataNode*> selectedNodes;
	selectedNodes.push_back(node);
 
	QmitkInfoDialog QmitkInfoDialog(selectedNodes);
	QmitkInfoDialog.exec();

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::ColorChanged()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(node)
	{
		QColor color = QColorDialog::getColor();
		m_ColorButton->setAutoFillBackground(true);
		std::cout << "Color: " << color.red() << " " << color.green() << " "<< color.blue() << "\n";
		node->SetProperty("color",mitk::ColorProperty::New(color.red()/255.0,color.green()/255.0,color.blue()/255.0));
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::ColorActionChanged()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	if(node)
	{
		mitk::Color color;
		mitk::ColorProperty::Pointer colorProp;
		node->GetProperty(colorProp,"color");
		if(colorProp.IsNull())
		return;
		color = colorProp->GetValue();

		QString styleSheet = "background-color:rgb(";
		styleSheet.append(QString::number(color[0]*255));
		styleSheet.append(",");
		styleSheet.append(QString::number(color[1]*255));
		styleSheet.append(",");
		styleSheet.append(QString::number(color[2]*255));
		styleSheet.append(")");
		m_ColorButton->setStyleSheet(styleSheet);
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::DataManager::ConvertSelectedNodes(bool b)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
	
	// Convert surface to binary image
	if(strcmp(node->GetData()->GetNameOfClass(), "Surface") == 0)
	{
		std::cout << "Create new binary segmentations from surfaces.\n";

		// TODO if there is a binary for the current surface
		// delete old binary and substitute with new created binary image 



		if(m_ThreeDEditing->GetUseTriangle())
		{
			// Triangulate first
			vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
			triangleFilter->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
			triangleFilter->Update();
			((mitk::Surface*)node->GetData())->SetVtkPolyData(triangleFilter->GetOutput());
		}
		if(m_ThreeDEditing->GetUseClean())
		{
			vtkSmartPointer<vtkCleanPolyData> removeDuplicatedPoints = vtkSmartPointer<vtkCleanPolyData>::New(); 
			removeDuplicatedPoints->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
			removeDuplicatedPoints->Update();
			((mitk::Surface*)node->GetData())->SetVtkPolyData(removeDuplicatedPoints->GetOutput());
		}
		if(m_ThreeDEditing->GetUseSmooth())
		{
			// Smooth
			vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
			smoother->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());//RC++
			smoother->SetNumberOfIterations( /*m_SmoothIteration*/ 50);
			smoother->SetRelaxationFactor( /*m_SmoothRelaxation*/ 0.1);
			smoother->SetFeatureAngle( 60 );
			smoother->FeatureEdgeSmoothingOff();
			smoother->BoundarySmoothingOn();
			smoother->SetConvergence( 0 );
			smoother->Update();
			((mitk::Surface*)node->GetData())->SetVtkPolyData(smoother->GetOutput());
		}

		// Create reference image for geometric information
		mitk::DataNode::Pointer node2 = mitk::DataNode::New();
		mitk::PixelType pixelType(typeid(short));
		mitk::Image::Pointer out = mitk::Image::New();
		mitk::Geometry3D::Pointer geo = mitk::Geometry3D::New();					
		geo = ((mitk::Surface*)node->GetData())->GetGeometry();					
		//geo->ChangeImageGeometryConsideringOriginOffset(true);
		geo->SetBounds( ((mitk::Surface*)node->GetData())->GetGeometry()->GetBounds() );
		
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
		filter->SetInput((mitk::Surface*)node->GetData());						
		filter->SetImage(out);
		filter->SetMakeOutputBinary(true);
		filter->Update();

		// Does surface already have binary child? -> substitude
		mitk::DataNode* child = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetNamedDerivedNode(node->GetName().c_str(), node);
		if(m_ThreeDEditing->GetLungDataset()->getDataStore()->Exists(child))
		{
			// remove old child
			m_ThreeDEditing->GetLungDataset()->getDataStore()->Remove(child);
			
			// delete child ?? todo
		}
	

		// Add binary image to data storage as a child of the surface
		node2->SetData((mitk::BaseData*)filter->GetOutput(0));
		node2->SetName(node->GetName());
		node2->SetBoolProperty("binary", true);

		// Add unique identifier for data nodes
		int newId = m_ThreeDEditing->GetUniqueId();
		node2->SetIntProperty("UniqueID", newId);

		m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(node2, node);


		// Collect and display binary images
		int countBinary = 0;
		m_ThreeDEditing->GetSelectSurfaceBox()->clear();
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			bool isBinary = false;
			allNodes->at(i)->GetBoolProperty("binary", isBinary);
			if(isBinary)
			{
				std::string name = allNodes->at(i)->GetName();
				std::cout << "Binary found: " << name << "\n";
				//m_CurrentSurfaces.push_back(name);
				m_ThreeDEditing->GetSelectSurfaceBox()->addItem(name.c_str());
				// set unique name
				allNodes->at(i)->SetIntProperty("UniqueNameBinary", countBinary);
				countBinary++;
			}
		}		
	}

	// Convert binary image to surface
	if(strcmp(node->GetData()->GetNameOfClass(), "Image") == 0)
	{
		std::cout << "Create surfaces from binary segmentations.\n";

		// Use vtkMarchingCubes to create Surface from binary image
		bool isBinary = false;
		node->GetBoolProperty("binary", isBinary);
		if(isBinary)
		{
			bool convert = false;
			if(!node->GetBoolProperty("ModifiedThusConvert", convert))
			{
				// There is no property Convert -> add
				node->SetBoolProperty("ModifiedThusConvert", false);
			}
			if(convert)
			{
				std::cout << "Node: " << node->GetName() << " has been modified. It will be converted.\n";

				// Try ImageToSurfaceFilter
				mitk::ImageToSurfaceFilter::Pointer surfaceCreator = mitk::ImageToSurfaceFilter::New();
				surfaceCreator->SetInput((mitk::Image*)node->GetData());
				surfaceCreator->Update();
				mitk::Surface::Pointer surface = surfaceCreator->GetOutput();
				mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
				surfaceNode->SetData(surface);
				surfaceNode->SetName(node->GetName());

				// Get Parent node (surface to current binary)
				mitk::DataNode* parentToCurrentBinary = NULL;
				int currentId = -1;
				if(!node->GetIntProperty("UniqueID", currentId))
					std::cout << "Data without UniqueID found (1)!\n";
				
				mitk::DataStorage::SetOfObjects::ConstPointer allParentNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
				for(unsigned int j = 0; j < allParentNodes->size(); j++)
				{
					// Get all derivation (childs) of the current node
					mitk::DataStorage::SetOfObjects::ConstPointer allChildToCurrentNode = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetDerivations(allParentNodes->at(j));	

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
				m_ThreeDEditing->GetLungDataset()->getDataStore()->Remove(node);						
				// Remove old parent from data storage (parent) if there is one
				if(!parentToCurrentBinary == NULL)
				{
					std::cout << "Was soll das?\n";
					if(m_ThreeDEditing->GetLungDataset()->getDataStore()->Exists(parentToCurrentBinary))
						m_ThreeDEditing->GetLungDataset()->getDataStore()->Remove(parentToCurrentBinary);
				}

				// Add unique identifier for data nodes
				int newId = m_ThreeDEditing->GetUniqueId();
				surfaceNode->SetIntProperty("UniqueID", newId);
				
				// Add surface to data storage
				m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(surfaceNode);

				// Add binary image as surface's child
				m_ThreeDEditing->GetLungDataset()->getDataStore()->Add(node, surfaceNode);

				mitk::RenderingManager::GetInstance()->RequestUpdateAll();
				surfaceCreator->Delete();

				node->SetBoolProperty("ModifiedThusConvert", false);

				m_ThreeDEditing->GetToolbar()->SetImagesToBottom();
			}	
		}
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}