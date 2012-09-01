#pragma once

#include "CGAL/Exact_predicates_inexact_constructions_kernel.h"
#include <CGAL/Polygon_2.h>
#include <CGAL/Bbox_2.h>
#include <vector>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Bbox_2.h>

namespace pdp {
class Thickening
{
public:

	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef K::Point_2 Point_2;
	typedef CGAL::Polygon_2<K> Polygon_2;
	typedef CGAL::Bbox_2 Bbox_2;
	typedef std::vector<Point_2> Points;
	typedef CGAL::Polygon_2<K>::Vertex_iterator VertexIterator;
	
	//constructor
	Thickening(Points boundaryP, int sliceNumm)
	{
		//this->boundary = boundary;
		boundary = Polygon_2(boundaryP.begin(), boundaryP.end());
		boundingBox = CGAL::bbox_2(boundary.edges_begin(), boundary.edges_end());
		in3d = 0;
		sliceNum = sliceNumm;
	};

	//Thickening(){};

	~Thickening(){};
	
	float getArea();
	Point_2 getCenterOfGravitiy();
	std::vector<Point_2> getBoundingBoxHack(double spacingX, double spacingY);
	std::vector<Point_2> getBoundingBox();
	Bbox_2& getBbox();
	int getSliceNum();
	Thickening getOverlapping(const Thickening& other);
	bool areOverlappedBox(Thickening& other);
	bool areOverlappedPolygon(Thickening& other);
	//std::vector<Point_2> getBoundary();
	Polygon_2& getBoundary();
	//void addPoint(Point_2 point);
	int belongTo3d();
	void setBelongTo3d();
private:
	//Points boundary;
	Polygon_2 boundary;
	Bbox_2 boundingBox;
	int in3d;
	int sliceNum;
	
	
};

}
