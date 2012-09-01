#include "single_load.h"

#include "mainwindow.h"
#include "app/thickening_detector.h"
#include "gui/single_thickenings.h"
#include "gui/progress.h"

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QVariant>
#include <QHeaderView>
#include <QProgressDialog>

#include <mitkGlobalInteraction.h>

pdp::SingleLoad::SingleLoad(QWidget *parent, MainWindow* mainwin)
    : QWidget(parent), m_mainwin(mainwin)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    ui_single_load.setupUi(this);

	// Setup Model for TreeView 
	m_itemModel = new QStandardItemModel(this);
	m_itemModel->setHeaderData(0, Qt::Horizontal, tr("DICOM Data"));

	// Items are non-editable by default
	QStandardItem* itemPrototype = new QStandardItem();
	itemPrototype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	m_itemModel->setItemPrototype(itemPrototype);

	ui_single_load.dicom_data_tree->setModel(m_itemModel);

    ui_single_load.mitkView->SetDataStorage(m_data.getDataStore());

    // Initialize the mitk view widget: make the bottom right one be 3D view and fullscreen the top left one.
    ui_single_load.mitkView->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
    ui_single_load.mitkView->GetRenderWindow1()->FullScreenMode(true);

    // Enable standard handler for levelwindow-slider
    ui_single_load.mitkView->EnableStandardLevelWindow();

    // Add the displayed views to the DataStorage to see their positions in 2D and 3D
    ui_single_load.mitkView->AddDisplayPlaneSubTree();
    ui_single_load.mitkView->AddPlanesToDataStorage();
    ui_single_load.mitkView->SetWidgetPlanesVisibility(true);

    // Moving the cut-planes to click-point
    ui_single_load.mitkView->EnableNavigationControllerEventListening();

    // Zooming and panning
    mitk::GlobalInteraction::GetInstance()->AddListener(ui_single_load.mitkView->GetMoveAndZoomInteractor());

    if(!this->onLoadOtherFile())
        throw UserCancelledException();

    QObject::connect(ui_single_load.btn_start, SIGNAL(clicked()),
                     this, SLOT(onDoAnalysis()));
    QObject::connect(ui_single_load.dicom_data_tree, SIGNAL(clicked(const QModelIndex&)),
                     this, SLOT(onDicomTreeClicked(const QModelIndex&)));

    // Add my tab to the notebook.
    mainwin->addTab(this);

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::SingleLoad::~SingleLoad()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;
    mitk::GlobalInteraction::GetInstance()->RemoveListener(ui_single_load.mitkView->GetMoveAndZoomInteractor());
    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

bool pdp::SingleLoad::onLoadOtherFile()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

	QSettings settings;

	// Select DICOM File
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("DICOM Datei wählen"),
		settings.value("common/StandardDICOMPath").toString(),
		tr("DICOM (*.dcm *.dicom DICOMDIR);;Alle (*)"),
		0,
		QFileDialog::ReadOnly
	);

    if(fileName.isEmpty()) {
        return false;
    }

    try {
        // Load the DICOMDIR structure. This doesn't load the images yet.
        m_openedFile = pdp::DicomDir(fileName);

        // If that worked, we can first unload the old data.
        m_data.clear();

		// Map DICOM Metadata to TreeView
		m_openedFile.fillTreeModel(m_itemModel);

    } catch(const std::runtime_error& err) {
        QMessageBox::critical(0, tr("PleuraDatPlus"), err.what());
        return false;
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
    return true;
}

void pdp::SingleLoad::onDicomTreeClicked(const QModelIndex& modelIndex)
{
	int dicomDataRole = m_itemModel->data(modelIndex, pdp::DicomDir::DICOM_DATA_ROLE).toInt();
	
	if(dicomDataRole == pdp::DicomDir::DICOM_SERIES_ROLE) {
		DicomDir::Series* series = static_cast<DicomDir::Series*>(m_itemModel->data(modelIndex, DicomDir::DICOM_SERIES_ROLE).value<void *>());

		// Setup Progress Dialog
		QProgressDialog* progressDialog = pdp::MainWindow::getInstance()->progressDialog();
		progressDialog->setMaximum(100);
		progressDialog->setLabelText(tr("Processing DICOM Dataset"));
		progressDialog->setWindowTitle(tr("Loading DICOM File"));
		progressDialog->reset();

		m_data.clear();
		m_data.loadFromDicom(series, SingleLoad::OnDicomReaderProgress);
		progressDialog->reset();
	}
}

void pdp::SingleLoad::onDoAnalysis()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    m_detector = new ThickeningDetector(m_data, m_data);
    m_progress = new Progress(this);

    QObject::connect(m_detector, SIGNAL(startStep(QString, float)),
                     m_progress, SLOT(onStartStep(QString, float)));
    QObject::connect(m_detector, SIGNAL(stepProgress(float)),
                     m_progress, SLOT(onStepProgress(float)));
    QObject::connect(m_detector, SIGNAL(done()),
                     this, SLOT(onDoneAnalysis()));

    m_progress->show();
    m_detector->run();

	//must update Geometry here
	//contour geometry may be different from Otsu geometry
	//this must be fixed in the future otherwise there will be mixed geometries
	//consequently the view is a mess
	//for the moment, just display one geometry at a time
	//this can be done by deleting the other data nodes
	m_data.getDataStore()->Remove(m_data.getDataStore()->GetNamedNode("Lungs"));
	
	m_data.updateGeometry();

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::SingleLoad::onDoneAnalysis()
{
    delete m_progress;
    delete m_detector;
    new SingleThickenings(this->parentWidget(), m_mainwin, m_data);
}

void pdp::SingleLoad::OnDicomReaderProgress(float progress) {
	pdp::MainWindow::getInstance()->progressDialog()->setValue((int)(progress * 1000));

	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}
