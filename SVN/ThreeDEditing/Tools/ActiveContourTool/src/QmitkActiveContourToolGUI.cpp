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

#include "QmitkActiveContourToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <QSpinBox>
#include <QLineEdit>

QmitkActiveContourToolGUI::QmitkActiveContourToolGUI()
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

	QPushButton* pbtn_DetectHoles = new QPushButton();
	pbtn_DetectHoles->setText("Start Active Surface");
	horizontalLayout3->addWidget(pbtn_DetectHoles);
	connect(pbtn_DetectHoles, SIGNAL(clicked()), this, SLOT(DetectHoles()));

	QPushButton* pbtn_ActiveContours = new QPushButton();
	pbtn_ActiveContours->setText("Start Active Surface 2");
	horizontalLayout4->addWidget(pbtn_ActiveContours);
	connect(pbtn_ActiveContours, SIGNAL(clicked()), this, SLOT(ActiveContours()));

	QPushButton* pbtn_ActiveContoursVOI = new QPushButton();
	pbtn_ActiveContoursVOI->setText("Start Active Surface Loop");
	horizontalLayout5->addWidget(pbtn_ActiveContoursVOI);
	connect(pbtn_ActiveContoursVOI, SIGNAL(clicked()), this, SLOT(ActiveContoursVOI()));


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


	QLabel* label2 = new QLabel( "Step size:", this );
	QFont f2 = label->font();
	f2.setBold(false);
	label2->setFont( f2 );
	horizontalLayout8->addWidget(label2);
	QSpinBox* selectStepSize = new QSpinBox();
	selectStepSize->setRange(1, 50);
	selectStepSize->setValue(1);
	horizontalLayout8->addWidget(selectStepSize);
	connect(selectStepSize, SIGNAL(valueChanged(int)), this, SLOT(SelectStepSize(int)));

	QLabel* label3 = new QLabel( "Iteration:", this );
	QFont f3 = label->font();
	f3.setBold(false);
	label3->setFont( f3 );
	horizontalLayout9->addWidget(label3);
	numberOfIterations = new QLineEdit(this);
	numberOfIterations->setText("0");
	numberOfIterations->setReadOnly(true);
	horizontalLayout9->addWidget(numberOfIterations);

	QPushButton* pbtn_ActiveContour3d = new QPushButton();
	pbtn_ActiveContour3d->setText("3D");
	horizontalLayout10->addWidget(pbtn_ActiveContour3d);
	connect(pbtn_ActiveContour3d, SIGNAL(clicked()), this, SLOT(ActiveContour3d()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkActiveContourToolGUI::~QmitkActiveContourToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_ActiveContourTool.IsNotNull())
	{
		m_ActiveContourTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>( this, &QmitkActiveContourToolGUI::RadiusValueChanged );
		m_ActiveContourTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkActiveContourToolGUI, std::vector<std::string>>( this, &QmitkActiveContourToolGUI::InitSurfaceSelection );
		m_ActiveContourTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>(this, &QmitkActiveContourToolGUI::UpdateSelectedSurfaceBox);
		m_ActiveContourTool->SetNumberOfIterations -= mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>(this, &QmitkActiveContourToolGUI::UpdateNumberOfIterations);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkActiveContourToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_ActiveContourTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_ActiveContourTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_ActiveContourTool = dynamic_cast<mitk::ActiveContourTool*>( tool );

	m_ActiveContourTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>( this, &QmitkActiveContourToolGUI::RadiusValueChanged );
	m_ActiveContourTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkActiveContourToolGUI, std::vector<std::string>>( this, &QmitkActiveContourToolGUI::InitSurfaceSelection );
	m_ActiveContourTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>(this, &QmitkActiveContourToolGUI::UpdateSelectedSurfaceBox);
	m_ActiveContourTool->SetNumberOfIterations += mitk::MessageDelegate1<QmitkActiveContourToolGUI, int>(this, &QmitkActiveContourToolGUI::UpdateNumberOfIterations);
	
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkActiveContourToolGUI::DetectHoles()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//m_ActiveContourTool->DetectHoles();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::ActiveContours()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->ActiveContours();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::ActiveContoursVOI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->ActiveContoursVOI();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}




void QmitkActiveContourToolGUI::Stop()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->Stop();
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::StepBack()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->StepBack();
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::ReversePlay()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->ReversePlay();
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::Pause()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->Pause();
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::Play()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->Play();
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::StepForward()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->StepForward();
	m_VCR->at(0)->setChecked(false);
	m_VCR->at(1)->setChecked(false);
	m_VCR->at(2)->setChecked(false);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::SelectStepSize(int stepSize)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->SelectStepSize(stepSize);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::UpdateNumberOfIterations(int iterations)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	QString text = QString("%1").arg(iterations);
	//text.arg(iterations);
	numberOfIterations->setText(text);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkActiveContourToolGUI::ActiveContour3d()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_ActiveContourTool->ActiveContour3d();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}