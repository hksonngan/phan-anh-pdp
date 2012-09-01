#ifndef PDP_HOME_H
#define PDP_HOME_H

#include "ui_home.h"

namespace pdp {
    class MainWindow;

class Home : public QWidget
{
    Q_OBJECT
public:
    Home(QWidget *parent, MainWindow* mainwin);
    virtual ~Home();

public slots:
    void onEnterSingleAnalysis();
    void onEnterMultiAnalysis();

protected:
    void resizeEvent(QResizeEvent*);

private:
    MainWindow* m_mainwin;
    Ui::HomeFrame ui_home;
};

} // namespace pdp

#endif // PDP_HOME_H
