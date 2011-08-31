#include "filter.h"
#include "common.h"
#include "bullet_support.h"

class between_filter : public map_filter<bool> {
public:
	between_filter(filter_input *input) : map_filter<bool>(input) {}
	
	bool compute(filter_param_set *params, bool &result, bool adding) {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!get_filter_param(this, params, "a", na) ||
		    !get_filter_param(this, params, "b", nb) ||
		    !get_filter_param(this, params, "c", nc))
		{
			return false;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		copy(pa.begin(), pa.end(), back_inserter(pc));
		
		result = (hull_distance(pb, pc) < 0);
		return true;
	}
};

/*
 Given 3 objects a, b, and c, returns whether all points in c are behind
 or in front of all points in a w.r.t. the vector from the centroid of
 a to the centroid of b. The parameter dir should be positive if an in
 front query is desired, and negative if a behind query is desired.
*/
class dir_order_filter : public map_filter<bool> {
public:
	dir_order_filter(filter_input *input, float dir) : map_filter<bool>(input), dir(dir) {}
	
	bool compute(filter_param_set *params, bool &result, bool adding) {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!get_filter_param(this, params, "a", na) ||
		    !get_filter_param(this, params, "b", nb) ||
		    !get_filter_param(this, params, "c", nc))
		{
			return false;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 u = (cb - ca).unit();
		
		result = (dir_separation(pa, pc, u) * dir >= 0);
		return true;
	}

private:
	float dir;
};

filter* _make_between_filter_(scene *scn, filter_input *input) {
	return new between_filter(input);
}

filter* _make_infront_filter_(scene *scn, filter_input *input) {
	return new dir_order_filter(input, 1.0);
}

filter* _make_behind_filter_(scene *scn, filter_input *input) {
	return new dir_order_filter(input, -1.0);
}
