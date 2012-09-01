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

#include "QmitkSurfaceRefinementToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkSurfaceRefinementToolGUI::QmitkSurfaceRefinementToolGUI()
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

	QLabel* labelRadius = new QLabel( "Radius", this );
	QFont fondRadius = labelRadius->font();
	fondRadius.setBold(false);
	labelRadius->setFont( fondRadius );
	m_HorizontalLayout2->addWidget(labelRadius);

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
	QBoxLayout* horizontalLayout8 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout8);
	QBoxLayout* horizontalLayout9 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout9);
	QBoxLayout* horizontalLayout10 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout10);
	QBoxLayout* horizontalLayout11 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout11);
	QBoxLayout* horizontalLayout12 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout12);
	QBoxLayout* horizontalLayout13 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout13);

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	m_PbtnSelectCells = new QPushButton();
	m_PbtnSelectCells->setCheckable(true);
	m_PbtnSelectCells->setDown(false);
	m_PbtnSelectCells->setText("Select Cells");
	horizontalLayout4->addWidget(m_PbtnSelectCells);
	connect(m_PbtnSelectCells, SIGNAL(clicked()), this, SLOT(SelectCells()));

	m_PbtnDeselectCells = new QPushButton();
	m_PbtnDeselectCells->setCheckable(true);
	m_PbtnDeselectCells->setText("Deselect Cells");
	horizontalLayout4->addWidget(m_PbtnDeselectCells);
	connect(m_PbtnDeselectCells, SIGNAL(clicked()), this, SLOT(DeselectCells()));

	m_PbtnSelectAllCells = new QPushButton();
	m_PbtnSelectAllCells->setText("Select All Cells");
	horizontalLayout5->addWidget(m_PbtnSelectAllCells);
	connect(m_PbtnSelectAllCells, SIGNAL(clicked()), this, SLOT(SelectAllCells()));

	m_PbtnDeselectAllCells = new QPushButton();
	m_PbtnDeselectAllCells->setText("Deselect All Cells");
	horizontalLayout5->addWidget(m_PbtnDeselectAllCells);
	connect(m_PbtnDeselectAllCells, SIGNAL(clicked()), this, SLOT(DeselectAllCells()));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout6->addWidget(frame1);

	QLabel* labelMethode = new QLabel( "Subdivide region of instrest:", this );
	QFont fontMethode = labelMethode->font();
	fontMethode.setBold(true);
	labelMethode->setFont( fontMethode );
	horizontalLayout7->addWidget(labelMethode);

	m_SelectRefinementMethodeBoxROI = new QComboBox();
	m_SelectRefinementMethodeBoxROI->addItem("Insert Point(s)");
	m_SelectRefinementMethodeBoxROI->addItem("Subdivide Edge(s)");
	horizontalLayout8->addWidget(m_SelectRefinementMethodeBoxROI);
	connect(m_SelectRefinementMethodeBoxROI, SIGNAL(activated(int)), this, SLOT(SelectRefinementMethodeROI(int)));

	QPushButton* pbtn_SubdivideSurfaceVOI = new QPushButton();
	pbtn_SubdivideSurfaceVOI->setText("Subdivide VOI");
	horizontalLayout9->addWidget(pbtn_SubdivideSurfaceVOI);
	connect(pbtn_SubdivideSurfaceVOI, SIGNAL(clicked()), this, SLOT(SubdivideSurfaceVOI()));

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout10->addWidget(frame2);

	QLabel* labelMethode2 = new QLabel( "Subdivide total mesh:", this );
	QFont fontMethode2 = labelMethode2->font();
	//fontMethode2.setBold(false);
	fontMethode2.setBold(true);
	labelMethode2->setFont( fontMethode2 );
	horizontalLayout11->addWidget(labelMethode2);

	m_SelectRefinementMethodeBox = new QComboBox();
	m_SelectRefinementMethodeBox->addItem("LinearSubdivision");
	m_SelectRefinementMethodeBox->addItem("LoopSubdivision");
	m_SelectRefinementMethodeBox->addItem("ButterflySubdivision");
	horizontalLayout12->addWidget(m_SelectRefinementMethodeBox);
	connect(m_SelectRefinementMethodeBox, SIGNAL(activated(int)), this, SLOT(SelectRefinementMethode(int)));

	QPushButton* pbtn_SubdivideSurface = new QPushButton();
	pbtn_SubdivideSurface->setText("Subdivide Surface");
	horizontalLayout13->addWidget(pbtn_SubdivideSurface);
	connect(pbtn_SubdivideSurface, SIGNAL(clicked()), this, SLOT(SubdivideSurface()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkSurfaceRefinementToolGUI::~QmitkSurfaceRefinementToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_SurfaceRefinementTool.IsNotNull())
	{
		m_SurfaceRefinementTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, int>( this, &QmitkSurfaceRefinementToolGUI::RadiusValueChanged );
		m_SurfaceRefinementTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, std::vector<std::string>>( this, &QmitkSurfaceRefinementToolGUI::InitSurfaceSelection );
		m_SurfaceRefinementTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, int>(this, &QmitkSurfaceRefinementToolGUI::UpdateSelectedSurfaceBox);
			m_SurfaceRefinementTool->ToogleSelectCellsButton -= mitk::MessageDelegate<QmitkSurfaceRefinementToolGUI>(this, &QmitkSurfaceRefinementToolGUI::ToogleSelectButton);
	m_SurfaceRefinementTool->ToogleDeselectCellsButton -= mitk::MessageDelegate<QmitkSurfaceRefinementToolGUI>(this, &QmitkSurfaceRefinementToolGUI::ToogleDeselectButton);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkSurfaceRefinementToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_SurfaceRefinementTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_SurfaceRefinementTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_SurfaceRefinementTool = dynamic_cast<mitk::SurfaceRefinementTool*>( tool );
	m_SurfaceRefinementTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, int>( this, &QmitkSurfaceRefinementToolGUI::RadiusValueChanged );
	m_SurfaceRefinementTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, std::vector<std::string>>( this, &QmitkSurfaceRefinementToolGUI::InitSurfaceSelection );
	m_SurfaceRefinementTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkSurfaceRefinementToolGUI, int>(this, &QmitkSurfaceRefinementToolGUI::UpdateSelectedSurfaceBox);
	m_SurfaceRefinementTool->ToogleSelectCellsButton += mitk::MessageDelegate<QmitkSurfaceRefinementToolGUI>(this, &QmitkSurfaceRefinementToolGUI::ToogleSelectButton);
	m_SurfaceRefinementTool->ToogleDeselectCellsButton += mitk::MessageDelegate<QmitkSurfaceRefinementToolGUI>(this, &QmitkSurfaceRefinementToolGUI::ToogleDeselectButton);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkSurfaceRefinementToolGUI::SelectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_PbtnDeselectCells->setChecked(false);
	m_SurfaceRefinementTool->SelectCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::DeselectCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_PbtnSelectCells->setChecked(false);
	m_SurfaceRefinementTool->DeselectCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SelectAllCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->SelectAllCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::DeselectAllCells()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->DeselectAllCells();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SelectRefinementMethode(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_SurfaceRefinementTool->SelectRefinementMethode(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SelectRefinementMethodeROI(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_SurfaceRefinementTool->SelectRefinementMethodeROI(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SubdivideSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->SubdivideSurface();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::SubdivideSurfaceVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SurfaceRefinementTool->SubdivideSurfaceVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::ToogleSelectButton()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_PbtnSelectCells->setChecked(true);
	m_PbtnDeselectCells->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSurfaceRefinementToolGUI::ToogleDeselectButton()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_PbtnDeselectCells->setChecked(true);
	m_PbtnSelectCells->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
