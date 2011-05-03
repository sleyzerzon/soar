#include "linalg.h"
#include <ostream>

using namespace std;

vec3::vec3() : x(0.0), y(0.0), z(0.0) { }

vec3::vec3(double x, double y, double z) : x(x), y(y), z(z) { }

vec3::vec3(const vec3 &v) : x(v.x), y(v.y), z(v.z) { }

double vec3::operator[](int i) const {
	switch (i) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: assert(false);
	}	
}

double &vec3::operator[](int i) {
	switch (i) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: assert(false);
	}	
}

vec3 vec3::operator+(const vec3 &v) const {
	return vec3(x+v.x, y+v.y, z+v.z);
}

bool vec3::operator==(const vec3 &v) const {
	return (x == v.x && y == v.y && z == v.z);
}

double vec3::dist(const vec3 &v) const {
	double dx = x-v.x, dy = y-v.y, dz = z-v.z;
	return dx * dx + dy * dy + dz * dz;
}

ostream& operator<<(ostream &os, const vec3 &v) {
	os << v.x << " " << v.y << " " << v.z;
	return os;
}

quaternion::quaternion() : a(0.0), b(0.0), c(0.0), d(0.0) {}
quaternion::quaternion(double a, double b, double c, double d) : a(a), b(b), c(c), d(d) {}
quaternion::quaternion(const quaternion &q) : a(q.a), b(q.b), c(q.c), d(q.d) {}

/* from april.jmat.LinAlg.rollPitchYawToQuat */
quaternion::quaternion(const vec3 &rpy) {
	double halfroll = rpy.x / 2;
	double halfpitch = rpy.y / 2;
	double halfyaw = rpy.z / 2;

	double sin_r2 = sin( halfroll );
	double sin_p2 = sin( halfpitch );
	double sin_y2 = sin( halfyaw );

	double cos_r2 = cos( halfroll );
	double cos_p2 = cos( halfpitch );
	double cos_y2 = cos( halfyaw );

	a = cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2;
	b = sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2;
	c = cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2;
	d = cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2;
}

/* from april.jmat.LinAlg.quatRotate */
vec3 quaternion::rotate(const vec3 &v) const {
	double t2, t3, t4, t5, t6, t7, t8, t9, t10;
	vec3 r;

	t2 = a*b;
	t3 = a*c;
	t4 = a*d;
	t5 = -b*b;
	t6 = b*c;
	t7 = b*d;
	t8 = -c*b;
	t9 = c*d;
	t10 = -d*d;

	r.x = 2*((t8+t10)*v.x + (t6-t4)*v.y  + (t3+t7)*v.z) + v.x;
	r.y = 2*((t4+t6)*v.x  + (t5+t10)*v.y + (t9-t2)*v.z) + v.y;
	r.z = 2*((t7-t3)*v.x  + (t2+t9)*v.y  + (t5+t8)*v.z) + v.z;
	
	return r;
}

/* from april.jmat.LinAlg.quatToRollPitchYaw */
vec3 quaternion::to_rpy() const {
	double roll_a = 2 * (a*b + c*d);
	double roll_b = 1 - 2 * (b*b + c*c);
	double pitch_sin = 2 * ( a*c - d*b );
	double yaw_a = 2 * (a*d + b*c);
	double yaw_b = 1 - 2 * (c*c + d*d);

	return vec3(atan2(roll_a, roll_b), asin(pitch_sin), atan2(yaw_a, yaw_b));
}

/* from april.jmat.LinAlg.quatMultiply */
quaternion quaternion::operator*(const quaternion &q) const {
	quaternion r;
	r.a = a*q.a - b*q.b - c*q.c - d*q.d;
	r.b = a*q.b + b*q.a + c*q.d - d*q.c;
	r.c = a*q.c - b*q.d + c*q.a + d*q.b;
	r.d = a*q.d + b*q.c - c*q.b + d*q.a;
	return r;
}

/* from april.jmat.LinAlg.quatMultiply */
quaternion quaternion::operator*=(const quaternion &q) {
	double ta, tb, tc, td;
	ta = a*q.a - b*q.b - c*q.c - d*q.d;
	tb = a*q.b + b*q.a + c*q.d - d*q.c;
	tc = a*q.c - b*q.d + c*q.a + d*q.b;
	td = a*q.d + b*q.c - c*q.b + d*q.a;
	a = ta; b = tb; c = tc; d = td;
}

