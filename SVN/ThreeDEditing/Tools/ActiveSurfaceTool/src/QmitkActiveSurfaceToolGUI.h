/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (vie, 12 sep 2008) $
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkActiveSurfaceToolGUI_h_Included
#define QmitkActiveSurfaceToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkActiveSurfaceTool.h"

class QComboBox;

/**
  \brief GUI for mitk::ComplexExternalTool.

  \sa ComplexExternalTool
  \ingroup Widgets
  \ingroup Reliver
  
  There is a separate page describing the general design of 
  QmitkInteractiveSegmentation: \li \ref QmitkInteractiveSegmentationTechnicalPage

  This is only meant as an example on how to create tools (and their GUIs) 
  as en external extension to MITK's InteractiveSegmentation
  (and similar functionalities). The full description of how this works
  can be found here: \li \ref toolextensions

  Last contributor: $Author$
*/

class QmitkActiveSurfaceToolGUI : public QmitkToolGUI
{
	Q_OBJECT
	public:
		/**
		* Needed because we use itk::SmartPointers and like
		* to have some typedefs just as most itk::Objects
		*/
		mitkClassMacro(QmitkActiveSurfaceToolGUI, QmitkToolGUI);
		/**
		* Created through itk::ObjectFactory, so we need a
		* ::New() method, which is created by this macro.
		* 
		* Look at QmitkToolGui.h to see how we marriaged
		* Qt widgets and itk::SmartPointers
		*/
		itkNewMacro(QmitkActiveSurfaceToolGUI);
		virtual ~QmitkActiveSurfaceToolGUI();
	signals:
	public slots:
		void InitSurfaceSelection(std::vector<std::string>);
		void SelectSurface(int);
		void SelectSurfaceViaMouse();
		void UpdateSelectedSurfaceBox(int selection);
		void RadiusValueChanged(int);
		void RadiusChanged(int value);
		void JumpToPosition();
		void ReinitSelection();
		void OnNewToolAssociated(mitk::Tool*);

		// Tool specific
		void InitImageSelection(std::vector<std::string>);
		void SelectImage(int);
		void UpdateSelectedImageBox(int selection);
		void RemoveBubble();
		void Stop();
		void StepBack();
		void ReversePlay();
		void Pause();
		void SetButtonsToPause(int dummy){Pause();};
		void Play();
		void StepForward();	
		void SelectStepSize(int);
		void UpdateNumberOfIterations(int);
		void CurvatureForceCross(int,int);
		void UpdateCurvatureForce(double);
		void EditedCurvatureForce();
		//void PropagationForceCross(int,int);
		//void UpdatePropagationForce(double);
		void AreaForceCross(int,int);
		void UpdateAreaForce(double);
		void EditedAreaForce();
		void VolumeForceCross(int,int);
		void UpdateVolumeForce(double);
		void EditedVolumeForce();
		void Closing();
		void ZoomCross(int,int);

	protected slots:
	protected:
		QComboBox* m_SelectSurfaceBox;
		QPushButton* m_PbtnSelectSurfaceViaMouse;
		QSlider* m_RadiusSlider;
		QPushButton* m_PbtnReinitSelection;
		QPushButton* m_PbtnJumpToPosition;
		QBoxLayout* m_HorizontalLayout;
		QBoxLayout* m_HorizontalLayout1;
		QBoxLayout* m_HorizontalLayout2;

		// Tool specific
		QmitkActiveSurfaceToolGUI();
		QComboBox* m_SelectImageBox;
		mitk::ActiveSurfaceTool::Pointer m_ActiveSurfaceTool;
		std::vector<QPushButton*>* m_VCR;  // 0 = play backward, 1 = pause, 2 = play forward
		QLineEdit* numberOfIterations;
		QLineEdit* m_CurvatureForce;
		QLineEdit* m_PropagationForce;
		QLineEdit* m_AreaForce;
		QLineEdit* m_VolumeForce;
};

#endif

