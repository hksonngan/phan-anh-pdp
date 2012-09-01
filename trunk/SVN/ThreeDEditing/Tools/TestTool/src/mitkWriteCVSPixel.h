// "pdp"
#include "mitkTestTool.h"
#include "mitkWriteGlobalToCVSPixel.h"

// MITK
#include <mitkDataNodeFactory.h>
#include "mitkSurfaceToImageFilter.h"
#include <mitkImageCast.h>
#include <mitkImageWriter.h>

// VTK
#include <vtkMassProperties.h>
#include <vtkSmartPointer.h>
#include <vtkSTLWriter.h>
#include <vtkPolyDataNormals.h>
#include <vtkImageData.h>

// QT
#include <QTextStream>
#include <QFile>
#include <QDir>

double CalculateVolumeViaPixel2(std::string);
double CalculateDiceKoeffizient(std::string, std::string);
double CalculateTanimotoKoeffizient(std::string, std::string);
double CalculateUebersegmentierung(std::string, std::string);
mitk::Image::Pointer LoadImage(std::string);
void PrintMap(std::map<int,std::vector<double>>);

void WriteToCVSPixel(mitk::TestTool* tt)
{
	std::cout << "hi from WriteToCVSPixel()" << std::endl;

	std::string m_Directory = tt->GetDirectory();
	std::cout << "Directory: " << m_Directory << "\n";
	bool m_ActiveSurface = tt->GetActiveSurfaceBool();
	bool m_AddSubstract1 = tt->GetAddSubstract1Bool();
	bool m_AddSubstract2 = tt->GetAddSubstract2Bool();
	bool m_Bulge = tt->GetBulgeBool();
	bool m_Drag = tt->GetDragBool();
	bool m_LiveWire = tt->GetLiveWireBool();
	bool m_PaintWipe1 = tt->GetPaintWipe1Bool();
	bool m_PaintWipe2 = tt->GetPaintWipe2Bool();
	std::vector<std::string> m_Names = tt->GetNames();
	mitk::DataStorage* m_Storage = tt->GetStorage();

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
			directory.cd("Pixel");
			//std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list2(directory.entryList());
			for(int i = 0; i < list2.size(); ++i)
			{
				if(list2.at(i).toStdString().compare(".") == 0)
					continue;
				if(list2.at(i).toStdString().compare("..") == 0)
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
				
				std::string outPath;
				std::string referenzPath;
				if(AT_HOME == 1)
				{
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.nrrd";
				}
				else
				{
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.nrrd";
				}

				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list2.at(i).toStdString();
				value.push_back(CalculateVolumeViaPixel2(completePath2));

				// Dice
				value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

				// Tanimoto
				value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));

				// Uebersegmentierung
				value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

				ActiveSurfaceMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			PrintMap(ActiveSurfaceMap);
			directory.cdUp();
			directory.cdUp();
			directory.cdUp();
		}

		if(m_AddSubstract1)
		{
			directory.cd("AddSubstract");
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
					
					std::string outPath;
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
					}
					else
					{
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
					}

					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list3.at(i).toStdString();
					value.push_back(CalculateVolumeViaPixel2(completePath2));

					// Dice
					value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

					// Tanimoto
					value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));	

					// Uebersegmentierung
					value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

					AddSubstractMap1.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
			}
			PrintMap(AddSubstractMap1);
			directory.cdUp();
		}

		if(m_AddSubstract2)
		{
			directory.cd("AddSubstract");
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
					
					std::string outPath;
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.nrrd";
					}
					else
					{
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.nrrd";
					}

					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list3.at(i).toStdString();
					value.push_back(CalculateVolumeViaPixel2(completePath2));

					// Dice
					value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

					// Tanimoto
					value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));	

					// Uebersegmentierung
					value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

					AddSubstractMap2.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
				//std::cout << secondsStr.toStdString().c_str() << "\n";			
			}
			PrintMap(AddSubstractMap2);
			directory.cdUp();
		}

		if(m_Bulge)
		{
			directory.cd("Bulge");
			directory.cd("Pixel");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list4(directory.entryList());
			for(int i = 0; i < list4.size(); ++i)
			{
				if(list4.at(i).toStdString().compare(".") == 0)
					continue;
				if(list4.at(i).toStdString().compare("..") == 0)
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
				
				std::string outPath;
				std::string referenzPath;
				if(AT_HOME == 1)
				{
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
				}
				else
				{
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
				}

				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list4.at(i).toStdString();
				value.push_back(CalculateVolumeViaPixel2(completePath2));

				// Dice
				value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

				// Tanimoto
				value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));		

				// Uebersegmentierung
				value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

				BulgeMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			PrintMap(BulgeMap);
			directory.cdUp();
			directory.cdUp();
		}

		if(m_Drag)
		{
			directory.cd("Drag");
			directory.cd("Pixel");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list5(directory.entryList());
			for(int i = 0; i < list5.size(); ++i)
			{
				if(list5.at(i).toStdString().compare(".") == 0)
					continue;
				if(list5.at(i).toStdString().compare("..") == 0)
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
				
				std::string outPath;
				std::string referenzPath;
				if(AT_HOME == 1)
				{
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
				}
				else
				{
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
				}

				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list5.at(i).toStdString();
				value.push_back(CalculateVolumeViaPixel2(completePath2));

				// Dice
				value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

				// Tanimoto
				value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));		

				// Uebersegmentierung
				value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

				DragMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			PrintMap(DragMap);
			directory.cdUp();
			directory.cdUp();
		}

		if(m_LiveWire)
		{
			directory.cd("LiveWire");
			directory.cd("Pixel");
			std::cout << "Number of files and folders in directory: " << directory.count() << "\n";
			QStringList list6(directory.entryList());
			for(int i = 0; i < list6.size(); ++i)
			{
				if(list6.at(i).toStdString().compare(".") == 0)
					continue;
				if(list6.at(i).toStdString().compare("..") == 0)
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

				std::string outPath;
				std::string referenzPath;
				if(AT_HOME == 1)
				{
					outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.nrrd";
				}
				else
				{
					outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
					referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.nrrd";
				}

				// Volume
				std::string completePath2;
				completePath2 += directory.absolutePath().toStdString();
				completePath2 += "\\";
				completePath2 += list6.at(i).toStdString();
				value.push_back(CalculateVolumeViaPixel2(completePath2));

				// Dice
				value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

				// Tanimoto
				value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));

				// Uebersegmentierung
				value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));
				
				LiveWireMap.insert(std::pair<int,std::vector<double>>(seconds,value));
			}
			PrintMap(LiveWireMap);
			directory.cdUp();
			directory.cdUp();
		}

		if(m_PaintWipe1)
		{
			directory.cd("PaintWipe");
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
				
					std::string outPath;
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
					}
					else
					{
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario1\\Scenario_1_FullBox.nrrd";
					}

					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list7.at(i).toStdString();
					value.push_back(CalculateVolumeViaPixel2(completePath2));

					// Dice
					value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

					// Tanimoto
					value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));		

					// Uebersegmentierung
					value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));

					PaintWipeMap1.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
			}
			PrintMap(PaintWipeMap1);
			directory.cdUp();
		}
		if(m_PaintWipe2)
		{
			directory.cd("PaintWipe");
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
					
					std::string outPath;
					std::string referenzPath;
					if(AT_HOME == 1)
					{
						outPath = "C:\\DA\\MITKpdp\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "C:\\DA\\Data\\Scenario2\\Goldstandard.nrrd";
					}
					else
					{
						outPath = "D:\\hachmann\\PDP-MITK\\release\\pdp-inst\\bin\\out.txt";
						referenzPath = "D:\\hachmann\\Data\\Scenario2\\Goldstandard.nrrd";
					}

					// Volume
					std::string completePath2;
					completePath2 += directory.absolutePath().toStdString();
					completePath2 += "\\";
					completePath2 += list7.at(i).toStdString();
					value.push_back(CalculateVolumeViaPixel2(completePath2));

					// Dice
					value.push_back(CalculateDiceKoeffizient(completePath2, referenzPath));

					// Tanimoto
					value.push_back(CalculateTanimotoKoeffizient(completePath2, referenzPath));

					// Uebersegmentierung
					value.push_back(CalculateUebersegmentierung(completePath2, referenzPath));
					
					PaintWipeMap2.insert(std::pair<int,std::vector<double>>(seconds,value));
				}
				//std::cout << secondsStr.toStdString().c_str() << "\n";			
			}
			PrintMap(PaintWipeMap2);
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
			fileName += "_Pixel.csv";
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
							stream << "AS: V; AS: D; AS: T; AS: UE;";
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
							stream << "AS1: V; AS1: D; AS1: T; AS1: UE;";
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
							stream << "AS2: V; AS2: D; AS2: T; AS2: UE;";
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
							stream << "B: V; B: D; B: T; B: UE;";
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
							stream << "D: V; D: D; D: T; D: UE;";
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
							stream << "L: V; L: D; L: T; L: UE;";
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
							stream << "P1: V; P1: D; P1: T; P 1: UE;";
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
							stream << "P2: V; P2: D; P2: T; P2: UE;";
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
		WriteGlobalMapToCVSPixel(globalActiveSurfaceMap, globalAddSubstractMap1, globalAddSubstractMap2, 
								 globalBulgeMap, globalDragMap, globalLiveWireMap, globalPaintWipeMap1, 
								 globalPaintWipeMap2, m_ActiveSurface, m_AddSubstract1, m_AddSubstract2,
								 m_Bulge, m_Drag, m_LiveWire, m_PaintWipe1, m_PaintWipe2);
	}	

	std::cout << "ciao from WriteToCVSPixel()" << std::endl;
}

