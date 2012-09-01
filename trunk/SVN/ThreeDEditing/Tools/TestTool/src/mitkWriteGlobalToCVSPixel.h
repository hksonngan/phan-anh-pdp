#include <QTextStream>
#include <QFile>

void WriteGlobalMapToCVSPixel(  std::map<int,std::vector<double>> globalActiveSurfaceMap,
								std::map<int,std::vector<double>> globalAddSubstractMap1,
								std::map<int,std::vector<double>> globalAddSubstractMap2,
								std::map<int,std::vector<double>> globalBulgeMap,
								std::map<int,std::vector<double>> globalDragMap,
								std::map<int,std::vector<double>> globalLiveWireMap,
								std::map<int,std::vector<double>> globalPaintWipeMap1,
								std::map<int,std::vector<double>> globalPaintWipeMap2,
								bool m_ActiveSurface, bool m_AddSubstract1, bool m_AddSubstract2,
								bool m_Bulge, bool m_Drag, bool m_LiveWire, bool m_PaintWipe1,
								bool m_PaintWipe2
								)
{
	std::cout << "hi from WriteGlobalMapToCVSPixel()" << std::endl;

	// Create global csv file (mean of all persons)
	QFile file("./globalPixel.csv");
	if (file.open(QFile::WriteOnly|QFile::Truncate))
	{
		QTextStream stream(&file);

		for(int i = 0; i < 125; i++)
		{
			stream << i << ";";
			
			// Volume
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: V;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract1)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract1\n";
						stream << "AS1: V;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract2)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract2\n";
						stream << "AS2: V;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Bulge)
				{
					if(i == 0)
					{
						std::cout << "CSV: Bulge\n";
						stream << "B: V;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Drag)
				{
					if(i == 0)
					{
						std::cout << "CSV: Drag\n";
						stream << "D: V;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_LiveWire)
				{
					if(i == 0)
					{
						std::cout << "CSV: LiveWire\n";
						stream << "LW: V;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe1)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe1\n";
						stream << "PW1: V;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe2)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe2\n";
						stream << "PW2: V;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][0] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
			}

			// Dice
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: D;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract1)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract1\n";
						stream << "AS1: D;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract2)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract2\n";
						stream << "AS2: D;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Bulge)
				{
					if(i == 0)
					{
						std::cout << "CSV: Bulge\n";
						stream << "B: D;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Drag)
				{
					if(i == 0)
					{
						std::cout << "CSV: Drag\n";
						stream << "D: D;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_LiveWire)
				{
					if(i == 0)
					{
						std::cout << "CSV: LiveWire\n";
						stream << "LW: D;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe1)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe1\n";
						stream << "PW1: D;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe2)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe2\n";
						stream << "PW2: D;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][1] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
			}

			// Tanimoto
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: T;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract1)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract1\n";
						stream << "AS1: T;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract2)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract2\n";
						stream << "AS2: T;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Bulge)
				{
					if(i == 0)
					{
						std::cout << "CSV: Bulge\n";
						stream << "B: T;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Drag)
				{
					if(i == 0)
					{
						std::cout << "CSV: Drag\n";
						stream << "D: T;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_LiveWire)
				{
					if(i == 0)
					{
						std::cout << "CSV: LiveWire\n";
						stream << "LW: T;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe1)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe1\n";
						stream << "PW1: T;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe2)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe2\n";
						stream << "PW2: T;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][2] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
			}

			// Uebersegmentierung
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: UE;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract1)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract1\n";
						stream << "AS1: UE;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_AddSubstract2)
				{
					if(i == 0)
					{
						std::cout << "CSV: AddSubstract2\n";
						stream << "AS2: UE;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Bulge)
				{
					if(i == 0)
					{
						std::cout << "CSV: Bulge\n";
						stream << "B: UE;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_Drag)
				{
					if(i == 0)
					{
						std::cout << "CSV: Drag\n";
						stream << "D: UE;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_LiveWire)
				{
					if(i == 0)
					{
						std::cout << "CSV: LiveWire\n";
						stream << "LW: UE;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe1)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe1\n";
						stream << "PW1: UE;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][3] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
				if(m_PaintWipe2)
				{
					if(i == 0)
					{
						std::cout << "CSV: PaintWipe2\n";
						stream << "PW2: UE;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][3] << ";";
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

	std::cout << "ciao from WriteGlobalMapToCVSPixel()" << std::endl;
}