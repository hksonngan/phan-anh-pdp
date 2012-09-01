/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (vie, 12 sep 2008) $
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef radiusInteractor_h_Included
#define radiusInteractor_h_Included

#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkStateMachine.h"
#include <mitkGlobalInteraction.h>

class RadiusInteractor : public mitk::Interactor
{
	public:
		mitk::Message1<int> UpdateRadius;

		mitkClassMacro(RadiusInteractor, Interactor);
		mitkNewMacro1Param(Self, const char*);
		RadiusInteractor(const char * type);
	protected:
		~RadiusInteractor();
		virtual bool ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent);
	private:
};


#endif


