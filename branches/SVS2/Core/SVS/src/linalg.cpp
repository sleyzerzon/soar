#include "linalg.h"
#include <ostream>

using namespace std;

ostream& operator<<(ostream &os, const vec3 &v) {
	os << v.x << " " << v.y << " " << v.z;
	return os;
}

transform3::transform3()
: m00(1.0), m01(0.0), m02(0.0), m03(0.0),
  m10(0.0), m11(1.0), m12(0.0), m13(0.0),
  m20(0.0), m21(0.0), m22(1.0), m23(0.0)
{}

transform3::transform3(double _00, double _01, double _02, double _03,
           double _10, double _11, double _12, double _13,
           double _20, double _21, double _22, double _23 )
: m00(_00), m01(_01), m02(_02), m03(_03),
  m10(_10), m11(_11), m12(_12), m13(_13),
  m20(_20), m21(_21), m22(_22), m23(_23)
{}

transform3::transform3(transform_tags::Translation t, vec3 v)
: m00(1.0), m01(0.0), m02(0.0), m03(v.x),
  m10(0.0), m11(1.0), m12(0.0), m13(v.y),
  m20(0.0), m21(0.0), m22(1.0), m23(v.z)
{}

/* this is from http://planning.cs.uiuc.edu/node102.html but it
   doesn't match up with my reference from cgkit. It must have
   something to do with the order of the component rotations.
   
transform3::transform3(transform_tags::Rotation t, vec3 v)
{
	double sina = sin(v.x), sinb = sin(v.y), sing = sin(v.z),
	       cosa = cos(v.x), cosb = cos(v.y), cosg = cos(v.z);
	
	m00=cosa*cosb; m01=cosa*sinb*sing-sina*cosg; m02=cosa*sinb*cosg+sina*sing; m03=0.0;
	m10=sina*cosb; m11=sina*sinb*sing+cosa*cosg; m12=sina*sinb*cosg-cosa*sing; m13=0.0;
	m20=-sinb;     m21=cosb*sing;                m22=cosb*cosg;                m23=0.0;
}

*/	

/* this one passes the tests, and agrees with 
   http://mathworld.wolfram.com/EulerAngles.html up to some sign differences
 */
transform3::transform3(transform_tags::Rotation t, vec3 v)
{
	double siny = sin(v.x), sinp = sin(v.y), sinr = sin(v.z),
	       cosy = cos(v.x), cosp = cos(v.y), cosr = cos(v.z);
	
	m00=cosp*cosy; m01=cosp*-siny; m02=sinp; m03=0;
	m10=sinr*sinp*cosy+cosr*siny; m11=-sinr*sinp*siny+cosr*cosy; m12=-sinr*cosp; m13=0;
	m20=cosr*-sinp*cosy+sinr*siny; m21=cosr*sinp*siny+sinr*cosy; m22=cosr*cosp; m23=0;
}

transform3::transform3(transform_tags::Scaling t, vec3 v)
: m00(v.x), m01(0.0), m02(0.0), m03(0.0),
  m10(0.0), m11(v.y), m12(0.0), m13(0.0),
  m20(0.0), m21(0.0), m22(v.z), m23(0.0)
{}

transform3 transform3::operator*(const transform3 &x) {
	return transform3(
		m00*x.m00+m01*x.m10+m02*x.m20,
		m00*x.m01+m01*x.m11+m02*x.m21,
		m00*x.m02+m01*x.m12+m02*x.m22,
		m00*x.m03+m01*x.m13+m02*x.m23+m03,
		m10*x.m00+m11*x.m10+m12*x.m20,
		m10*x.m01+m11*x.m11+m12*x.m21,
		m10*x.m02+m11*x.m12+m12*x.m22,
		m10*x.m03+m11*x.m13+m12*x.m23+m13,
		m20*x.m00+m21*x.m10+m22*x.m20,
		m20*x.m01+m21*x.m11+m22*x.m21,
		m20*x.m02+m21*x.m12+m22*x.m22,
		m20*x.m03+m21*x.m13+m22*x.m23+m23);
}

vec3 transform3::operator()(const vec3 &v) {
	return vec3(
		m00*v.x+m01*v.y+m02*v.z+m03,
		m10*v.x+m11*v.y+m12*v.z+m13,
		m20*v.x+m21*v.y+m22*v.z+m23);
}

/*
Used to generate the unrolled matrix multiplication

awk 'BEGIN {
	for (i = 0; i <= 3; i++) {
		for (j = 0; j <= 3; j++) {
			op = ""
			for (r = 0; r <= 3; r++) {
				if ((i == 3 && r != 3) || (r == 3 && j != 3))
					continue
				if (i == 3 && r == 3)
					printf("%sx.m%d%d", op, r, j)
				else if (r == 3 && j == 3)
					printf("%sm%d%d", op, i, r)
				else
					printf("%sm%d%d*x.m%d%d", op, i, r, r, j)
				op = "+"
			}
			print ","
		}
	}
	exit
}'

*/