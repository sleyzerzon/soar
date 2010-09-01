#ifndef LINALG_H
#define LINALG_H

#include <math.h>
#include <ostream>
#include <vector>

class vec3 {
public:
	vec3( )
	: x(0.0), y(0.0), z(0.0)
	{}
	
	vec3(double _x, double _y, double _z)
	: x(_x), y(_y), z(_z)
	{}
	
	double operator[](int i) {
		switch (i) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				return 0.0;
		}
	}
	
	double x; double y; double z;
};

std::ostream& operator<<(std::ostream &os, const vec3 &v);

typedef std::vector<vec3> ptlist;

namespace transform_tags {
	class Translation {};
	class Rotation {};
	class Scaling {};
	class Identity {};
	
	extern const Translation TRANSLATION;
	extern const Rotation    ROTATION;
	extern const Scaling     SCALING;
};

class transform3 {
public:
	transform3();
	
	transform3(double _00, double _01, double _02, double _03,
	           double _10, double _11, double _12, double _13,
	           double _20, double _21, double _22, double _23 );
	
	transform3(transform_tags::Translation t, vec3 v);
	transform3(transform_tags::Rotation t, vec3 v);
	transform3(transform_tags::Scaling t, vec3 v);
	
	transform3 operator*(const transform3 &x);
	vec3 operator()(const vec3 &v);
	
private:
	double m00, m01, m02, m03,
	       m10, m11, m12, m13,
	       m20, m21, m22, m23;
};

#endif
