#pragma once
#include <QWidget>
#include <QString>

/** 
* Base for Settings Widgets.
* 
* Use this as a base class if you want to add a Page to the Settings Dialog.
* 
* \note QWidget::windowIcon and QWidget::windowTitle are used for markup as well -> fill them!
* You will have to implement tryHide() (place saving logic there).  
* 
* Use SettingsDialog::registerPage() to register your Page.
**/
class SettingsBaseWidget: public QWidget
{
	Q_OBJECT
public:
	SettingsBaseWidget(void);
	virtual ~SettingsBaseWidget(void);

	/**
	 * Returns Menu Entry/Title.
	 *
	 * \return	title
	**/
	const QString& title() const;

	/**
	 * Executed before on Setting Page is hided.
	 *
	 * Use this to save your stuff. You can avoid hiding when you return false.
	 * 
	 * \return	true if allowed to hide, false if page has to do more stuff.
	**/
	virtual bool tryHide() = 0;
protected:

	//! Used as title for the menu entry
	QString m_title;
};
