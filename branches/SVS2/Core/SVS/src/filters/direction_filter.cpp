#include "direction_filter.h"
#include "bbox.h"
#include "bbox_filter.h"

direction_filter::direction_filter(char dir, filter *a, filter *b)
: dir(dir), container(this, new bbox_filter(a), new bbox_filter(b))
{ }

bool direction_filter::get_direction(vec3 &result) {
	int i;
	bbox a, b;
	double a1[3], a2[3], b1[3], b2[3], r[3];
	
	if (!get_bbox_filter_result_value(this, container.get(0), a) ||
	    !get_bbox_filter_result_value(this, container.get(1), b))
	{
		return false;
	}
	
	a.get_vals(a1[0], a1[1], a1[2], a2[0], a2[1], a2[2]);
	b.get_vals(b1[0], b1[1], b1[2], b2[0], b2[1], b2[2]);
	
	for(i = 0; i < 3; i++) {
		if (a2[i] <= b1[i]) {
			r[i] = -1;
		} else if (b2[i] <= a1[i]) {
			r[i] = 1;
		} else {
			r[i] = 0;
		}
	}

	result = vec3(r[0],r[1],r[2]);
	return true;
}

filter_result *direction_filter::calc_result() {
	vec3 d;
	bool result;
	if (!get_direction(d)) {
		return NULL;
	}
	switch (dir) {
		case 'v':  // vertically aligned
			result = (d[0] == 0);
			break;
		case 'e':
			result = (d[0] == 1);
			break;
		case 'w':
			result = (d[0] == -1);
			break;
		case 'h': // horizontally aligned
			result = (d[1] == 0);
			break;
		case 'n':
			result = (d[1] == 1);
			break;
		case 's':
			result = (d[1] == -1);
			break;
	}
	return new bool_filter_result(result);
}

filter *make_direction_filter(const filter_params &p, char dir) {
	filter_params::const_iterator i;
	filter *a, *b;
	if ((i = p.find("a")) == p.end()) {
		return NULL;
	}
	a = i->second;
	if ((i = p.find("b")) == p.end()) {
		return NULL;
	}
	b = i->second;
	return new direction_filter(dir, a, b);
}

filter *_make_north_of_filter_(const filter_params &p) {
	return make_direction_filter(p, 'n');
}

filter *_make_south_of_filter_(const filter_params &p) {
	return make_direction_filter(p, 's');
}

filter *_make_east_of_filter_(const filter_params &p) {
	return make_direction_filter(p, 'e');
}

filter *_make_west_of_filter_(const filter_params &p) {
	return make_direction_filter(p, 'w');
}

filter *_make_vertically_aligned_filter_(const filter_params &p) {
	return make_direction_filter(p, 'v');
}

filter *_make_horizontally_aligned_filter_(const filter_params &p) {
	return make_direction_filter(p, 'h');
}

