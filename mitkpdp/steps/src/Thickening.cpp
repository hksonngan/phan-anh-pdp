//refer to the page 1671 of cgal manual for more detail about polygon algorithm
//we need to #include <itkImageAndPathToImageFilter.h>
//to produce an images of many bounding boxes as paths
#include "steps/inc/thickening.h"


std::vector<pdp::Thickening::Point_2> pdp::Thickening::getBoundingBoxHack(double spacingX, double spacingY)
{
	//CGAL::Bbox_2 boundingBox = CGAL::bbox_2(boundary.begin(), boundary.end());
	CGAL::Bbox_2 boundingBox = CGAL::bbox_2(boundary.edges_begin (), boundary.edges_end());
	//hack because pathToImage filter fails if the bounding box is so small
	//that the first and last points meet at the same pixel
	//see the link below for more detail
	//http://itk-insight-users.2283740.n2.nabble.com/Problem-with-PolylineMask2DImageFilter-td4865560.html
	//Thus, we have to increase the size of the bounding box
	//dynamically based on the thickening size.	
	//need more advanced technique
	//this is mainly due to memory leak
	//the path goes infinitely or it consumes the whole memory and stop
	
	//double x = boundingBox.xmax() - boundingBox.xmin();
	//double y = boundingBox.ymax() - boundingBox.ymin();
	std::vector<Point_2> bb;
	double addX = 0;
	double addY = 0;
	//bool close = true;

	//if (x < spacingX*50 || y < spacingY*50)
	//{

	//	close = false;

	//}

	
	Point_2 p1 = Point_2(boundingBox.xmin() - addX, boundingBox.ymin() - addY);
	Point_2 p2 = Point_2(boundingBox.xmax() + addX, boundingBox.ymin() - addY);
	Point_2 p3 = Point_2(boundingBox.xmax() + addX, boundingBox.ymax() + addY);
	Point_2 p4 = Point_2(boundingBox.xmin() - addX, boundingBox.ymax() + addY);
	
	bb.push_back(p1);
	bb.push_back(p2);
	bb.push_back(p3);
	bb.push_back(p4);
	//close the loop
	//if (close) bb.push_back(p1);

	return bb;
}
std::vector<pdp::Thickening::Point_2> pdp::Thickening::getBoundingBox()
{
	std::vector<Point_2> bb;
	double addX = 0;
	double addY = 0;
	Point_2 p1 = Point_2(boundingBox.xmin() - addX, boundingBox.ymin() - addY);
	Point_2 p2 = Point_2(boundingBox.xmax() + addX, boundingBox.ymin() - addY);
	Point_2 p3 = Point_2(boundingBox.xmax() + addX, boundingBox.ymax() + addY);
	Point_2 p4 = Point_2(boundingBox.xmin() - addX, boundingBox.ymax() + addY);
	
	bb.push_back(p1);
	bb.push_back(p2);
	bb.push_back(p3);
	bb.push_back(p4);

	return bb;

}
pdp::Thickening::Bbox_2& pdp::Thickening::getBbox()
{
	return boundingBox;
}
//std::vector<pdp::Thickening::Point_2> pdp::Thickening::getBoundary()
pdp::Thickening::Polygon_2& pdp::Thickening::getBoundary()
{
	return boundary;
}

bool pdp::Thickening::areOverlappedBox(Thickening& other){
	return CGAL::do_overlap(boundingBox, other.getBbox());
}
bool pdp::Thickening::areOverlappedPolygon(Thickening& other){
	bool over=false;
	VertexIterator it;
	if(CGAL::do_overlap(boundingBox, other.getBbox())){ //first boxes just to make sure it is worth it
		it = boundary.vertices_begin();
		for ( ; it != boundary.vertices_end(); ++it){ //recognises 'this' inside 'other'
			if(other.getBoundary().bounded_side(*it)==CGAL::ON_BOUNDED_SIDE || other.getBoundary().bounded_side(*it)==CGAL::ON_BOUNDARY)
			{
				over = true;
				break;
			}
		}
		if(!over){
			std::cout<<"!over";
			it = other.getBoundary().vertices_begin();
			std::cout<<"before for";
			for ( ; it != other.getBoundary().vertices_end(); ++it){ //recognises 'other' inside 'this'
				std::cout<<"in for";
				if(boundary.bounded_side(*it)==CGAL::ON_BOUNDED_SIDE || boundary.bounded_side(*it)==CGAL::ON_BOUNDARY)
				{
					over = true;
					break;
				}
			}
		}
	}
	return over;
}
int pdp::Thickening::belongTo3d(){
	//std::cout<<" in belong in3d!!! "<<in3d<<" "; 
	return in3d;
}
void pdp::Thickening::setBelongTo3d(){
	//std::cout<<" in set!!! "<<b<<" "; 
	in3d = 1;
	//std::cout<<" in set3d!!! "<<in3d<<"licenum"<<sliceNum<<" "<<std::endl;
}
int pdp::Thickening::getSliceNum(){ 
	return sliceNum;
}