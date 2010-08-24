#include "linalg.h"
#include <ostream>

using namespace std;

ostream& operator<<(ostream &os, const vec3 &v) {
	os << v.x << " " << v.y << " " << v.z;
	return os;
}
