/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-14 09:48:51 +0200 (Di, 14 Sep 2010) $
Version:   $Revision: 26074 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSelectSurfaceWithMouseInteractor.h"
#include "mitkToolExtension.h"
#include "mitkSurface.h"
#include "mitkInteractionConst.h"
#include <mitkDataNode.h>
#include "mitkDisplayPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkProperties.h"

//for an temporary update
#include "mitkRenderingManager.h"

//## Default Constructor
mitk::SelectSurfaceWithMouseInteractor
::SelectSurfaceWithMouseInteractor(const char * type, DataNode* dataNode, mitk::ToolExtension* ext)
:Interactor(type, dataNode)
{
	m_Ext = ext;
}

mitk::SelectSurfaceWithMouseInteractor::~SelectSurfaceWithMouseInteractor()
{
}

bool mitk::SelectSurfaceWithMouseInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
	bool ok = false;

	/*Each case must watch the type of the event!*/
	switch (action->GetActionId())
	{
		case AcDONOTHING:
			ok = true;
			break;
		case AcCHECKELEMENT: 
			/*
			* picking: Answer the question if the given position within stateEvent is close enough to select an object
			* send yes if close enough and no if not picked
			*/
			{
				mitk::DisplayPositionEvent const *posEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
				if (posEvent == NULL)
				{
					MITK_WARN<<"Wrong usage of mitkMoveSurfaceInteractor! Aborting interaction!\n";
					return false;
				}

				mitk::Point3D worldPoint = posEvent->GetWorldPosition();
				/* now we have a worldpoint. check if it is inside our object and select/deselect it accordingly */

				mitk::StateEvent* newStateEvent = NULL;
				const Geometry3D* geometry = GetData()->GetUpdatedTimeSlicedGeometry()->GetGeometry3D( m_TimeStep );
				if (geometry->IsInside(worldPoint))
				newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
				else
				newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());

				/* write new state (selected/not selected) to the property */      
				this->HandleEvent( newStateEvent );

				ok = true;
				break;
			}
		case AcSELECT:
			// select the data
			{
				mitk::BoolProperty::Pointer selected = dynamic_cast<mitk::BoolProperty*>(m_DataNode->GetProperty("selected"));
				if ( selected.IsNull() ) 
				{
					selected = mitk::BoolProperty::New();
					m_DataNode->GetPropertyList()->SetProperty("selected", selected);
				}

				selected->SetValue(true);
	
				// search for surface in surface list (by name)
				int count = 0;
				std::vector<std::string> currentSurfaces = m_Ext->GetCurrentSurfaceVector();
				for(std::vector<std::string>::iterator it = currentSurfaces.begin(); it != currentSurfaces.end(); it++)
				{
					if(*it == m_DataNode->GetName())
					{
						//std::cout << "Found Surface with name: " << *it << "\n";
						// Select Surface and set as segmentation
						//m_Ext->SelectSurface(count);
						m_Ext->UpdateSelectedSurfaceBox(count);

					}		 
					count++;
				} 

				//update rendering
				mitk::RenderingManager::GetInstance()->RequestUpdateAll();

				ok = true;
				break;
			}
		case AcDESELECT:
			//deselect the data
			{
				mitk::BoolProperty::Pointer selected = dynamic_cast<mitk::BoolProperty*>(m_DataNode->GetProperty("selected"));
				if ( selected.IsNull() ) 
				{
					selected = mitk::BoolProperty::New();
					m_DataNode->GetPropertyList()->SetProperty("selected", selected);
				}

				selected = mitk::BoolProperty::New(false);
	
				//update rendering
				mitk::RenderingManager::GetInstance()->RequestUpdateAll();

				ok = true;
				break;
			}
		case AcMOVE:
			{
				ok = true;
				break;
			}

		default:
			return Superclass::ExecuteAction( action, stateEvent );
	}

	return ok;
}

