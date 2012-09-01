#include "settings_common.h"

#include <QObject>
#include <QEvent>
#include <QSettings>
#include <QFileDialog>

SettingsCommonWidget::SettingsCommonWidget(void)
{
	m_title = tr("Allgemein");
	ui_settings_common.setupUi(this);
	(ui_settings_common.txtStandardDICOMPath)->installEventFilter(this);

	// Load and set Settings
	QSettings settings;
	(ui_settings_common.txtStandardDICOMPath)->setText(settings.value(QString("common/StandardDICOMPath")).toString());
}

SettingsCommonWidget::~SettingsCommonWidget(void)
{
}

bool SettingsCommonWidget::eventFilter(QObject* object, QEvent* event) {
	if(object == ui_settings_common.txtStandardDICOMPath && event->type() == QEvent::MouseButtonPress) {
		// Setup FileDialog to "DirectoryMode"
		QFileDialog fileDialog(this); 
		fileDialog.setDirectory( (ui_settings_common.txtStandardDICOMPath)->text() );
		fileDialog.setFileMode(QFileDialog::Directory);
		fileDialog.setOption(QFileDialog::ShowDirsOnly);
		
		QStringList fileNames;
		if (fileDialog.exec()) {
			fileNames = fileDialog.selectedFiles();
			if(fileNames.count() == 1) { // there should only one selected dir
				ui_settings_common.txtStandardDICOMPath->setText(fileNames[0]);
			}
		}
    }
    return false;
}

bool SettingsCommonWidget::tryHide(void)
{
	return true;
}


void SettingsCommonWidget::OnStandardDICOMPathChanged(const QString& newPath)
{
	QSettings settings;
	settings.setValue(QString("common/StandardDICOMPath"), newPath); 
}

