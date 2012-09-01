#ifndef PDP_DATAMANAGER_H
#define PDP_DATAMANAGER_H

#include <QWidget>

class QmitkDataStorageTreeModel;
class QTreeView;
class QMenu;
class QAction;
class QSlider;
class QWidgetAction;
class QPushButton;

namespace pdp {
	class MainWindow;
	class ThreeDEditing;

class DataManager : public QWidget
{
Q_OBJECT
public:
		DataManager(ThreeDEditing* editing);
		~DataManager();

		QTreeView* GetTreeView(){return m_NodeTreeView;};
		QmitkDataStorageTreeModel* GetTreeModel(){return m_NodeTreeModel;};
		QMenu* GetMenu(){return m_NodeMenu;};
		void UpdateTreeModel();

public slots:
		void OpenContextMenu(const QPoint&);
		void ReinitSelectedNodes(bool checked);
		void VolumeRenderingMenuAboutToShow();
		void VolumeRenderingToggled(bool checked);
		void CreatePolygonModelMenuAboutToShow();
		void CreatePolygonModelActionToggled(bool checked);
		void SurfaceRepresentationMenuAboutToShow();
		void SurfaceRepresentationActionToggled(bool checked);
		void ReduceContourSet();
		void OpacityChanged(int value);
	    void OpacityActionChanged();
		void DataNodeProperties();
		void ColorChanged();
		void ColorActionChanged();
		void ConvertSelectedNodes(bool);

private:
	ThreeDEditing* m_ThreeDEditing;

	QTreeView* m_NodeTreeView;
	QmitkDataStorageTreeModel* m_NodeTreeModel;
	QMenu* m_NodeMenu;

	QAction* m_SurfaceRepresentation;
	QAction* m_ReinitAction;
	QAction* m_ToggleVolumeRenderingAction;
	QAction* m_CreatePolygonModelAction;
	QAction* m_ReduceContourSetAction;
	QAction* m_DataNodePropertiesAction;
	QSlider* m_OpacitySlider;
	QWidgetAction* m_OpacityAction;
	QPushButton* m_ColorButton;
	QWidgetAction* m_ColorAction;
	QAction* m_ConvertAction;

	std::string m_RenderingMode;
};

} // namespace pdp

#endif // PDP_DATAMANAGER_H