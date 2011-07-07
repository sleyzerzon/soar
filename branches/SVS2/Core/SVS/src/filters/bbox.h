#ifndef BBOX_H
#define BBOX_H

#include "linalg.h"
#include <ostream>

class bbox {
public:
	bbox() {}
	
	/* bounding box around single point */
	bbox(vec3 &v) {
		min = v;
		max = v;
	}
	
	bbox(ptlist &pts) {
		min = pts[0];
		max = pts[0];
		
		for(int i = 1; i < pts.size(); ++i) {
			include(pts[i]);
		}
	}
	
	void include(vec3 &v) {
		for(int d = 0; d < 3; ++d) {
			if (v[d] < min[d]) { min[d] = v[d]; }
			if (v[d] > max[d]) { max[d] = v[d]; }
		}
	}
	
	void include(ptlist &pts) {
		ptlist::iterator i;
		for(i = pts.begin(); i != pts.end(); ++i) {
			include(*i);
		}
	}
	
	bool intersects(bbox &b) {
		int d;
		for (d = 0; d < 3; ++d) {
			if (max[d] < b.min[d] || min[d] > b.max[d]) {
				return false;
			}
		}
		return true;
	}
	
	bool contains(bbox &b) {
		int d;
		for (d = 0; d < 3; ++d) {
			if (max[d] < b.max[d] || min[d] > b.min[d]) {
				return false;
			}
		}
		return true;
	}
	
	void get_vals(vec3 &minv, vec3 &maxv)
	{
		minv = min; maxv = max;
	}
	
	friend std::ostream& operator<<(std::ostream &os, const bbox &b);
	
private:
	vec3 min;
	vec3 max;
};

inline std::ostream& operator<<(std::ostream &os, const bbox &b) {
	os << b.min[0] << " " << b.min[1] << " " << b.min[2] << " " << b.max[0] << " " << b.max[1] << " " << b.max[2];
	return os;
}

#endif
