#include "vtkVoxelizePolyData.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"

#include "vtkPolyData.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"

//for testing only
#include "vtkXMLImageDataWriter.h"

vtkCxxRevisionMacro(vtkVoxelizePolyData, "$Revision: 1.70 $");
vtkStandardNewMacro(vtkVoxelizePolyData);

vtkVoxelizePolyData::vtkVoxelizePolyData()
{
  
  this->NumberOfCellsX = 10;
  this->NumberOfCellsY = 10;
  this->NumberOfCellsZ = 10;
  
  int inputPorts = this->GetNumberOfInputPorts();
  cout << "input ports: " << inputPorts << endl;
  
  int outputPorts = this->GetNumberOfOutputPorts();
  cout << "output ports: " << outputPorts << endl;
  //this->SetNumberOfInputPorts(1);
  //this->SetNumberOfOutputPorts(1);
  
}

vtkVoxelizePolyData::~vtkVoxelizePolyData()
{

}

//----------------------------------------------------------------------------
int vtkVoxelizePolyData::FillInputPortInformation(
                                            int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

int vtkVoxelizePolyData::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the input and ouptut
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData *input = vtkPolyData::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
		  outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  //cout << "input polydata: " << *input << endl;
  
  //get the bounds of the input PolyData
  double bounds[6];
  input->GetBounds(bounds);
  //xmin, xmax, ymin, ymax, zmin, zmax
  double xmin = bounds[0];
  double xmax = bounds[1];
  double ymin = bounds[2];
  double ymax = bounds[3];
  double zmin = bounds[4];
  double zmax = bounds[5];

  //create a grid the size of the point set
  vtkSmartPointer<vtkImageData> grid = 
      vtkSmartPointer<vtkImageData>::New();
  
  //set the bottom left corner of the grid to the bottom left corner of the data bounding box
  grid->SetOrigin(xmin, ymin, zmin);
  
  //create a grid of voxels of size (NumberOfCellsX x NumberOfCellsY x NumberOfCellsZ)
  //there are (NumberOfCellsX+1 x NumberOfCellsY+1 x NumberOfCellsZ+1) points to make this size voxel grid
  grid->SetExtent(0, this->NumberOfCellsX, 0, this->NumberOfCellsY, 0, this->NumberOfCellsZ);
  
  //set the size of each element/cell/voxel so that the grid spans the entire input point set volume
  grid->SetSpacing((xmax-xmin)/static_cast<double>(this->NumberOfCellsX), (ymax-ymin)/static_cast<double>(this->NumberOfCellsY), (zmax-zmin)/static_cast<double>(this->NumberOfCellsZ));
  
  vtkSmartPointer<vtkXMLImageDataWriter> writer = 
      vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName("inside.vti");
  writer->SetInput(grid);
  writer->Write();
  
  //cout << "inside: " << endl << *grid << endl;
  
  output->ShallowCopy(grid);
  //output->DeepCopy(grid);
  
  {
  vtkSmartPointer<vtkXMLImageDataWriter> writer = 
      vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName("inside_output.vti");
  writer->SetInput(output);
  writer->Write();
  }
  
  return 1;
}


//----------------------------------------------------------------------------
void vtkVoxelizePolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}



//////////////////////////
//// Helper functions ////
//////////////////////////
void GetCellCenter(vtkImageData* image, unsigned int i, unsigned int j, unsigned int k, double center[3])
{
  double origin[3];
  image->GetOrigin(origin);
  
  double spacing[3];
  image->GetSpacing(spacing);
  
  center[0] = origin[0] + i * spacing[0] + spacing[0]/2.0;
  center[1] = origin[1] + j * spacing[1] + spacing[1]/2.0;
  center[2] = origin[2] + k * spacing[2] + spacing[2]/2.0;
  
}

void GetCellCorner(vtkImageData* image, unsigned int i, unsigned int j, unsigned int k, double corner[3])
{
  double origin[3];
  image->GetOrigin(origin);
  
  double spacing[3];
  image->GetSpacing(spacing);
  
  corner[0] = origin[0] + i * spacing[0];
  corner[1] = origin[1] + j * spacing[1];
  corner[2] = origin[2] + k * spacing[2];
}