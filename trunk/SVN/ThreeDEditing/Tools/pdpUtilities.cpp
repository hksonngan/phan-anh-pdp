#include <iostream>

// ITK
#include "itkLineCell.h"
#include "itkTriangleCell.h"
#include "itkPoint.h"

// VTK
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

// PDP
#include "pdpUtilities.h"

//#include "itkDefaultDynamicMeshTraits.h"

pdp::Utilities::Utilities(mitk::ToolExtension* ext)
{
	m_Ext = ext;
}

pdp::Utilities::~Utilities()
{}

pdp::Utilities::MeshType::Pointer pdp::Utilities::VtkSurfaceToItk(vtkPolyData* polyData)
{
	
// This define is needed to deal with double/float changes in VTK
//#ifndef vtkFloatingPointType
//#define vtkFloatingPointType float
//#endif

	MeshType::Pointer  mesh = MeshType::New();

	//
	// Transfer the points from the vtkPolyData into the itk::Mesh
	//
	const unsigned int numberOfPoints = polyData->GetNumberOfPoints();
	vtkPoints * vtkpoints = polyData->GetPoints();
	mesh->GetPoints()->Reserve( numberOfPoints );
	for(unsigned int p =0; p < numberOfPoints; p++)
	{
		vtkFloatingPointType * apoint = vtkpoints->GetPoint( p );
		mesh->SetPoint( p, MeshType::PointType( apoint ));
	}

	//
	// Transfer the cells from the vtkPolyData into the itk::Mesh
	//
	vtkCellArray * triangleStrips = polyData->GetStrips();
	vtkIdType  * cellPoints;
	vtkIdType    numberOfCellPoints;

	//
	// First count the total number of triangles from all the triangle strips.
	//
	unsigned int numberOfTriangles = 0;
	triangleStrips->InitTraversal();
	while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
	{
		numberOfTriangles += numberOfCellPoints-2;
	}
	vtkCellArray * polygons = polyData->GetPolys();
	polygons->InitTraversal();
	while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
	{
		if( numberOfCellPoints == 3 )
		{
			numberOfTriangles ++;
		}
	}

	//
	// Reserve memory in the itk::Mesh for all those triangles
	//
	mesh->GetCells()->Reserve( numberOfTriangles );

	// 
	// Copy the triangles from vtkPolyData into the itk::Mesh
	//
	//
	typedef MeshType::CellType   CellType;
	typedef itk::TriangleCell< CellType > TriangleCellType;
	int cellId = 0;

	// first copy the triangle strips
	//triangleStrips->InitTraversal();
	//while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
	//{
	//	unsigned int numberOfTrianglesInStrip = numberOfCellPoints - 2;

	//	unsigned long pointIds[3];
	//	pointIds[0] = cellPoints[0];
	//	pointIds[1] = cellPoints[1];
	//	pointIds[2] = cellPoints[2];

	//	for( unsigned int t=0; t < numberOfTrianglesInStrip; t++ )
	//	{
	//		MeshType::CellAutoPointer c;
	//		TriangleCellType * tcell = new TriangleCellType;
	//		tcell->SetPointIds( pointIds );
	//		c.TakeOwnership( tcell );
	//		mesh->SetCell( cellId, c );
	//		cellId++;
	//		pointIds[0] = pointIds[1];
	//		pointIds[1] = pointIds[2];
	//		pointIds[2] = cellPoints[t+3];
	//	}
	//}

	// then copy the normal triangles
	polygons->InitTraversal();
	while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
	{
		if( numberOfCellPoints !=3 ) // skip any non-triangle.
		{
			continue;
		}
		MeshType::CellAutoPointer c;
		TriangleCellType * t = new TriangleCellType;
		t->SetPointIds( (unsigned long*)cellPoints );
		c.TakeOwnership( t );
		mesh->SetCell( cellId, c );
		cellId++;
	}

	std::cout << "Mesh  " << std::endl;
	std::cout << "Number of Points =   " << mesh->GetNumberOfPoints() << std::endl;
	std::cout << "Number of Cells  =   " << mesh->GetNumberOfCells()  << std::endl;

	return mesh; 	
}

