#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sg_node.h"
#include "scene.h"

using namespace std;

/*
 This filter takes a "name" parameter and outputs a pointer to the node
 with that name in the scene graph.
*/
class node_filter : public map_filter<sg_node*>, public sg_listener {
public:
	node_filter(scene *scn, filter_input *input) : map_filter<sg_node*>(input), scn(scn) {}
	
	bool compute(filter_param_set *params, sg_node *&n, bool adding) {
		filter_val *nameval;
		string name;
		
		if (!adding) {
			sg_node *old = n;
			map<sg_node*, filter_param_set*>::iterator i = node2param.find(old);
			assert(i != node2param.end());
			old->unlisten(this);
			node2param.erase(i);
		}
		
		if (!get_filter_param(this, params, "name", name)) {
			return false;
		}
		if ((n = scn->get_node(name)) == NULL) {
			stringstream ss;
			ss << "no node called \"" << name << "\"";
			set_error(ss.str());
			return false;
		}
		
		n->listen(this);
		node2param[n] = params;
		return true;
	}
	
	void node_update(sg_node *n, sg_node::change_type t, int added) {
		if (t == sg_node::DELETED || t == sg_node::POINTS_CHANGED) {
			filter_param_set *s;
			if (!map_get(node2param, n, s)) {
				assert(false);
			}
			mark_stale(s);
		}
	}

private:
	scene *scn;
	map<sg_node*, filter_param_set*> node2param;
};

filter *_make_node_filter_(scene *scn, filter_input *input) {
	return new node_filter(scn, input);
}
