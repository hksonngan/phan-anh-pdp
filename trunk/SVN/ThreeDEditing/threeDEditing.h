#ifndef PDP_THREEDEDITING_H
#define PDP_THREEDEDITING_H

#include <QWidget>
#include <QIcon>
#include <QToolBar>
#include <mitkDisplayVectorInteractorScroll.h>

#include "../../mitkpdp/app/lung_dataset.h"
#include "timerThread.h"
#include "MyTimer.h"
#include "QThreadEx.h"

class QmitkDataStorageTreeModel;
class QmitkStdMultiWidget;
class QTreeView;
class QMenu;
class QHBoxLayout;
class QmitkToolSelectionBox;
class QmitkToolGUIArea;

namespace pdp {
	class MainWindow;
	class Toolbar;
	class DataManager;

class ThreeDEditing : public QWidget
{
Q_OBJECT
public:
	ThreeDEditing(MainWindow* mainwin);
    ~ThreeDEditing();

	MainWindow* GetMainWindow(){return m_MainWindow;};
	LungDataset* GetLungDataset(){return m_Data;};
	DataManager* GetDataManager(){return m_DataManager;};
	Toolbar* GetToolbar(){return m_Toolbar;};
	QmitkStdMultiWidget* GetMultiWidget(){return m_MitkView;};
	QHBoxLayout* GetLayout(){return m_Layout;};
	QmitkToolSelectionBox* GetManualToolSelectionBox(){return m_ManualToolSelectionBox;};
	QmitkToolGUIArea* GetManualToolGUIContainer(){return m_ManualToolGUIContainer;};
	QTabWidget* GetToolTabs(){return m_Tabs;};

	bool GetUseTriangle(){return m_UseTriangle;};
	bool GetUseClean(){return m_UseClean;};
	bool GetUseSmooth(){return m_UseSmooth;};
	QComboBox* GetSelectSurfaceBox(){return m_SelectSurfaceBox;};

	void AutoLoadFile();
	void CatchChangedNodesEvents(const mitk::DataNode*);
	void ActiveToolChangedChangeTabs();
	int GetUniqueId();

	QLineEdit* m_Time;
	bool m_Play;
	QToolButton* m_Btn_StartSegmentation;
	QTimer* m_Timer;

signals:

public slots:
	void Run();
	void SetUpMitkView();
	void AddTools();
	void PrintDataStorage();
	void CreateSurface();
	void UseMITKSegmentationTools();

	void TakeScreenshot();
	void FirstScenario();
	void SecondScenario();
	void CalculateVolume();
	void CurrentVolume();
	void StartSegmentation();
	void TimerEvent();

	void CurrentTab(int);
	void SelectBinaryImage(int);
	void CreateEmptySegmentation();
	void ReferenceImage(int);
	void ReferenceImageWithNewSegmentation(int);
	void ConvertData();
	void UseTriangle();
	void UseClean();
	void UseSmooth();
	void ChangeReinitSurrounding(int);

	QIcon windowIcon();
	QString windowTitle();

protected:
	void SetUpGui();

private:
	MainWindow* m_MainWindow;
	Toolbar* m_Toolbar;
	DataManager* m_DataManager;
	LungDataset* m_Data;
	QmitkStdMultiWidget* m_MitkView;
	std::vector<mitk::ColorProperty::Pointer> m_Colors;
	int m_LastColorIndex;
	QHBoxLayout* m_Layout;
	QVBoxLayout* m_Page1Layout;
	QVBoxLayout* m_Page2Layout;
	QVBoxLayout* m_Page3Layout;
	QHBoxLayout* m_SelectReinitSurroundingLayout;
	QTabWidget* m_Tabs;
	QmitkToolSelectionBox* m_ManualToolSelectionBox;
	QmitkToolGUIArea* m_ManualToolGUIContainer;
	QBoxLayout* m_SelectSurfaceLayout;
	QBoxLayout* m_SelectSurfaceLayout1;
	QBoxLayout* m_SelectSurfaceLayout2;
	QComboBox* m_SelectSurfaceBox;
	QPushButton* m_NewSurfaceButton;
	bool m_ConvertSegmentationData;
	QLabel* m_WorkingDatasetLabel;
	std::vector<std::string> m_CurrentSurfaces;		// brauche ich die noch??
	QBoxLayout* m_SelectReferenceImageLayout;
	QComboBox* m_SelectReferenceImageBox;
	QComboBox* m_SelectReferenceImageBoxWithNewSegmentation;
	std::vector<std::string> m_CurrentReferenceImages; // brauche ich die noch??
	bool m_HasWorkingDataSet;
	bool m_HasReferenceDataSet;
	bool m_UseTriangle;
	bool m_UseClean;
	bool m_UseSmooth;
	mitk::DisplayVectorInteractorScroll::Pointer m_Scroll;

	int m_ElapsedTime;
	long m_StartSeconds;

	int m_DurationOfTest;
	int m_SaveDataEveryXSeconds;
};

} // namespace pdp

#endif // PDP_THREEDEDITING_H