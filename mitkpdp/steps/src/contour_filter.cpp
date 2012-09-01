#include "steps/inc/contour_filter.h"

void pdp::ContourFilter::itkToCgal(const PathTypePointer& path)
{
	itk::PolyLineParametricPath< 2 >::VertexListType::ConstIterator vertexIterator = path->GetVertexList()->Begin();
	while(vertexIterator != path->GetVertexList()->End())
    {
		//get() and value() return the vertexType which is ContinuousIndexType
		//in image coordinate
		//std::cout << vertexIterator->Value() << std::endl;
		
		points.push_back(Point_2(vertexIterator->Value()[0],vertexIterator->Value()[1]));
		++vertexIterator;	
    }
}

itk::PolyLineParametricPath< 2 >::Pointer pdp::ContourFilter::cgalToItk(const Points& polygon)
{
	typedef itk::PolyLineParametricPath< 2 > PathType;
	PathType::Pointer path = PathType::New();
	path->Initialize();
	typedef PathType::ContinuousIndexType ContinuousIndexType;
	
	for (int i=0; i<polygon.size(); i++)
	{
		ContinuousIndexType point;
		point[0] = polygon[i][0];
		point[1] = polygon[i][1];
		path->AddVertex( point );
	}
	
	//close the loop from start to end points
	//ContinuousIndexType point;
	//point[0] = polygon[0][0];
	//point[1] = polygon[0][1];
	//path->AddVertex( point );

	return path;
}
itk::PolyLineParametricPath< 2 >::Pointer pdp::ContourFilter::cgalToItk(Polygon_2 polygon)
{
	typedef itk::PolyLineParametricPath< 2 > PathType;
	PathType::Pointer path = PathType::New();
	path->Initialize();
	typedef PathType::ContinuousIndexType ContinuousIndexType;
	
	for (int i=0; i<polygon.size(); i++)
	{
		ContinuousIndexType point;
		point[0] = polygon[i][0];
		point[1] = polygon[i][1];
		path->AddVertex( point );
	}
	
	//close the loop from start to end points
	//ContinuousIndexType point;
	//point[0] = polygon[0][0];
	//point[1] = polygon[0][1];
	//path->AddVertex( point );

	return path;
}
/**
 *Function used for separating lungs connected by bottlelike constriction.  
 *
 * @param leftLung - if we should return the left or right lung.
 * @param top - the index of the higher point.
 * @param bottom - the index of the lower point.
 */
itk::PolyLineParametricPath< 2 >::Pointer pdp::ContourFilter::separateLungs(bool leftLung, int top, int bottom)
{
	typedef itk::PolyLineParametricPath< 2 > PathType;
	typedef PathType::ContinuousIndexType ContinuousIndexType;
	ContinuousIndexType p;
	
	int jump=0;
	if(jump+10>points.size())jump=points.size()-1;
	else
		jump=10;

	//std::cout<<points.size()<<" "<<top<<" "<<bottom;
	if(leftLung){
		if(top<bottom && points[top][0]<points[top+jump][0]){
			//std::cout<<"L1";
			//for(int i=points[top][1];i<points[bottom][1];i--){
				p[1]=(points[top][1]+points[bottom][1])/2;
				p[0]=(points[top][0]+points[bottom][0])/2;
				points.erase(points.begin()+top, points.begin()+bottom);
				//points.insert(points.begin()+top+1,Point_2(p[0],p[1]) );
				//points.insert(points.begin(),Point_2(100,100) );
			//}
		}
		else if (top<bottom && points[top][0]>points[top+jump][0]){
			//std::cout<<"L2"<<" "<<(bottom-top)<<points.size();
			points.erase(points.begin(), points.begin()+top);
			points.erase(points.begin()+(bottom-top), points.end());
		}
		else if (top>bottom && points[top][0]<points[top+jump][0]){
			//std::cout<<"L3";
			points.erase(points.begin(), points.begin()+bottom);
			points.erase(points.begin()+(top-bottom), points.end());
		}
		else if (top>bottom && points[top][0]>points[top+jump][0]){
			//std::cout<<"L4";
			points.erase(points.begin()+bottom, points.begin()+top);
		}
	}
	else{
		if(top<bottom && points[top][0]<points[top+jump][0]){
			//std::cout<<"R1";
			points.erase(points.begin()+bottom, points.end());
			points.erase(points.begin(), points.begin()+top);
			
		}
		else if (top<bottom && points[top][0]>points[top+jump][0]){
			//std::cout<<"R2";
			points.erase(points.begin()+top, points.begin()+bottom);
			
		}
		else if (top>bottom && points[top][0]<points[top+jump][0]){
			//std::cout<<"R3";
			points.erase(points.begin()+bottom, points.begin()+top);
			
		}
		else if (top>bottom && points[top][0]>points[top+jump][0]){
			//std::cout<<"R4";
			points.erase(points.begin()+top, points.end());
			points.erase(points.begin(), points.begin()+bottom);
			
		}
	}
	return cgalToItk(points);
}
#include <CGAL/Polygon_2_algorithms.h>
itk::PolyLineParametricPath< 2 >::Pointer pdp::ContourFilter::convexHull()
{
	//melkman algorithm is for simple polylines only
	//the result may be correct but the order of the hulls
	//may not be correct.
	//this affect the thickening detection algorithm
	//need more inspection. at the moment, we use the general convex hull algorithm
	//bool simple = CGAL::is_simple_2(points.begin(), points.end(), K());
	//if (!simple) 
	//{
	//	std::cout << "the contour is not simple!!!" << std::endl;
		CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));
	//}
	//else
	//{
	//	CGAL::ch_melkman( points.begin(), points.end(), std::back_inserter(hull) );
	//}
	//
	return cgalToItk(hull);
}




std::vector<pdp::Thickening> pdp::ContourFilter::generateThickenings(int thickeningContourSize, int sliceNumm)
{
	//temporary list containing points on a thickening
	std::vector<Point_2> temp;
	std::vector<Point_2> lastSegment;
	bool isLast = true;
	int j = 0;	//index keeps track of the convex hull
	std::set<Point_2> polygon;
	std::set<Point_2>::iterator it;

	//init set of points on the convex hull
	//this will help retrieve the hull points from the contour
	for (int k = 0; k < hull.size(); k++)
	{
		polygon.insert(hull[k]);
	}

	for (int i = 0; i < points.size(); i++)
	{
		it = polygon.find(points[i]);
		//first
		if (it != polygon.end() && j == 0)
		{
			isLast = false;
			lastSegment.push_back(points[i]);
		}

		if (isLast) lastSegment.push_back(points[i]);
		else temp.push_back(points[i]);

		//check point
		if (j < polygon.size() && it != polygon.end())
		{
			j++;
			
			if (j != 1)
			{
				//if the temp has some points in between 2 vertices
				//then it is a thickening
				if (temp.size() > 2 && temp.size() > thickeningContourSize)
				{
					Thickening thick(temp, sliceNumm);
					thickenings.push_back(thick);
					temp.clear();
				}else	//just a convex segment
				{
					temp.clear();
				}

				//overlap the joint
				temp.push_back(points[i]);

			}else	//is the beginning 
			{
			}

			if (j == polygon.size())
			{
				isLast = true;
				lastSegment.push_back(points[i]);
			}

		}


	}

	if (lastSegment.size() > 2 && lastSegment.size() > thickeningContourSize)
	{
		Thickening thick(lastSegment, sliceNumm);
		thickenings.push_back(thick);
		lastSegment.clear();
	}

	return thickenings;
}