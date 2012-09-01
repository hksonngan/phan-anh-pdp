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

#ifndef mitkActiveSurfaceTool_h_Included
#define mitkActiveSurfaceTool_h_Included

#include <mitkTool.h>
#include "../../mitkToolExtension.h"
#include "../../pdpUtilities.h"
#include "StartStopChanVese.h"

#include <mitkMoveSurfaceInteractor.h>
#include <mitkGlobalInteraction.h>

#include "itkImage.h"
#include "itkAtanRegularizedHeavisideStepFunction.h"
#include "itkFastMarchingImageFilter.h"

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class ActiveSurfaceTool : public mitk::Tool
	{
		public:
			static const unsigned int m_Dimension = 3;
			typedef float ScalarPixelType;
			typedef itk::Image< ScalarPixelType, m_Dimension > InternalImageType;
			typedef itk::ScalarChanAndVeseLevelSetFunctionData< InternalImageType, InternalImageType > DataHelperType;
			typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData< InternalImageType, InternalImageType, DataHelperType > SharedDataHelperType;
			typedef itk::ScalarChanAndVeseLevelSetFunction< InternalImageType, InternalImageType, SharedDataHelperType > LevelSetFunctionType;

			typedef itk::StartStopChanVese< InternalImageType, InternalImageType, InternalImageType, LevelSetFunctionType, SharedDataHelperType > MultiLevelSetType;
			//typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter< InternalImageType,	InternalImageType, InternalImageType, LevelSetFunctionType,	SharedDataHelperType > MultiLevelSetType;

			typedef itk::AtanRegularizedHeavisideStepFunction< ScalarPixelType,	ScalarPixelType >  DomainFunctionType;
			typedef itk::FastMarchingImageFilter< InternalImageType, InternalImageType > FastMarchingFilterType;

			Message1<int> SetSliderToLastRadius;
			Message1<std::vector<std::string>> GetCurrentSurfaces;	
			Message1<int> UpdateSelectedSurface;
			Message1<int> SetNumberOfIterations;
			Message1<std::vector<std::string>> GetCurrentImages;	
			Message1<int> UpdateSelectedImages;
			Message1<int> SetButtonsToPause;
			
			Message1<double> SetCurvatureForce;
			Message1<double> SetAreaForce;
			Message1<double> SetVolumeForce;
			/**
			 * Needed because we use itk::SmartPointers and like
			 * to have some typedefs just as most itk::Objects
			 */
			mitkClassMacro(ActiveSurfaceTool, mitk::Tool);	    
			/**
			 * Created through itk::ObjectFactory, so we need a
			 * ::New() method, which is created by this macro.
			 */
			itkNewMacro(ActiveSurfaceTool);
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
			std::vector<std::string> GetReferenceImages();
			void SelectImage(int);
			void CreateReferenceImageVOI();
			bool SetBubble(Action* action, const StateEvent* stateEvent);
			void RemoveBubble();
			bool RemoveBubbleWithMouse(Action* action, const StateEvent* stateEvent);
			void Run();
			void Initialize();
			void Reset();
			void SoftReset();

			itk::Image< float, 3 >::Pointer GetInputImage();
			itk::Image< float, 3 >::Pointer CreateDistanceMap(itk::Image< float, 3 >::Pointer input);
			mitk::DataNode::Pointer AddToDataStorage(itk::Image< float, 3 >::Pointer data, char* name, double opacity);

			void Stop();
			void StepBack();
			void ReversePlay();
			void Pause();
			void Play();
			void StepForward();	

			void SelectStepSize(int);

			itk::Image< float, 3 >::Pointer CreateThickeningMask(itk::Image< float, 3 >::Pointer input);
			itk::Image< float, 3 >::Pointer CreateSeedPointMask(itk::Image< float, 3 >::Pointer input, int seedpoint);
				
			void CurvatureForceCross(int,int);
			void CurvatureForceLineEdit(double);
			void AreaForceCross(int,int);
			void AreaForceLineEdit(double);
			void VolumeForceCross(int,int);
			void VolumeForceLineEdit(double);
			void Closing();
			void Zoom(int);


		protected:
			ActiveSurfaceTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~ActiveSurfaceTool();
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
			std::vector<std::string> m_CurrentImages;
			mitk::DataNode::Pointer m_ReferenceImageNode;
			mitk::Image::Pointer m_ReferenceImage;
			mitk::DataNode::Pointer m_FeatureImageNode;
			mitk::Image::Pointer m_FeatureImage;
			mitk::DataNode::Pointer m_DistanceImageNode;
			mitk::Image::Pointer m_DistanceImage;

			MultiLevelSetType::Pointer m_LevelSetFilter;
			FastMarchingFilterType::Pointer  m_FastMarching;

			mitk::DataNode::Pointer m_OutNode;
			mitk::Point3D m_SavedOrigin;

			mitk::Geometry3D::Pointer m_SavedGeometry;

			int m_StepSize;
			int m_DirectedPlay; // 0 = pause, 1 = forward play; 2 = backward play
			bool m_Initialize;
			int m_CurrentNumberOfIterations;

			std::vector<mitk::Surface*> m_Bubbles;
			std::vector<mitk::DataNode*> m_BubbleNodes;
			std::vector<double> m_BubbleRadii;
			std::vector<mitk::Point3D> m_BubbleOrigins;

			double m_Curvature_weight;
			double m_PropagationForce;
			double m_Area_weight;
			double m_Volume_weight;

			std::vector<itk::Image< float, 3 >::Pointer> m_History;
			itk::Image< float, 3 >::Pointer m_CurrentFeatureImage;
	};
} // namespace

#endif


