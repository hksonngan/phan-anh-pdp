#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <vector>
#include <itkPolyLineParametricPath.h>
namespace pdp {
/**
 *Class storing contours - left and right lung
 *
 */
class sliceContour
{
public:
	typedef itk::PolyLineParametricPath< 2 >::Pointer PathPointer;
	
	
	
	//constructor
	sliceContour()
	{
		whichLungs=0;
	};

	//Thickening(){};

	~sliceContour(){};

	bool isLeftLung();
	bool isRightLung();
	bool isBothLungs();
	void setLeftLung(PathPointer& path);
	void setRightLung(PathPointer& path);

	PathPointer getLeftLung();
	PathPointer getRightLung();

	void setLeftHull(PathPointer& path);
	void setRightHull(PathPointer& path);

	PathPointer getLeftHull();
	PathPointer getRightHull();


	
private:
	int whichLungs; //0 none, 1-left, 2-right, 3-both
	PathPointer leftLung;
	PathPointer rightLung;
	PathPointer leftHull;
	PathPointer rightHull;
};

}
