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

#include "QmitkRegistrationToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

#include <QmitkCrossWidget.h>

QmitkRegistrationToolGUI::QmitkRegistrationToolGUI()
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
	QBoxLayout* horizontalLayout3 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout3);
	QBoxLayout* horizontalLayout34 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout34);
	QBoxLayout* horizontalLayout4 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout4);
	QBoxLayout* horizontalLayout45 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout45);
	QBoxLayout* horizontalLayout5 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout5);
	QBoxLayout* horizontalLayout56 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout56);
	QBoxLayout* horizontalLayout6 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout6);
	QBoxLayout* horizontalLayout7 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout7);
	QBoxLayout* horizontalLayout8 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout8);
	QBoxLayout* horizontalLayout89 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout89);
	QBoxLayout* horizontalLayout9 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout9);
	QBoxLayout* horizontalLayout10 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout10);
	QBoxLayout* horizontalLayout1011 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout1011);
	QBoxLayout* horizontalLayout11 = new QHBoxLayout();
	verticalLayout->addLayout(horizontalLayout11);

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
	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame->setLineWidth(10);
	frame->setFixedSize(200,10);
	QColor color(214,247,202);
	QPalette colorPalette(color);
	colorPalette.setColor(QPalette::Foreground, color);
	frame->setPalette(colorPalette);
	horizontalLayout3->addWidget(frame);

	QLabel* label1 = new QLabel( "Use arrow keys on the keyboard to \ntranslate the surface.", this );
	QFont f1 = label1->font();
	f1.setBold(false);
	label1->setFont( f1 );
	horizontalLayout34->addWidget(label1);

	QLabel* label2 = new QLabel( "Translation speed", this );
	QFont f2 = label2->font();
	f2.setBold(false);
	label2->setFont( f2 );
	horizontalLayout4->addWidget(label2);
	m_MoveSurfaceSpeedSlider = new QSlider(Qt::Horizontal);
	m_MoveSurfaceSpeedSlider->setSliderPosition(20);
	horizontalLayout4->addWidget(m_MoveSurfaceSpeedSlider);
	QObject::connect(m_MoveSurfaceSpeedSlider, SIGNAL(sliderMoved(int)), this, SLOT(MoveSurfaceSpeed(int)));

	QFrame *frame1 = new QFrame( this );
	frame1->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame1->setLineWidth(10);
	frame1->setFixedSize(200,10);
	frame1->setPalette(colorPalette);
	horizontalLayout45->addWidget(frame1);

	QLabel* label3 = new QLabel( "Scale", this );
	QFont f3 = label3->font();
	f3.setBold(false);
	label3->setFont( f3 );
	horizontalLayout5->addWidget(label3);
	QmitkCrossWidget* crossScale = new QmitkCrossWidget();
	QPixmap pix(":/threeDEditing/res/threeDEditing/CrossArrow.png");
	crossScale->setPixmap(pix.scaledToWidth(100));
	crossScale->setFixedSize(100,20);
	crossScale->setToolTip("Drag with a left mouse click from the center circle to the sides to scale the surface.");
	horizontalLayout5->addWidget(crossScale);
	connect( crossScale, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossScale( int, int ) ) );

	QFrame *frame2 = new QFrame( this );
	frame2->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame2->setLineWidth(10);
	frame2->setFixedSize(200,10);
	frame2->setPalette(colorPalette);
	horizontalLayout56->addWidget(frame2);

	QLabel* label4 = new QLabel( "Rotate Transversal", this );
	QFont f4 = label4->font();
	f4.setBold(false);
	label4->setFont( f4 );
	horizontalLayout6->addWidget(label4);
	QmitkCrossWidget* crossRotateX = new QmitkCrossWidget();
	QPixmap pixRotateX(":/threeDEditing/res/threeDEditing/CrossArrowTransversal.png");
	crossRotateX->setPixmap(pixRotateX.scaledToWidth(100));
	crossRotateX->setFixedSize(100,20);
	crossRotateX->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout6->addWidget(crossRotateX);
	connect( crossRotateX, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossRotateX( int, int ) ) );

	QLabel* label5 = new QLabel( "Rotate Sagittal", this );
	QFont f5 = label5->font();
	f5.setBold(false);
	label5->setFont( f5 );
	horizontalLayout7->addWidget(label5);
	QmitkCrossWidget* crossRotateY = new QmitkCrossWidget();
	QPixmap pixRotateY(":/threeDEditing/res/threeDEditing/CrossArrowSagittal.png");
	crossRotateY->setPixmap(pixRotateY.scaledToWidth(100));
	crossRotateY->setFixedSize(100,20);
	crossRotateY->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout7->addWidget(crossRotateY);
	connect( crossRotateY, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossRotateY( int, int ) ) );

	QLabel* label6 = new QLabel( "Rotate Coronal", this );
	QFont f6 = label6->font();
	f6.setBold(false);
	label6->setFont( f6 );
	horizontalLayout8->addWidget(label6);
	QmitkCrossWidget* crossRotateZ = new QmitkCrossWidget();
	QPixmap pixRotateZ(":/threeDEditing/res/threeDEditing/CrossArrowCoronal.png");
	crossRotateZ->setPixmap(pixRotateZ.scaledToWidth(100));
	crossRotateZ->setFixedSize(100,20);
	crossRotateZ->setToolTip("Drag with a left mouse click from the center circle to the sides.");
	horizontalLayout8->addWidget(crossRotateZ);
	connect( crossRotateZ, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossRotateZ( int, int ) ) );

	QFrame *frame3 = new QFrame( this );
	frame3->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame3->setLineWidth(10);
	frame3->setFixedSize(200,10);
	frame3->setPalette(colorPalette);
	horizontalLayout89->addWidget(frame3);

	QLabel* label7 = new QLabel( "Translate", this );
	QFont f7 = label7->font();
	f6.setBold(false);
	label7->setFont( f7 );
	horizontalLayout9->addWidget(label7);

	QmitkCrossWidget* crossTranslateTransversal = new QmitkCrossWidget();
	QPixmap pixTranslateTransversal(":/threeDEditing/res/threeDEditing/CrossArrowTranslateTransversal.png");
	crossTranslateTransversal->setPixmap(pixTranslateTransversal.scaledToWidth(70));
	//crossTranslateTransversal->setFixedSize(100,20);
	crossTranslateTransversal->setToolTip("Drag with a left mouse click from the center circle up/down or to the sides.");
	horizontalLayout10->addWidget(crossTranslateTransversal);
	connect( crossTranslateTransversal, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossTranslateTransversal( int, int ) ) );

	QmitkCrossWidget* crossTranslateSagital = new QmitkCrossWidget();
	QPixmap pixTranslateSagital(":/threeDEditing/res/threeDEditing/CrossArrowTranslateSagittal.png");
	crossTranslateSagital->setPixmap(pixTranslateSagital.scaledToWidth(70));
	//crossTranslateSagital->setFixedSize(100,20);
	crossTranslateSagital->setToolTip("Drag with a left mouse click from the center circle up/down or to the sides.");
	horizontalLayout10->addWidget(crossTranslateSagital);
	connect( crossTranslateSagital, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossTranslateSagittal( int, int ) ) );

	QmitkCrossWidget* crossTranslateCoronal = new QmitkCrossWidget();
	QPixmap pixTranslateCoronal(":/threeDEditing/res/threeDEditing/CrossArrowTranslateCoronal.png");
	crossTranslateCoronal->setPixmap(pixTranslateCoronal.scaledToWidth(70));
	//crossTranslateCoronal->setFixedSize(100,20);
	crossTranslateCoronal->setToolTip("Drag with a left mouse click from the center circle up/down or to the sides.");
	horizontalLayout10->addWidget(crossTranslateCoronal);
	connect( crossTranslateCoronal, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( CrossTranslateCoronal( int, int ) ) );

	QFrame *frame4 = new QFrame( this );
	frame4->setFrameStyle( QFrame::Box | QFrame::Plain );
	frame4->setLineWidth(10);
	frame4->setFixedSize(200,10);
	frame4->setPalette(colorPalette);
	horizontalLayout1011->addWidget(frame4);

	QPushButton* pbtnReinitAutomatical = new QPushButton();
	pbtnReinitAutomatical->setText("Reinit Automatical");
	pbtnReinitAutomatical->setCheckable(true);
	horizontalLayout11->addWidget(pbtnReinitAutomatical);
	connect(pbtnReinitAutomatical, SIGNAL(clicked()), this, SLOT(ReinitAutomatical()));

	QPushButton* pbtnReinitNow = new QPushButton();
	pbtnReinitNow->setText("Reinit Now");
	horizontalLayout11->addWidget(pbtnReinitNow);
	connect(pbtnReinitNow, SIGNAL(clicked()), this, SLOT(ReinitNow()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkRegistrationToolGUI::~QmitkRegistrationToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// !!!
	if(m_RegistrationTool.IsNotNull())
	{
		m_RegistrationTool->SetSliderToLastRadius -= mitk::MessageDelegate1<QmitkRegistrationToolGUI, int>( this, &QmitkRegistrationToolGUI::RadiusValueChanged );
		m_RegistrationTool->GetCurrentSurfaces -= mitk::MessageDelegate1<QmitkRegistrationToolGUI, std::vector<std::string>>( this, &QmitkRegistrationToolGUI::InitSurfaceSelection );
		m_RegistrationTool->UpdateSelectedSurface -= mitk::MessageDelegate1<QmitkRegistrationToolGUI, int>(this, &QmitkRegistrationToolGUI::UpdateSelectedSurfaceBox);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::InitSurfaceSelection(std::vector<std::string> currentSurfaces)
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

void QmitkRegistrationToolGUI::SelectSurface(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "Selected Surface: " << selection << "\n";
	m_RegistrationTool->SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::SelectSurfaceViaMouse()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->SelectSurfaceViaMouse();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::UpdateSelectedSurfaceBox(int selection)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	m_SelectSurfaceBox->setCurrentIndex(selection);
	SelectSurface(selection);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::ReinitSelection()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->ReinitSelection();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::JumpToPosition()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->JumpToPosition();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::RadiusChanged(int value)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Value: " << value << "\n";
	m_RegistrationTool->RadiusChanged(value);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_RegistrationTool = dynamic_cast<mitk::RegistrationTool*>( tool );
	m_RegistrationTool->SetSliderToLastRadius += mitk::MessageDelegate1<QmitkRegistrationToolGUI, int>( this, &QmitkRegistrationToolGUI::RadiusValueChanged );
	m_RegistrationTool->GetCurrentSurfaces += mitk::MessageDelegate1<QmitkRegistrationToolGUI, std::vector<std::string>>( this, &QmitkRegistrationToolGUI::InitSurfaceSelection );
	m_RegistrationTool->UpdateSelectedSurface += mitk::MessageDelegate1<QmitkRegistrationToolGUI, int>(this, &QmitkRegistrationToolGUI::UpdateSelectedSurfaceBox);
	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::RadiusValueChanged(int currentRadius)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Radius sent to GUI: " << currentRadius << "\n";
	
	m_HorizontalLayout2->removeWidget(m_RadiusSlider);
	m_RadiusSlider->setSliderPosition(currentRadius);
	m_HorizontalLayout2->addWidget(m_RadiusSlider);

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}


// Tool Specific ********************************************************************************
void QmitkRegistrationToolGUI::MoveSurfaceSpeed(int speed)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->MoveSurfaceSpeed(speed);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

//void QmitkRegistrationToolGUI::ShrinkSurface(int size)
//{
//	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
//	m_RegistrationTool->ShrinkSurface(size);
//	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
//}

void QmitkRegistrationToolGUI::CrossScale(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Cross - X: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->ScaleChangedByCross(x);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::CrossRotateX(int x,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Rotate X: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossRotateX(x);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}
void QmitkRegistrationToolGUI::CrossRotateY(int x,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Rotate X: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossRotateY(x);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::CrossRotateZ(int x,int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	std::cout << "Rotate X: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossRotateZ(x);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::CrossTranslateTransversal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Translate Transversal: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossTranslateTransversal(x,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::CrossTranslateSagittal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Translate Sagittal: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossTranslateSagittal(x,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::CrossTranslateCoronal(int x, int y)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	//std::cout << "Translate Coronal: " << x << ", Y: " << y << "\n";
	m_RegistrationTool->CrossTranslateCoronal(x,y);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::ReinitAutomatical()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->ReinitAutomatical();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkRegistrationToolGUI::ReinitNow()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_RegistrationTool->ReinitNow();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}