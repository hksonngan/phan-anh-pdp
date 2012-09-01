#include "settings.h"

#include <QWidget>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(void)
{
	m_activePage = NULL;
	ui_settings.setupUi(this);
	ui_settings.centralWidget->setLayout(new QHBoxLayout());
}

SettingsDialog::~SettingsDialog(void)
{
}

void SettingsDialog::onShowSettingsPanel(QListWidgetItem* lwiActivatedPanel)
{
	if(m_activePage)
		if(!m_activePage->tryHide()) // If page dont want to be hidden, do nothing
			return;

	SettingsBaseWidget* showWidget = m_settingPages[lwiActivatedPanel->text()];

	ui_settings.lblTitle->setText( showWidget->windowTitle() );

	ui_settings.centralWidget->layout()->addWidget( showWidget );
	showWidget->show();

	m_activePage = showWidget;
}

bool SettingsDialog::registerPage(SettingsBaseWidget* page)
{
	// Check if item exists
	// QMap<T,X>::find returns QMap<T,X>::end if key doesnt exists ..
	if(m_settingPages.find(page->title()) != m_settingPages.end())
		return false;

	// Take .. eh .. remove ownership 
	page->setParent( NULL );

	m_settingPages[page->title()] = page;
	
	ui_settings.lwCategorys->addItem(new QListWidgetItem(page->windowIcon(), page->title()));

	return true;
}


const QWidget* SettingsDialog::activePage(void) const
{
	return m_activePage;
}


bool SettingsDialog::OnAccepted(void)
{
	if(m_activePage)
		if(!m_activePage->tryHide())
			return false;

	accept();
	return true;
	
}

bool SettingsDialog::OnRejected(void)
{
	reject();
	return true;
}

