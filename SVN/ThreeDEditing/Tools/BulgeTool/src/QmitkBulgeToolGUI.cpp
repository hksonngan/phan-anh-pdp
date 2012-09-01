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

#include "QmitkBulgeToolGUI.h"

#include <QIcon>
#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include "mousepointer.xpm"
#include "mitkBulgeTool.xpm"

QmitkBulgeToolGUI::QmitkBulgeToolGUI()
:QmitkToolGUI()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
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
	QBoxLayout* horizontalLayout8 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout8);
	QBoxLayout* horizontalLayout9 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout9);
	QBoxLayout* horizontalLayout10 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout10);
	QBoxLayout* horizontalLayout11 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout11);

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QLabel* label2 = new QLabel( "Select move points technique:", this );
	QFont f2 = label2->font();
	f2.setBold(false);
	label->setFont( f2 );
	horizontalLayout4->addWidget(label2);

	m_pbtn_Collinear = new QPushButton();
	m_pbtn_Collinear->setText("Collinear");
	m_pbtn_Collinear->setToolTip("Move points in the same direction as the mouse moves.");
	m_pbtn_Collinear->setCheckable(true);
	m_pbtn_Collinear->setChecked(true);
	horizontalLayout5->addWidget(m_pbtn_Collinear);
	connect(m_pbtn_Collinear, SIGNAL(clicked()), this, SLOT(Collinear()));

	m_pbtn_Radial = new QPushButton();
	m_pbtn_Radial->setText("Radial");
	m_pbtn_Radial->setToolTip("Move points outside radial from the origin of the tool.");
	m_pbtn_Radial->setCheckable(true);
	horizontalLayout5->addWidget(m_pbtn_Radial);
	connect(m_pbtn_Radial, SIGNAL(clicked()), this, SLOT(Radial()));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout6->addWidget(frame1);

	QLabel* label3 = new QLabel( "The tool can add further points while bulging.", this );
	QFont f3 = label3->font();
	f3.setBold(false);
	label3->setFont( f );
	horizontalLayout7->addWidget(label3);

	m_SelectRefinementMethodeBox = new QComboBox();
	m_SelectRefinementMethodeBox->addItem("Off. Don't add points");
	m_SelectRefinementMethodeBox->addItem("Automatic");
	m_SelectRefinementMethodeBox->addItem("Insert Point(s)");
	m_SelectRefinementMethodeBox->addItem("Subdivide Edge(s)");
	horizontalLayout8->addWidget(m_SelectRefinementMethodeBox);
	connect(m_SelectRefinementMethodeBox, SIGNAL(activated(int)), this, SLOT(SelectRefinementMethode(int)));

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout9->addWidget(frame2);

	QPushButton* pbtn_ShowTool = new QPushButton();
	pbtn_ShowTool->setText("Show Bulge Tool");
	pbtn_ShowTool->setCheckable(true);
	horizontalLayout10->addWidget(pbtn_ShowTool);
	connect(pbtn_ShowTool, SIGNAL(clicked()), this, SLOT(ShowTool()));

	QPushButton* pbtnReinitNow = new QPushButton();
	pbtnReinitNow->setText("Reinit Now");
	horizontalLayout11->addWidget(pbtnReinitNow);
	connect(pbtnReinitNow, SIGNAL(clicked()), this, SLOT(ReinitNow()));

	connect(this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)));
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkBulgeToolGUI::~QmitkBulgeToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_BulgeTool.IsNotNull())
	{
		m_BulgeTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkBulgeToolGUI, int>(this, &QmitkBulgeToolGUI::RadiusValueChanged);
		m_BulgeTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkBulgeToolGUI, std::vector<std::string>>(this, &QmitkBulgeToolGUI::InitSurfaceSelection);
		m_BulgeTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkBulgeToolGUI, int>(this, &QmitkBulgeToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkBulgeToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_BulgeTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_BulgeTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_BulgeTool = dynamic_cast<mitk::BulgeTool*>( tool );

	m_BulgeTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkBulgeToolGUI, int>(this, &QmitkBulgeToolGUI::RadiusValueChanged);
	m_BulgeTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkBulgeToolGUI, std::vector<std::string>>(this, &QmitkBulgeToolGUI::InitSurfaceSelection);
	m_BulgeTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkBulgeToolGUI, int>(this, &QmitkBulgeToolGUI::UpdateSelectedSurfaceBox);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

// Tool Specific ********************************************************************************
void QmitkBulgeToolGUI::ShowTool()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->ShowTool();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::Collinear()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->Collinear();
	m_pbtn_Radial->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::Radial()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->Radial();
	m_pbtn_Collinear->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::SelectRefinementMethode(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->SetRefinementTechnique(selection);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkBulgeToolGUI::ReinitNow()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_BulgeTool->ReinitNow();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}