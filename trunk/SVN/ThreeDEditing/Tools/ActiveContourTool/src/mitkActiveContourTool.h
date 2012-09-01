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

#ifndef mitkActiveContourTool_h_Included
#define mitkActiveContourTool_h_Included

#include <mitkTool.h>
#include "../../mitkToolExtension.h"
#include "../../pdpUtilities.h"
#include "StartStopChanVese.h"

#include <mitkMoveSurfaceInteractor.h>
#include <mitkGlobalInteraction.h>

#include "itkImage.h"
#include "itkAtanRegularizedHeavisideStepFunction.h"

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class ActiveContourTool : public mitk::Tool
	{
		public:
			static const unsigned int m_Dimension = 2;
			typedef float ScalarPixelType;
			typedef itk::Image< ScalarPixelType, m_Dimension > InternalImageType;
			typedef itk::ScalarChanAndVeseLevelSetFunctionData< InternalImageType, InternalImageType > DataHelperType;
			typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData< InternalImageType, InternalImageType, DataHelperType > SharedDataHelperType;
			typedef itk::ScalarChanAndVeseLevelSetFunction< InternalImageType, InternalImageType, SharedDataHelperType > LevelSetFunctionType;
			typedef itk::StartStopChanVese< InternalImageType, InternalImageType, InternalImageType, LevelSetFunctionType, SharedDataHelperType > MultiLevelSetType;
			typedef itk::AtanRegularizedHeavisideStepFunction< ScalarPixelType,	ScalarPixelType >  DomainFunctionType;

			Message1<int> SetSliderToLastRadius;
			Message1<std::vector<std::string>> GetCurrentSurfaces;	
			Message1<int> UpdateSelectedSurface;
			Message1<int> SetNumberOfIterations;
			/**
			 * Needed because we use itk::SmartPointers and like
			 * to have some typedefs just as most itk::Objects
			 */
			mitkClassMacro(ActiveContourTool, mitk::Tool);	    
			/**
			 * Created through itk::ObjectFactory, so we need a
			 * ::New() method, which is created by this macro.
			 */
			itkNewMacro(ActiveContourTool);
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
			void ActiveContours();
			void ActiveContoursVOI();
			mitk::Image::Pointer mitk::ActiveContourTool::Iteration(int,itk::Image<float,2>::Pointer);
			void Run();
			void Initialize();
			void Reset();

			itk::Image< float, 2 >::Pointer GetInputImage();
			itk::Image< float, 2 >::Pointer CreateDistanceMap(itk::Image< float, 2 >::Pointer input);
			mitk::DataNode::Pointer AddToDataStorage(itk::Image< float, 2 >::Pointer data, char* name, double opacity);
			int GetUniqueId();
			//void SetBubble(NodeContainer::Pointer seeds, InternalImageType::IndexType newSeedPoint, double radius);

			void Stop();
			void StepBack();
			void ReversePlay();
			void Pause();
			void Play();
			void StepForward();	

			void SelectStepSize(int);

			void ActiveContour3d();
	

		protected:
			ActiveContourTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~ActiveContourTool();
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
		private:
			// Tool extension interface
			mitk::ToolExtension* m_Ext;
			
			// Tool Specific
			pdp::Utilities* m_Ut;

			MultiLevelSetType::Pointer m_LevelSetFilter;
			mitk::DataNode::Pointer m_OutNode;

			int m_StepSize;
			int m_DirectedPlay; // 0 = pause, 1 = forward play; 2 = backward play
			bool m_Initialize;
			int m_CurrentNumberOfIterations;
	};
} // namespace

#endif


