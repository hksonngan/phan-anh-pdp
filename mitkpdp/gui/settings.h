#pragma once
#include "ui_settings.h"
#include "settings_base.h"
#include <QWidget>
#include <QMap>
#include <QString>

class SettingsDialog :
	public QDialog
{
	Q_OBJECT
public:
	SettingsDialog(void);
	virtual ~SettingsDialog(void);

	/**
	 * Registers a new Settings Page.
	 *
	 * \param [in]				Widget Page, Settings take ownership of the Widget
	 *
	 * \return	true if page was added, else page of same name already exists
	**/
	bool registerPage(SettingsBaseWidget* page);

	/**
	 * Active Settings Page.
	 *
	 * \return	QWidget Active Settings Page
	**/
	const QWidget* activePage(void) const;


protected slots:;
	/**
	 * Called when a Page is selected.
	 *
	 * Trys to hide the activate page. 
	 * Then shows the new one.
	 *
	 * \param [in]	clicked Item in lwCategorys
	**/
	void onShowSettingsPanel(QListWidgetItem*);

	/**
	 * Called when user clicked OK.
	**/
	bool OnAccepted();

	/**
	 * Called when user clicked Cancel.
	**/
	bool OnRejected();

private:
	Ui::Settings ui_settings;

	//! Active Settings Page
	SettingsBaseWidget*	m_activePage;

	//! List of available Settings Pages
	QMap<QString, SettingsBaseWidget*> m_settingPages; // not static, so we can add stuff on runtime easily
};
