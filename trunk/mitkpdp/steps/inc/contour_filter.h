#pragma once

#include "steps/inc/thickening.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_melkman.h>
#include <vector>
#include <set>
#include <itkPolyLineParametricPath.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/Polygon_2.h>

namespace pdp {

//is contour simple i.e. not self-intersection
//if so we can use Melkman algorithm
class ContourFilter
{
public:
	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef K::Point_2 Point_2;
	typedef std::vector<Point_2> Points;
	typedef CGAL::Polygon_2<K> Polygon_2;
	typedef itk::PolyLineParametricPath< 2 >::Pointer PathTypePointer;

	//constructor
	ContourFilter(const PathTypePointer& path){
		itkToCgal(path);
	}

	~ContourFilter(){}

	//return the convex hull of the polyline using Melkman algorithm
	//this is only correct for the polyline but not all configuration of point cloud
	//Melkman only works for simple contour
	PathTypePointer convexHull();

	//traverse the contour and use the function CGAL::bounded_side_2
	//to determine the boundary of each thickening
	//the boundary is defined within the intersection between hull and contour
	std::vector<Thickening> generateThickenings(int thickeningContourSize, int sliceNumm);

	std::vector<Thickening> getThickenings(){return thickenings;};
	
	//used for separating left and right lungs if they are connected
	PathTypePointer separateLungs(bool leftLung, int top, int bottom);
	//only for the output
	static PathTypePointer cgalToItk(const Points& polygon);
	static PathTypePointer cgalToItk(Polygon_2 polygon);

private:
	//store the thickening results
	std::vector<Thickening> thickenings;
	//store the converted contour type and the convex hull
	Points points, hull;
	//convert from contour in itk to points in cgal
	void itkToCgal(const PathTypePointer& path);

};

}
