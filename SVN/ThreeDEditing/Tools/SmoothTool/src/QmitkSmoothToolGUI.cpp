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

#include "QmitkSmoothToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkSmoothToolGUI::QmitkSmoothToolGUI()
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

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QLabel* label1 = new QLabel( "Region-Of-Intrest based tools", this );
	QFont f1 = label1->font();
	f1.setBold(true);
	label1->setFont( f1 );
	horizontalLayout4->addWidget(label1);

	QPushButton* pbtn_SmoothVOI = new QPushButton();
	pbtn_SmoothVOI->setText("Smooth VOI");
	horizontalLayout5->addWidget(pbtn_SmoothVOI);
	connect(pbtn_SmoothVOI, SIGNAL(clicked()), this, SLOT(SmoothVOI()));

	QPushButton* pbtn_DecimateVOI = new QPushButton();
	pbtn_DecimateVOI->setText("Decimate VOI");
	horizontalLayout5->addWidget(pbtn_DecimateVOI);
	connect(pbtn_DecimateVOI, SIGNAL(clicked()), this, SLOT(DecimateVOI()));

	QPushButton* pbtn_TriangulateVOI = new QPushButton();
	pbtn_TriangulateVOI->setText("Triangulate VOI");
	horizontalLayout6->addWidget(pbtn_TriangulateVOI);
	connect(pbtn_TriangulateVOI, SIGNAL(clicked()), this, SLOT(TriangulateVOI()));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout7->addWidget(frame1);

	QLabel* label2 = new QLabel( "Tools working on total mesh", this );
	QFont f2 = label2->font();
	f2.setBold(true);
	label2->setFont( f2 );
	horizontalLayout8->addWidget(label2);

	QPushButton* pbtn_SmoothSurface = new QPushButton();
	pbtn_SmoothSurface->setText("Smooth Surface");
	horizontalLayout9->addWidget(pbtn_SmoothSurface);
	connect(pbtn_SmoothSurface, SIGNAL(clicked()), this, SLOT(SmoothSurface()));

	QPushButton* pbtn_DecimateSurface = new QPushButton();
	pbtn_DecimateSurface->setText("Decimate Surface");
	horizontalLayout9->addWidget(pbtn_DecimateSurface);
	connect(pbtn_DecimateSurface, SIGNAL(clicked()), this, SLOT(DecimateSurface()));

	QPushButton* pbtn_TriangulateSurface = new QPushButton();
	pbtn_TriangulateSurface->setText("Triangulate Surface");
	horizontalLayout10->addWidget(pbtn_TriangulateSurface);
	connect(pbtn_TriangulateSurface, SIGNAL(clicked()), this, SLOT(TriangulateSurface()));

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout11->addWidget(frame2);

	QLabel* label3 = new QLabel( "Smooth Iterations", this );
	QFont f3 = label3->font();
	f3.setBold(false);
	label3->setFont( f3 );
	horizontalLayout12->addWidget(label3);

	QSlider* smoothSlider = new QSlider(Qt::Horizontal);
	smoothSlider->setSliderPosition(5);
	smoothSlider->setRange(0,30);
	horizontalLayout12->addWidget(smoothSlider);
	QObject::connect(smoothSlider, SIGNAL(sliderMoved(int)), this, SLOT(SmoothChanged(int)));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkSmoothToolGUI::~QmitkSmoothToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_SmoothTool.IsNotNull())
	{
		m_SmoothTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkSmoothToolGUI, int>( this, &QmitkSmoothToolGUI::RadiusValueChanged );
		m_SmoothTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkSmoothToolGUI, std::vector<std::string>>( this, &QmitkSmoothToolGUI::InitSurfaceSelection );
		m_SmoothTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkSmoothToolGUI, int>(this, &QmitkSmoothToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkSmoothToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_SmoothTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_SmoothTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_SmoothTool = dynamic_cast<mitk::SmoothTool*>( tool );
	m_SmoothTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkSmoothToolGUI, int>( this, &QmitkSmoothToolGUI::RadiusValueChanged );
	m_SmoothTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkSmoothToolGUI, std::vector<std::string>>( this, &QmitkSmoothToolGUI::InitSurfaceSelection );
	m_SmoothTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkSmoothToolGUI, int>(this, &QmitkSmoothToolGUI::UpdateSelectedSurfaceBox);
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

// Tool Specific ********************************************************************************


void QmitkSmoothToolGUI::SmoothVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->SmoothVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::DecimateVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->DecimateVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::TriangulateVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->TriangulateVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::SmoothSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->SmoothSurface();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::TriangulateSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->TriangulateSurface();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::DecimateSurface()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_SmoothTool->DecimateSurface();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkSmoothToolGUI::SmoothChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Smooth Iteration: " << value << "\n";
	m_SmoothTool->SmoothChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
