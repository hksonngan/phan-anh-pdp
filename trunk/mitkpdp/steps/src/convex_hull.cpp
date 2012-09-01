#include "steps/inc/convex_hull.h"
#include "steps/inc/contour_filter.h"

#include <itkPolyLineParametricPath.h>

pdp::convexHull::convexHull(QString info)
    : info(info)
{ }

QString pdp::convexHull::name() const
{
    return "convexHull: " + info;
}

float pdp::convexHull::relativePercent() const
{
    return 1.0;
}

void pdp::convexHull::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the convexHull step!" << std::endl;
    typedef itk::PolyLineParametricPath< 2 >::Pointer PathPointer;

	for (int i = 0; i < input.slicesContours.size(); ++i)
	{	
		std::vector<PathPointer> paths;
		if(input.slicesContours[i].isLeftLung()){
			ContourFilter contourFilterLeft(input.slicesContours[i].getLeftLung());
			PathPointer hull = contourFilterLeft.convexHull();
			input.slicesContours[i].setLeftHull(hull);
			input.slicesThickenings[i].addLeftThickening(contourFilterLeft.generateThickenings(20, i));
			//std::cout << "number of thickenings detected: "  << thicks.size() << std::endl;				
		}
		if(input.slicesContours[i].isRightLung()){
			ContourFilter contourFilterRight(input.slicesContours[i].getRightLung());
			PathPointer hull = contourFilterRight.convexHull();
			input.slicesContours[i].setRightHull(hull);
			input.slicesThickenings[i].addRightThickening(contourFilterRight.generateThickenings(20, i));
			//std::cout << "number of thickenings detected: "  << thicks.size() << std::endl;	
		}
		emit stepProgress(1.0f/input.slicesContours.size()*i);
	}
}
