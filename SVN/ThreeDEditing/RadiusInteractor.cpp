#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"

#include "RadiusInteractor.h"
  
/*
 * \brief
 * Constuctor.
 */
RadiusInteractor::RadiusInteractor(const char * type)
   : mitk::Interactor(type,NULL)
{}
 
bool RadiusInteractor::ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent)
{
	bool ok=false;
	int actionId = action->GetActionId();

	switch(actionId)
	{
	case mitk::AcMOVE:
		{
			mitk::IntProperty* plus = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_+"));
			mitk::IntProperty* minus = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_-"));

			//std::cout << "Plus: " << plus->GetValue() << ", Minus: " << minus->GetValue() << "\n";

			if(plus->GetValue() > 0)
			{
				UpdateRadius(+1);
			}
			else if(minus->GetValue() > 0)
			{
				UpdateRadius(-1);
			}

			ok = true;
			break;
		}
	default:
		ok = false;
	break;
	}
	return ok;
}
 
/*
 * \brief
 * Destructor.
 */
RadiusInteractor::~RadiusInteractor()
{
}