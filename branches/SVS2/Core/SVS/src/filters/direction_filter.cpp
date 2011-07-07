#include "filter.h"
#include "bbox.h"

class direction_filter : public map_filter<bool> {
public:
	direction_filter(filter_input *input, char axis, int comp)
	: map_filter<bool>(input), axis(axis), comp(comp) {}
	
	bool compute(filter_param_set *p, bool &res, bool adding) {
		int i, dir[3];
		bbox a, b;
		vec3 amin, amax, bmin, bmax;
		
		if (!get_filter_param(this, p, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, p, "b", b)) {
			return false;
		}
		
		a.get_vals(amin, amax);
		b.get_vals(bmin, bmax);
		
		/*
		 dir[i] = [-1, 0, 1] if a is [less than, overlapping,
		 greater than] b in that dimension.
		*/
		for(i = 0; i < 3; i++) {
			if (amax[i] <= bmin[i]) {
				dir[i] = -1;
			} else if (bmax[i] <= amin[i]) {
				dir[i] = 1;
			} else {
				dir[i] = 0;
			}
		}
	
		switch (axis) {
			case 'x':
				res = (comp == dir[0]);
				return true;
			case 'y':
				res = (comp == dir[1]);
				return true;
			case 'z':
				res = (comp == dir[2]);
				return true;
			default:
				assert(false);
		}
	}
	
private:
	char axis;
	int comp;
};

filter *_make_north_of_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'y', 1);
}

filter *_make_south_of_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'y', -1);
}

filter *_make_east_of_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'x', 1);
}

filter *_make_west_of_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'x', -1);
}

filter *_make_vertically_aligned_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'x', 0);
}

filter *_make_horizontally_aligned_filter_(scene *scn, filter_input *input) {
	return new direction_filter(input, 'y', 0);
}

