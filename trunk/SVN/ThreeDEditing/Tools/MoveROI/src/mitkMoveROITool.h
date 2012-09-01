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

#ifndef mitkMoveROITool_h_Included
#define mitkMoveROITool_h_Included

#include "mitkCommon.h"
#include <mitkTool.h>
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"
#include "mitkDataStorage.h"
#include <mitkSurface.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include "QmitkStdMultiWidget.h"

class mitk::Image;

namespace mitk
{

/**
  \warning Only to be instantiated by mitk::ToolManager.
*/
class MoveROITool : public mitk::Tool//, public  QmitkFunctionality
{
  public:
    
	Message1<int> SetSliderToLastRadius;
	Message1<std::vector<std::string>> GetCurrentSurfaces;

    /**
     * Needed because we use itk::SmartPointers and like
     * to have some typedefs just as most itk::Objects
     */
    mitkClassMacro(MoveROITool, mitk::Tool);
    
    /**
     * Created through itk::ObjectFactory, so we need a
     * ::New() method, which is created by this macro.
     */
	itkNewMacro(MoveROITool);

    /**
     * \brief Returns an icon for this tool.
     *
     * Saved in the XPM format, can be written e.g. by The Gimp.
     */
   /* virtual*/ const char** GetXPM() const;

    /**
     * \brief Returns a (short) name for this tool.
     *
     * E.g. in InteractiveSegmentation, this string
     * would be displayed in a toolbutton right below
     * the tool's icon.
     */
    /*virtual*/ const char* GetName() const;
    
    /**
     * \brief Returns a group name for this tool.
     * Tools can be grouped, and each group is described
     * by a string.
     */
    /*virtual*/ const char* GetGroup() const;

	void RadiusChanged(int value);
	int GetRadius(){return m_Radius;};
	
	void CombineSurfaces();
	void ToggleBulging(){if(m_bulging){m_bulging = false;}else{m_bulging = true;}};
	void PatchHoles();
	void SmoothROI();
	void SmoothChanged(int value);
	void Subdivide();
	void SubdivideVOI();
	void ColorSurface();
	void SelectSurface(int);

	void JumpToPosition();
	void UpdateRenderer();
	vtkSmartPointer<vtkPolyData> createTestContour();

  protected:
    
    MoveROITool(); // purposely hidden, should always be done with itk::Objects that are create by New()
    /*virtual*/ ~MoveROITool();
    
    /**
     * \brief Called when tool becomes active.
     */
    /*virtual */void Activated();
    
    /**
     * \brief Called when tool is deactivated.
     */
    /*virtual */void Deactivated();

    /**
     * \brief Overloaded to process mouse move events.
     */
    /*virtual */bool OnMouseMoved   (Action*, const StateEvent*);

	//void CreateQtPartControl(QWidget* parent){};

  private:
	int m_Counter;
	double m_Radius;
	double m_RadiusFactor;
	int m_SmoothIteration;
	int m_ContourResolution;
	bool m_bulging;

	bool m_JumpToPosition;
	bool m_SurfaceHadBeenHit;
	std::vector<std::string> m_CurrentSurfaces;
	
	int m_OldVoidCenterVectorIndex;
	int m_MouseMovementVectorSumLength;
	double* m_OldVOICenterVectors;
	double* m_OldVOICenterVector;
	double m_MouseMovementVector[3];
	double m_MouseMovementVectorSum[3];

	mitk::DataNode::Pointer m_PositionTrackerNode;
	mitk::DataNode::Pointer m_BallNode;	
	mitk::DataNode::Pointer m_IntersectionNode;
	mitk::DataNode::Pointer m_SurfaceNode;

	mitk::Surface::Pointer m_Surface;
	mitk::Surface::Pointer m_Ball;
	mitk::Surface::Pointer m_IntersectionData;
	
	vtkSmartPointer<vtkIntersectionPolyDataFilter> m_IntersectionPolyDataFilter;
	vtkSphereSource* m_ContourSphere;
    
	DataStorage* m_DataStorage;
	QmitkStdMultiWidget* m_MitkView;
};

} // namespace

#endif


