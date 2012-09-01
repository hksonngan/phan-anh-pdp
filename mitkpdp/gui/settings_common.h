#pragma once
#include "ui_settings_common.h"
#include "settings_base.h"

class QObject;
class QEvent;

/**
 * Common Settings Page.
**/
class SettingsCommonWidget: public SettingsBaseWidget
{
	Q_OBJECT
public:
	SettingsCommonWidget(void);
	virtual ~SettingsCommonWidget(void);

	virtual bool tryHide(void);

protected slots:;
	/**
	 * Saves new DICOM Standard Path.
	 *
	 * \param	newPath	Full new standard path.
	**/
	void OnStandardDICOMPathChanged(const QString& newPath);

protected:

	/**
	 * Event Filter to intercept Mouse Clicks on txtStandardDICOMPath.
	 *
	 * There is no clicked() signal for QLineEdit's, so we intercept it here 
	 * and show a Dialog to select a dir.
	 *
	 * \return	false, always
	**/
	bool eventFilter(QObject* object, QEvent* event);

private:
	Ui::SettingsCommon ui_settings_common;
};
