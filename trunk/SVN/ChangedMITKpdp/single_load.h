#pragma once

#include "ui_single_load.h"

#include "app/dicom_dir.h"
#include "app/lung_dataset.h"

class QStandardItemModel;

namespace pdp {
    class MainWindow;
    class ThickeningDetector;
    class Progress;

class SingleLoad : public QWidget
{
    Q_OBJECT
public:
    SingleLoad(QWidget *parent, MainWindow* mainwin);
    virtual ~SingleLoad();
	LungDataset* GetDataSet(){return &m_data;};

public slots:
    bool onLoadOtherFile();
    void onDoAnalysis();
    void onDoneAnalysis();
    void onDicomTreeClicked(const QModelIndex&);


private:
    MainWindow* m_mainwin;
    Ui::SingleLoad ui_single_load;

	QStandardItemModel* m_itemModel;

    DicomDir m_openedFile;
    LungDataset m_data;
    ThickeningDetector* m_detector;
    Progress* m_progress;

	
	/**
	* Implements Callback for mitk::DicomSeriesReader::LoadDicomSeries.	
	*
	* Shows Progess of DicomSeriesReader::LoadDicomSeries in a nice ProgressBar. 
	*
	* \param progress is in the intervall [0,1]
	*/
	static void OnDicomReaderProgress(float progress);

};

} // namespace pdp