#pragma once

#include "steps/inc/thickening.h" 

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <vector>

namespace pdp {
/**
 * Class storing 3D thickenings - sliceThickenings from different slices, left and right lung separately
 *
 */
class Thickening3d
{
	
public:
	
	//constructor
	Thickening3d()
	{
		whichLungs=0;
	};

	~Thickening3d(){};

	bool isLeftThickenings();
	bool isRightThickenings();
	bool isBothThickenings();
	void addLeftThickening(Thickening thick);
	void addRightThickening(Thickening thick);
	void removeLeftThickening(int position);
	void removeRightThickening(int position);
	int sizeLeft();
	int sizeRight();

	Thickening getLeftThickening(int position);
	Thickening getRightThickening(int position);

private:
	int whichLungs; //0 none, 1-left, 2-right, 3-both
	std::vector<Thickening> leftThickenings; 
	std::vector<Thickening> rightThickenings; 
};
}
