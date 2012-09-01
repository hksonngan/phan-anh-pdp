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

#ifndef mitkDragTool_h_Included
#define mitkDragTool_h_Included

#include <mitkTool.h>
#include "../../mitkToolExtension.h"
#include "../../pdpUtilities.h"
#include "../../pdpPt.h"
#include "../../../RadiusInteractor.h"

#include <mitkMoveSurfaceInteractor.h>
#include <mitkGlobalInteraction.h>

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class DragTool : public mitk::Tool
	{
		public:
			Message1<int> SetSliderToLastRadius;
			Message1<std::vector<std::string>> GetCurrentSurfaces;	
			Message1<int> UpdateSelectedSurface;
			Message1<int> UpdateRadiusSlider2;
			Message1<std::vector<std::string>> GetCurrentWeights;	
			/**
			 * Needed because we use itk::SmartPointers and like
			 * to have some typedefs just as most itk::Objects
			 */
			mitkClassMacro(DragTool, mitk::Tool);	    
			/**
			 * Created through itk::ObjectFactory, so we need a
			 * ::New() method, which is created by this macro.
			 */
			itkNewMacro(DragTool);
			/**
			 * \brief Returns an icon for this tool.
			 *
			 * Saved in the XPM format, can be written e.g. by The Gimp.
			 */
			const char** GetXPM() const;
			/**
			 * \brief Returns a (short) name for this tool.
			 *
			 * E.g. in InteractiveSegmentation, this string
			 * would be displayed in a toolbutton right below
			 * the tool's icon.
			 */
			const char* GetName() const;
			/**
			 * \brief Returns a group name for this tool.
			 * Tools can be grouped, and each group is described
			 * by a string.
			 */
			const char* GetGroup() const;

			// Tool extension interface
			void SelectSurface(int);
			void SelectSurfaceViaMouse()
			{
				mitk::GlobalInteraction::GetInstance()->RemoveListener(this);
				m_Ext->SelectSurfaceViaMouse();
			};
			void UpdateSelectedSurfaceBox(int selection)
			{
				UpdateSelectedSurface.Send(selection);
				mitk::GlobalInteraction::GetInstance()->AddListener(this);
			};
			void UpdateRadiusSlider(int radius)
			{
				SetSliderToLastRadius(radius * m_Ext->GetRadiusFactor()-1);
			}
			void ReinitSelection(){m_Ext->ReinitSelection();};
			void JumpToPosition(){m_Ext->JumpToPosition();};
			void RadiusChanged(int value){m_Ext->RadiusChanged(value);};
			void UpdateRenderer(){m_Ext->UpdateRenderer();};
			vtkSmartPointer<vtkPolyData> CreateTestContour(){m_Ext->CreateTestContour();};

			// Tool Specific
			void ROIRadiusChanged(int);
			void SetColors();
			void Zoom(int);

			void CreateLMatrixWithW1();
			void CreateLMatrixWithW2();
			void CreateLMatrixWithW3();
			void SelectWeight(int);

		protected:
			DragTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~DragTool();
			/**
			 * \brief Called when tool becomes active.
			 */
			void Activated();
			/**
			 * \brief Called when tool is deactivated.
			 */
			void Deactivated();
			/**
			 * \brief Overloaded to process mouse move events.
			 */
			bool OnMouseMoved(Action*, const StateEvent*);
			bool LeftDrag(Action*, const StateEvent*);
			bool LeftClick(Action*, const StateEvent*);
			bool LeftClickRelease(Action*, const StateEvent*);
			void ColorNodes();
			void UpdateRadiusViaInteractor(int value);
		private:
			// Tool extension interface
			mitk::ToolExtension* m_Ext;
			
			// Tool Specific
			pdp::Utilities* m_Ut;

			bool m_CursorOnSurface;

			vtkIdType m_HandlePointID;
			pdp::Pt m_HandlePoint;
			pdp::Pt m_LastPoint;
			double m_ROIRadius;
			std::vector<vtkIdType> m_PointsInROI;
			std::vector<vtkIdType> m_AnchorPointsOutsideROI;
			std::vector<vtkIdType> m_AllPoints;
			double** m_LMatrix;
			double** m_LMatrixWithAnchors;

			vtkSphereSource* m_ContourSphere2;
			mitk::DataNode::Pointer m_BallNode2;
			mitk::Surface::Pointer m_Ball2;

			bool m_DragOnce;
			bool m_Color;
			RadiusInteractor* m_ChangeRadius;
			std::vector<std::string> m_Weights;
			int m_SelectWeight;
	};
} // namespace

#endif


