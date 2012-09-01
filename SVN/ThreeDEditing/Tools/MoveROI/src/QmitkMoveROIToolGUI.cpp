/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2008-08-01 14:25:23 +0200 (vie, 01 ago 2008) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkMoveROIToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>

QmitkMoveROIToolGUI::QmitkMoveROIToolGUI()
:QmitkToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	// create the visible widgets
	QBoxLayout* verticalLayout = new QVBoxLayout( this );

	QBoxLayout* horizontalLayout = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout);
	QBoxLayout* horizontalLayout1 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout1);
	QBoxLayout* horizontalLayout2 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout2);
	QBoxLayout* horizontalLayout3 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout3);
	QBoxLayout* horizontalLayout4 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout4);
	QBoxLayout* horizontalLayout5 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout5);
	QBoxLayout* horizontalLayout6 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout6);

	m_SelectSurfaceBox = new QComboBox();
	horizontalLayout->addWidget(m_SelectSurfaceBox);
	connect(m_SelectSurfaceBox, SIGNAL(activated(int)), this, SLOT(SelectSurface(int)));
	
	QPushButton* pbtn_JumpToPosition = new QPushButton();
	pbtn_JumpToPosition->setText("Jump to Position");
	horizontalLayout->addWidget(pbtn_JumpToPosition);
	connect(pbtn_JumpToPosition, SIGNAL(clicked()), this, SLOT(JumpToPosition()));

	QLabel* label = new QLabel( "Radius", this );
	QFont f = label->font();
	f.setBold(false);
	label->setFont( f );
	horizontalLayout1->addWidget(label);

	m_RadiusSlider = new QSlider(Qt::Horizontal);
	m_RadiusSlider->setSliderPosition(20);
	horizontalLayout1->addWidget(m_RadiusSlider);
	QObject::connect(m_RadiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(RadiusChanged(int)));

	QPushButton* pbtn_CreateSurface = new QPushButton();
	pbtn_CreateSurface->setText("Create Surface");
	horizontalLayout2->addWidget(pbtn_CreateSurface);
	connect(pbtn_CreateSurface, SIGNAL(clicked()), this, SLOT(CombineSurfaces()));

	QPushButton* pbtn_Bulging = new QPushButton();
	pbtn_Bulging->setText("Bulging");
	horizontalLayout2->addWidget(pbtn_Bulging);
	connect(pbtn_Bulging, SIGNAL(clicked()), this, SLOT(Bulging()));

	QPushButton* pbtn_PatchHoles = new QPushButton();
	pbtn_PatchHoles->setText("Patch Holes");
	horizontalLayout3->addWidget(pbtn_PatchHoles);
	connect(pbtn_PatchHoles, SIGNAL(clicked()), this, SLOT(PatchHoles()));

	QPushButton* pbtn_SmoothROI = new QPushButton();
	pbtn_SmoothROI->setText("Smooth ROI");
	horizontalLayout3->addWidget(pbtn_SmoothROI);
	connect(pbtn_SmoothROI, SIGNAL(clicked()), this, SLOT(SmoothROI()));

	QLabel* label2 = new QLabel( "Smooth Iterations", this );
	QFont f2 = label2->font();
	f2.setBold(false);
	label2->setFont( f2 );
	horizontalLayout4->addWidget(label2);

	QSlider* smoothSlider = new QSlider(Qt::Horizontal);
	smoothSlider->setSliderPosition(5);
	horizontalLayout4->addWidget(smoothSlider);
	QObject::connect(smoothSlider, SIGNAL(sliderMoved(int)), this, SLOT(SmoothChanged(int)));

	QPushButton* pbtn_Subdivide = new QPushButton();
	pbtn_Subdivide->setText("Subdivide Surface");
	horizontalLayout5->addWidget(pbtn_Subdivide);
	connect(pbtn_Subdivide, SIGNAL(clicked()), this, SLOT(Subdivide()));

	QPushButton* pbtn_SubdivideVOI = new QPushButton();
	pbtn_SubdivideVOI->setText("Subdivide VOI");
	horizontalLayout5->addWidget(pbtn_SubdivideVOI);
	connect(pbtn_SubdivideVOI, SIGNAL(clicked()), this, SLOT(SubdivideVOI()));

	QPushButton* pbtn_ColorSurface = new QPushButton();
	pbtn_ColorSurface->setText("Color Surface");
	horizontalLayout6->addWidget(pbtn_ColorSurface);
	connect(pbtn_ColorSurface, SIGNAL(clicked()), this, SLOT(ColorSurface()));


	 connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkMoveROIToolGUI::~QmitkMoveROIToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	for(std::vector<std::string>::iterator it = currentSurfaces.begin(); it != currentSurfaces.end(); it++)
	{
		m_SelectSurfaceBox->addItem(it->c_str());
	}

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::SelectSurface(int selection)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::cout << "Selected Surface: " << selection << "\n";
	m_MoveROITool->SelectSurface(selection);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_MoveROITool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::CombineSurfaces()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->CombineSurfaces();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::Bulging()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->ToggleBulging();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::PatchHoles()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->PatchHoles();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::SmoothROI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->SmoothROI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::SmoothChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Smooth Iteration: " << value*10 << "\n";
	m_MoveROITool->SmoothChanged(value*10);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::Subdivide()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->Subdivide();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::SubdivideVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->SubdivideVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::ColorSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_MoveROITool->ColorSurface();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_MoveROITool = dynamic_cast<mitk::MoveROITool*>( tool );

	m_MoveROITool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkMoveROIToolGUI, int>( this, &QmitkMoveROIToolGUI::RadiusValueChanged );
	m_MoveROITool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkMoveROIToolGUI, std::vector<std::string>>( this, &QmitkMoveROIToolGUI::InitSurfaceSelection );
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkMoveROIToolGUI::RadiusValueChanged(int currentRadius)
{
	std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	m_RadiusSlider->setSliderPosition(currentRadius);
}