#include "steps/inc/example_step.h"

pdp::ExampleStep::ExampleStep(QString info)
    : info(info)
{ }

QString pdp::ExampleStep::name() const
{
    return "Example step: " + info;
}

float pdp::ExampleStep::relativePercent() const
{
    return 1.0;
}

void pdp::ExampleStep::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the example step!" << std::endl;
    const DWORD pause = 1000;
    Sleep(pause);
    emit stepProgress(0.33f);
    Sleep(pause);
    emit stepProgress(0.66f);
    Sleep(pause);
    emit stepProgress(1.00f);
}
