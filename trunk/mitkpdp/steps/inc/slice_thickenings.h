#pragma once

#include "steps/inc/thickening.h" 

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <vector>

namespace pdp {
/**
 *Class storing thickenings contours of one slice - of left and right lung separately
 *
 */
#ifndef SLICETHICKENINGS_H
#define SLICETHICKENINGS_H
class sliceThickenings
{
	
public:
	
	//constructor
	sliceThickenings()
	{
		whichLungs=0;
	};

	~sliceThickenings(){};
	
	void noLeftThickenings();
	void noRightThickenings();
	bool isLeftThickenings();
	bool isRightThickenings();
	bool isBothThickenings();
	void addLeftThickening(Thickening thick);
	void addLeftThickening(std::vector<Thickening> thicks);
	void addRightThickening(Thickening thick);
	void addRightThickening(std::vector<Thickening> thicks);
	void removeLeftThickening(int position);
	void removeRightThickening(int position);
	int sizeLeft();
	int sizeRight();

	Thickening& getLeftThickening(int position);
	Thickening& getRightThickening(int position);

private:
	int whichLungs; //0 none, 1-left, 2-right, 3-both
	std::vector<Thickening> leftThickenings; 
	std::vector<Thickening> rightThickenings; 
};
#endif
}