pdp::Utilities::TriangleMeshType::Pointer pdp::Utilities::VtkSurfaceToItk2(vtkPolyData* m_PolyData)
{
	
	//typedef itk::DefaultDynamicMeshTraits<double, 3, 3,double,double> TriangleMeshTraits;
	//typedef itk::Mesh<double,3, TriangleMeshTraits> TriangleMeshType;


// This define is needed to deal with double/float changes in VTK
//#ifndef vtkFloatingPointType
//#define vtkFloatingPointType float
//#endif

	//MeshType::Pointer  m_itkMesh = MeshType::New();
	TriangleMeshType::Pointer  m_itkMesh = TriangleMeshType::New();

	//
  // Transfer the points from the vtkPolyData into the itk::Mesh
  //
  const unsigned int numberOfPoints = m_PolyData->GetNumberOfPoints();
  vtkPoints * vtkpoints =  m_PolyData->GetPoints();
 
  m_itkMesh->GetPoints()->Reserve( numberOfPoints );
  
  for(int p =0; p < numberOfPoints; p++)
    {

    vtkFloatingPointType * apoint = vtkpoints->GetPoint( p );
    
    // Need to convert the point to PoinType
    TriangleMeshType::PointType pt;
    for(unsigned int i=0;i<3; i++)
      {
       pt[i] = apoint[i];
       }
     m_itkMesh->SetPoint( p, pt);

    }
  //
  // Transfer the cells from the vtkPolyData into the itk::Mesh
  //
  vtkCellArray * triangleStrips = m_PolyData->GetStrips();

  vtkIdType  * cellPoints;
  vtkIdType    numberOfCellPoints;

  //
  // First count the total number of triangles from all the triangle strips.
  //
  unsigned int numberOfTriangles = 0;

  triangleStrips->InitTraversal();
  while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
    {
    numberOfTriangles += numberOfCellPoints-2;
    }

   vtkCellArray * polygons = m_PolyData->GetPolys();
  
   polygons->InitTraversal();

   while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     if( numberOfCellPoints == 3 )
       {
        numberOfTriangles ++;
       }
     }

   //
  // Reserve memory in the itk::Mesh for all those triangles
  //
   m_itkMesh->GetCells()->Reserve( numberOfTriangles );

  // 
  // Copy the triangles from vtkPolyData into the itk::Mesh
  //
  //

   typedef TriangleMeshType::CellType   CellType;

   typedef itk::TriangleCell< CellType > TriangleCellType;

  // first copy the triangle strips
   int cellId = 0;
   triangleStrips->InitTraversal();
   while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     unsigned int numberOfTrianglesInStrip = numberOfCellPoints - 2;

     unsigned long pointIds[3];
     pointIds[0] = cellPoints[0];
     pointIds[1] = cellPoints[1];
     pointIds[2] = cellPoints[2];
       
     for( unsigned int t=0; t < numberOfTrianglesInStrip; t++ )
       {
        TriangleMeshType::CellAutoPointer c;
        TriangleCellType * tcell = new TriangleCellType;
        tcell->SetPointIds( pointIds );
        c.TakeOwnership( tcell );
        m_itkMesh->SetCell( cellId, c );
        cellId++;
        pointIds[0] = pointIds[1];
        pointIds[1] = pointIds[2];
        pointIds[2] = cellPoints[t+3];
       }
       
     }

   // then copy the triangles 
   polygons->InitTraversal();
   while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     if( numberOfCellPoints !=3 ) // skip any non-triangle.
       {
       continue;
       }
     TriangleMeshType::CellAutoPointer c;
     TriangleCellType * t = new TriangleCellType;
     t->SetPointIds( (unsigned long*)cellPoints );
     c.TakeOwnership( t );
     m_itkMesh->SetCell( cellId, c );
     cellId++;
     } 



	std::cout << "Mesh  " << std::endl;
	std::cout << "Number of Points =   " << m_itkMesh->GetNumberOfPoints() << std::endl;
	std::cout << "Number of Cells  =   " << m_itkMesh->GetNumberOfCells()  << std::endl;

	return m_itkMesh; 	
}

vtkPolyData* pdp::Utilities::ItkSurfaceToVtk(MeshType::Pointer mesh)
{
	//Creat a new vtkPolyData*
	vtkPolyData* newPolyData = vtkPolyData::New();

	//Creat vtkPoints for insertion into newPolyData
	vtkPoints *points = vtkPoints::New();
	std::cout<<"Points = "<<mesh->GetNumberOfPoints()<<std::endl;

	//Copy all points into the vtkPolyData structure
	typedef MeshType::PointsContainer::ConstIterator  PointIterator;
	PointIterator pntIterator = mesh->GetPoints()->Begin();
	PointIterator pntItEnd = mesh->GetPoints()->End();
	for (int i = 0; pntIterator != pntItEnd; ++i, ++pntIterator)
	{
		//itk::Point pnt = pntIterator.Value();  
		MeshType::PointType pnt = pntIterator.Value();  
		points->InsertPoint(i, pnt[0], pnt[1], pnt[2]);
		//       std::cout<<i<<"-th point:  ";
		//       std::cout<<pnt[0]<<std::endl;
		//       std::cout<<"               "<<pntIterator.Value()<<std::endl;
		//      ++pntIterator;
	}
	newPolyData->SetPoints(points);
	points->Delete();

	//Copy all cells into the vtkPolyData structure
	//Creat vtkCellArray into which the cells are copied
	vtkCellArray* triangle = vtkCellArray::New();

	typedef MeshType::CellsContainer::ConstIterator CellIterator;
	typedef MeshType::CellType CellType;
	typedef CellType::PointIdIterator PointIdIterator;
	CellIterator cellIt = mesh->GetCells()->Begin();
	CellIterator cellItEnd = mesh->GetCells()->End();
	for (int it = 0; cellIt != cellItEnd; ++it, ++cellIt)
	{
		CellType * cellptr = cellIt.Value();
		//    LineType * line = dynamic_cast<LineType *>( cellptr );
		//    std::cout << line->GetNumberOfPoints() << std::endl;
		//      std::cout << cellptr->GetNumberOfPoints() << std::endl;
		PointIdIterator pntIdIter = cellptr->PointIdsBegin();
		PointIdIterator pntIdEnd = cellptr->PointIdsEnd();
		vtkIdList* pts = vtkIdList::New();
		for (; pntIdIter != pntIdEnd; ++pntIdIter)
		{
			pts->InsertNextId( *pntIdIter );
			//          std::cout<<"           "<<tempCell[it1]<<std::endl;
		}
		triangle->InsertNextCell(pts);
	}
	newPolyData->SetPolys(triangle);
	triangle->Delete();	
	
	return newPolyData;
}

void pdp::Utilities::AddPolyDataToDataStorage(vtkPolyData* polyData, std::string name)
{
		mitk::Surface::Pointer outSurface = mitk::Surface::New();
		outSurface->SetVtkPolyData(polyData);
		mitk::DataNode::Pointer outNode = mitk::DataNode::New();
		outNode->SetData(outSurface);
		outNode->SetName(name);
		m_Ext->GetDataStorage()->Add(outNode);
}
