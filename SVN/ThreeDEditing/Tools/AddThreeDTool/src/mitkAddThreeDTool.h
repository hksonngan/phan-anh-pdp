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

#ifndef pdpAddThreeDTool_h_Included
#define pdpAddThreeDTool_h_Included

#include "mitkCommon.h"
#include <mitkTool.h>
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"

class mitk::Image;

namespace mitk
{

/**
  \brief Example tool (external extension)

  \sa FeedbackContourTool
  
  \ingroup Interaction
  \ingroup Reliver
  
  There is a separate page describing the general design of 
  QmitkInteractiveSegmentation: \li \ref QmitkInteractiveSegmentationTechnicalPage

  This is no functional tool, it is only meant as an example on how to
  create tools as en external extension to MITK's InteractiveSegmentation
  (and similar functionalities). The full description of how this works
  can be found here: \li \ref toolextensions

  In contrast to PlainExternalTool, this tool here comes with a GUI.

  \warning Only to be instantiated by mitk::ToolManager.
*/
class AddThreeDTool : public mitk::Tool
{
  public:
    
    /**
     * Needed because we use itk::SmartPointers and like
     * to have some typedefs just as most itk::Objects
     */
    mitkClassMacro(AddThreeDTool, mitk::Tool);
    
    /**
     * Created through itk::ObjectFactory, so we need a
     * ::New() method, which is created by this macro.
     */
    itkNewMacro(AddThreeDTool);

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

  protected:
    
    AddThreeDTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
    /*virtual*/ ~AddThreeDTool();
    
    /**
     * \brief Called when tool becomes active.
     */
    /*virtual */void Activated();
    
    /**
     * \brief Called when tool is deactivated.
     */
    /*virtual */void Deactivated();
    
    /**
     * \brief Overloaded to process mouse down events.
     */
    /*virtual */bool OnMousePressed (Action*, const StateEvent*);

    /**
     * \brief Overloaded to process mouse move events.
     */
    /*virtual */bool OnMouseMoved   (Action*, const StateEvent*);

    /**
     * \brief Overloaded to process mouse release events.
     */
   /* virtual */bool OnMouseReleased(Action*, const StateEvent*);
};

} // namespace

#endif


