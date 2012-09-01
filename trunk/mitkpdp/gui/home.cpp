#include "home.h"

#include "single_load.h"
#include "mainwindow.h"

pdp::Home::Home(QWidget *parent, MainWindow* mainwin)
    : QWidget(parent)
    , m_mainwin(mainwin)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    ui_home.setupUi(this);

    QObject::connect(ui_home.btn_Single, SIGNAL(clicked()),
                     this, SLOT(onEnterSingleAnalysis()));
    QObject::connect(ui_home.btn_Multi, SIGNAL(clicked()),
                     this, SLOT(onEnterMultiAnalysis()));

    // Add my tab to the notebook.
    mainwin->addTab(this);

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::Home::~Home()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;
}

void pdp::Home::onEnterSingleAnalysis()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    try {
        new SingleLoad(this->parentWidget(), m_mainwin);
    } catch(const UserCancelledException&) {
        // When the user cancelled, m_singleLoad is still 0.
        // We don't want to do anything (stay in this step).
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Home::onEnterMultiAnalysis()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    // TODO!
    // Add the multi load tab only if not opened yet.
    //if(!m_multiLoad) {
        //m_multiLoad = new MultiLoad(this->parentWidget(), m_mainwin);
    //}

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::Home::resizeEvent(QResizeEvent*)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    // Rescale the two images in the home tab.
    QPixmap origMulti(QString::fromUtf8(":/home/res/home/preview_multi.png"));
    QPixmap origSingl(QString::fromUtf8(":/home/res/home/preview_single.png"));
    QSize scaledSizeMulti = origMulti.size();
    QSize scaledSizeSingl = origSingl.size();
    scaledSizeMulti.scale(ui_home.img_Multi->size(), Qt::KeepAspectRatio);
    scaledSizeSingl.scale(ui_home.img_Single->size(), Qt::KeepAspectRatio);

    std::cout << "scaledSizeMulti: " << scaledSizeMulti.width() << "x" << scaledSizeMulti.height() << std::endl;
    std::cout << "origMulti.size(): " << origMulti.size().width() << "x" << origMulti.size().height() << std::endl;

    // We don't want to scale up, only down.
    if(scaledSizeMulti.width() < origMulti.size().width()) {
        ui_home.img_Multi->setPixmap(origMulti.scaled(ui_home.img_Multi->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if(scaledSizeSingl.width() < origSingl.size().width()) {
        ui_home.img_Single->setPixmap(origSingl.scaled(ui_home.img_Single->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
