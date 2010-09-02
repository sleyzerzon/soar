#ifndef BBOX_H
#define BBOX_H

#include "linalg.h"
#include <ostream>

class bbox {
public:
	bbox() {
		for(int d = 0; d < 3; ++d) {
			min[d] = 0.0;
			max[d] = 0.0;
		}
	}
	
	bbox(vec3 &v) {
		for(int d = 0; d < 3; ++d) {
			min[d] = v[d];
			max[d] = v[d];
		}
	}
	
	bbox(ptlist &pts) {
		ptlist::iterator i = pts.begin();
		for(int d = 0; d < 3; ++d) {
			min[d] = (*i)[d];
			max[d] = (*i)[d];
		}
		
		for(++i; i != pts.end(); ++i) {
			include(*i);
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
	
	void get_vals(double &x1, double &y1, double &z1, 
	              double &x2, double &y2, double &z2)
	{
		x1 = min[0]; y1 = min[1]; z1 = min[2];
		x2 = max[0]; y2 = max[1]; z2 = max[2];
	}
	
	friend std::ostream& operator<<(std::ostream &os, bbox &b);
	
private:
	double min[3];
	double max[3];
};

inline std::ostream& operator<<(std::ostream &os, bbox &b) {
	os << b.min[0] << " " << b.min[1] << " " << b.min[2] << " " << b.max[0] << " " << b.max[1] << " " << b.max[2];
	return os;
}

#endif
