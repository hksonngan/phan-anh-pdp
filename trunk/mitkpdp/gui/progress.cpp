#include "progress.h"

#include <iostream>

pdp::Progress::Progress(QWidget *parent)
    : QDialog(parent)
    , m_currentTotalPercentDone(0.0f)
    , m_currentStepPercent(0.0f)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    ui_progress.setupUi(this);

    // To remove the close X button, this was not available in QtDesigner.
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::Progress::~Progress()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Progress::onStartStep(QString stepname, float steppercent)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    std::cout << "starting step " << stepname.toStdString() << " which is about " << steppercent*100.0f << "% of total progress." << std::endl;

    // Reset the "step" progressbar to 0.
    ui_progress.bar_step->setValue(0);
    ui_progress.lbl_step_name->setText(stepname);

    // Somehow the text doesn't update itself while the progressbar does. Force it.
    ui_progress.lbl_step_name->update();
    qApp->processEvents();

    m_currentTotalPercentDone = static_cast<float>(ui_progress.bar_total->value())/static_cast<float>(ui_progress.bar_total->maximum());
    m_currentStepPercent = steppercent;

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Progress::onStepProgress(float totalsteppercent)
{
    //std::cout << "hi from " << __FUNCSIG__ << std::endl;

    std::cout << "progressing to " << totalsteppercent*100.0f << "% in current step." << std::endl;

    // Update the "step" progressbar.
    ui_progress.bar_step->setValue(static_cast<int>(totalsteppercent*static_cast<float>(ui_progress.bar_total->maximum())));

    // Update the "total" progressbar.
    float currtotal = m_currentTotalPercentDone + totalsteppercent*m_currentStepPercent;
    ui_progress.bar_total->setValue(static_cast<int>(currtotal*static_cast<float>(ui_progress.bar_total->maximum())));

    //std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Progress::onStepDone()
{
    // Update the "step" progressbar.
    ui_progress.bar_step->setValue(ui_progress.bar_total->maximum());
}
