#ifndef WHEELSCROLLINTERACTOR_H
#define WHEELSCROLLINTERACTOR_H
 
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkStateMachine.h"

#include "QmitkStdMultiWidget.h" 

#include <vtkSmartPointer.h>
#include <vtkCornerAnnotation.h>

class WheelScrollInteractor : public mitk::Interactor
{
	public:
		mitkClassMacro(WheelScrollInteractor, Interactor);
		mitkNewMacro1Param(Self, const char*);
		virtual void ExecuteOperation(mitk::Operation* operation);
		WheelScrollInteractor(const char * type);
		void HideAnnotations();
		void ShowAnnotations();
	protected:  
		virtual ~WheelScrollInteractor();
		virtual bool ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent);
	private:
		vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation1;
		vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation2;
		vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation3;

		QmitkStdMultiWidget* m_MitkView; 
};
 
 #endif 