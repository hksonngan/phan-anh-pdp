#include "mainwindow.h"
#include "home.h"
#include "settings.h"
#include "settings_common.h"

#include "../../../SVN/ThreeDEditing/global.h"

#include <iostream>
#include <QProgressDialog>

pdp::MainWindow* pdp::MainWindow::m_singleton = NULL;

pdp::MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_settingsDialog = new SettingsDialog;

	m_settingsDialog->registerPage(new SettingsCommonWidget());

    ui_main.setupUi(this);
	this->setupUi();

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


void pdp::MainWindow::setupUi() {
	// Add back button
	QPushButton* back = new QPushButton(QIcon(":/common/res/common/back.png"), QApplication::translate("MainWindow", "Zur\303\274ck", 0, QApplication::UnicodeUTF8));
    ui_main.tabWidget->setCornerWidget(back, Qt::TopRightCorner);
    QObject::connect(back, SIGNAL(clicked()), this, SLOT(onBack()));
	
    // Add the "start" tab.
    new pdp::Home(this->ui_main.tabWidget, this);

	// Creates progressDialog
	m_progressDialog = new QProgressDialog("", QString(), 0, 100, this);
	m_progressDialog->setMinimumDuration(0);
	//m_progressDialog->setWindowModality(Qt::WindowModal);

	// Correction of automatic segmentation via 3d interactive segmentation	
	m_NumberOfInstancesOfThreeDEditing = 0;
	QMenu *menuWerkzeug;
	menuWerkzeug = new QMenu(ui_main.menubar);
    menuWerkzeug->setObjectName(QString::fromUtf8("menuWerkzeug"));
	menuWerkzeug->setTitle(QApplication::translate("MainWindow", "Werkzeug", 0, QApplication::UnicodeUTF8));
	ui_main.menubar->addMenu(menuWerkzeug);
    
	QAction *actionThreeDEditing = new QAction(this);
	actionThreeDEditing->setObjectName(QString::fromUtf8("actionThreeDEditing"));
	actionThreeDEditing->setIconText("3DEditing");
	QIcon icn_menu;
	icn_menu.addFile(":/threeDEditing/res/threeDEditing/Rubber-32.png");
	actionThreeDEditing->setIcon(icn_menu);
	menuWerkzeug->addAction(actionThreeDEditing);

	QObject::connect(actionThreeDEditing, SIGNAL(triggered()), this, SLOT(CreateThreeDEditing()));
	
	// AutoRun
	if(AUTO_IMPORT == 1)
		CreateThreeDEditing();
}

void pdp::MainWindow::CreateThreeDEditing()
{
	if(m_NumberOfInstancesOfThreeDEditing == 0)
	{
		m_ThreeDEditing = new ThreeDEditing(this);
		m_ThreeDEditing->Run();
		m_NumberOfInstancesOfThreeDEditing++;
	}
	else
	{
		std::cout << "3D Editing is already running.\n";
	}
}
int* pdp::MainWindow::GetNumberOfInstancesOfThreeDEditing()
{
	return &m_NumberOfInstancesOfThreeDEditing;
}

void pdp::MainWindow::onShowSettings() {
	m_settingsDialog->show();
}

pdp::MainWindow::~MainWindow()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    while(!m_tabs.empty()) {
        delete m_tabs.top();
        m_tabs.pop();
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::MainWindow::onBack()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    if(m_tabs.size() > 1) {
        ui_main.tabWidget->removeTab(m_tabs.size()-1);
        delete m_tabs.top();
        m_tabs.pop();
        ui_main.tabWidget->setTabEnabled(m_tabs.size()-1, true);
    }

    ui_main.tabWidget->cornerWidget()->setEnabled(m_tabs.size() > 1);

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

std::string pdp::trimmed(const std::string& str, const char* sepSet)
{
    std::string::size_type const first = str.find_first_not_of(sepSet);
    return first != std::string::npos ? str.substr(first, str.find_last_not_of(sepSet)-first+1) : std::string();
}
