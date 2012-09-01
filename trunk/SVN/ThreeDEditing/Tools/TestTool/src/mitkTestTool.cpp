// "pdp"
#include "mitkTestTool.h"
#include "mitkTestTool.xpm"
#include <../../../../RenderingManagerProperty.h>
#include <../../../../global.h>
#include "mitkWriteCVS.h"
#include "mitkWriteCVSPixel.h"

// QT
#include <QDir>
#include <QTextStream>

// MITK
#include "mitkToolManager.h"
#include <mitkStateEvent.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkSurfaceVtkWriter.h>
#include <mitkDataNodeFactory.h>
#include "mitkSurfaceToImageFilter.h"
#include "mitkManualSegmentationToSurfaceFilter.h"
#include <mitkImageCast.h>
#include <mitkImageWriter.h>

// VTK
#include <vtkSTLWriter.h>
#include <vtkMassProperties.h>
#include <vtkPolyDataNormals.h>

// ITK
#include <itkBinaryCrossStructuringElement.h>
#include <itkGrayscaleMorphologicalClosingImageFilter.h>


mitk::TestTool::TestTool()
:Tool("PositionTracker")
{
	// great magic numbers
	//CONNECT_ACTION( 92, OnMouseMoved );
}

mitk::TestTool::~TestTool()
{}

const char** mitk::TestTool::GetXPM() const
{
	return mitkTestTool_xpm;
}

const char* mitk::TestTool::GetName() const
{
	return "Test Tool";
}

const char* mitk::TestTool::GetGroup() const
{
	return "default";
}

void mitk::TestTool::Activated()
{
	Superclass::Activated();
	//std::cout << "Activated.\n";	

	// store datastorage
	m_Storage = m_ToolManager->GetDataStorage();
	//storage->Print(std::cout);

	if(AT_HOME == 1)
	{
		m_Directory = "C:\\DA\\Data\\Auswertungsdaten\\";
	}
	else
	{
		m_Directory = "D:\\hachmann\\Data\\Auswertungsdaten\\";
	}
	m_Names.push_back("Eva");
	m_Names.push_back("Michael");
	m_Names.push_back("Nadine");
	m_Names.push_back("Ulli");
	m_Names.push_back("Pierre");
	m_Names.push_back("Bernd");
	m_Names.push_back("Markus");
	m_Names.push_back("Peter");
	m_Names.push_back("Georg");
	m_Names.push_back("Erik");

	m_ActiveSurface = true;
	m_AddSubstract1 = true;
	m_AddSubstract2 = true;
	m_Bulge = true;
	m_Drag = true;
	m_LiveWire = true;
	m_PaintWipe1 = true;
	m_PaintWipe2 = true;
}

void mitk::TestTool::Deactivated()
{
	//std::cout << "Deactivated.\n";
	Superclass::Deactivated();	
}

//bool mitk::TestTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
//{
//	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
//	bool ok = true;
//
//	return ok;
//	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
//}