double CalculateVolumeViaPixel2(std::string path)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load surface
	mitk::Image::Pointer image = LoadImage(path);

	double pixelCounter = 0;
	std::string ref2 = "Image";
	if(ref2.compare(image->GetNameOfClass()) == 0)
	{
		// count pixels
		vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
		int* dims = imageData->GetDimensions();  
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

double CalculateDiceKoeffizient(std::string path, std::string ref)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load image
	mitk::Image::Pointer image = LoadImage(path);
	// load reference
	mitk::Image::Pointer refImage = LoadImage(ref);

	double pixelCounter = 0;
	double pixelCounterImage = 0;
	double pixelCounterRef = 0;
	double dice = 0;
	//std::cout << "Name of Class: " << data->GetNameOfClass() << "\n";
	std::string ref2 = "Image";
	if(ref2.compare(image->GetNameOfClass()) == 0)
	{
		// count pixels
		vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
		vtkSmartPointer<vtkImageData> refData = refImage->GetVtkImageData();
		int* dims = imageData->GetDimensions();  
		//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
		for (int z = 0; z < dims[2]; z++)
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					unsigned char* refPixel = static_cast<unsigned char*>(refData->GetScalarPointer(x,y,z));
					if(pixel[0] > 0 && refPixel[0] > 0)
					{
						pixelCounter++;
					}
					if(pixel[0] > 0)
					{
						pixelCounterImage++;
					}
					if(refPixel[0] > 0)
					{
						pixelCounterRef++;
					}
				}
			}
		}
		std::cout << "Number of intersection of both images: " << pixelCounter << "\n";
		dice = 2 * pixelCounter / (pixelCounterImage + pixelCounterRef);
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return dice;
}

