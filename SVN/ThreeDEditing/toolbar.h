#ifndef PDP_TOOLBAR_H
#define PDP_TOOLBAR_H

#include <QWidget>
#include <QToolBar>

#include <vtkSmartPointer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAssembly.h>
#include <vtkCornerAnnotation.h>

#include "WheelScrollInteractor.h"

namespace pdp {
	class ThreeDEditing;
	
class Toolbar : public QWidget
{
	Q_OBJECT
	public:
		Toolbar(ThreeDEditing* editing);
		~Toolbar();
		void AddToolbar();
		void RemoveToolbar();
		QToolBar* GetToolbar(){return m_Toolbar;};
		void SetToolManagerToggled(bool toggled){m_ToolManagerToggled = toggled;};
		void SetToolManagerInitialize(bool toggled){m_ToolManagerInitialize = toggled;};
	public slots:
		void OpenFile();
		void SaveSelectedNode();
		void GetCurrentDataStorage();
		void RemoveSelectedNode();
		void SetImagesToBottom();

		void Undo();
		void Redo();

		void ToggleDataManager();
		void ToggleToolManager();

		void ToggleView();
		void ToggleSurfaceColor();
		void ToggleOrientationWidget();
		void ToggleAnnotation();

	private:
		ThreeDEditing* m_ThreeDEditing;
		QToolBar* m_Toolbar;
		bool m_DataManagerToggled;
		bool m_ToolManagerToggled;
		bool m_ToolManagerInitialize;
		bool m_OneColor;
		vtkSmartPointer<vtkOrientationMarkerWidget> m_Widget;
		int m_ToggleOrientationWidget;
		bool m_InitOrientationWidget;
		vtkSmartPointer<vtkAxesActor> m_Axes;
		vtkSmartPointer<vtkAnnotatedCubeActor> m_AnnotatedCube;
		bool m_ShowAnnotation;
		vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation;
		WheelScrollInteractor* m_WheelInteractor;
};

} // namespace pdp

#endif // PDP_TOOLBAR_H