void mitk::TestTool::StartAuswertung()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	std::map<int,std::vector<double>> globalActiveSurfaceMap;
	std::map<int,std::vector<double>> globalAddSubstractMap1;
	std::map<int,std::vector<double>> globalAddSubstractMap2;
	std::map<int,std::vector<double>> globalBulgeMap;
	std::map<int,std::vector<double>> globalDragMap;
	std::map<int,std::vector<double>> globalLiveWireMap;
	std::map<int,std::vector<double>> globalPaintWipeMap1;
	std::map<int,std::vector<double>> globalPaintWipeMap2;

	// fill with zeros
	std::vector<double> dummy;
	dummy.push_back(0.0);
	dummy.push_back(0.0);
	dummy.push_back(0.0);
	dummy.push_back(0.0);
	dummy.push_back(0.0);
	dummy.push_back(0.0);
	dummy.push_back(0.0);

	for(int i = 0; i < 125; i++)
	{
		globalActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalAddSubstractMap1.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalAddSubstractMap2.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalBulgeMap.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalDragMap.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalLiveWireMap.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalPaintWipeMap1.insert(std::pair<int,std::vector<double>>(i,dummy));
		globalPaintWipeMap2.insert(std::pair<int,std::vector<double>>(i,dummy));
	}

	// walk the folder structur
	for(std::vector<std::string>::iterator it = m_Names.begin(); it != m_Names.end(); it++)
	{
		std::map<int,std::vector<double>> ActiveSurfaceMap;
		std::map<int,std::vector<double>> AddSubstractMap1;
		std::map<int,std::vector<double>> AddSubstractMap2;
		std::map<int,std::vector<double>> BulgeMap;
		std::map<int,std::vector<double>> DragMap;
		std::map<int,std::vector<double>> LiveWireMap;
		std::map<int,std::vector<double>> PaintWipeMap1;
		std::map<int,std::vector<double>> PaintWipeMap2;

		QString currentPath(m_Directory.c_str());
		currentPath.append(it->c_str());
		QDir directory(currentPath);
		QStringList list(directory.entryList());

		if(m_ActiveSurface)
		{
			directory.cd("ActiveSurface");
			directory.cd("Surface");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list2(directory.entryList());
			for(int i = 0; i < list2.size(); ++i)
			{
				if(list2.at(i).toStdString().compare(".") == 0)
					continue;
				if(list2.at(i).toStdString().compare("..") == 0)
					continue;
				if(list2.at(i).toStdString().compare("Pixel") == 0)
					continue;

				//std::cout << list2.at(i).toLocal8Bit().constData() << "\n";

				// get seconds
				QString secondsStr(list2.at(i).mid(7,3));
				//std::cout << secondsStr.toStdString().c_str() << "\n";
				if(secondsStr[2] == QChar('s'))
					secondsStr = secondsStr.mid(0,2);
				std::cout << secondsStr.toStdString().c_str() << "\n";
				int seconds = secondsStr.toInt();
				std::vector<double> value;
				std::string referenzPath;
				
				std::string outPath;
				if(AT_HOME == 1)
				{
					std::string completePath;
					completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list2.at(i).toStdString();
					completePath += "\" \"C:\\DA\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.stl";
				}
				else
				{
					std::string completePath;
					completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list2.at(i).toStdString();
					completePath += "\" \"D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl";
				}
				QFile file(outPath.c_str());
				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
					 return;
				QTextStream in(&file);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if(line.indexOf("#") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
					}
					if(line.indexOf("	area") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString areaStr(line.mid(8,20));
						//std::cout << areaStr.toStdString().c_str() << "\n";
						double area = areaStr.toDouble();
						//std::cout << area << "\n";
						value.push_back(area);
					}
					if(line.indexOf("Hausdorff distance") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString hausdorffStr(line.mid(20,5));
						//std::cout << hausdorffStr.toStdString().c_str() << "\n";
						double hausdorff = hausdorffStr.toDouble();
						//std::cout << hausdorff << "\n";
						value.push_back(hausdorff);
					}
					if(line.indexOf("  mean") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString meanStr(line.mid(9,10));
						//std::cout << meanStr.toStdString().c_str() << "\n";
						double mean = meanStr.toDouble();
						//std::cout << mean << "\n";
						value.push_back(mean);
					}
				}	
				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list2.at(i).toStdString();
				value.push_back(CalculateVolume(completePath2));

				// Volume 2
				value.push_back(CalculateVolume(referenzPath));

				ActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			for(std::map<int,std::vector<double>>::iterator it = ActiveSurfaceMap.begin(); it != ActiveSurfaceMap.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
			directory.cdUp();
		}

		if(m_AddSubstract1)
		{
			directory.cd("AddSubstract");
			directory.cd("Surface");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list3(directory.entryList());
			for(int i = 0; i < list3.size(); ++i)
			{
				if(list3.at(i).toStdString().compare(".") == 0)
					continue;
				if(list3.at(i).toStdString().compare("..") == 0)
					continue;
				//std::cout << list3.at(i).toLocal8Bit().constData() << "\n";
				
				// Fallunterscheidung: Scenario 1 vs. Scenario 2
				QString scenarioStr(list3.at(i).mid(0,10));
				if(scenarioStr.toStdString().compare("Scenario_1") == 0)
				{
					//std::cout << "S1: " << list3.at(i).toLocal8Bit().constData() << "\n";

					// get seconds
					QString secondsStr(list3.at(i).mid(27,3));
					//std::cout << secondsStr.toStdString().c_str() << "\n";
					if(secondsStr[2] == QChar('s'))
						secondsStr = secondsStr.mid(0,2);
					std::cout << secondsStr.toStdString().c_str() << "\n";
					int seconds = secondsStr.toInt();
					std::vector<double> value;
					std::string referenzPath;
					
					std::string outPath;
					if(AT_HOME == 1)
					{
						std::string completePath;
						completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list3.at(i).toStdString();
						completePath += "\" \"C:\\DA\\Data\\Scenario1\\Box4.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario1\\Box4.stl";
					}
					else
					{
						std::string completePath;
						completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list3.at(i).toStdString();
						completePath += "\" \"D:\\hachmann\\Data\\Scenario1\\Box4.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Box4.stl";
					}
					QFile file(outPath.c_str());
					if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
						 return;
					QTextStream in(&file);
					while (!in.atEnd()) {
						QString line = in.readLine();
						if(line.indexOf("#") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
						}
						if(line.indexOf("	area") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString areaStr(line.mid(8,20));
							//std::cout << areaStr.toStdString().c_str() << "\n";
							double area = areaStr.toDouble();
							//std::cout << area << "\n";
							value.push_back(area);
						}
						if(line.indexOf("Hausdorff distance") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString hausdorffStr(line.mid(20,5));
							//std::cout << hausdorffStr.toStdString().c_str() << "\n";
							double hausdorff = hausdorffStr.toDouble();
							//std::cout << hausdorff << "\n";
							value.push_back(hausdorff - 0.88);
						}
						if(line.indexOf("  mean") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString meanStr(line.mid(9,10));
							//std::cout << meanStr.toStdString().c_str() << "\n";
							double mean = meanStr.toDouble();
							//std::cout << mean << "\n";
							value.push_back(mean);
						}
					}	
					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list3.at(i).toStdString();
					value.push_back(CalculateVolume(completePath2));

					// Volume 2
					value.push_back(CalculateVolume(referenzPath));

					AddSubstractMap1.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
			}
			
			for(std::map<int,std::vector<double>>::iterator it = AddSubstractMap1.begin(); it != AddSubstractMap1.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
			directory.cdUp();
		}
		if(m_AddSubstract2)
		{
			directory.cd("AddSubstract");
			directory.cd("Surface");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list3(directory.entryList());
			for(int i = 0; i < list3.size(); ++i)
			{
				if(list3.at(i).toStdString().compare(".") == 0)
					continue;
				if(list3.at(i).toStdString().compare("..") == 0)
					continue;
				//std::cout << list3.at(i).toLocal8Bit().constData() << "\n";
				
				// Fallunterscheidung: Scenario 1 vs. Scenario 2
				QString scenarioStr(list3.at(i).mid(0,10));
				if(scenarioStr.toStdString().compare("Scenario_2") == 0)
				{
					std::cout << "S2: " << list3.at(i).toLocal8Bit().constData() << "\n";

					// get seconds
					QString secondsStr(list3.at(i).mid(27,3));
					//std::cout << secondsStr.toStdString().c_str() << "\n";
					if(secondsStr[2] == QChar('s'))
						secondsStr = secondsStr.mid(0,2);
					std::cout << secondsStr.toStdString().c_str() << "\n";
					int seconds = secondsStr.toInt();
					std::vector<double> value;
					std::string referenzPath;
					
					std::string outPath;
					if(AT_HOME == 1)
					{
						std::string completePath;
						completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list3.at(i).toStdString();
						completePath += "\" \"C:\\DA\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.stl";
					}
					else
					{
						std::string completePath;
						completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list3.at(i).toStdString();
						completePath += "\" \"D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl";
					}
					QFile file(outPath.c_str());
					if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
						 return;
					QTextStream in(&file);
					while (!in.atEnd()) {
						QString line = in.readLine();
						if(line.indexOf("#") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
						}
						if(line.indexOf("	area") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString areaStr(line.mid(8,20));
							//std::cout << areaStr.toStdString().c_str() << "\n";
							double area = areaStr.toDouble();
							//std::cout << area << "\n";
							value.push_back(area);
						}
						if(line.indexOf("Hausdorff distance") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString hausdorffStr(line.mid(20,5));
							//std::cout << hausdorffStr.toStdString().c_str() << "\n";
							double hausdorff = hausdorffStr.toDouble();
							//std::cout << hausdorff << "\n";
							value.push_back(hausdorff);
						}
						if(line.indexOf("  mean") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString meanStr(line.mid(9,10));
							//std::cout << meanStr.toStdString().c_str() << "\n";
							double mean = meanStr.toDouble();
							//std::cout << mean << "\n";
							value.push_back(mean);
						}
					}	
					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list3.at(i).toStdString();
					value.push_back(CalculateVolume(completePath2));

					// Volume 2
					value.push_back(CalculateVolume(referenzPath));

					AddSubstractMap2.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
				//std::cout << secondsStr.toStdString().c_str() << "\n";			
			}

			for(std::map<int,std::vector<double>>::iterator it = AddSubstractMap2.begin(); it != AddSubstractMap2.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}	
			directory.cdUp();
			directory.cdUp();
		}

		if(m_Bulge)
		{
			directory.cd("Bulge");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list4(directory.entryList());
			for(int i = 0; i < list4.size(); ++i)
			{
				if(list4.at(i).toStdString().compare(".") == 0)
					continue;
				if(list4.at(i).toStdString().compare("..") == 0)
					continue;
				if(list4.at(i).toStdString().compare("Pixel") == 0)
					continue;
				//std::cout << list4.at(i).toLocal8Bit().constData() << "\n";
				
				// get seconds
				QString secondsStr(list4.at(i).mid(27,3));
				//std::cout << secondsStr.toStdString().c_str() << "\n";
				if(secondsStr[2] == QChar('s'))
					secondsStr = secondsStr.mid(0,2);
				std::cout << secondsStr.toStdString().c_str() << "\n";
				int seconds = secondsStr.toInt();
				std::vector<double> value;
				std::string referenzPath;
				
				std::string outPath;
				if(AT_HOME == 1)
				{
					std::string completePath;
					completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list4.at(i).toStdString();
					completePath += "\" \"C:\\DA\\Data\\Scenario1\\Box4.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario1\\Box4.stl";
				}
				else
				{
					std::string completePath;
					completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list4.at(i).toStdString();
					completePath += "\" \"D:\\hachmann\\Data\\Scenario1\\Box4.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario1\\Box4.stl";
				}

				QFile file(outPath.c_str());

				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
					 return;
				QTextStream in(&file);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if(line.indexOf("#") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
					}
					if(line.indexOf("	area") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString areaStr(line.mid(8,20));
						//std::cout << areaStr.toStdString().c_str() << "\n";
						double area = areaStr.toDouble();
						//std::cout << area << "\n";
						value.push_back(area);
					}
					if(line.indexOf("Hausdorff distance") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString hausdorffStr(line.mid(20,5));
						//std::cout << hausdorffStr.toStdString().c_str() << "\n";
						double hausdorff = hausdorffStr.toDouble();
						//std::cout << hausdorff << "\n";
						value.push_back(hausdorff);
					}
					if(line.indexOf("  mean") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString meanStr(line.mid(9,10));
						//std::cout << meanStr.toStdString().c_str() << "\n";
						double mean = meanStr.toDouble();
						//std::cout << mean << "\n";
						value.push_back(mean);
					}
				}	
				
				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list4.at(i).toStdString();
				value.push_back(CalculateVolume(completePath2));

				// Volume 2
				value.push_back(CalculateVolume(referenzPath));

				BulgeMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			for(std::map<int,std::vector<double>>::iterator it = BulgeMap.begin(); it != BulgeMap.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
		}

		if(m_Drag)
		{
			directory.cd("Drag");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list5(directory.entryList());
			for(int i = 0; i < list5.size(); ++i)
			{
				if(list5.at(i).toStdString().compare(".") == 0)
					continue;
				if(list5.at(i).toStdString().compare("..") == 0)
					continue;
				if(list5.at(i).toStdString().compare("Pixel") == 0)
					continue;
				//std::cout << list4.at(i).toLocal8Bit().constData() << "\n";
				
				// get seconds
				QString secondsStr(list5.at(i).mid(27,3));
				//std::cout << secondsStr.toStdString().c_str() << "\n";
				if(secondsStr[2] == QChar('s'))
					secondsStr = secondsStr.mid(0,2);
				std::cout << secondsStr.toStdString().c_str() << "\n";
				int seconds = secondsStr.toInt();
				std::vector<double> value;
				std::string referenzPath;
				
				std::string outPath;
				if(AT_HOME == 1)
				{
					std::string completePath;
					completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list5.at(i).toStdString();
					completePath += "\" \"C:\\DA\\Data\\Scenario1\\Box4.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario1\\Box4.stl";
				}
				else
				{
					std::string completePath;
					completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list5.at(i).toStdString();
					completePath += "\" \"D:\\hachmann\\Data\\Scenario1\\Box4.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario1\\Box4.stl";
				}

				QFile file(outPath.c_str());

				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
					 return;
				QTextStream in(&file);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if(line.indexOf("#") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
					}
					if(line.indexOf("	area") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString areaStr(line.mid(8,20));
						//std::cout << areaStr.toStdString().c_str() << "\n";
						double area = areaStr.toDouble();
						//std::cout << area << "\n";
						value.push_back(area);
					}
					if(line.indexOf("Hausdorff distance") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString hausdorffStr(line.mid(20,5));
						//std::cout << hausdorffStr.toStdString().c_str() << "\n";
						double hausdorff = hausdorffStr.toDouble();
						//std::cout << hausdorff << "\n";
						value.push_back(hausdorff);
					}
					if(line.indexOf("  mean") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString meanStr(line.mid(9,10));
						//std::cout << meanStr.toStdString().c_str() << "\n";
						double mean = meanStr.toDouble();
						//std::cout << mean << "\n";
						value.push_back(mean);
					}
				}
				
				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list5.at(i).toStdString();
				value.push_back(CalculateVolume(completePath2));

				// Volume 2
				value.push_back(CalculateVolume(referenzPath));

				DragMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			for(std::map<int,std::vector<double>>::iterator it = DragMap.begin(); it != DragMap.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
		}

		if(m_LiveWire)
		{
			directory.cd("LiveWire");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list6(directory.entryList());
			for(int i = 0; i < list6.size(); ++i)
			{
				if(list6.at(i).toStdString().compare(".") == 0)
					continue;
				if(list6.at(i).toStdString().compare("..") == 0)
					continue;
				if(list6.at(i).toStdString().compare("Pixel") == 0)
					continue;
				//std::cout << list4.at(i).toLocal8Bit().constData() << "\n";
				
				// get seconds
				QString secondsStr(list6.at(i).mid(27,3));
				//std::cout << secondsStr.toStdString().c_str() << "\n";
				if(secondsStr[2] == QChar('s'))
					secondsStr = secondsStr.mid(0,2);
				std::cout << secondsStr.toStdString().c_str() << "\n";
				int seconds = secondsStr.toInt();
				std::vector<double> value;
				std::string referenzPath;
				
				std::string outPath;
				if(AT_HOME == 1)
				{
					std::string completePath;
					completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list6.at(i).toStdString();
					completePath += "\" \"C:\\DA\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.stl";
				}
				else
				{
					std::string completePath;
					completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
					completePath += directory.absolutePath().toStdString();
					completePath += "\\";
					completePath += list6.at(i).toStdString();
					completePath += "\" \"D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
					//std::cout << "System call for metro: " << completePath.c_str() << "\n";
					//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
					system(completePath.c_str());
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl";
				}

				QFile file(outPath.c_str());

				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
					 return;
				QTextStream in(&file);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if(line.indexOf("#") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
					}
					if(line.indexOf("	area") == 0)          
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString areaStr(line.mid(8,20));
						//std::cout << areaStr.toStdString().c_str() << "\n";
						double area = areaStr.toDouble();
						//std::cout << area << "\n";
						value.push_back(area);
					}
					if(line.indexOf("Hausdorff distance") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString hausdorffStr(line.mid(20,5));
						//std::cout << hausdorffStr.toStdString().c_str() << "\n";
						double hausdorff = hausdorffStr.toDouble();
						//std::cout << hausdorff << "\n";
						value.push_back(hausdorff);
					}
					if(line.indexOf("  mean") == 0)
					{
						//std::cout << line.toStdString().c_str() << "\n";
						QString meanStr(line.mid(9,10));
						//std::cout << meanStr.toStdString().c_str() << "\n";
						double mean = meanStr.toDouble();
						//std::cout << mean << "\n";
						value.push_back(mean);
					}
				}	

				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list6.at(i).toStdString();
				value.push_back(CalculateVolume(completePath2));

				// Volume 2
				value.push_back(CalculateVolume(referenzPath));

				LiveWireMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			for(std::map<int,std::vector<double>>::iterator it = LiveWireMap.begin(); it != LiveWireMap.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
		}

		if(m_PaintWipe1)
		{
			directory.cd("PaintWipe");
			directory.cd("Surface");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list7(directory.entryList());
			for(int i = 0; i < list7.size(); ++i)
			{
				if(list7.at(i).toStdString().compare(".") == 0)
					continue;
				if(list7.at(i).toStdString().compare("..") == 0)
					continue;
				//std::cout << list3.at(i).toLocal8Bit().constData() << "\n";
				
				// Fallunterscheidung: Scenario 1 vs. Scenario 2
				QString scenarioStr(list7.at(i).mid(0,10));
				if(scenarioStr.toStdString().compare("Scenario_1") == 0)
				{
					//std::cout << "S1: " << list3.at(i).toLocal8Bit().constData() << "\n";

					// get seconds
					QString secondsStr(list7.at(i).mid(27,3));
					//std::cout << secondsStr.toStdString().c_str() << "\n";
					if(secondsStr[2] == QChar('s'))
						secondsStr = secondsStr.mid(0,2);
					std::cout << secondsStr.toStdString().c_str() << "\n";
					int seconds = secondsStr.toInt();
					std::vector<double> value;
					std::string referenzPath;
					
					std::string outPath;
					if(AT_HOME == 1)
					{
						std::string completePath;
						completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list7.at(i).toStdString();
						completePath += "\" \"C:\\DA\\Data\\Scenario1\\Box4.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario1\\Box4.stl";
					}
					else
					{
						std::string completePath;
						completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list7.at(i).toStdString();
						completePath += "\" \"D:\\hachmann\\Data\\Scenario1\\Box4.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Box4.stl";
					}
					QFile file(outPath.c_str());
					if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
						 return;
					QTextStream in(&file);
					while (!in.atEnd()) {
						QString line = in.readLine();
						if(line.indexOf("#") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
						}
						if(line.indexOf("	area") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString areaStr(line.mid(8,20));
							//std::cout << areaStr.toStdString().c_str() << "\n";
							double area = areaStr.toDouble();
							//std::cout << area << "\n";
							value.push_back(area);
						}
						if(line.indexOf("Hausdorff distance") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString hausdorffStr(line.mid(20,5));
							//std::cout << hausdorffStr.toStdString().c_str() << "\n";
							double hausdorff = hausdorffStr.toDouble();
							//std::cout << hausdorff << "\n";
							value.push_back(hausdorff - 0.88);
						}
						if(line.indexOf("  mean") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString meanStr(line.mid(9,10));
							//std::cout << meanStr.toStdString().c_str() << "\n";
							double mean = meanStr.toDouble();
							//std::cout << mean << "\n";
							value.push_back(mean);
						}
					}	
					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list7.at(i).toStdString();
					value.push_back(CalculateVolume(completePath2));

					// Volume 2
					value.push_back(CalculateVolume(referenzPath));

					PaintWipeMap1.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
			}
			for(std::map<int,std::vector<double>>::iterator it = PaintWipeMap1.begin(); it != PaintWipeMap1.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}
			directory.cdUp();
			directory.cdUp();
		}
		if(m_PaintWipe2)
		{
			directory.cd("PaintWipe");
			directory.cd("Surface");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list7(directory.entryList());
			for(int i = 0; i < list7.size(); ++i)
			{
				if(list7.at(i).toStdString().compare(".") == 0)
					continue;
				if(list7.at(i).toStdString().compare("..") == 0)
					continue;
				//std::cout << list3.at(i).toLocal8Bit().constData() << "\n";
				
				// Fallunterscheidung: Scenario 1 vs. Scenario 2
				QString scenarioStr(list7.at(i).mid(0,10));
				if(scenarioStr.toStdString().compare("Scenario_2") == 0)
				{
					std::cout << "S2: " << list7.at(i).toLocal8Bit().constData() << "\n";

					// get seconds
					QString secondsStr(list7.at(i).mid(27,3));
					//std::cout << secondsStr.toStdString().c_str() << "\n";
					if(secondsStr[2] == QChar('s'))
						secondsStr = secondsStr.mid(0,2);
					std::cout << secondsStr.toStdString().c_str() << "\n";
					int seconds = secondsStr.toInt();
					std::vector<double> value;
					std::string referenzPath;
					
					std::string outPath;
					if(AT_HOME == 1)
					{
						std::string completePath;
						completePath += "C:\\DA\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list7.at(i).toStdString();
						completePath += "\" \"C:\\DA\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.stl";
					}
					else
					{
						std::string completePath;
						completePath += "D:\\hachmann\\Data\\Auswertungsdaten\\Metro\\metro.exe \"";
						completePath += directory.absolutePath().toStdString();
						completePath += "\\";
						completePath += list7.at(i).toStdString();
						completePath += "\" \"D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl\" > out.txt";
						//std::cout << "System call for metro: " << completePath.c_str() << "\n";
						//std::cout << "Current File: " << list4.at(i).toStdString() << "\n";
						system(completePath.c_str());
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.stl";
					}
					QFile file(outPath.c_str());
					if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
						 return;
					QTextStream in(&file);
					while (!in.atEnd()) {
						QString line = in.readLine();
						if(line.indexOf("#") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
						}
						if(line.indexOf("	area") == 0)          
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString areaStr(line.mid(8,20));
							//std::cout << areaStr.toStdString().c_str() << "\n";
							double area = areaStr.toDouble();
							//std::cout << area << "\n";
							value.push_back(area);
						}
						if(line.indexOf("Hausdorff distance") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString hausdorffStr(line.mid(20,5));
							//std::cout << hausdorffStr.toStdString().c_str() << "\n";
							double hausdorff = hausdorffStr.toDouble();
							//std::cout << hausdorff << "\n";
							value.push_back(hausdorff);
						}
						if(line.indexOf("  mean") == 0)
						{
							//std::cout << line.toStdString().c_str() << "\n";
							QString meanStr(line.mid(9,10));
							//std::cout << meanStr.toStdString().c_str() << "\n";
							double mean = meanStr.toDouble();
							//std::cout << mean << "\n";
							value.push_back(mean);
						}
					}	
					// Volume 1
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list7.at(i).toStdString();
					value.push_back(CalculateVolume(completePath2));

					// Volume 2
					value.push_back(CalculateVolume(referenzPath));

					PaintWipeMap2.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
				//std::cout << secondsStr.toStdString().c_str() << "\n";			
			}
			for(std::map<int,std::vector<double>>::iterator it = PaintWipeMap2.begin(); it != PaintWipeMap2.end(); it++)
			{
				std::cout << it->first << " ";
				for(int i = 0; i < it->second.size(); i++)
				{ 
					std::cout << (it->second)[i] << " ";
				}
				std::cout << "\n";
			}	
			directory.cdUp();
			directory.cdUp();
		}

		if(1)
		{
			// interpolate missing points
			if(m_ActiveSurface)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = ActiveSurfaceMap.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = ActiveSurfaceMap.begin(); it != ActiveSurfaceMap.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					ActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (ActiveSurfaceMap.rbegin())->first;
				std::vector<double> lastLine = (ActiveSurfaceMap.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					ActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (ActiveSurfaceMap.begin())->first;
				std::vector<double> firstLine = (ActiveSurfaceMap.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					ActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_AddSubstract1)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = AddSubstractMap1.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = AddSubstractMap1.begin(); it != AddSubstractMap1.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					AddSubstractMap1.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (AddSubstractMap1.rbegin())->first;
				std::vector<double> lastLine = (AddSubstractMap1.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					AddSubstractMap1.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (AddSubstractMap1.begin())->first;
				std::vector<double> firstLine = (AddSubstractMap1.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					AddSubstractMap1.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_AddSubstract2)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = AddSubstractMap2.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = AddSubstractMap2.begin(); it != AddSubstractMap2.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					AddSubstractMap2.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (AddSubstractMap2.rbegin())->first;
				std::vector<double> lastLine = (AddSubstractMap2.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					AddSubstractMap2.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (AddSubstractMap2.begin())->first;
				std::vector<double> firstLine = (AddSubstractMap2.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					AddSubstractMap2.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_Bulge)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = BulgeMap.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = BulgeMap.begin(); it != BulgeMap.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					BulgeMap.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (BulgeMap.rbegin())->first;
				std::vector<double> lastLine = (BulgeMap.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					BulgeMap.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (BulgeMap.begin())->first;
				std::vector<double> firstLine = (BulgeMap.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					BulgeMap.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_Drag)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = DragMap.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = DragMap.begin(); it != DragMap.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					DragMap.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (DragMap.rbegin())->first;
				std::vector<double> lastLine = (DragMap.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					DragMap.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (DragMap.begin())->first;
				std::vector<double> firstLine = (DragMap.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					DragMap.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_LiveWire)
			{
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = LiveWireMap.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = LiveWireMap.begin(); it != LiveWireMap.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					LiveWireMap.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (LiveWireMap.rbegin())->first;
				std::vector<double> lastLine = (LiveWireMap.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					LiveWireMap.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (LiveWireMap.begin())->first;
				std::vector<double> firstLine = (LiveWireMap.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					LiveWireMap.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_PaintWipe1)
			{
				// interpolate missing points
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = PaintWipeMap1.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = PaintWipeMap1.begin(); it != PaintWipeMap1.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					PaintWipeMap1.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (PaintWipeMap1.rbegin())->first;
				std::vector<double> lastLine = (PaintWipeMap1.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					PaintWipeMap1.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (PaintWipeMap1.begin())->first;
				std::vector<double> firstLine = (PaintWipeMap1.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					PaintWipeMap1.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
			if(m_PaintWipe2)
			{
				// interpolate missing points
				std::map<int,std::vector<double>> addToMap;
				std::map<int,std::vector<double>>::iterator it = PaintWipeMap2.begin();
				it++;
				for(std::map<int,std::vector<double>>::iterator it2 = PaintWipeMap2.begin(); it != PaintWipeMap2.end()--; it2++, it++)
				{
					// calculate difference
					int diffIndex = it->first - it2->first;
					//std::cout << "2first: " << it2->first << " ,1first: " << it->first <<" ,DiffIndex: " << diffIndex << "\n";
					for(int j = 1; j < diffIndex; j++)
					{
						int currentIndex = it2->first + j;
						std::vector<double> dummy;
						for(int i = 0; i < it->second.size(); i++)
						{ 
							//std::cout << "Second[i] orig: 1: " << it->second[i] << ", 2: " << it2->second[i] << "\n";
							double diffValues = it->second[i] - it2->second[i];
							double stepSize = diffValues / diffIndex;
							//std::cout << "StepSize: " << stepSize << ", diffValues: " << diffValues << "\n";

							dummy.push_back(it2->second[i] + j*stepSize);
						}
						//std::cout << "Add to map: index: currentIndex: " << currentIndex << " , vector: ";
						//for(std::vector<double>::iterator it4 = dummy.begin(); it4 != dummy.end(); it4++)
						//{
						//	std::cout << *it4 << " ";
						//}
						//std::cout << "\n";
						addToMap.insert(std::pair<int,std::vector<double>>(currentIndex, dummy));
					}
				}
				// add lines to original map
				//int k = 0;
				for(std::map<int,std::vector<double>>::iterator it3 = addToMap.begin(); it3 != addToMap.end(); it3++)
				{
					//std::cout << k << "\n";
					//k++;
					PaintWipeMap2.insert(std::pair<int,std::vector<double>>(it3->first, it3->second));
				}
				// interpolate last part
				int lastIndex = (PaintWipeMap2.rbegin())->first;
				std::vector<double> lastLine = (PaintWipeMap2.find(lastIndex))->second;
				for(int i = lastIndex+1; i < 125; i++)
				{
					PaintWipeMap2.insert(std::pair<int,std::vector<double>>(i, lastLine));
				}
				// interpolate first part (if necessary)
				int firstIndex = (PaintWipeMap2.begin())->first;
				std::vector<double> firstLine = (PaintWipeMap2.find(firstIndex))->second;
				for(int i = 0; i < firstIndex; i++)
				{
					PaintWipeMap2.insert(std::pair<int,std::vector<double>>(i, firstLine));
				}
			}
		}

		// write to csv
		if(1)
		{
			std::cout << "Write to csv file\n";
			// pro person
			std::string fileName;
			fileName += "./";
			fileName += *it;
			fileName += ".csv";
			QFile file(fileName.c_str());
			//QFile file("./file.csv");
			if (file.open(QFile::WriteOnly|QFile::Truncate))
			{
				QTextStream stream(&file);

				for(int i = 0; i < 125; i++)
				{
					stream << i << ";";
						
					if(m_ActiveSurface)
					{
						if(i == 0)
						{
							//std::cout << "CSV: ActiveSurface\n";
							stream << "AS: A1; AS: A2; AS: M1; AS: M2; AS: Hausdorff; AS: V1; AS: V2;";
						}
						else
						{
							//std::cout << "CSV: ActiveSurface: " << i << "\n";
							for(int j = 0; j < (ActiveSurfaceMap.begin()->second).size(); j++)
							{
								if(ActiveSurfaceMap.find(i) != ActiveSurfaceMap.end())
								{
									stream << ActiveSurfaceMap[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_AddSubstract1)
					{
						if(i == 0)
						{
							//std::cout << "CSV: AddSubstract1\n";
							stream << "AS1: A1; AS1: A2; AS1: M1; AS1: M2; AS1: Hausdorff; AS1: V1; AS1: V2;";
						}
						else
						{
							//std::cout << "CSV: AddSubstract1: " << i << "\n";
							for(int j = 0; j < (AddSubstractMap1.begin()->second).size(); j++)
							{
								if(AddSubstractMap1.find(i) != AddSubstractMap1.end())
								{
									stream << AddSubstractMap1[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_AddSubstract2)
					{
						if(i == 0)
						{
							//std::cout << "CSV: AddSubstract2\n";
							stream << "AS2: A1; AS2: A2; AS2: M1; AS2: M2; AS2: Hausdorff; AS2: V1; AS2: V2;";
						}
						else
						{
							//std::cout << "CSV: AddSubstract2: " << i << "\n";
							for(int j = 0; j < (AddSubstractMap2.begin()->second).size(); j++)
							{
								if(AddSubstractMap2.find(i) != AddSubstractMap2.end())
								{
									stream << AddSubstractMap2[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_Bulge)
					{
						if(i == 0)
						{
							//std::cout << "CSV: Bulge\n";
							stream << "B: A1; B: A2; B: M1; B: M2; B: Hausdorff; B: V1; B: V2;";
						}
						else
						{
							//std::cout << "CSV: Bulge: " << i << "\n";
							for(int j = 0; j < (BulgeMap.begin()->second).size(); j++)
							{
								if(BulgeMap.find(i) != BulgeMap.end())
								{
									stream << BulgeMap[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_Drag)
					{
						if(i == 0)
						{
							//std::cout << "CSV: Drag\n";
							stream << "D: A1; D: A2; D: M1; D: M2; D: Hausdorff; D: V1; D: V2;";
						}
						else
						{
							//std::cout << "CSV: Drag: " << i << "\n";
							for(int j = 0; j < (DragMap.begin()->second).size(); j++)
							{
								if(DragMap.find(i) != DragMap.end())
								{
									stream << DragMap[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_LiveWire)
					{
						if(i == 0)
						{
							//std::cout << "CSV: LiveWire\n";
							stream << "L: A1; L: A2; L: M1; L: M2; L: Hausdorff; L: V1; L: V2;";
						}
						else
						{
							//std::cout << "CSV: LiveWire: " << i << "\n";
							for(int j = 0; j < (LiveWireMap.begin()->second).size(); j++)
							{
								if(LiveWireMap.find(i) != LiveWireMap.end())
								{
									stream << LiveWireMap[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_PaintWipe1)
					{
						if(i == 0)
						{
							//std::cout << "CSV: PaintWipe1\n";
							stream << "P1: A1; P1: A2; P1: M1; P1: M2; P1: Hausdorff; P1: V1; P1: V2;";
						}
						else
						{
							//std::cout << "CSV: PaintWipe1: " << i << "\n";
							for(int j = 0; j < (PaintWipeMap1.begin()->second).size(); j++)
							{
								if(PaintWipeMap1.find(i) != PaintWipeMap1.end())
								{
									stream << PaintWipeMap1[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << ";";

					if(m_PaintWipe2)
					{
						if(i == 0)
						{
							//std::cout << "CSV: PaintWipe2\n";
							stream << "P2: A1; P2: A2; P2: M1; P2: M2; P2: Hausdorff; P2: V1; P2: V2;";
						}
						else
						{
							//std::cout << "CSV: PaintWipe2: " << i << "\n";
							for(int j = 0; j < (PaintWipeMap2.begin()->second).size(); j++)
							{
								if(PaintWipeMap2.find(i) != PaintWipeMap2.end())
								{
									stream << PaintWipeMap2[i][j] << ";";
								}
								else
								{
									stream << ";";
								}
							}
						}
					}
					stream << "\n";
				}
				file.close();
			}
		}

		if(1)
		{
			// Create global (for all persons) maps
			// Assumption: all maps are interpolated
			if(m_ActiveSurface)
			{
				for(std::map<int,std::vector<double>>::iterator it = ActiveSurfaceMap.begin(); it != ActiveSurfaceMap.end(); it++)
				{
					if(globalActiveSurfaceMap.find(it->first) != globalActiveSurfaceMap.end())
					{
						std::vector<double> globalLine = globalActiveSurfaceMap.find(it->first)->second;
						for(int i = 0; i < globalActiveSurfaceMap.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalActiveSurfaceMap[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_AddSubstract1)
			{
				for(std::map<int,std::vector<double>>::iterator it = AddSubstractMap1.begin(); it != AddSubstractMap1.end(); it++)
				{
					if(globalAddSubstractMap1.find(it->first) != globalAddSubstractMap1.end())
					{
						std::vector<double> globalLine = globalAddSubstractMap1.find(it->first)->second;
						for(int i = 0; i < globalAddSubstractMap1.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalAddSubstractMap1[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_AddSubstract2)
			{
				for(std::map<int,std::vector<double>>::iterator it = AddSubstractMap2.begin(); it != AddSubstractMap2.end(); it++)
				{
					if(globalAddSubstractMap2.find(it->first) != globalAddSubstractMap2.end())
					{
						std::vector<double> globalLine = globalAddSubstractMap2.find(it->first)->second;
						for(int i = 0; i < globalAddSubstractMap2.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalAddSubstractMap2[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_Bulge)
			{
				for(std::map<int,std::vector<double>>::iterator it = BulgeMap.begin(); it != BulgeMap.end(); it++)
				{
					if(globalBulgeMap.find(it->first) != globalBulgeMap.end())
					{
						std::vector<double> globalLine = globalBulgeMap.find(it->first)->second;
						for(int i = 0; i < globalBulgeMap.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalBulgeMap[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_Drag)
			{
				for(std::map<int,std::vector<double>>::iterator it = DragMap.begin(); it != DragMap.end(); it++)
				{
					if(globalDragMap.find(it->first) != globalDragMap.end())
					{
						std::vector<double> globalLine = globalDragMap.find(it->first)->second;
						for(int i = 0; i < globalDragMap.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalDragMap[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_LiveWire)
			{
				for(std::map<int,std::vector<double>>::iterator it = LiveWireMap.begin(); it != LiveWireMap.end(); it++)
				{
					if(globalLiveWireMap.find(it->first) != globalLiveWireMap.end())
					{
						std::vector<double> globalLine = globalLiveWireMap.find(it->first)->second;
						for(int i = 0; i < globalLiveWireMap.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalLiveWireMap[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_PaintWipe1)
			{
				for(std::map<int,std::vector<double>>::iterator it = PaintWipeMap1.begin(); it != PaintWipeMap1.end(); it++)
				{
					if(globalPaintWipeMap1.find(it->first) != globalPaintWipeMap1.end())
					{
						std::vector<double> globalLine = globalPaintWipeMap1.find(it->first)->second;
						for(int i = 0; i < globalPaintWipeMap1.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalPaintWipeMap1[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
			if(m_PaintWipe2)
			{
				for(std::map<int,std::vector<double>>::iterator it = PaintWipeMap2.begin(); it != PaintWipeMap2.end(); it++)
				{
					if(globalPaintWipeMap2.find(it->first) != globalPaintWipeMap2.end())
					{
						std::vector<double> globalLine = globalPaintWipeMap2.find(it->first)->second;
						for(int i = 0; i < globalPaintWipeMap2.find(it->first)->second.size(); i++)
						{
							globalLine[i] += it->second[i] / m_Names.size();
						}
						globalPaintWipeMap2[it->first] = globalLine;
					}
					else
					{
						std::cout << "Error: Index not in map\n";
					}
				}
			}
		}
	}

	if(1)
	{
		WriteGlobalMapToCVS(globalActiveSurfaceMap, globalAddSubstractMap1, globalAddSubstractMap2, 
			                globalBulgeMap, globalDragMap, globalLiveWireMap, globalPaintWipeMap1, 
							globalPaintWipeMap2, m_ActiveSurface, m_AddSubstract1, m_AddSubstract2,
						    m_Bulge, m_Drag, m_LiveWire, m_PaintWipe1, m_PaintWipe2);
	}	
	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::TestTool::ConvertData()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// walk the folder structur
	for(std::vector<std::string>::iterator it = m_Names.begin(); it != m_Names.end(); it++)
	{
		QString currentPath(m_Directory.c_str());
		currentPath.append(it->c_str());
		QDir directory(currentPath);
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		QStringList list(directory.entryList());

		directory.cd("ActiveSurface");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		QStringList list2(directory.entryList());
		int numberOfFiles = directory.count();
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		for(int i = 0; i < list2.size(); ++i)
		{
			if(list2.at(i).toStdString().compare(".") == 0)
				continue;
			if(list2.at(i).toStdString().compare("..") == 0)
				continue;
			//std::cout << list2.at(i).toLocal8Bit().constData() << "\n";

			if(list2.at(i).endsWith(".nrrd"))
			{
				// check if already converted
				//std::cout << "Nrrd file found\n";

				directory.cd("Surface");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list2.at(i).toStdString();

					mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
					nodeReader->SetFileName(completePath);
					try
					{
						nodeReader->Update();
					}
					catch (itk::ExceptionObject &ex)
					{
						std::cout << ex << std::endl;
					}
					catch (std::exception &ex1)
					{
						ex1.what();
					}

					// Adding file/image to the DataNode
					mitk::DataNode::Pointer node;
					node = nodeReader->GetOutput(0);
					mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
					mitk::Surface::Pointer surface = ConvertAndSmoothData(image);

					// Add to Surface folder			
					int stringSize = list2.at(i).size();
					QString nameWithSuffix(list2.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					saveDir += "stl";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();
					mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
					surfaceWriter->SetInput( surface );
					surfaceWriter->SetFileName(saveDir.c_str());
					surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
					surfaceWriter->Write();
				}
				directory.cdUp();
			}
		}
		directory.cdUp();

		directory.cd("AddSubstract");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		QStringList list3(directory.entryList());
		numberOfFiles = directory.count();
		for(int i = 0; i < list3.size(); ++i)
		{
			if(list3.at(i).toStdString().compare(".") == 0)
				continue;
			if(list3.at(i).toStdString().compare("..") == 0)
				continue;
			//std::cout << list3.at(i).toLocal8Bit().constData() << "\n";

			if(list3.at(i).endsWith(".nrrd"))
			{
				// check if already converted
				//std::cout << "Nrrd file found\n";

				directory.cd("Surface");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list3.at(i).toStdString();

					mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
					nodeReader->SetFileName(completePath);
					try
					{
						nodeReader->Update();
					}
					catch (itk::ExceptionObject &ex)
					{
						std::cout << ex << std::endl;
					}
					catch (std::exception &ex1)
					{
						ex1.what();
					}

					// Adding file/image to the DataNode
					mitk::DataNode::Pointer node;
					node = nodeReader->GetOutput(0);
					mitk::Surface::Pointer surface = ConvertAndSmoothData((mitk::Image*)node->GetData());

					// Add to Surface folder			
					int stringSize = list3.at(i).size();
					QString nameWithSuffix(list3.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					saveDir += "stl";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();
					mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
					surfaceWriter->SetInput( surface );
					surfaceWriter->SetFileName(saveDir.c_str());
					surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
					surfaceWriter->Write();
				}
				directory.cdUp();
			}
		}
		directory.cdUp();

		directory.cd("PaintWipe");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		numberOfFiles = directory.count();
		QStringList list4(directory.entryList());
		for(int i = 0; i < list4.size(); ++i)
		{
			if(list4.at(i).toStdString().compare(".") == 0)	// get als 2 files durch, bei .count()
				continue;
			if(list4.at(i).toStdString().compare("..") == 0)
				continue;
			std::cout << list4.at(i).toLocal8Bit().constData() << "\n";

			if(list4.at(i).endsWith(".nrrd"))
			{
				// check if already converted
				//std::cout << "Nrrd file found\n";

				directory.cd("Surface");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list4.at(i).toStdString();

					mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
					nodeReader->SetFileName(completePath);
					try
					{
						nodeReader->Update();
					}
					catch (itk::ExceptionObject &ex)
					{
						std::cout << ex << std::endl;
					}
					catch (std::exception &ex1)
					{
						ex1.what();
					}

					// Adding file/image to the DataNode
					mitk::DataNode::Pointer node;
					node = nodeReader->GetOutput(0);
					mitk::Surface::Pointer surface = ConvertAndSmoothData((mitk::Image*)node->GetData());

					// Add to Surface folder			
					int stringSize = list4.at(i).size();
					QString nameWithSuffix(list4.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					saveDir += "stl";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					mitk::Surface::Pointer data = (mitk::Surface*)node->GetData();
					mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
					surfaceWriter->SetInput( surface );
					surfaceWriter->SetFileName(saveDir.c_str());
					surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
					surfaceWriter->Write();
				}
				directory.cdUp();
			}
		}

		std::cout << "Surface to Pixel\n";
		directory.cdUp();
		std::cout << "Current Path LiveWire: " << directory.absolutePath().toStdString().c_str() << "\n";
		directory.cd("LiveWire");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		numberOfFiles = directory.count();
		QStringList list5(directory.entryList());
		for(int i = 0; i < list5.size(); ++i)
		{
			if(list5.at(i).toStdString().compare(".") == 0)	// get als 2 files durch, bei .count()
				continue;
			if(list5.at(i).toStdString().compare("..") == 0)
				continue;
			std::cout << list5.at(i).toLocal8Bit().constData() << "\n";

			if(list5.at(i).endsWith(".stl"))
			{
				// check if already converted
				//std::cout << "Stl file found\n";

				directory.cd("Pixel");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list5.at(i).toStdString();

					// Get reference image for geometry
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						referenzPath = "C:\\DA\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd";
					}
					else
					{
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd";
					}

					// Add to Pixel folder			
					int stringSize = list5.at(i).size();
					QString nameWithSuffix(list5.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					//saveDir += ".nrrd";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					LoadConvertAndSave(referenzPath, completePath, saveDir);
				}
				directory.cdUp();
			}
		}
		directory.cdUp();
		std::cout << "Current Path Bulge: " << directory.absolutePath().toStdString().c_str() << "\n";
		directory.cd("Bulge");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		numberOfFiles = directory.count();
		QStringList list6(directory.entryList());
		for(int i = 0; i < list6.size(); ++i)
		{
			if(list6.at(i).toStdString().compare(".") == 0)	// get als 2 files durch, bei .count()
				continue;
			if(list6.at(i).toStdString().compare("..") == 0)
				continue;
			std::cout << list6.at(i).toLocal8Bit().constData() << "\n";

			if(list6.at(i).endsWith(".stl"))
			{
				// check if already converted
				//std::cout << "Stl file found\n";

				directory.cd("Pixel");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list6.at(i).toStdString();

					// Get reference image for geometry
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_Segmentation_2D.nrrd";
					}
					else
					{
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_Segmentation_2D.nrrd";
					}

					// Add to Pixel folder			
					int stringSize = list6.at(i).size();
					QString nameWithSuffix(list6.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					//saveDir += ".nrrd";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					LoadConvertAndSave(referenzPath, completePath, saveDir);
				}
				directory.cdUp();
			}
		}
		directory.cdUp();
		std::cout << "Current Path Drag: " << directory.absolutePath().toStdString().c_str() << "\n";
		directory.cd("Drag");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		numberOfFiles = directory.count();
		QStringList list7(directory.entryList());
		for(int i = 0; i < list7.size(); ++i)
		{
			if(list7.at(i).toStdString().compare(".") == 0)	// get als 2 files durch, bei .count()
				continue;
			if(list7.at(i).toStdString().compare("..") == 0)
				continue;
			std::cout << list7.at(i).toLocal8Bit().constData() << "\n";

			if(list7.at(i).endsWith(".stl"))
			{
				// check if already converted
				//std::cout << "Stl file found\n";

				directory.cd("Pixel");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list7.at(i).toStdString();

					// Get reference image for geometry
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_Segmentation_2D.nrrd";
					}
					else
					{
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_Segmentation_2D.nrrd";
					}

					// Add to Pixel folder			
					int stringSize = list7.at(i).size();
					QString nameWithSuffix(list7.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					//saveDir += ".nrrd";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					LoadConvertAndSave(referenzPath, completePath, saveDir);
				}
				directory.cdUp();
			}
		}
		directory.cdUp();
		std::cout << "Current Path ActiveSurface: " << directory.absolutePath().toStdString().c_str() << "\n";
		directory.cd("ActiveSurface");
		directory.cd("Surface");
		//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
		numberOfFiles = directory.count();
		QStringList list8(directory.entryList());
		for(int i = 0; i < list8.size(); ++i)
		{
			if(list8.at(i).toStdString().compare(".") == 0)	// get als 2 files durch, bei .count()
				continue;
			if(list8.at(i).toStdString().compare("..") == 0)
				continue;
			std::cout << list8.at(i).toLocal8Bit().constData() << "\n";

			if(list8.at(i).endsWith(".stl"))
			{
				// check if already converted
				//std::cout << "Stl file found\n";

				directory.cd("Pixel");
				//std::cout << "Number of entries in Surface folder: " << directory.count() << "\n";
				if(numberOfFiles != directory.count()+1)
				{
					std::cout << "Conversion!\n";
					
					std::string completePath;
					completePath += directory.absolutePath().toStdString();
					completePath += "\\..\\";
					completePath += list8.at(i).toStdString();

					// Get reference image for geometry
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						referenzPath = "C:\\DA\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd";
					}
					else
					{
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Scenario_2_Segmentation_2D.nrrd";
					}

					// Add to Pixel folder			
					int stringSize = list8.at(i).size();
					QString nameWithSuffix(list8.at(i));
					nameWithSuffix.resize(stringSize-4);
					std::string saveDir;
					saveDir += directory.absolutePath().toStdString();
					saveDir += "\\";
					saveDir += nameWithSuffix.toStdString();
					//saveDir += ".nrrd";
					std::cout << "Save path: " << saveDir.c_str() << "\n";

					LoadConvertAndSave(referenzPath, completePath, saveDir);
				}
				directory.cdUp();
			}
		}
	}	

	// Sum up data for all datasets


	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

mitk::Surface::Pointer mitk::TestTool::ConvertAndSmoothData(mitk::Image::Pointer image)
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;
	
	if(image.IsNull()) std::cout << "Error: Image is NULL\n";

	bool smooth(true);
	bool applyMedian(false);
	bool decimateMesh(true);
	unsigned int medianKernelSize(3u);
	float gaussianSD(2.5f);
	float reductionRate(0.80f);

	mitk::ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = mitk::ManualSegmentationToSurfaceFilter::New();
	surfaceFilter->SetInput( image );
	surfaceFilter->SetThreshold( 1 ); //expects binary image with zeros and ones

	surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
	if (smooth)
	{
		surfaceFilter->InterpolationOn();
		surfaceFilter->SetGaussianStandardDeviation( gaussianSD );
	}

	surfaceFilter->SetMedianFilter3D(applyMedian); // apply median to segmentation before marching cubes ?
	if (applyMedian)
	{
		surfaceFilter->SetMedianKernelSize(medianKernelSize, medianKernelSize, medianKernelSize); // apply median to segmentation before marching cubes
	}

	//fix to avoid vtk warnings see bug #5390
	if ( image->GetDimension() > 3 )
		decimateMesh = false;

	if (decimateMesh)
	{
		surfaceFilter->SetDecimate( mitk::ImageToSurfaceFilter::QuadricDecimation );
		surfaceFilter->SetTargetReduction( reductionRate );
	}
	else
	{
		surfaceFilter->SetDecimate( mitk::ImageToSurfaceFilter::NoDecimation );
	}

	surfaceFilter->UpdateLargestPossibleRegion();

	// calculate normals for nicer display
	mitk::Surface::Pointer m_Surface = surfaceFilter->GetOutput();

	vtkPolyData* polyData = m_Surface->GetVtkPolyData();

	if (!polyData) throw std::logic_error("Could not create polygon model");

	polyData->SetVerts(0);
	polyData->SetLines(0);

	if ( smooth || applyMedian || decimateMesh)
	{
		vtkPolyDataNormals* normalsGen = vtkPolyDataNormals::New();
		normalsGen->SetInput( polyData );
		normalsGen->Update();
		m_Surface->SetVtkPolyData( normalsGen->GetOutput() );
		normalsGen->Delete();
	}
	else
	{
		m_Surface->SetVtkPolyData( polyData );
	}

	//mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
	//surfaceNode->SetData(m_Surface);
	//surfaceNode->SetName("Surface");
	//m_Data->getDataStore()->Add(surfaceNode);

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return m_Surface;
}

void mitk::TestTool::LoadConvertAndSave(std::string refPath, std::string surfacePath, std::string imagePath)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::GrayscaleMorphologicalClosingImageFilter< SegmentationType, SegmentationType, CrossType > CrossClosingFilterType;

	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
	nodeReader->SetFileName(surfacePath);
	try
	{
		nodeReader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
	}
	catch (std::exception &ex1)
	{
		ex1.what();
	}

	// Adding file/image to the DataNode
	mitk::DataNode::Pointer node;
	node = nodeReader->GetOutput(0);

	// Get reference image for geometry
	mitk::DataNodeFactory::Pointer nodeReader2 = mitk::DataNodeFactory::New();
	nodeReader2->SetFileName(refPath);
	try
	{
		nodeReader2->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
	}
	catch (std::exception &ex1)
	{
		ex1.what();
	}

	// Adding file/image to the DataNode
	mitk::DataNode::Pointer nodeRef;
	nodeRef = nodeReader2->GetOutput(0);

	// Create reference image for geometric information
	mitk::DataNode::Pointer node2 = mitk::DataNode::New();
	mitk::PixelType pixelType(typeid(short));
	mitk::Image::Pointer out = mitk::Image::New();
	mitk::Geometry3D::Pointer geo = mitk::Geometry3D::New();					
	geo = ((mitk::Image*)nodeRef->GetData())->GetGeometry();					
	//geo->ChangeImageGeometryConsideringOriginOffset(true);
	//geo->SetBounds( ((mitk::Surface*)node->GetData())->GetGeometry()->GetBounds() );			
	out->Initialize(pixelType, *geo);
	mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
	filter->SetInput((mitk::Surface*)node->GetData());						
	filter->SetImage(out);
	filter->SetMakeOutputBinary(true);
	filter->Update();

	// Closing von 1
	SegmentationType::Pointer itkImage = SegmentationType::New();
	mitk::CastToItkImage((mitk::Image*)((mitk::BaseData*)filter->GetOutput(0)), itkImage);
	mitk::Image::Pointer new_image = mitk::Image::New();

	CrossType m_Cross;
	CrossClosingFilterType::Pointer m_CrossClosingFilter;

	m_Cross.SetRadius(1.0);
	m_Cross.CreateStructuringElement();

	m_CrossClosingFilter = CrossClosingFilterType::New();
	m_CrossClosingFilter->SetKernel(m_Cross);
	m_CrossClosingFilter->SetInput(itkImage);
	//m_CrossClosingFilter->SetForegroundValue(1);
	//m_CrossClosingFilter->SetBackgroundValue(0);
	m_CrossClosingFilter->SetSafeBorder(true);
	m_CrossClosingFilter->UpdateLargestPossibleRegion();

	mitk::CastToMitkImage(m_CrossClosingFilter->GetOutput(), new_image);

	mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
	imageWriter->SetInput(new_image);
	imageWriter->SetFileName(imagePath.c_str());
	imageWriter->SetExtension(".nrrd");
	imageWriter->Write();

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

double mitk::TestTool::CalculateVolume(std::string path)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load surface
	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
	nodeReader->SetFileName(path);
	try
	{
		nodeReader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
	}
	catch (std::exception &ex1)
	{
		ex1.what();
	}

	// Adding file/image to the DataNode
	mitk::DataNode::Pointer node;
	node = nodeReader->GetOutput(0);

	// calculate volume
	vtkSmartPointer<vtkMassProperties> vol = vtkMassProperties::New();
	vol->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
	std::cout << "Volume: " << vol->GetVolume() << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return vol->GetVolume();
}

double mitk::TestTool::CalculateVolumeViaPixel(std::string path)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load surface
	mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
	nodeReader->SetFileName(path);
	try
	{
		nodeReader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
	}
	catch (std::exception &ex1)
	{
		ex1.what();
	}

	// Adding file/image to the DataNode
	mitk::DataNode::Pointer node;
	node = nodeReader->GetOutput(0);
	double pixelCounter = 0;
	mitk::BaseData::Pointer data = node->GetData();
	std::cout << "Name of Class: " << data->GetNameOfClass() << "\n";
	std::string ref = "Surface";
	if(ref.compare(data->GetNameOfClass()) == 0)
	{
		// Surface found
		std::cout << "Surface found\n";

		// Calculate Properties
		vtkSmartPointer<vtkMassProperties> vol = vtkMassProperties::New();
		vol->SetInput(((mitk::Surface*)node->GetData())->GetVtkPolyData());
		vol->Update();
		std::cout << "Volume: " << vol->GetVolume() << "\n";

		// Convert surface
		mitk::PixelType pixelType(typeid(short));
		mitk::Image::Pointer out = mitk::Image::New();
		mitk::Geometry3D::Pointer geo = mitk::Geometry3D::New();					
		geo = ((mitk::Surface*)node->GetData())->GetGeometry();					
		//geo->ChangeImageGeometryConsideringOriginOffset(true);
		geo->SetBounds( ((mitk::Surface*)node->GetData())->GetGeometry()->GetBounds() );
		out->Initialize(pixelType, *geo);

		mitk::SurfaceToImageFilter::Pointer filter = mitk::SurfaceToImageFilter::New();
		filter->SetInput((mitk::Surface*)node->GetData());						
		filter->SetImage(out);
		filter->SetMakeOutputBinary(true);
		filter->Update();
		mitk::Image::Pointer image = (mitk::Image*)filter->GetOutput(0);

		// count pixels
		vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
		int* dims = imageData->GetDimensions();  // TODO: Warum stürzt der hier ab?
		// int dims[3]; // can't do this
		//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
		for (int z = 0; z < dims[2]; z++)
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					if(pixel[0] > 0)
					{
						pixelCounter++;
					}
				}
			}
		}
		std::cout << "Number of pixels in Image: " << pixelCounter << "\n";
	}
	std::string ref2 = "Image";
	if(ref2.compare(data->GetNameOfClass()) == 0)
	{
		// Image found
		std::cout << "Image found\n";

		// count pixels
		vtkSmartPointer<vtkImageData> imageData = ((mitk::Image*)node->GetData())->GetVtkImageData();
		int* dims = imageData->GetDimensions();  // TODO: Warum stürzt der hier ab?
		// int dims[3]; // can't do this
		//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
		for (int z = 0; z < dims[2]; z++)
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					if(pixel[0] > 0)
					{
						pixelCounter++;
					}
				}
			}
		}
		std::cout << "Number of pixels in Image: " << pixelCounter << "\n";
	}


	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return pixelCounter;
}

void mitk::TestTool::Closing()
{
	std::cout << "hi from " << __FUNCSIG__ << std::endl;

	typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::GrayscaleMorphologicalClosingImageFilter< SegmentationType, SegmentationType, CrossType > CrossClosingFilterType;

	// walk the folder structur
	for(std::vector<std::string>::iterator it = m_Names.begin(); it != m_Names.end(); it++)
	{
		QString currentPath(m_Directory.c_str());
		currentPath.append(it->c_str());
		QDir directory(currentPath);
		QStringList list(directory.entryList());

		directory.cd("ActiveSurface");

		if(!directory.exists("OriginalData"))
		{
			std::cout << "No Original Data Folder -> start closing\n";
			directory.mkdir("OriginalData");
			QStringList list2(directory.entryList());
			for(int i = 0; i < list2.size(); ++i)
			{
				if(list2.at(i).toStdString().compare(".") == 0)
					continue;
				if(list2.at(i).toStdString().compare("..") == 0)
					continue;
				std::cout << list2.at(i).toLocal8Bit().constData() << "\n";

				if(list2.at(i).endsWith(".nrrd"))
				{
					QString newPath;
					newPath += "OriginalData\\";
					newPath += list2.at(i);
					std::cout << "Orig: " << list2.at(i).toStdString().c_str() << ", new path: " << newPath.toStdString().c_str() << "\n";
					directory.rename(list2.at(i), newPath);
				}
			}
				
			QString basePath;
			basePath = directory.absolutePath();
			directory.cd("OriginalData");
			QStringList list3(directory.entryList());
			for(int i = 0; i < list3.size(); ++i)
			{
				if(list3.at(i).toStdString().compare(".") == 0)
					continue;
				if(list3.at(i).toStdString().compare("..") == 0)
					continue;
				std::cout << list3.at(i).toLocal8Bit().constData() << "\n";

				if(list3.at(i).endsWith(".nrrd"))
				{
					QString newPath;
					newPath += directory.absolutePath();
					newPath += "//";
					newPath += list3.at(i);

					mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
					nodeReader->SetFileName(newPath.toStdString().c_str());
					try
					{
						nodeReader->Update();
					}
					catch (itk::ExceptionObject &ex)
					{
						std::cout << ex << std::endl;
					}
					catch (std::exception &ex1)
					{
						ex1.what();
					}

					// Adding file/image to the DataNode
					mitk::DataNode::Pointer node;
					node = nodeReader->GetOutput(0);

					SegmentationType::Pointer itkImage = SegmentationType::New();
					mitk::CastToItkImage((mitk::Image*)(node->GetData()), itkImage);
					mitk::Image::Pointer new_image = mitk::Image::New();

					CrossType m_Cross;
					CrossClosingFilterType::Pointer m_CrossClosingFilter;

					m_Cross.SetRadius(1.0);
					m_Cross.CreateStructuringElement();

					m_CrossClosingFilter = CrossClosingFilterType::New();
					m_CrossClosingFilter->SetKernel(m_Cross);
					m_CrossClosingFilter->SetInput(itkImage);
					//m_CrossClosingFilter->SetForegroundValue(1);
					//m_CrossClosingFilter->SetBackgroundValue(0);
					m_CrossClosingFilter->SetSafeBorder(true);
					m_CrossClosingFilter->UpdateLargestPossibleRegion();

					mitk::CastToMitkImage(m_CrossClosingFilter->GetOutput(), new_image);

					//mitk::DataNode::Pointer newImageNode = mitk::DataNode::New();
					//newImageNode->SetData(new_image);
					//newImageNode->SetName("ClosedImage");
					//m_Storage->Add(newImageNode);

					QString savePath;
					savePath += basePath;
					savePath += "\\";
					savePath += list3.at(i);

					mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
					imageWriter->SetInput(new_image);
					imageWriter->SetFileName(savePath.toStdString().c_str());
					imageWriter->SetExtension("");
					imageWriter->Write();
				}
			}
		}
	}
	// Go into Active Surface Folder
	// Create Original Folder
	// Copy original data to this folder
	// Close data by 1

	std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void mitk::TestTool::AuswertungPixel()
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;
	WriteToCVSPixel(this);
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}