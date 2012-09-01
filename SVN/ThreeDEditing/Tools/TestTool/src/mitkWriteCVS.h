#include <QTextStream>
#include <QFile>

void WriteGlobalMapToCVS(std::map<int,std::vector<double>> globalActiveSurfaceMap,
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
	std::cout << "hi from WriteGlobalMapToCVS()" << std::endl;

	// Create global csv file (mean of all persons)
	QFile file("./global.csv");
	if (file.open(QFile::WriteOnly|QFile::Truncate))
	{
		QTextStream stream(&file);

		for(int i = 0; i < 125; i++)
		{
			stream << i << ";";
			
			// Area1
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: A1;";
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
						stream << "AS1: A1;";
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
						stream << "AS2: A1;";
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
						stream << "B: A1;";
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
						stream << "D: A1;";
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
						stream << "LW: A1;";
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
						stream << "PW1: A1;";
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
						stream << "PW2: A1;";
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

			// Area2
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: A2;";
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
						stream << "AS1: A2;";
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
						stream << "AS2: A2;";
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
						stream << "B: A2;";
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
						stream << "D: A2;";
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
						stream << "LW: A2;";
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
						stream << "PW1: A2;";
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
						stream << "PW2: A2;";
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

			// Mean1
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: M1;";
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
						stream << "AS1: M1;";
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
						stream << "AS2: M1;";
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
						stream << "B: M1;";
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
						stream << "D: M1;";
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
						stream << "LW: M1;";
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
						stream << "PW1: M1;";
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
						stream << "PW2: M1;";
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

			// Mean2
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: M2;";
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
						stream << "AS1: M2;";
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
						stream << "AS2: M2;";
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
						stream << "B: M2;";
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
						stream << "D: M2;";
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
						stream << "LW: M2;";
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
						stream << "PW1: M2;";
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
						stream << "PW2: M2;";
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

			// Hausdorff
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: Hausdorff;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][4] << ";";
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
						stream << "AS1: Hausdorff;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][4] << ";";
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
						stream << "AS2: Hausdorff;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][4] << ";";
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
						stream << "B: Hausdorff;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][4] << ";";
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
						stream << "D: Hausdorff;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][4] << ";";
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
						stream << "LW: Hausdorff;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][4] << ";";
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
						stream << "PW1: Hausdorff;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][4] << ";";
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
						stream << "PW2: Hausdorff;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][4] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
			}

			// Volume 1
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: V1;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][5] << ";";
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
						stream << "AS1: V1;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][5] << ";";
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
						stream << "AS2: V1;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][5] << ";";
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
						stream << "B: V1;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][5] << ";";
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
						stream << "D: V1;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][5] << ";";
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
						stream << "LW: V1;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][5] << ";";
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
						stream << "PW1: V1;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][5] << ";";
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
						stream << "PW2: V1;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][5] << ";";
						}
						else
						{
							stream << ";";
						}
					}
				}
			}

			// Volume 2
			if(1)
			{
				if(m_ActiveSurface)
				{
					if(i == 0)
					{
						std::cout << "CSV: ActiveSurface\n";
						stream << "AS: V2;";
					}
					else
					{
						if(globalActiveSurfaceMap.find(i) != globalActiveSurfaceMap.end())
						{
							stream << globalActiveSurfaceMap[i][6] << ";";
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
						stream << "AS1: V2;";
					}
					else
					{
						if(globalAddSubstractMap1.find(i) != globalAddSubstractMap1.end())
						{
							stream << globalAddSubstractMap1[i][6] << ";";
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
						stream << "AS2: V2;";
					}
					else
					{
						if(globalAddSubstractMap2.find(i) != globalAddSubstractMap2.end())
						{
							stream << globalAddSubstractMap2[i][6] << ";";
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
						stream << "B: V2;";
					}
					else
					{
						if(globalBulgeMap.find(i) != globalBulgeMap.end())
						{
							stream << globalBulgeMap[i][6] << ";";
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
						stream << "D: V2;";
					}
					else
					{
						if(globalDragMap.find(i) != globalDragMap.end())
						{
							stream << globalDragMap[i][6] << ";";
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
						stream << "LW: V2;";
					}
					else
					{
						if(globalLiveWireMap.find(i) != globalLiveWireMap.end())
						{
							stream << globalLiveWireMap[i][6] << ";";
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
						stream << "PW1: V2;";
					}
					else
					{
						if(globalPaintWipeMap1.find(i) != globalPaintWipeMap1.end())
						{
							stream << globalPaintWipeMap1[i][6] << ";";
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
						stream << "PW2: V2;";
					}
					else
					{
						if(globalPaintWipeMap2.find(i) != globalPaintWipeMap2.end())
						{
							stream << globalPaintWipeMap2[i][6] << ";";
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

	std::cout << "ciao from WriteGlobalMapToCVS()" << std::endl;
}