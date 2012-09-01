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

#include "QmitkRegionGrowingToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkRegionGrowingToolGUI::QmitkRegionGrowingToolGUI()
:QmitkToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	// create the visible widgets
	QBoxLayout* verticalLayout = new QVBoxLayout( this );

	m_HorizontalLayout = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout);
	m_HorizontalLayout1 = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout1);
	m_HorizontalLayout2 = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout2);

	m_SelectSurfaceBox = new QComboBox();
	m_HorizontalLayout->addWidget(m_SelectSurfaceBox);
	connect(m_SelectSurfaceBox, SIGNAL(activated(int)), this, SLOT(SelectSurface(int)));

	m_PbtnSelectSurfaceViaMouse = new QPushButton();
	QIcon icn_OpenFile;
	icn_OpenFile.addFile(":/threeDEditing/res/threeDEditing/mousepointer.png");
	m_PbtnSelectSurfaceViaMouse->setIcon(icn_OpenFile);
	m_PbtnSelectSurfaceViaMouse->setFixedSize(22,22);
	m_HorizontalLayout->addWidget(m_PbtnSelectSurfaceViaMouse);
	connect(m_PbtnSelectSurfaceViaMouse, SIGNAL(clicked()), this, SLOT(SelectSurfaceViaMouse()));
	
	m_PbtnReinitSelection = new QPushButton();
	m_PbtnReinitSelection->setText("Reinit Selection");
	m_PbtnReinitSelection->setCheckable(true);
	m_HorizontalLayout1->addWidget(m_PbtnReinitSelection);
	connect(m_PbtnReinitSelection, SIGNAL(clicked()), this, SLOT(ReinitSelection()));

	m_PbtnJumpToPosition = new QPushButton();
	m_PbtnJumpToPosition->setText("Jump to Position");
	m_PbtnJumpToPosition->setCheckable(true);
	m_HorizontalLayout1->addWidget(m_PbtnJumpToPosition);
	connect(m_PbtnJumpToPosition, SIGNAL(clicked()), this, SLOT(JumpToPosition()));

	QLabel* label = new QLabel( "Radius", this );
	QFont f = label->font();
	f.setBold(false);
	label->setFont( f );
	m_HorizontalLayout2->addWidget(label);

	m_RadiusSlider = new QSlider(Qt::Horizontal);
	m_RadiusSlider->setSliderPosition(20);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);
	QObject::connect(m_RadiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(RadiusChanged(int)));


	// Tool Specific
	QBoxLayout* horizontalLayout3 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout3);
	QBoxLayout* horizontalLayout4 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout4);

	QPushButton* pbtn_DetectHoles = new QPushButton();
	pbtn_DetectHoles->setText("Detect Holes");
	horizontalLayout3->addWidget(pbtn_DetectHoles);
	connect(pbtn_DetectHoles, SIGNAL(clicked()), this, SLOT(DetectHoles()));

	QPushButton* pbtn_RegionGrowings = new QPushButton();
	pbtn_RegionGrowings->setText("Fill Holes");
	horizontalLayout3->addWidget(pbtn_RegionGrowings);
	connect(pbtn_RegionGrowings, SIGNAL(clicked()), this, SLOT(RegionGrowings()));

	QPushButton* pbtn_RegionGrowingsVOI = new QPushButton();
	pbtn_RegionGrowingsVOI->setText("Fill Holes VOI");
	horizontalLayout4->addWidget(pbtn_RegionGrowingsVOI);
	connect(pbtn_RegionGrowingsVOI, SIGNAL(clicked()), this, SLOT(RegionGrowingsVOI()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkRegionGrowingToolGUI::~QmitkRegionGrowingToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_RegionGrowingTool.IsNotNull())
	{
		m_RegionGrowingTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, int>( this, &QmitkRegionGrowingToolGUI::RadiusValueChanged );
		m_RegionGrowingTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, std::vector<std::string>>( this, &QmitkRegionGrowingToolGUI::InitSurfaceSelection );
		m_RegionGrowingTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, int>(this, &QmitkRegionGrowingToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_HorizontalLayout1->removeWidget(m_PbtnJumpToPosition);
	m_SelectSurfaceBox->clear();
	for(std::vector<std::string>::iterator it = currentSurfaces.begin(); it != currentSurfaces.end(); it++)
	{
		m_SelectSurfaceBox->addItem(it->c_str());
	}
	m_HorizontalLayout1->addWidget(m_PbtnJumpToPosition);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_RegionGrowingTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_RegionGrowingTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_RegionGrowingTool = dynamic_cast<mitk::RegionGrowingTool*>( tool );

	m_RegionGrowingTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, int>( this, &QmitkRegionGrowingToolGUI::RadiusValueChanged );
	m_RegionGrowingTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, std::vector<std::string>>( this, &QmitkRegionGrowingToolGUI::InitSurfaceSelection );
	m_RegionGrowingTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkRegionGrowingToolGUI, int>(this, &QmitkRegionGrowingToolGUI::UpdateSelectedSurfaceBox);
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkRegionGrowingToolGUI::DetectHoles()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->DetectHoles();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::RegionGrowings()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->RegionGrowings();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegionGrowingToolGUI::RegionGrowingsVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegionGrowingTool->RegionGrowingsVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