double CalculateTanimotoKoeffizient(std::string path, std::string ref)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load image
	mitk::Image::Pointer image = LoadImage(path);
	// load reference
	mitk::Image::Pointer refImage = LoadImage(ref);

	double pixelIntersection = 0;
	double pixelAggregate = 0;
	double tanimoto = 0;
	std::string ref2 = "Image";
	if(ref2.compare(image->GetNameOfClass()) == 0)
	{
		// count pixels
		vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
		vtkSmartPointer<vtkImageData> refData = refImage->GetVtkImageData();
		int* dims = imageData->GetDimensions();  
		//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
		for (int z = 0; z < dims[2]; z++)
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					unsigned char* refPixel = static_cast<unsigned char*>(refData->GetScalarPointer(x,y,z));

					//std::cout << "Tanimoto Referenz Pixel Value: " << refPixel[0] << "\n";
					//std::cout << "Tanimoto Image Pixel Value: " << pixel[0] << "\n";

					if(pixel[0] > 0 && refPixel[0] > 0)
					{
						pixelIntersection++;
					}
					if(pixel[0] > 0 || refPixel[0] > 0)
					{
						pixelAggregate++;
					}
				}
			}
		}
		std::cout << "Number of intersection of both images: " << pixelIntersection << "\n";
		tanimoto = pixelIntersection / pixelAggregate;
	}

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return tanimoto;
}

double CalculateUebersegmentierung(std::string path, std::string ref)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	// load image
	mitk::Image::Pointer image = LoadImage(path);
	// load reference
	mitk::Image::Pointer refImage = LoadImage(ref);

	double VolUebersegmentierung = 0;
	std::string ref2 = "Image";
	if(ref2.compare(image->GetNameOfClass()) == 0)
	{
		// count pixels
		vtkSmartPointer<vtkImageData> imageData = image->GetVtkImageData();
		vtkSmartPointer<vtkImageData> refData = refImage->GetVtkImageData();
		int* dims = imageData->GetDimensions();  
		//std::cout << "Dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << std::endl;			
		for (int z = 0; z < dims[2]; z++)
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					unsigned char* pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(x,y,z));
					unsigned char* refPixel = static_cast<unsigned char*>(refData->GetScalarPointer(x,y,z));
				
					//std::cout << "Uebersegmentierung Referenz Pixel Value: " << refPixel[0] << "\n";

					if(pixel[0] > 0 && refPixel[0] == 0)
					{
						VolUebersegmentierung++;
					}
				}
			}
		}
	}
	std::cout << "Uebersegmentierung Pixels: " << VolUebersegmentierung << "\n";

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
	return VolUebersegmentierung;
}

mitk::Image::Pointer LoadImage(std::string path)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

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

	mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(nodeReader->GetOutput(0)->GetData());

	return image;
	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void PrintMap(std::map<int,std::vector<double>> map)
{
	//std::cout << "hi from " << __FUNCSIG__ << std::endl;

	for(std::map<int,std::vector<double>>::iterator it = map.begin(); it != map.end(); it++)
	{
		std::cout << it->first << " ";
		for(int i = 0; i < it->second.size(); i++)
		{ 
			std::cout << (it->second)[i] << " ";
		}
		std::cout << "\n";
	}	

	//std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}