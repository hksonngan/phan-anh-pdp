/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-08-25 13:01:29 +0200 (Mi, 25 Aug 2010) $
Version:   $Revision: 25766 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSELECTSURFACEWITHMOUSEINTERACTOR_H
#define MITKSELECTSURFACEWITHMOUSEINTERACTOR_H

#include <mitkInteractor.h>

namespace mitk
{
	class DataNode;
	class ToolExtension;

	/**
	* \brief Interaction to select a surface with a mouse click.
	*/
	class SelectSurfaceWithMouseInteractor : public Interactor
	{
		public:
			mitkClassMacro(SelectSurfaceWithMouseInteractor, Interactor);
			mitkNewMacro3Param(Self, const char*, DataNode*, mitk::ToolExtension*);

		protected:
			/**
			* \brief Constructor 
			*/
			SelectSurfaceWithMouseInteractor(const char * type, DataNode* dataNode, mitk::ToolExtension*);

			/**
			* \brief Default Destructor
			**/
			virtual ~SelectSurfaceWithMouseInteractor();

			/**
			* @brief Convert the given Actions to Operations and send to data and UndoController
			**/
			virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );
		
		private:
			mitk::ToolExtension* m_Ext;
	};
}
#endif 
