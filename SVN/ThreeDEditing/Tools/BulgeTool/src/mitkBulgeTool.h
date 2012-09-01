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

#ifndef mitkBulgeTool_h_Included
#define mitkBulgeTool_h_Included

#include <mitkTool.h>
#include "../../mitkToolExtension.h"
#include "../../pdpPt.h"

#include <mitkMoveSurfaceInteractor.h>
#include <mitkGlobalInteraction.h>

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class BulgeTool : public mitk::Tool
	{
		public:
			Message1<int> SetSliderToLastRadius;
			Message1<std::vector<std::string>> GetCurrentSurfaces;
			Message1<int> UpdateSelectedSurface;
			/**
			* Needed because we use itk::SmartPointers and like
			* to have some typedefs just as most itk::Objects
			*/
			mitkClassMacro(BulgeTool, mitk::Tool);
			/**
			* Created through itk::ObjectFactory, so we need a
			* ::New() method, which is created by this macro.
			*/
			itkNewMacro(BulgeTool);
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
			void AddPoints();
			void ShowTool();
			void MovePointsInMouseMovementDirection();
			void MovePointsInRadialDirection();
			void AutomaticInsertPoints(std::vector<vtkIdType> cells);
			void SubdivideByInsertPoint(std::vector<vtkIdType> cells);
			void SubdivideByLongestEdge(std::vector<vtkIdType> cells);
			void Collinear();
			void Radial();
			void SetRefinementTechnique(int selection);
			void ResetNormals();
			void ReinitNow();

		protected:
			BulgeTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~BulgeTool();
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
			bool OnMouseLeftClick(Action*, const StateEvent*);
			bool OnMouseDrag(Action*, const StateEvent*);
			bool OnMouseLeftClickRelease(Action*, const StateEvent*);
		private:
			// Tool extension interface
			mitk::ToolExtension* m_Ext;

			// Tool Specific
			bool m_SurfaceHadBeenHit;

			int m_OldVoidCenterVectorIndex;
			int m_MouseMovementVectorSumLength;
			double* m_OldVOICenterVectors;
			pdp::Pt m_OldVOICenterVector;
			pdp::Pt m_MouseMovementVectorSum;
			pdp::Pt m_MouseMovementVector;

			bool m_RadialDirection;
			bool m_ShowBall;
			bool m_SurfaceChanged;

			int m_RefinementTechnique;

			double m_AutoInsertPointThreshold;
			double m_RadiusTresholdFactor;
	};

} // namespace

#endif