transform3::transform3()
: m00(1.0), m01(0.0), m02(0.0), m03(0.0),
  m10(0.0), m11(1.0), m12(0.0), m13(0.0),
  m20(0.0), m21(0.0), m22(1.0), m23(0.0)
{}

transform3::transform3(double m00, double m01, double m02, double m03,
                       double m10, double m11, double m12, double m13,
                       double m20, double m21, double m22, double m23 )
: m00(m00), m01(m01), m02(m02), m03(m03),
  m10(m10), m11(m11), m12(m12), m13(m13),
  m20(m20), m21(m21), m22(m22), m23(m23)
{}

transform3::transform3(transform_tags::Translation t, vec3 v)
: m00(1.0), m01(0.0), m02(0.0), m03(v.x),
  m10(0.0), m11(1.0), m12(0.0), m13(v.y),
  m20(0.0), m21(0.0), m22(1.0), m23(v.z)
{}

/* Application order - roll, pitch, yaw
   http://mathworld.wolfram.com/EulerAngles.html */
transform3::transform3(transform_tags::Rotation t, vec3 v)
{
	double sinr = sin(v.x), sinp = sin(v.y), siny = sin(v.z),
	       cosr = cos(v.x), cosp = cos(v.y), cosy = cos(v.z);
	
	m00=cosy*cosp; m01=-siny*cosr+cosy*sinp*sinr; m02=-siny*-sinr+cosy*sinp*cosr; m03=0;
	m10=siny*cosp; m11=cosy*cosr+siny*sinp*sinr;  m12=cosy*-sinr+siny*sinp*cosr;  m13=0;
	m20=-sinp;     m21=cosp*sinr;                 m22=cosp*cosr;                  m23=0;
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
# Used to generate the unrolled matrix multiplication

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

# Poor man's symbolic matrix multiplication with awk. Unrolls
# the transform matrix for 3D rotation using roll, pitch, and yaw.

awk 'BEGIN {
	ys="cosy -siny 0.0 0.0 " \
	   "siny  cosy 0.0 0.0 " \
	   " 0.0   0.0 1.0 0.0 " \
	   " 0.0   0.0 0.0 1.0 "

	ps=" cosp 0.0 sinp 0.0 " \
	   "  0.0 1.0  0.0 0.0 " \
	   "-sinp 0.0 cosp 0.0 " \
	   "  0.0 0.0  0.0 1.0 "

	rs="1.0  0.0   0.0 0.0 " \
	   "0.0 cosr -sinr 0.0 " \
	   "0.0 sinr  cosr 0.0 " \
	   "0.0  0.0   0.0 1.0 "
	
	parse(RY, ys)
	parse(RP, ps)
	parse(RR, rs)

	mult(RY, RP, F)
	mult(F, RR, FF)
	
	printmat(FF)
	exit
}

function printmat(M) {
	print "{"
	for (i = 0; i <= 3; i++) {
		printf "{ "
		sep = ""
		for (j = 0; j <= 3; j++) {
			printf sep M[i,j]
			sep = ", "
		}
		print "},"
	}
	print "}"
}

function parse(M, s) {
	split(s, xxx)
	for (i = 0; i <= 3; i++)
		for (j = 0; j <= 3; j++)
			M[i, j] = xxx[i * 4 + j + 1]
}

function mult(A, B, AB) {
	for (i = 0; i <= 3; i++) {
		for (j = 0; j <= 3; j++) {
			line=""
			op = ""
			for (r = 0; r <= 3; r++) {
				if (A[i,r] == 0.0 || B[r,j] == 0.0)
					continue
				if (A[i,r] == 1.0) {
					line = line sprintf("%s%s", op, B[r, j])
				} else if (B[r, j] == 1.0) {
					line = line sprintf("%s%s", op, A[i, r])
				} else {
					line = line sprintf("%s%s*%s", op, A[i, r], B[r, j])
				}
				op = "+"
			}
			if (length(line) == 0)
				AB[i,j] = 0.0
			else
				AB[i,j]=line
		}
	}
}'


*/