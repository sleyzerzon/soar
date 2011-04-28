#ifndef LINALG_H
#define LINALG_H

#include <math.h>
#include <assert.h>
#include <ostream>
#include <vector>

class vec3 {
public:
	double x, y, z;
	
	vec3();
	vec3(double x, double y, double z);
	vec3(const vec3 &v);
	
	double  operator[](int i) const;
	double &operator[](int i);
	vec3 operator+(const vec3 &v) const;
};

class quaternion {
public:
	double a, b, c, d;
	
	quaternion();
	quaternion(double a, double b, double c, double d);
	quaternion(const quaternion &q);
	quaternion(const vec3 &rpy);  // roll, pitch, yaw -> quaternion
	
	vec3 rotate(const vec3 &v) const;
	vec3 to_rpy() const;
	quaternion operator*(const quaternion &q) const;
	quaternion operator*=(const quaternion &q);
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
