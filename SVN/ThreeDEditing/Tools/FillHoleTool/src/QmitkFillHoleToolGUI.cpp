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

#include "QmitkFillHoleToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkFillHoleToolGUI::QmitkFillHoleToolGUI()
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
	QBoxLayout* horizontalLayout5 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout5);
	QBoxLayout* horizontalLayout6 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout6);

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QPushButton* pbtn_DetectHoles = new QPushButton();
	pbtn_DetectHoles->setText("Detect Holes");
	horizontalLayout4->addWidget(pbtn_DetectHoles);
	connect(pbtn_DetectHoles, SIGNAL(clicked()), this, SLOT(DetectHoles()));

	QPushButton* pbtn_FillHoles = new QPushButton();
	pbtn_FillHoles->setText("Fill Holes");
	horizontalLayout5->addWidget(pbtn_FillHoles);
	connect(pbtn_FillHoles, SIGNAL(clicked()), this, SLOT(FillHoles()));

	QPushButton* pbtn_FillHolesVOI = new QPushButton();
	pbtn_FillHolesVOI->setText("Fill Holes VOI");
	horizontalLayout6->addWidget(pbtn_FillHolesVOI);
	connect(pbtn_FillHolesVOI, SIGNAL(clicked()), this, SLOT(FillHolesVOI()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkFillHoleToolGUI::~QmitkFillHoleToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_FillHoleTool.IsNotNull())
	{
		m_FillHoleTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkFillHoleToolGUI, int>( this, &QmitkFillHoleToolGUI::RadiusValueChanged );
		m_FillHoleTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkFillHoleToolGUI, std::vector<std::string>>( this, &QmitkFillHoleToolGUI::InitSurfaceSelection );
		m_FillHoleTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkFillHoleToolGUI, int>(this, &QmitkFillHoleToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkFillHoleToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_FillHoleTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_FillHoleTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_FillHoleTool = dynamic_cast<mitk::FillHoleTool*>( tool );

	m_FillHoleTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkFillHoleToolGUI, int>( this, &QmitkFillHoleToolGUI::RadiusValueChanged );
	m_FillHoleTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkFillHoleToolGUI, std::vector<std::string>>( this, &QmitkFillHoleToolGUI::InitSurfaceSelection );
	m_FillHoleTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkFillHoleToolGUI, int>(this, &QmitkFillHoleToolGUI::UpdateSelectedSurfaceBox);
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkFillHoleToolGUI::DetectHoles()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->DetectHoles();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::FillHoles()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->FillHoles();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkFillHoleToolGUI::FillHolesVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_FillHoleTool->FillHolesVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
