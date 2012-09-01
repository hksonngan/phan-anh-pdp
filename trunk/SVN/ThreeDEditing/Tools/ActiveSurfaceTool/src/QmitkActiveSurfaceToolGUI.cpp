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

#include "QmitkActiveSurfaceToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <QSpinBox>
#include <QLineEdit>
#include <QDoubleValidator>

#include <QmitkCrossWidget.h>

QmitkActiveSurfaceToolGUI::QmitkActiveSurfaceToolGUI()
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
	QBoxLayout* horizontalLayout13 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout13);
	QBoxLayout* horizontalLayout14 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout14);

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QLabel* label2 = new QLabel( "Select feature image:", this );
	QFont f2 = label2->font();
	f2.setBold(false);
	label2->setFont( f2 );
	horizontalLayout4->addWidget(label2);

	m_SelectImageBox = new QComboBox();
	horizontalLayout5->addWidget(m_SelectImageBox);
	connect(m_SelectImageBox, SIGNAL(activated(int)), this, SLOT(SelectImage(int)));

	QLabel* label3 = new QLabel( "Set seed point bubble\n(left click):\nRemove bubble\n(shift + left click)", this );
	QFont f3 = label3->font();
	f3.setPointSize(7);
	f3.setBold(false);
	label3->setFont( f3 );
	horizontalLayout6->addWidget(label3);

	QPushButton* pbtn_RemoveBubble = new QPushButton();
	pbtn_RemoveBubble->setText("Remove Bubble");
	horizontalLayout6->addWidget(pbtn_RemoveBubble);
	connect(pbtn_RemoveBubble, SIGNAL(clicked()), this, SLOT(RemoveBubble()));

	m_VCR = new std::vector<QPushButton*>;
	int size = 24;

	QPushButton* pbtn_Stop = new QPushButton();
	QIcon icn_Stop;
	icn_Stop.addFile(":/threeDEditing/res/threeDEditing/Backward_01.png");
	pbtn_Stop->setIcon(icn_Stop);
	pbtn_Stop->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_Stop);	
	connect(pbtn_Stop, SIGNAL(clicked()), this, SLOT(Stop()));

	QPushButton* pbtn_StepBack = new QPushButton();
	QIcon icn_StepBack;
	icn_StepBack.addFile(":/threeDEditing/res/threeDEditing/left.png");
	pbtn_StepBack->setIcon(icn_StepBack);
	pbtn_StepBack->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_StepBack);	
	connect(pbtn_StepBack, SIGNAL(clicked()), this, SLOT(StepBack()));

	QPushButton* pbtn_ReversePlay = new QPushButton();
	m_VCR->push_back(pbtn_ReversePlay);
	QIcon icn_ReversePlay;
	icn_ReversePlay.addFile(":/threeDEditing/res/threeDEditing/Backward.png");
	pbtn_ReversePlay->setIcon(icn_ReversePlay);
	pbtn_ReversePlay->setCheckable(true);
	pbtn_ReversePlay->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_ReversePlay);	
	connect(pbtn_ReversePlay, SIGNAL(clicked()), this, SLOT(ReversePlay()));

	QPushButton* pbtn_Pause = new QPushButton();
	m_VCR->push_back(pbtn_Pause);
	QIcon icn_Pause;
	icn_Pause.addFile(":/threeDEditing/res/threeDEditing/pause.png");
	pbtn_Pause->setIcon(icn_Pause);
	pbtn_Pause->setCheckable(true);
	pbtn_Pause->setChecked(true);
	pbtn_Pause->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_Pause);	
	connect(pbtn_Pause, SIGNAL(clicked()), this, SLOT(Pause()));

	QPushButton* pbtn_Play = new QPushButton();
	m_VCR->push_back(pbtn_Play);
	QIcon icn_Play;
	icn_Play.addFile(":/threeDEditing/res/threeDEditing/Forward.png");
	pbtn_Play->setIcon(icn_Play);
	pbtn_Play->setCheckable(true);
	pbtn_Play->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_Play);	
	connect(pbtn_Play, SIGNAL(clicked()), this, SLOT(Play()));

	QPushButton* pbtn_StepForward = new QPushButton();
	QIcon icn_StepForward;
	icn_StepForward.addFile(":/threeDEditing/res/threeDEditing/right.png");
	pbtn_StepForward->setIcon(icn_StepForward);
	pbtn_StepForward->setIconSize(QSize(size,size));
	horizontalLayout7->addWidget(pbtn_StepForward);	
	connect(pbtn_StepForward, SIGNAL(clicked()), this, SLOT(StepForward()));

	QLabel* label5 = new QLabel( "Step size:", this );
	QFont f5 = label5->font();
	f5.setBold(false);
	label5->setFont( f5 );
	horizontalLayout8->addWidget(label5);
	QSpinBox* selectStepSize = new QSpinBox();
	selectStepSize->setRange(1, 50);
	selectStepSize->setValue(1);
	horizontalLayout8->addWidget(selectStepSize);
	connect(selectStepSize, SIGNAL(valueChanged(int)), this, SLOT(SelectStepSize(int)));

	QLabel* label6 = new QLabel( "Iteration:", this );
	QFont f6 = label6->font();
	f6.setBold(false);
	label6->setFont( f6 );
	horizontalLayout8->addWidget(label6);
	numberOfIterations = new QLineEdit(this);
	numberOfIterations->setText("0");
	numberOfIterations->setReadOnly(true);
	horizontalLayout8->addWidget(numberOfIterations);

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout9->addWidget(frame1);

	// Parameters
	QLabel* label7 = new QLabel( "Parameters:", this );
	QFont f7 = label7->font();
	f7.setBold(true);
	label7->setFont( f7 );
	horizontalLayout10->addWidget(label7);


	// make the "InputMask" for values between 1.11 and 17.93 with 2 digits after the comma
	QValidator *inputRange = new QDoubleValidator(0.000, 100000.000, 3, this);

	// table layout
	QBoxLayout* verticalLayout1 = new QVBoxLayout();
	horizontalLayout11->addLayout(verticalLayout1);
	QBoxLayout* verticalLayout2 = new QVBoxLayout();
	horizontalLayout11->addLayout(verticalLayout2);
	QBoxLayout* verticalLayout3 = new QVBoxLayout();
	horizontalLayout11->addLayout(verticalLayout3);

	QLabel* label8 = new QLabel( "Curvature\nForce:", this );
	QFont f8 = label8->font();
	f8.setBold(false);
	f8.setPointSize(7);
	label8->setFont( f8 );
	verticalLayout1->addWidget(label8);
	m_CurvatureForce = new QLineEdit(this);
	m_CurvatureForce->setText("0");
	m_CurvatureForce->setReadOnly(false);
	m_CurvatureForce->setValidator(inputRange);
	QObject::connect(m_CurvatureForce, SIGNAL(returnPressed()), this, SLOT(EditedCurvatureForce()));
	verticalLayout2->addWidget(m_CurvatureForce);
	QmitkCrossWidget* curvatureForceCross = new QmitkCrossWidget();
	QPixmap pixCurvatureForce(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	curvatureForceCross->setPixmap(pixCurvatureForce.scaledToWidth(100));
	curvatureForceCross->setFixedSize(100,20);
	curvatureForceCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	verticalLayout3->addWidget(curvatureForceCross);
	connect( curvatureForceCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CurvatureForceCross( int, int ) ) );

	QLabel* label10 = new QLabel( "Area\nForce:", this );
	QFont f10 = label10->font();
	f10.setBold(false);
	f10.setPointSize(7);
	label10->setFont( f10 );
	verticalLayout1->addWidget(label10);
	m_AreaForce = new QLineEdit(this);
	m_AreaForce->setText("0");
	m_AreaForce->setReadOnly(false);
	m_AreaForce->setValidator(inputRange);
	QObject::connect(m_AreaForce, SIGNAL(returnPressed()), this, SLOT(EditedAreaForce()));
	verticalLayout2->addWidget(m_AreaForce);
	QmitkCrossWidget* areaForceCross = new QmitkCrossWidget();
	QPixmap pixAreaForce(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	areaForceCross->setPixmap(pixAreaForce.scaledToWidth(100));
	areaForceCross->setFixedSize(100,20);
	areaForceCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	verticalLayout3->addWidget(areaForceCross);
	connect( areaForceCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( AreaForceCross( int, int ) ) );

	QLabel* label11 = new QLabel( "Volume\nForce:", this );
	QFont f11 = label11->font();
	f11.setBold(false);
	f11.setPointSize(7);
	label11->setFont( f11 );
	verticalLayout1->addWidget(label11);
	m_VolumeForce = new QLineEdit(this);
	m_VolumeForce->setText("0");
	m_VolumeForce->setReadOnly(false);
	m_VolumeForce->setValidator(inputRange);
	QObject::connect(m_VolumeForce, SIGNAL(returnPressed()), this, SLOT(EditedVolumeForce()));
	verticalLayout2->addWidget(m_VolumeForce);
	QmitkCrossWidget* volumeForceCross = new QmitkCrossWidget();
	QPixmap pixVolumeForce(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	volumeForceCross->setPixmap(pixVolumeForce.scaledToWidth(100));
	volumeForceCross->setFixedSize(100,20);
	volumeForceCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	verticalLayout3->addWidget(volumeForceCross);
	connect( volumeForceCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( VolumeForceCross( int, int ) ) );

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout12->addWidget(frame2);

	QPushButton* pbtnClosing = new QPushButton();
	pbtnClosing->setText("Closing by one pixel");
	pbtnClosing->setCheckable(true);
	horizontalLayout13->addWidget(pbtnClosing);
	connect(pbtnClosing, SIGNAL(clicked()), this, SLOT(Closing()));

	QLabel* label12 = new QLabel( "Zoom:", this );
	QFont f12 = label12->font();
	f12.setBold(false);
	//f12.setPointSize(6);
	label12->setFont( f12 );
	horizontalLayout14->addWidget(label12);
	QmitkCrossWidget* zoomCross = new QmitkCrossWidget();
	QPixmap pixZoom(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	zoomCross->setPixmap(pixZoom.scaledToWidth(100));
	zoomCross->setFixedSize(100,20);
	zoomCross->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout14->addWidget(zoomCross);
	connect( zoomCross, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( ZoomCross( int, int ) ) );

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkActiveSurfaceToolGUI::~QmitkActiveSurfaceToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_ActiveSurfaceTool.IsNotNull())
	{
		m_ActiveSurfaceTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>( this, &QmitkActiveSurfaceToolGUI::RadiusValueChanged );
		m_ActiveSurfaceTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, std::vector<std::string>>( this, &QmitkActiveSurfaceToolGUI::InitSurfaceSelection );
		m_ActiveSurfaceTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::UpdateSelectedSurfaceBox);
		m_ActiveSurfaceTool->SetNumberOfIterations -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::UpdateNumberOfIterations);
		m_ActiveSurfaceTool->GetCurrentImages -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, std::vector<std::string>>( this, &QmitkActiveSurfaceToolGUI::InitImageSelection );
		m_ActiveSurfaceTool->SetCurvatureForce -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateCurvatureForce);
		m_ActiveSurfaceTool->SetAreaForce -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateAreaForce);
		m_ActiveSurfaceTool->SetVolumeForce -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateVolumeForce);
		m_ActiveSurfaceTool->SetButtonsToPause -= mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::SetButtonsToPause);
	}
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkActiveSurfaceToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_ActiveSurfaceTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_ActiveSurfaceTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_ActiveSurfaceTool = dynamic_cast<mitk::ActiveSurfaceTool*>( tool );

	m_ActiveSurfaceTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>( this, &QmitkActiveSurfaceToolGUI::RadiusValueChanged );
	m_ActiveSurfaceTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, std::vector<std::string>>( this, &QmitkActiveSurfaceToolGUI::InitSurfaceSelection );
	m_ActiveSurfaceTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::UpdateSelectedSurfaceBox);
	m_ActiveSurfaceTool->SetNumberOfIterations += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::UpdateNumberOfIterations);
	m_ActiveSurfaceTool->GetCurrentImages += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, std::vector<std::string>>( this, &QmitkActiveSurfaceToolGUI::InitImageSelection );
	m_ActiveSurfaceTool->SetCurvatureForce += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateCurvatureForce);
	m_ActiveSurfaceTool->SetAreaForce += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateAreaForce);
	m_ActiveSurfaceTool->SetVolumeForce += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, double>(this, &QmitkActiveSurfaceToolGUI::UpdateVolumeForce);
	m_ActiveSurfaceTool->SetButtonsToPause += mitk::MessageDelegate1<QmitkActiveSurfaceToolGUI, int>(this, &QmitkActiveSurfaceToolGUI::SetButtonsToPause);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkActiveSurfaceToolGUI::InitImageSelection(std::vector<std::string> currentImages)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//m_HorizontalLayout1->removeWidget(m_PbtnJumpToPosition);
	m_SelectImageBox->clear();
	for(std::vector<std::string>::iterator it = currentImages.begin(); it != currentImages.end(); it++)
	{
		m_SelectImageBox->addItem(it->c_str());
	}
	//m_HorizontalLayout1->addWidget(m_PbtnJumpToPosition);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::SelectImage(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_ActiveSurfaceTool->SelectImage(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateSelectedImageBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectImageBox->setCurrentIndex(selection);
	SelectImage(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::RemoveBubble()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->RemoveBubble();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::Stop()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(true);
	m_VCR->at(2)->setChecked(false);
	m_ActiveSurfaceTool->Stop();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::StepBack()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	m_ActiveSurfaceTool->StepBack();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::ReversePlay()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	
	m_ActiveSurfaceTool->ReversePlay();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::Pause()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(true);
	m_VCR->at(2)->setChecked(false);
	m_ActiveSurfaceTool->Pause();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::Play()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(true);
	m_ActiveSurfaceTool->Play();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::StepForward()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(true);
	m_VCR->at(2)->setChecked(false);
	m_ActiveSurfaceTool->StepForward();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::SelectStepSize(int stepSize)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->SelectStepSize(stepSize);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateNumberOfIterations(int iterations)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	QString text = QString("%1").arg(iterations);
	//text.arg(iterations);
	numberOfIterations->setText(text);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

// Parameters
void QmitkActiveSurfaceToolGUI::CurvatureForceCross(int force,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->CurvatureForceCross(force,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateCurvatureForce(double curvatureForce)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = QString("%1").arg(curvatureForce);
	//text.arg(iterations);
	m_CurvatureForce->setText(text);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
	
void QmitkActiveSurfaceToolGUI::EditedCurvatureForce()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = m_CurvatureForce->text();
	double force = text.toDouble();
	m_ActiveSurfaceTool->CurvatureForceLineEdit(force);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::AreaForceCross(int force,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->AreaForceCross(force,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateAreaForce(double areaForce)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = QString("%1").arg(areaForce);
	//text.arg(iterations);
	m_AreaForce->setText(text);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::EditedAreaForce()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = m_AreaForce->text();
	double force = text.toDouble();
	m_ActiveSurfaceTool->AreaForceLineEdit(force);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::VolumeForceCross(int force,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->VolumeForceCross(force,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::UpdateVolumeForce(double volumeForce)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = QString("%1").arg(volumeForce);
	//text.arg(iterations);
	m_VolumeForce->setText(text);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::EditedVolumeForce()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	QString text = m_VolumeForce->text();
	double force = text.toDouble();
	m_ActiveSurfaceTool->VolumeForceLineEdit(force);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::Closing()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->Closing();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveSurfaceToolGUI::ZoomCross(int zoom, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveSurfaceTool->Zoom(zoom);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}