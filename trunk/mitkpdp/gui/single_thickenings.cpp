#include "single_thickenings.h"

#include "mainwindow.h"

#include <mitkDicomSeriesReader.h>
#include <mitkGlobalInteraction.h>
#include <mitkRenderingManager.h>

pdp::SingleThickenings::SingleThickenings(QWidget *parent, MainWindow* mainwin, LungDataset data)
    : QWidget(parent)
    , m_mainwin(mainwin)
    , m_data(data)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    ui_single_thickenings.setupUi(this);

    ui_single_thickenings.mitkView->SetDataStorage(m_data.getDataStore());
    m_data.updateGeometry();

    // Initialize the mitk view widget: make the bottom right one be 3D view and fullscreen the top left one.
    ui_single_thickenings.mitkView->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
    ui_single_thickenings.mitkView->GetRenderWindow1()->FullScreenMode(true);

    // Enable standard handler for levelwindow-slider
    ui_single_thickenings.mitkView->EnableStandardLevelWindow();

    // Add the displayed views to the DataStorage to see their positions in 2D and 3D
    ui_single_thickenings.mitkView->AddDisplayPlaneSubTree();
    ui_single_thickenings.mitkView->AddPlanesToDataStorage();
    ui_single_thickenings.mitkView->SetWidgetPlanesVisibility(true);
	//ui_single_thickenings.mitkView->GetRenderWindow1()->GetSliceNavigationController()->GetSlice()->AddObserver

    // Moving the cut-planes to click-point
    ui_single_thickenings.mitkView->EnableNavigationControllerEventListening();

    // Zooming and panning
    mitk::GlobalInteraction::GetInstance()->AddListener(ui_single_thickenings.mitkView->GetMoveAndZoomInteractor());

    QObject::connect(ui_single_thickenings.btn_delete, SIGNAL(clicked()),
                     this, SLOT(onDelete()));
    QObject::connect(ui_single_thickenings.btn_confirm, SIGNAL(clicked()),
                     this, SLOT(onConfirm()));
    QObject::connect(ui_single_thickenings.btn_report, SIGNAL(clicked()),
                     this, SLOT(onCreateReport()));

    // Add my tab to the notebook.
    mainwin->addTab(this);

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::SingleThickenings::~SingleThickenings()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    mitk::GlobalInteraction::GetInstance()->RemoveListener(ui_single_thickenings.mitkView->GetMoveAndZoomInteractor());
    m_data.clearAllButOriginal();

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::SingleThickenings::onDelete()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;


    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::SingleThickenings::onConfirm()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;


    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::SingleThickenings::onCreateReport()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;


    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
