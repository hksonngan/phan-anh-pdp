#include "pdpPt.h"

pdp::Pt::Pt()
{
	m_Point = new double[3];
}

pdp::Pt::~Pt()
{
	;//delete m_Point;
}

pdp::Pt::Pt(double value)
{
	m_Point = new double[3];
	m_Point[0] = value;
	m_Point[1] = value;
	m_Point[2] = value;
}

pdp::Pt::Pt(double value[3])
{
	m_Point = new double[3];
	m_Point[0] = value[0];
	m_Point[1] = value[1];
	m_Point[2] = value[2];
}

pdp::Pt::Pt(double value1, double value2, double value3)
{
	m_Point = new double[3];
	m_Point[0] = value1;
	m_Point[1] = value2;
	m_Point[2] = value3;
}

pdp::Pt::Pt(pdp::Pt& value)
{
	m_Point = new double[3]; 
	m_Point[0] = value[0];
	m_Point[1] = value[1];
	m_Point[2] = value[2];
}

pdp::Pt::Pt(mitk::Point3D value)
{
	m_Point = new double[3]; 
	m_Point[0] = value[0];
	m_Point[1] = value[1];
	m_Point[2] = value[2];
}

/* Operatoren *********************************************************************************************/
double& pdp::Pt::operator=(double const rhs[3])
{
	m_Point[0] = rhs[0];
	m_Point[1] = rhs[1];
	m_Point[2] = rhs[2];

	return *m_Point; 
}

mitk::Point3D pdp::Pt::toItk()
{
	mitk::Point3D newPoint;
	newPoint[0] = m_Point[0];
	newPoint[1] = m_Point[1];
	newPoint[2] = m_Point[2];
	return newPoint;
}

double* pdp::Pt::toVtk()
{
	return m_Point;
}

/* double[3] ****************************************************************************************************/
pdp::Pt pdp::Pt::operator+( double const rhs[])
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] + rhs[0];
	newPoint[1] = m_Point[1] + rhs[1];
	newPoint[2] = m_Point[2] + rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator-( double const rhs[])
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] - rhs[0];
	newPoint[1] = m_Point[1] - rhs[1];
	newPoint[2] = m_Point[2] - rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator*( double const rhs[])
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] * rhs[0];
	newPoint[1] = m_Point[1] * rhs[1];
	newPoint[2] = m_Point[2] * rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator/( double const rhs[])
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] / rhs[0];
	newPoint[1] = m_Point[1] / rhs[1];
	newPoint[2] = m_Point[2] / rhs[2];
	return newPoint;
}

/* pdp::Pt ****************************************************************************************************/
pdp::Pt pdp::Pt::operator+(pdp::Pt rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] + rhs[0];
	newPoint[1] = m_Point[1] + rhs[1];
	newPoint[2] = m_Point[2] + rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator-(pdp::Pt rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] - rhs[0];
	newPoint[1] = m_Point[1] - rhs[1];
	newPoint[2] = m_Point[2] - rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator*(pdp::Pt rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] * rhs[0];
	newPoint[1] = m_Point[1] * rhs[1];
	newPoint[2] = m_Point[2] * rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator/(pdp::Pt rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] / rhs[0];
	newPoint[1] = m_Point[1] / rhs[1];
	newPoint[2] = m_Point[2] / rhs[2];
	return newPoint;
}

/* mitk::Point3D ****************************************************************************************************/
pdp::Pt pdp::Pt::operator+(mitk::Point3D rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] + rhs[0];
	newPoint[1] = m_Point[1] + rhs[1];
	newPoint[2] = m_Point[2] + rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator-(mitk::Point3D rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] - rhs[0];
	newPoint[1] = m_Point[1] - rhs[1];
	newPoint[2] = m_Point[2] - rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator*(mitk::Point3D rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] * rhs[0];
	newPoint[1] = m_Point[1] * rhs[1];
	newPoint[2] = m_Point[2] * rhs[2];
	return newPoint;
}

pdp::Pt pdp::Pt::operator/(mitk::Point3D rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] / rhs[0];
	newPoint[1] = m_Point[1] / rhs[1];
	newPoint[2] = m_Point[2] / rhs[2];
	return newPoint;
}

/* fuer values ************************************************************************************************/
pdp::Pt pdp::Pt::operator+(double rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] + rhs;
	newPoint[1] = m_Point[1] + rhs;
	newPoint[2] = m_Point[2] + rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator-(double rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] - rhs;
	newPoint[1] = m_Point[1] - rhs;
	newPoint[2] = m_Point[2] - rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator*(double rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] * rhs;
	newPoint[1] = m_Point[1] * rhs;
	newPoint[2] = m_Point[2] * rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator/(double rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] / rhs;
	newPoint[1] = m_Point[1] / rhs;
	newPoint[2] = m_Point[2] / rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator+(int rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] + rhs;
	newPoint[1] = m_Point[1] + rhs;
	newPoint[2] = m_Point[2] + rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator-(int rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] - rhs;
	newPoint[1] = m_Point[1] - rhs;
	newPoint[2] = m_Point[2] - rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator*(int rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] * rhs;
	newPoint[1] = m_Point[1] * rhs;
	newPoint[2] = m_Point[2] * rhs;
	return newPoint;
}

pdp::Pt pdp::Pt::operator/(int rhs)
{
	pdp::Pt newPoint; 
	newPoint[0] = m_Point[0] / rhs;
	newPoint[1] = m_Point[1] / rhs;
	newPoint[2] = m_Point[2] / rhs;
	return newPoint;
}

/* X= operationen *********************************************************************************************/
pdp::Pt pdp::Pt::operator+=(pdp::Pt rhs)
{
	m_Point[0] = m_Point[0] + rhs[0];
	m_Point[1] = m_Point[1] + rhs[1];
	m_Point[2] = m_Point[2] + rhs[2];
	return m_Point;
}

pdp::Pt pdp::Pt::operator-=(pdp::Pt rhs)
{
	m_Point[0] = m_Point[0] - rhs[0];
	m_Point[1] = m_Point[1] - rhs[1];
	m_Point[2] = m_Point[2] - rhs[2];
	return m_Point;
}

pdp::Pt pdp::Pt::operator*=(pdp::Pt rhs)
{
	m_Point[0] = m_Point[0] * rhs[0];
	m_Point[1] = m_Point[1] * rhs[1];
	m_Point[2] = m_Point[2] * rhs[2];
	return m_Point;
}

pdp::Pt pdp::Pt::operator/=(pdp::Pt rhs)
{
	m_Point[0] = m_Point[0] / rhs[0];
	m_Point[1] = m_Point[1] / rhs[1];
	m_Point[2] = m_Point[2] / rhs[2];
	return m_Point;
}

/* Other ***********************************************************************************************************/
double pdp::Pt::Length()
{
	double length = sqrt(m_Point[0]*m_Point[0] + m_Point[1]*m_Point[1] + m_Point[2]*m_Point[2]);
	return length;
}

//pdp::Pt pdp::Pt::operator*(double lhs, pdp::Pt rhs)
//{
//	pdp::Pt newPoint; 
//	newPoint[0] = lhs * rhs[0];
//	newPoint[1] = lhs * rhs[1];
//	newPoint[2] = lhs * rhs[2];
//	return newPoint;
//}

