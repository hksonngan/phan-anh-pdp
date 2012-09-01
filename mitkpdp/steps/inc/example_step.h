#pragma once

#include "app/thickening_detection_step_base.h"

#include <QString>

namespace pdp {

class ExampleStep : public ThickeningDetectionStep {
public:
    ExampleStep(QString info);

    virtual QString name() const;
    virtual float relativePercent() const;
    virtual void work(LungDataset& input, LungDataset& output);

private:
    QString info;
};

} // namespace pdp
