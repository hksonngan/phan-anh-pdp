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

#include "QmitkDragToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <QmitkCrossWidget.h>

QmitkDragToolGUI::QmitkDragToolGUI()
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

	//QLabel* label = new QLabel( "Radius", this );
	//QFont f = label->font();
	//f.setBold(false);
	//label->setFont( f );
	//m_HorizontalLayout2->addWidget(label);

	//m_RadiusSlider = new QSlider(Qt::Horizontal);
	//m_RadiusSlider->setSliderPosition(20);
	//m_HorizontalLayout2->addWidget(m_RadiusSlider);
	//QObject::connect(m_RadiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(RadiusChanged(int)));


	// Tool Specific
	QBoxLayout* horizontalLayout3 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout3);
	QBoxLayout* horizontalLayout4 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout4);

	m_HorizontalLayout3 = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout3);

	QBoxLayout* horizontalLayout5 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout5);
	QBoxLayout* horizontalLayout6 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout6);
	QBoxLayout* horizontalLayout7 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout7);
	QBoxLayout* horizontalLayout8 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout8);

	QLabel* label = new QLabel( "Radius", this );
	QFont f = label->font();
	f.setBold(false);
	label->setFont( f );
	m_HorizontalLayout2->addWidget(label);
	m_RadiusSlider2 = new QSlider(Qt::Horizontal);
	m_RadiusSlider2->setMaximum(300); 
	m_RadiusSlider2->setSliderPosition(14);
	m_HorizontalLayout2->addWidget(m_RadiusSlider2);
	QObject::connect(m_RadiusSlider2, SIGNAL(sliderMoved(int)), this, SLOT(RadiusChanged2(int)));

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QLabel* label2 = new QLabel( "Select weighting of neighbouring\nvertices:", this );
	QFont f2 = label2->font();
	f2.setBold(true);
	label2->setFont( f2 );
	horizontalLayout4->addWidget(label2);

	m_SelectWeightBox = new QComboBox();
	m_HorizontalLayout3->addWidget(m_SelectWeightBox);
	connect(m_SelectWeightBox, SIGNAL(activated(int)), this, SLOT(SelectWeight(int)));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout5->addWidget(frame1);

	QPushButton* pbtn_SetColors = new QPushButton();
	pbtn_SetColors->setText("Highlight ROI");
	pbtn_SetColors->setCheckable(true);
	horizontalLayout6->addWidget(pbtn_SetColors);
	connect(pbtn_SetColors, SIGNAL(clicked()), this, SLOT(SetColors()));

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout7->addWidget(frame2);

	QLabel* label12 = new QLabel( "Zoom:", this );
	QFont f12 = label12->font();
	f12.setBold(false);
	//f12.setPointSize(6);
	label12->setFont( f12 );
	horizontalLayout8->addWidget(label12);
	QmitkCrossWidget* zoomCross = new QmitkCrossWidget();
	QPixmap pixZoom(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	zoomCross->setPixmap(pixZoom.scaledToWidth(100));
	zoomCross->setFixedSize(100,20);
	zoomCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout8->addWidget(zoomCross);
	connect( zoomCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( ZoomCross( int, int ) ) );

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkDragToolGUI::~QmitkDragToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_DragTool.IsNotNull())
	{
		m_DragTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkDragToolGUI, int>( this, &QmitkDragToolGUI::RadiusValueChanged );
		m_DragTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkDragToolGUI, std::vector<std::string>>( this, &QmitkDragToolGUI::InitSurfaceSelection );
		m_DragTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkDragToolGUI, int>(this, &QmitkDragToolGUI::UpdateSelectedSurfaceBox);

		m_DragTool->UpdateRadiusSlider2 -= mitk::MessageDelegate1<QmitkDragToolGUI, int>( this, &QmitkDragToolGUI::RadiusValueChanged2 );
		m_DragTool->GetCurrentWeights -= mitk::MessageDelegate1<QmitkDragToolGUI, std::vector<std::string>>( this, &QmitkDragToolGUI::InitWeightSelection );
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkDragToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_DragTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DragTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DragTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DragTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_DragTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_DragTool = dynamic_cast<mitk::DragTool*>( tool );

	m_DragTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkDragToolGUI, int>( this, &QmitkDragToolGUI::RadiusValueChanged );
	m_DragTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkDragToolGUI, std::vector<std::string>>( this, &QmitkDragToolGUI::InitSurfaceSelection );
	m_DragTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkDragToolGUI, int>(this, &QmitkDragToolGUI::UpdateSelectedSurfaceBox);
	
	m_DragTool->UpdateRadiusSlider2 += mitk::MessageDelegate1<QmitkDragToolGUI, int>( this, &QmitkDragToolGUI::RadiusValueChanged2 );
	m_DragTool->GetCurrentWeights += mitk::MessageDelegate1<QmitkDragToolGUI, std::vector<std::string>>( this, &QmitkDragToolGUI::InitWeightSelection );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	//m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	//m_RadiusSlider->setSliderPosition(currentRadius);
	//m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkDragToolGUI::RadiusChanged2(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_DragTool->ROIRadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::RadiusValueChanged2(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider2);
	m_RadiusSlider2->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider2);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::InitWeightSelection(std::vector<std::string> currentWeights)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//m_HorizontalLayout3->removeWidget(m_PbtnJumpToPosition);
	m_SelectWeightBox->clear();
	for(std::vector<std::string>::iterator it = currentWeights.begin(); it != currentWeights.end(); it++)
	{
		m_SelectWeightBox->addItem(it->c_str());
	}
	//m_HorizontalLayout3->addWidget(m_PbtnJumpToPosition);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::SelectWeight(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_DragTool->SelectWeight(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::SetColors()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DragTool->SetColors();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkDragToolGUI::ZoomCross(int zoom, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_DragTool->Zoom(zoom);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}