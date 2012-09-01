#ifndef pdpUtilities_h_Included
#define pdpUtilities_h_Included

#include "itkMesh.h"
#include "vtkPolyData.h"

#include "mitkToolExtension.h"

#include "itkDefaultDynamicMeshTraits.h"


namespace pdp
{
	class Utilities
	{
		public:
			Utilities(mitk::ToolExtension*); 
			~Utilities();

			typedef itk::DefaultStaticMeshTraits< 
					    vtkFloatingPointType, 
					    3,
					    2, 
					    vtkFloatingPointType, 
					    vtkFloatingPointType  >       MeshTraits;

			typedef itk::Mesh<
					    vtkFloatingPointType, 
					    3, 
					    MeshTraits              >     MeshType;

			typedef itk::DefaultDynamicMeshTraits<double, 3, 3,double,double> TriangleMeshTraits;
			typedef itk::Mesh<double,3, TriangleMeshTraits> TriangleMeshType;

			//void SetDataStorage(mitk::DataStorage* storage){m_DataStorage = storage;};
			//mitk::DataStorage* GetDataStorage(){return m_DataStorage;};
			// void SetPositionTrackerNode(mitk::DataNode* node){m_PositionTrackerNode = node;};
			// mitk::DataNode* GetPositionTrackerNode(){return m_PositionTrackerNode;};
			// void SetCurrentSurfaceVector(std::vector<std::string> surfaces){m_CurrentSurfaces = surfaces;};
			// std::vector<std::string> GetCurrentSurfaceVector(){return m_CurrentSurfaces;};
			// void SetSurfaceNode(mitk::DataNode* node){m_SurfaceNode = node;};
			// mitk::DataNode* GetSurfaceNode(){return m_SurfaceNode;};
			// void SetSurface(mitk::Surface* surface){m_Surface = surface;};
			// mitk::Surface* GetSurface(){return m_Surface;};
			// void SetSurfacePolyData(vtkPolyData* surfacePolyData){m_Surface->SetVtkPolyData(surfacePolyData);};
			// vtkPolyData* GetSurfacePolyData(){return m_Surface->GetVtkPolyData();};
			// void SetBallNode(mitk::DataNode* node){m_BallNode = node;};
			// mitk::DataNode* GetBallNode(){return m_BallNode;};
			// void SetIntersectionNode(mitk::DataNode* node){m_IntersectionNode = node;};
			// mitk::DataNode* GetIntersectionNode(){return m_IntersectionNode;};
			// void SetBall(mitk::Surface* surface){m_Ball = surface;};
			// mitk::Surface* GetBall(){return m_Ball;};
			// void SetIntersectionData(mitk::Surface* surface){m_IntersectionData = surface;};
			// mitk::Surface* GetIntersectionData(){return m_IntersectionData;};
			// void SetIntersectionPolyDataFilter(vtkSmartPointer<vtkIntersectionPolyDataFilter> filter){m_IntersectionPolyDataFilter = filter;};
			// vtkSmartPointer<vtkIntersectionPolyDataFilter> GetIntersectionPolyDataFilter(){return m_IntersectionPolyDataFilter;};
			// void SetContourSphere(vtkSphereSource* sphere){m_ContourSphere = sphere;};
			// vtkSphereSource* GetContourSphere(){return m_ContourSphere;};
			// void SetMitkView(QmitkStdMultiWidget* mitkView){m_MitkView = mitkView;};
			// QmitkStdMultiWidget* GetMitkView(){return m_MitkView;};
			// void SetRadius(double radius){m_Radius = radius;};
			// double GetRadius(){return m_Radius;};
			// void SetRadiusFactor(double radiusFactor){m_RadiusFactor = radiusFactor;};
			// double GetRadiusFactor(){return m_RadiusFactor;};			
			// void SetCalcIntersection(bool CalcIntersection){m_CalcIntersection = CalcIntersection;};
			// bool GetCalcIntersection(){return m_CalcIntersection;};
			
			//itk::Mesh* VtkSurfaceToItk(vtkPolyData*);
			MeshType::Pointer VtkSurfaceToItk(vtkPolyData*);
			TriangleMeshType::Pointer VtkSurfaceToItk2(vtkPolyData*);
			vtkPolyData* ItkSurfaceToVtk(MeshType::Pointer);
			void AddPolyDataToDataStorage(vtkPolyData*, std::string);

		protected:
	    
		private:
			mitk::ToolExtension* m_Ext;
			//DataStorage* m_DataStorage;
			// QmitkStdMultiWidget* m_MitkView;

			// mitk::DataNode::Pointer m_PositionTrackerNode;
			// mitk::DataNode::Pointer m_SurfaceNode;			
			// mitk::DataNode::Pointer m_BallNode;	
			// mitk::DataNode::Pointer m_IntersectionNode;

			// mitk::Surface::Pointer m_Ball;
			// mitk::Surface::Pointer m_IntersectionData;
			// mitk::Surface::Pointer m_Surface;

			// vtkSmartPointer<vtkIntersectionPolyDataFilter> m_IntersectionPolyDataFilter;
			// vtkSphereSource* m_ContourSphere;

	};

} // namespace

#endif


