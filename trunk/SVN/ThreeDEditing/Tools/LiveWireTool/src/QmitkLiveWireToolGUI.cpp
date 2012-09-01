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

#include "QmitkLiveWireToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <QmitkCrossWidget.h>
#include <QProgressBar>

QmitkLiveWireToolGUI::QmitkLiveWireToolGUI()
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

	/*QLabel* label = new QLabel( "Radius", this );
	QFont f = label->font();
	f.setBold(false);
	label->setFont( f );
	m_HorizontalLayout2->addWidget(label);

	m_RadiusSlider = new QSlider(Qt::Horizontal);
	m_RadiusSlider->setSliderPosition(20);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);
	QObject::connect(m_RadiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(RadiusChanged(int)));*/


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

	QPushButton* pbtnInterpolate3D = new QPushButton();
	pbtnInterpolate3D->setText("Interpolate 3D\n by Laplacian Framework");
	pbtnInterpolate3D->setShortcut(QKeySequence("i"));
	horizontalLayout4->addWidget(pbtnInterpolate3D);
	connect(pbtnInterpolate3D, SIGNAL(clicked()), this, SLOT(Interpolate3D()));

	m_ProgressBar = new QProgressBar(this);
	m_ProgressBar->setRange(0,100);
	m_ProgressBar->setValue(100);
	horizontalLayout5->addWidget(m_ProgressBar);

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout6->addWidget(frame1);

	QLabel* label12 = new QLabel( "Zoom:", this );
	QFont f12 = label12->font();
	f12.setBold(false);
	//f12.setPointSize(6);
	label12->setFont( f12 );
	horizontalLayout7->addWidget(label12);
	QmitkCrossWidget* zoomCross = new QmitkCrossWidget();
	QPixmap pixZoom(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	zoomCross->setPixmap(pixZoom.scaledToWidth(100));
	zoomCross->setFixedSize(100,20);
	zoomCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout7->addWidget(zoomCross);
	connect( zoomCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( ZoomCross( int, int ) ) );

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkLiveWireToolGUI::~QmitkLiveWireToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_LiveWireTool.IsNotNull())
	{
		m_LiveWireTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::RadiusValueChanged );
		m_LiveWireTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkLiveWireToolGUI, std::vector<std::string>>( this, &QmitkLiveWireToolGUI::InitSurfaceSelection );
		m_LiveWireTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>(this, &QmitkLiveWireToolGUI::UpdateSelectedSurfaceBox);

		m_LiveWireTool->UpdateRadiusSlider2 -= mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::RadiusValueChanged2 );
		m_LiveWireTool->UpdateProgressBar -= mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::UpdateProgressBar );
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkLiveWireToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_LiveWireTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_LiveWireTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_LiveWireTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_LiveWireTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_LiveWireTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_LiveWireTool = dynamic_cast<mitk::LiveWireTool*>( tool );

	m_LiveWireTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::RadiusValueChanged );
	m_LiveWireTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkLiveWireToolGUI, std::vector<std::string>>( this, &QmitkLiveWireToolGUI::InitSurfaceSelection );
	m_LiveWireTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>(this, &QmitkLiveWireToolGUI::UpdateSelectedSurfaceBox);

	m_LiveWireTool->UpdateRadiusSlider2 += mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::RadiusValueChanged2 );
	m_LiveWireTool->UpdateProgressBar += mitk::MessageDelegate1<QmitkLiveWireToolGUI, int>( this, &QmitkLiveWireToolGUI::UpdateProgressBar );
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
//	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
//	m_RadiusSlider->setSliderPosition(currentRadius);
//	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkLiveWireToolGUI::RadiusChanged2(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_LiveWireTool->ROIRadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::RadiusValueChanged2(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider2);
	m_RadiusSlider2->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider2);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::Interpolate3D()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_LiveWireTool->Interpolate3D();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::ZoomCross(int zoom, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_LiveWireTool->Zoom(zoom);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkLiveWireToolGUI::UpdateProgressBar(int value)
{
	m_ProgressBar->setValue(value);
}