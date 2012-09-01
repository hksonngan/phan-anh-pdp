#pragma once

#include "lung_dataset.h"
#include <QObject>

namespace pdp {

/**
 * Manages Workflow/StateMachine and all processing data and results.
 *
 *
**/
class ThickeningDetector : public QObject
{
	Q_OBJECT
public:
    ThickeningDetector(LungDataset& input, LungDataset& output);

    void run();
    void exit();
 
signals:
    void startStep(QString stepName, float stepPercent);
    void stepProgress(float totalpercent);
    void stepDone();
    void done();

protected slots:
    void onStepProgress(float totalpercent);

private:
    LungDataset& m_input;
    LungDataset& m_output;
};

} // namespace pdp

