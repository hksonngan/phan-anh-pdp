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

#include "QmitkTestToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <vector>

QmitkTestToolGUI::QmitkTestToolGUI()
:QmitkToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	// create the visible widgets
	QBoxLayout* verticalLayout = new QVBoxLayout( this );

	QHBoxLayout* m_HorizontalLayout = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout);
	QHBoxLayout* m_HorizontalLayout1 = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout1);
	QHBoxLayout* m_HorizontalLayout2 = new QHBoxLayout();
	verticalLayout->addLayout(m_HorizontalLayout2);
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

	m_StartAuswertung = new QPushButton();
	m_StartAuswertung->setText("Starte die Auswertung");
	m_StartAuswertung->setCheckable(false);
	m_HorizontalLayout->addWidget(m_StartAuswertung);
	connect(m_StartAuswertung, SIGNAL(clicked()), this, SLOT(StartAuswertung()));

	m_ConvertData = new QPushButton();
	m_ConvertData->setText("Convert Data");
	m_ConvertData->setCheckable(false);
	m_HorizontalLayout1->addWidget(m_ConvertData);
	connect(m_ConvertData, SIGNAL(clicked()), this, SLOT(ConvertData()));

	m_Closing = new QPushButton();
	m_Closing->setText("Close Active Surface Data");
	m_Closing->setCheckable(false);
	m_HorizontalLayout2->addWidget(m_Closing);
	connect(m_Closing, SIGNAL(clicked()), this, SLOT(Closing()));

	m_AuswertungPixel = new QPushButton();
	m_AuswertungPixel->setText("Auswertung Pixelbereich");
	m_AuswertungPixel->setCheckable(false);
	horizontalLayout3->addWidget(m_AuswertungPixel);
	connect(m_AuswertungPixel, SIGNAL(clicked()), this, SLOT(AuswertungPixel()));

	connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

QmitkTestToolGUI::~QmitkTestToolGUI()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkTestToolGUI::StartAuswertung()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_TestTool->StartAuswertung();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkTestToolGUI::ConvertData()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_TestTool->ConvertData();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkTestToolGUI::Closing()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_TestTool->Closing();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkTestToolGUI::AuswertungPixel()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	m_TestTool->AuswertungPixel();
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void QmitkTestToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	m_TestTool = dynamic_cast<mitk::TestTool*>( tool );

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}