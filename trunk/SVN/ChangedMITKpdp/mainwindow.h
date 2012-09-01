#ifndef PDP_MAINWINDOW_H
#define PDP_MAINWINDOW_H

#include "ui_mainwindow.h"
#include "settings.h"
#include "../../SVN/ThreeDEditing/threeDEditing.h"

#include <stack>
#include <QProgressDialog>

namespace pdp {

class UserCancelledException {};

/**
 * Form for viewing the main. 
**/

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent);
    virtual ~MainWindow();

	static MainWindow* getInstance() {
		if(MainWindow::m_singleton == NULL)
			m_singleton = new MainWindow(NULL);
		return m_singleton;
	};

	std::stack<QWidget*>* GetTabs(){return &m_tabs;};

	QProgressDialog* progressDialog() const { return m_progressDialog; };

    template<class UiClass> void addTab(UiClass* ui);

	int* GetNumberOfInstancesOfThreeDEditing();

public slots:
    void onBack();
	void onShowSettings();
	void CreateThreeDEditing();
private:
    Ui::MainWindow ui_main;

    std::stack<QWidget*> m_tabs;

	static MainWindow* m_singleton;

	SettingsDialog* m_settingsDialog;

	ThreeDEditing* m_ThreeDEditing;
	int m_NumberOfInstancesOfThreeDEditing;

	QProgressDialog* m_progressDialog;

	void setupUi();
};

// Some util to be put in some util.h soon.
static const char kBlankChars[] = " \t\n\r";

/// Returns a string with leading/trailing characters of a set stripped
std::string trimmed(const std::string& str, const char* sepSet=kBlankChars);

} // namespace pdp

#include "mainwindow.txx"

#endif // PDP_MAINWINDOW_H
