#ifndef PDP_PROGRESS_H
#define PDP_PROGRESS_H

#include "ui_progress.h"

namespace pdp {

class Progress : public QDialog
{
    Q_OBJECT
public:
    Progress(QWidget *parent);
    virtual ~Progress();

public slots:
    void onStartStep(QString stepname, float steppercent);
    void onStepProgress(float totalpercent);
    void onStepDone();

private:
    Ui::Progress ui_progress;

    float m_currentTotalPercentDone;
    float m_currentStepPercent;
};

} // namespace pdp

#endif // PDP_PROGRESS_H
