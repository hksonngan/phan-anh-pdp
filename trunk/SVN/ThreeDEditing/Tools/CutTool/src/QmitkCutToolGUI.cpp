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

#include "QmitkCutToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkCutToolGUI::QmitkCutToolGUI()
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
	QBoxLayout* horizontalLayout7 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout7);

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QPushButton* pbtn_CutVOI = new QPushButton();
	pbtn_CutVOI->setText("Cut VOI");
	horizontalLayout4->addWidget(pbtn_CutVOI);
	connect(pbtn_CutVOI, SIGNAL(clicked()), this, SLOT(CutVOI()));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout5->addWidget(frame1);

	QPushButton* pbtn_SelectCells = new QPushButton();
	pbtn_SelectCells->setText("Select Cells");
	horizontalLayout6->addWidget(pbtn_SelectCells);
	connect(pbtn_SelectCells, SIGNAL(clicked()), this, SLOT(SelectCells()));

	QPushButton* pbtn_DeselectCells = new QPushButton();
	pbtn_DeselectCells->setText("Deselect Cells");
	horizontalLayout6->addWidget(pbtn_DeselectCells);
	connect(pbtn_DeselectCells, SIGNAL(clicked()), this, SLOT(DeselectCells()));

	QPushButton* pbtn_CutSelection = new QPushButton();
	pbtn_CutSelection->setText("Cut Selected Cells");
	horizontalLayout7->addWidget(pbtn_CutSelection);
	connect(pbtn_CutSelection, SIGNAL(clicked()), this, SLOT(CutSelection()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkCutToolGUI::~QmitkCutToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_CutTool.IsNotNull())
	{
		m_CutTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkCutToolGUI, int>( this, &QmitkCutToolGUI::RadiusValueChanged );
		m_CutTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkCutToolGUI, std::vector<std::string>>( this, &QmitkCutToolGUI::InitSurfaceSelection );
		m_CutTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkCutToolGUI, int>(this, &QmitkCutToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkCutToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_CutTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_CutTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_CutTool = dynamic_cast<mitk::CutTool*>( tool );
	m_CutTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkCutToolGUI, int>( this, &QmitkCutToolGUI::RadiusValueChanged );
	m_CutTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkCutToolGUI, std::vector<std::string>>( this, &QmitkCutToolGUI::InitSurfaceSelection );
	m_CutTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkCutToolGUI, int>(this, &QmitkCutToolGUI::UpdateSelectedSurfaceBox);
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkCutToolGUI::SelectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->SelectCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::DeselectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->DeselectCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::CutVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->CutVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkCutToolGUI::CutSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_CutTool->CutSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}




