#ifndef PDP_SINGLE_THICKENINGS_H
#define PDP_SINGLE_THICKENINGS_H

#include "ui_single_thickenings.h"

#include "app/lung_dataset.h"

namespace pdp {
    class MainWindow;

class SingleThickenings : public QWidget
{
    Q_OBJECT
public:
    SingleThickenings(QWidget *parent, MainWindow* mainwin, LungDataset data);
    virtual ~SingleThickenings();

public slots:
    void onDelete();
    void onConfirm();
    void onCreateReport();

private:
    MainWindow* m_mainwin;
    Ui::SingleThickenings ui_single_thickenings;

    LungDataset m_data;
};

} // namespace pdp

#endif // PDP_SINGLE_THICKENINGS_H
