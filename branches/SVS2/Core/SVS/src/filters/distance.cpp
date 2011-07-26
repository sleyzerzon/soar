#include "filter.h"

using namespace std;

class distance_filter : public map_filter<float> {
public:
	distance_filter(filter_input *input) : map_filter<float>(input) {}

	bool compute(filter_param_set *params, float &v, bool adding) {
		sgnode *a, *b;
		vec3 ac, bc;
		ptlist apts, bpts;
		
		if (!get_filter_param(this, params, "a", a) ||
		    !get_filter_param(this, params, "b", b))
		{
			return false;
		}
		a->get_world_points(apts);
		b->get_world_points(bpts);
		ac = calc_centroid(apts);
		bc = calc_centroid(bpts);
		
		v = ac.dist(bc);
		return true;
	}
	
private:
	vec3 calc_centroid(const ptlist &pts) {
		vec3 c;
		ptlist::const_iterator i;
		for (i = pts.begin(); i != pts.end(); ++i) {
			c += *i;
		}
		c /= pts.size();
		return c;
	}
};

filter *_make_distance_filter_(scene *scn, filter_input *input) {
	return new distance_filter(input);
}
