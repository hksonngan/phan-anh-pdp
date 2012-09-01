// .NAME vtkVoxelizePolyData - 
// .SECTION Description
// 

#ifndef __vtkVoxelizePolyData_h
#define __vtkVoxelizePolyData_h

class vtkImageData;
#include "vtkImageAlgorithm.h"

class vtkVoxelizePolyData : public vtkImageAlgorithm  
{
public:
  
  vtkTypeRevisionMacro(vtkVoxelizePolyData,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkVoxelizePolyData *New();
  
  virtual int FillInputPortInformation( int port, vtkInformation* info );
  
protected:
  vtkVoxelizePolyData();
  ~vtkVoxelizePolyData();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkVoxelizePolyData(const vtkVoxelizePolyData&);  // Not implemented.
  void operator=(const vtkVoxelizePolyData&);  // Not implemented.
  
  unsigned int NumberOfCellsX, NumberOfCellsY, NumberOfCellsZ;
};

//Helper functions
void GetCellCenter(vtkImageData* image, unsigned int i, unsigned int j, unsigned int k, double center[3]);
//void GetCellCenter(vtkImageData* image, unsigned int i, unsigned int j, unsigned int k, double* center);
void GetCellCorner(vtkImageData* image, unsigned int i, unsigned int j, unsigned int k, double corner[3]);

#endif
