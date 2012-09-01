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

#ifndef mitkLiveWireTool_h_Included
#define mitkLiveWireTool_h_Included

#include <mitkTool.h>
#include "../../mitkToolExtension.h"
#include "../../pdpPt.h"
#include "../../../RadiusInteractor.h"

#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include "vtkDijkstraImageGeodesicPath.h"
#include "itkRegionOfInterestImageFilter.h"

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class LiveWireTool : public mitk::Tool
	{
		public:
			Message1<int> SetSliderToLastRadius;
			Message1<std::vector<std::string>> GetCurrentSurfaces;	
			Message1<int> UpdateSelectedSurface;
			Message1<int> UpdateRadiusSlider2;
			Message1<int> UpdateProgressBar;

			typedef itk::Image< float, 2 > ImageType;

			/**
			 * Needed because we use itk::SmartPointers and like
			 * to have some typedefs just as most itk::Objects
			 */
			mitkClassMacro(LiveWireTool, mitk::Tool);	    
			/**
			 * Created through itk::ObjectFactory, so we need a
			 * ::New() method, which is created by this macro.
			 */
			itkNewMacro(LiveWireTool);
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
			void UpdateRadiusViaInteractor(int value);
			vtkSmartPointer<vtkImageData> GetCostImage();
			vtkSmartPointer<vtkImageData> GetCostImageWithOutROI();
			void ColorSurface();
			void ColorNodesROI();
			void Zoom(int);

			void Interpolate3D();
			void InitializeLaplacianFramework();
			void MoveSurfacePoints();
			void CalculateLaplacianFramework();
						
			void CreateLMatrixWithW1();
			//void CreateLMatrixWithW2();
			//void CreateLMatrixWithW3();
			//void SelectWeight(int);

		protected:
			LiveWireTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~LiveWireTool();
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
			bool LeftClick(Action*, const StateEvent*);
			bool LeftClickRelease(Action*, const StateEvent*);
			bool LeftDrag(Action*, const StateEvent*);
			bool LeftDragInit(Action*, const StateEvent*);
		private:
			// Tool extension interface
			mitk::ToolExtension* m_Ext;
			
			// Tool Specific
			pdp::Pt m_WorldPoint;
			//mitk::PointSet::Pointer m_PointSet2;
			mitk::DataNode::Pointer m_PointSetNode;
			int m_PointId2;

			vtkSmartPointer<vtkImageData> m_CostImage;
			double m_LastBoundingBox[6];
			vtkDijkstraImageGeodesicPath* m_PathCreator;
			ImageType::IndexType m_Start;
			ImageType::SizeType m_Size;
			mitk::Vector3D m_Spacings;
			mitk::Point3D m_Origin;
			mitk::Point2D m_LastPoint2D;
			bool m_FirstPoint;
			mitk::Surface::Pointer m_IntersectionContour;
			mitk::DataNode::Pointer m_IntersectionContourNode;

			std::vector<vtkIdType> m_StartStopContour;

			bool m_CursorOnSurface;
			RadiusInteractor* m_ChangeRadius;
			double m_ROIRadius;

			vtkSphereSource* m_ContourSphere2;
			mitk::DataNode::Pointer m_BallNode2;
			mitk::Surface::Pointer m_Ball2;

			bool m_FirstDrag;

			vtkIdType m_HandlePointID;
			pdp::Pt m_HandlePoint;
			vtkIdType m_SurfaceHandlePointID;

			vtkIdType m_BeginVertId;
			vtkIdType m_EndVertId;
			double m_StartPoint[3];
			double m_StopPoint[3];

			std::vector<vtkIdType> m_SurfaceIds;
			//mitk::PointSet::Pointer m_LeftContour;
			mitk::DataNode::Pointer m_LeftContourNode;
			//mitk::PointSet::Pointer m_RightContour;
			mitk::DataNode::Pointer m_RightContourNode;
			std::vector<vtkIdType> m_FirstContourPointsToBeDeleted;
			std::vector<vtkIdType> m_SecondContourPointsToBeDeleted;
			std::vector<vtkIdType> m_FirstContourPointsToBeDeletedSurfaceId;
			std::vector<vtkIdType> m_SecondContourPointsToBeDeletedSurfaceId;
			pdp::Pt m_FirstEndPoint;
			pdp::Pt m_SecondEndPoint;

			// Laplacian Framework
			std::vector<vtkIdType> m_PointsInROI;
			std::vector<vtkIdType> m_AnchorPointsOutsideROI;
			std::vector<vtkIdType> m_AllPoints;	
			double** m_LMatrix;
			double** m_LMatrixWithAnchors;
			std::map<vtkIdType, mitk::Point3D> m_FurtherHandlePoints;

			bool m_Color; // todo
	};
} // namespace

#endif


