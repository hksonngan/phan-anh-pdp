#include "gui/mainwindow.h"

#include <QmitkRegisterClasses.h>
#include <QApplication>

//hide console
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char* argv[])
{
    QApplication qtapplication( argc, argv );

	QCoreApplication::setOrganizationName("RWTH Aachen");
    QCoreApplication::setOrganizationDomain("lfb.rwth-aachen.de");
    QCoreApplication::setApplicationName("PDPlus");

	// QTextCodec::setCodecForTr(QTextCodec::codecForName("eucKR"));

    // Register Qmitk-dependent global instances
    QmitkRegisterClasses();

	pdp::MainWindow* mainWindow = pdp::MainWindow::getInstance();
	mainWindow->show();

    return qtapplication.exec();
}
