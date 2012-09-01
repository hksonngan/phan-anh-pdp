
template<class UiClass>
void pdp::MainWindow::addTab(UiClass* ui)
{
    int tabIdx = this->ui_main.tabWidget->addTab(ui, ui->windowIcon(), ui->windowTitle());
    this->ui_main.tabWidget->setCurrentIndex(tabIdx);
    this->ui_main.tabWidget->setTabEnabled(tabIdx-1, false);
    m_tabs.push(ui);

    this->ui_main.tabWidget->cornerWidget()->setEnabled(m_tabs.size() > 1);
}
