#include <sstream>
#include <vector>
#include "filter.h"
#include "nsg_node.h"
#include "scene.h"

using namespace std;
int gen_counter = 0;

/*
 Need to have better memory management of nodes in general. Right now
 whenever a node is deleted all its children are also deleted, which
 may break the caching that's going on in the map_filter class.
*/
class gen_filter : public map_filter<sg_node*> {
public:
	gen_filter(filter_input *input) : map_filter<sg_node*>(input) {}

	bool compute(filter_param_set *params, sg_node *&res, bool adding) {
		string name;
		vec3 pos, rot, scale, singlept;
		filter_val *ptsval;
		ptlist *pts;
		bool dealloc_pts = false;
		stringstream ss;
		
		if (!get_filter_param(NULL, params, "name", name)) {
			ss << "gen_node_" << gen_counter++;
			name = ss.str();
		}
		if (!get_filter_param(NULL, params, "pos", pos)) {
			pos = vec3(0., 0., 0.);
		}
		if (!get_filter_param(NULL, params, "rot", rot)) {
			rot = vec3(0., 0., 0.);
		}
		if (!get_filter_param(NULL, params, "scale", scale)) {
			pos = vec3(1., 1., 1.);
		}
		
		if (get_filter_param(NULL, params, "points", pts)) {
			res = new nsg_node(name, *pts);
		} else {
			if (get_filter_param(this, params, "points", singlept)) {
				pts = new ptlist();
				dealloc_pts = true;
				pts->push_back(singlept);
				res = new nsg_node(name, *pts);
			} else {
				res = new nsg_node(name);
			}
		}
		
		res->set_trans('p', pos);
		res->set_trans('r', rot);
		res->set_trans('s', scale);
		return true;
	}
};

filter* _make_gen_filter_(scene *scn, filter_input *input) {
	return new gen_filter(input);
}
