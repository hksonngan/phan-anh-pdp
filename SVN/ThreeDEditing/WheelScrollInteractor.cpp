#include "mitkInteractionConst.h"
//#include "QmitkStdMultiWidget.h" 
#include "mitkDisplayPositionEvent.h"
#include "mitkStateEvent.h"

#include <vtkRendererCollection.h>

#include "RenderingManagerProperty.h"
#include "WheelScrollInteractor.h"
  
/*
 * \brief
 * Constuctor.
 */
WheelScrollInteractor::WheelScrollInteractor(const char * type)
   : mitk::Interactor(type,NULL)
{
	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	m_MitkView = properties->GetStdMultiWidget();

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d1 = m_MitkView->GetRenderWindow1()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d1 = rendererCollection2d1->GetFirstRenderer();
	cornerAnnotation1 = vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation1->SetLinearFontScaleFactor( 2 );
	cornerAnnotation1->SetNonlinearFontScaleFactor( 1 );
	cornerAnnotation1->SetMaximumFontSize( 20 );
	cornerAnnotation1->GetTextProperty()->SetColor( 1,0,0);
	//renderer2d1->AddVolume(cornerAnnotation1);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d2 = m_MitkView->GetRenderWindow2()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d2 = rendererCollection2d2->GetFirstRenderer();
	cornerAnnotation2 = vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation2->SetLinearFontScaleFactor( 2 );
	cornerAnnotation2->SetNonlinearFontScaleFactor( 1 );
	cornerAnnotation2->SetMaximumFontSize( 20 );
	cornerAnnotation2->GetTextProperty()->SetColor( 1,0,0);
	//renderer2d2->AddVolume(cornerAnnotation2);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d3 = m_MitkView->GetRenderWindow3()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d3 = rendererCollection2d3->GetFirstRenderer();
	cornerAnnotation3 = vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation3->SetLinearFontScaleFactor( 2 );
	cornerAnnotation3->SetNonlinearFontScaleFactor( 1 );
	cornerAnnotation3->SetMaximumFontSize( 20 );
	cornerAnnotation3->GetTextProperty()->SetColor( 1,0,0);
	//renderer2d3->AddVolume(cornerAnnotation3);
}

void WheelScrollInteractor::ExecuteOperation(mitk::Operation* itkNotUsed( operation ) )
{}
 
bool WheelScrollInteractor::ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent)
{
	bool ok=false;
 
	int actionId = action->GetActionId();

	const mitk::DisplayPositionEvent* posEvent=dynamic_cast<const mitk::DisplayPositionEvent*>(stateEvent->GetEvent());
	if(posEvent==NULL) return false;
	mitk::BaseRenderer::Pointer m_Sender = posEvent->GetSender();

	switch(actionId)
	{
	case mitk::AcWHEEL:
		{
			int currentSlice =  m_MitkView->GetRenderWindow1()->GetSliceNavigationController()->GetSlice()->GetPos();
			char ctext1[30];
			_itoa_s(currentSlice, ctext1, 10);
			std::string text1 = "Slice: ";
			text1 += ctext1;
			cornerAnnotation1->ClearAllTexts();
			cornerAnnotation1->SetText( 2, text1.c_str() );

			currentSlice =  m_MitkView->GetRenderWindow2()->GetSliceNavigationController()->GetSlice()->GetPos();
			char ctext2[30];
			_itoa_s(currentSlice, ctext2, 10);
			std::string text2 = "Slice: ";
			text2 += ctext2;
			cornerAnnotation2->ClearAllTexts();
			cornerAnnotation2->SetText( 2, text2.c_str() );

			currentSlice =  m_MitkView->GetRenderWindow3()->GetSliceNavigationController()->GetSlice()->GetPos();
			char ctext3[30];
			_itoa_s(currentSlice, ctext3, 10);
			std::string text3 = "Slice: ";
			text3 += ctext3;
			cornerAnnotation2->ClearAllTexts();
			cornerAnnotation2->SetText( 2, text3.c_str() );

			mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

			ok = true;
			break;
		}
	default:
		ok = false;
	break;
	}
	return ok;
}

void WheelScrollInteractor::HideAnnotations()
{
	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	m_MitkView = properties->GetStdMultiWidget();

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d1 = m_MitkView->GetRenderWindow1()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d1 = rendererCollection2d1->GetFirstRenderer();
	renderer2d1->RemoveVolume(cornerAnnotation1);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d2 = m_MitkView->GetRenderWindow2()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d2 = rendererCollection2d2->GetFirstRenderer();
	renderer2d2->RemoveVolume(cornerAnnotation2);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d3 = m_MitkView->GetRenderWindow3()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d3 = rendererCollection2d3->GetFirstRenderer();
	renderer2d3->RemoveVolume(cornerAnnotation3);

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void WheelScrollInteractor::ShowAnnotations()
{
	int currentSlice =  m_MitkView->GetRenderWindow1()->GetSliceNavigationController()->GetSlice()->GetPos();
	char ctext1[30];
	_itoa_s(currentSlice, ctext1, 10);
	std::string text1 = "Slice: ";
	text1 += ctext1;
	cornerAnnotation1->ClearAllTexts();
	cornerAnnotation1->SetText( 2, text1.c_str() );

	currentSlice =  m_MitkView->GetRenderWindow2()->GetSliceNavigationController()->GetSlice()->GetPos();
	char ctext2[30];
	_itoa_s(currentSlice, ctext2, 10);
	std::string text2 = "Slice: ";
	text2 += ctext2;
	cornerAnnotation2->ClearAllTexts();
	cornerAnnotation2->SetText( 2, text2.c_str() );

	currentSlice =  m_MitkView->GetRenderWindow3()->GetSliceNavigationController()->GetSlice()->GetPos();
	char ctext3[30];
	_itoa_s(currentSlice, ctext3, 10);
	std::string text3 = "Slice: ";
	text3 += ctext3;
	cornerAnnotation2->ClearAllTexts();
	cornerAnnotation2->SetText( 2, text3.c_str() );

	const char* propertyKey = "Properties"; 
	pdp::RenderingManagerProperty* properties = (pdp::RenderingManagerProperty*)mitk::RenderingManager::GetInstance()->GetProperty(propertyKey);
	m_MitkView = properties->GetStdMultiWidget();

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d1 = m_MitkView->GetRenderWindow1()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d1 = rendererCollection2d1->GetFirstRenderer();
	renderer2d1->AddVolume(cornerAnnotation1);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d2 = m_MitkView->GetRenderWindow2()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d2 = rendererCollection2d2->GetFirstRenderer();
	renderer2d2->AddVolume(cornerAnnotation2);

	vtkSmartPointer<vtkRendererCollection> rendererCollection2d3 = m_MitkView->GetRenderWindow3()->GetRenderWindow()->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer2d3 = rendererCollection2d3->GetFirstRenderer();
	renderer2d3->AddVolume(cornerAnnotation3);

	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

/*
 * \brief
 * Destructor.
 */
WheelScrollInteractor::~WheelScrollInteractor()
{
}