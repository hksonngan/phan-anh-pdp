#ifndef PDP_RENDERINGMANAGERPROPERTY_H
#define PDP_RENDERINGMANAGERPROPERTY_H

#include <mitkPropertyList.h>
#include <QTabWidget>

class QmitkStdMultiWidget;

namespace pdp {

class RenderingManagerProperty: public mitk::BaseProperty
 {
	public:
		RenderingManagerProperty();
		~RenderingManagerProperty();
		bool RenderingManagerProperty::operator ==(const mitk::BaseProperty &) const;
		virtual std::string GetValueAsString() const;

		QmitkStdMultiWidget* GetStdMultiWidget(){return m_MitkView;};
		void SetStdMultiWidget(QmitkStdMultiWidget* view){m_MitkView = view;};
		int GetReinitSurrounding(){return m_ReinitSurrounding;};
		void SetReinitSurrounding(int surrounding){m_ReinitSurrounding = surrounding;};
	private:
		QmitkStdMultiWidget* m_MitkView;
		int m_ReinitSurrounding;
 };

} // namespace pdp

#endif // PDP_RENDERINGMANAGERPROPERTY_H