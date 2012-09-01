#ifndef PDP_PT_H
#define PDP_PT_H

#pragma once

#include "mitkPointSet.h"

namespace pdp {

class Pt
{
	public:
		Pt();
		Pt(double value);
		Pt(double value[3]);
		Pt(double, double, double);
		Pt(pdp::Pt& value);
		Pt(mitk::Point3D rhs);
		~Pt();

		double& operator[](int index){return m_Point[index];};
		double& operator=(double const rhs[3]);
		
		mitk::Point3D toItk();
		double* toVtk();

		pdp::Pt operator+(double const rhs[3]);
		pdp::Pt operator-(double const rhs[3]);
		pdp::Pt operator*(double const rhs[3]);
		pdp::Pt operator/(double const rhs[3]);

		pdp::Pt operator+(pdp::Pt rhs);
		pdp::Pt operator-(pdp::Pt rhs);
		pdp::Pt operator*(pdp::Pt rhs);
		pdp::Pt operator/(pdp::Pt rhs);
		
		pdp::Pt operator+(mitk::Point3D rhs);
		pdp::Pt operator-(mitk::Point3D rhs);
		pdp::Pt operator*(mitk::Point3D rhs);
		pdp::Pt operator/(mitk::Point3D rhs);

		pdp::Pt operator+(double rhs);
		pdp::Pt operator-(double rhs);
		pdp::Pt operator*(double rhs);
		pdp::Pt operator/(double rhs);

		pdp::Pt operator+(int rhs);
		pdp::Pt operator-(int rhs);
		pdp::Pt operator*(int rhs);
		pdp::Pt operator/(int rhs);

		pdp::Pt operator+=(pdp::Pt rhs);
		pdp::Pt operator-=(pdp::Pt rhs);
		pdp::Pt operator*=(pdp::Pt rhs);
		pdp::Pt operator/=(pdp::Pt rhs);

		double Length();

	
	private:
		double* m_Point;

};

//pdp::Pt operator*(double lhs, pdp::Pt rhs)
//{
//	pdp::Pt newPoint(rhs);
//	newPoint *= lhs;
//	return newPoint;
//}
// ??????????????

} // namespace pdp

#endif // PDP_PT_H

