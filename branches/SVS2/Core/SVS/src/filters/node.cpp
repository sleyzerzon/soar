#include <iostream>
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
	
	~node_filter() {
		map<sg_node*, filter_param_set*>::iterator i;
		for (i = node2param.begin(); i != node2param.end(); ++i) {
			i->first->unlisten(this);
		}
	}
	
	bool compute(filter_param_set *params, sg_node *&n, bool adding) {
		filter_val *nameval;
		string name;
		if (!adding) {
			sg_node *old = n;
			old->unlisten(this);
			node2param.erase(old);
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

/* Return all nodes from the scene */
class all_nodes_filter : public filter, public sg_listener {
public:
	all_nodes_filter(scene *scn) : scn(scn), first(true) {}
	
	~all_nodes_filter() {
		map<sg_node*, filter_val*>::iterator i;
		for (i = results.begin(); i != results.end(); ++i) {
			i->first->unlisten(this);
		}
	}
	
	bool update_results() {
		vector<sg_node*> nodes;
		vector<sg_node*>::iterator i;
		
		if (!first) {
			return true;
		}
		
		scn->get_all_nodes(nodes);
		for (i = nodes.begin(); i != nodes.end(); ++i) {
			add_node(*i);
		}
		first = false;
		return true;
	}
	
	void node_update(sg_node *n, sg_node::change_type t, int added_child) {
		filter_val *r;
		switch (t) {
			case sg_node::CHILD_ADDED:
				add_node(n->get_child(added_child));
				break;
			case sg_node::DELETED:
				if (!map_get(results, n, r)) {
					assert(false);
				}
				remove_result(r);
				break;
			case sg_node::POINTS_CHANGED:
				if (!map_get(results, n, r)) {
					assert(false);
				}
				change_result(r);
				break;
		}
	}
	
private:
	filter_val *add_node(sg_node *n) {
		n->listen(this);
		filter_val *r = new filter_val_c<sg_node*>(n);
		results[n] = r;
		add_result(r, NULL);
		return r;
	}
	
	scene *scn;
	bool first;
	
	map<sg_node*, filter_val*> results;
};

filter *_make_node_filter_(scene *scn, filter_input *input) {
	return new node_filter(scn, input);
}

filter *_make_all_nodes_filter_(scene *scn, filter_input *input) {
	return new all_nodes_filter(scn);
}
