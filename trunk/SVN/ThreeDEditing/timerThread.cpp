#include <iostream>
#include <vtkSTLWriter.h>
#include <mitkSurfaceVtkWriter.h>
#include <mitkImageWriter.h>
#include <QmitkToolSelectionBox.h>

#include "ThreeDEditing.h"
#include "global.h"

#include "timerThread.h"

pdp::TimerThread::TimerThread(/*ThreeDEditing* editing*/) : QThread()
{
	/*m_ThreeDEditing = editing;*/
	m_Timer = new QTimer();
	m_ElapsedTime = 0;
}

pdp::TimerThread::~TimerThread()
{

}

void pdp::TimerThread::run()
{
	connect(m_Timer, SIGNAL(timeout()), this, SLOT(TimerEvent()), Qt:: DirectConnection);
	m_Timer->start(1000);
	
	exec();
	std::cout << "Thread run finished.\n";
}

//void TimerThread::start()
//{
//	connect(m_Timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
//	m_Timer->start(1000);
//	
//	exec();
//}

void pdp::TimerThread::TimerEvent()
{
	std::cout << "Timer Event.\n";
	m_ElapsedTime++;
	char buffer [33];
	std::string text = _itoa(m_ElapsedTime,buffer,10);
	text += " seconds";
	//m_ThreeDEditing->m_Time->setText(text.c_str());

	if(m_ElapsedTime % 5 == 0)
	{
		//this->start();
		this->setPriority(QThread::TimeCriticalPriority);
		std::cout << "Thread is running: " << this->isRunning() << "\n";;
	}
	/*
	if(m_ElapsedTime % 5 == 0)
	{
		// alle paar sekunden surface bzw. binary rausschreiben
		std::cout << "Alle 10 sec? " << m_ElapsedTime << "\n";

		// Save segmentation
		mitk::DataNode::Pointer node;
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = m_ThreeDEditing->GetLungDataset()->getDataStore()->GetAll();
		for(unsigned int i = 0; i < allNodes->size(); i++)
		{
			if((allNodes->at(i)->GetName()).compare("Segmentation") == 0)	
			{
				node = allNodes->at(i);
			}
			else
			{
				//std::cout << "Error: Segmentation node not found!\n";
				continue;
			}
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Surface") == 0)
			{
				std::string dir;
				std::string text2 = node->GetName();
				text2 += " ";
				text2 += text;
				mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();

				if(AT_HOME == 1)
				{
					dir += "C:\\DA\\Data\\Segmentations\\";
					dir += text2;
					dir += ".stl";
				}
				else
				{
					dir += "D:\\hachmann\\Data\\Segmentations\\";
					dir += text2;
					dir += ".stl";
				}

				mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
				surfaceWriter->SetInput( data );
				surfaceWriter->SetFileName(dir.c_str());
				surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
				surfaceWriter->Write();
			}
			if(strcmp(allNodes->at(i)->GetData()->GetNameOfClass(), "Image") == 0)
			{
				std::string dir;
				std::string text2 = node->GetName();
				text2 += " ";
				text2 += text;
				mitk::Image::Pointer data = (mitk::Image*)node->GetData();

				if(AT_HOME == 1)
				{
					dir += "C:\\DA\\Data\\Segmentations\\";
					dir += text2;
					//dir += ".nrrd";
				}
				else
				{
					dir += "D:\\hachmann\\Data\\Segmentations\\";
					dir += text2;
					//dir += ".nrrd";
				}

				mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
				imageWriter->SetInput(data);
				imageWriter->SetFileName(dir.c_str());
				imageWriter->SetExtension(".nrrd");
				imageWriter->Write();
			}
		}
	}
	int totalSegmentationTime = 60;
	if(m_ElapsedTime == totalSegmentationTime)
	{
		// time is up
		std::cout << "Time is up!\n";
		m_ThreeDEditing->GetManualToolSelectionBox()->OnGeneralToolMessage("Time is up!");
		m_ThreeDEditing->GetManualToolSelectionBox()->OnToolGUIProcessEventsMessage();

		QIcon icn_StartSegmentation;
		icn_StartSegmentation.addFile(":/threeDEditing/res/threeDEditing/Forward.png");
		m_ThreeDEditing->m_Btn_StartSegmentation->setIcon(icn_StartSegmentation);

		m_Timer->stop();
		m_ElapsedTime = 0;

		m_ThreeDEditing->m_Play = true;
	}
	*/
}

//void TimerThread::stop()
//{
//
//}


