#include <string>
#include "filter.h"
#include "sg_node.h"

using namespace std;

class node_filter : public filter, public sg_listener {
public:
	node_filter(sg_node *node) 
	: node(node)
	{
		if (node) {
			node->listen(this);
		}
	}
	
	virtual ~node_filter() {
		if (node) {
			node->unlisten(this);
		}
	}
	
	void update(sg_node *n, sg_node::change_type t, int added) {
		set_dirty();
		if (t == sg_node::DELETED) {
			node = NULL;
			set_error("node deleted");
		}
	}

	filter_result *calc_result() {
		if (node) {
			return new node_filter_result(node);
		}
		return NULL;
	}

private:
	sg_node *node;
};

filter *_make_node_filter_(scene *scn, const filter_params &params) {
	filter_params::const_iterator i;
	string name;
	sg_node *n;
	
	if ((i = params.find("name")) == params.end()) {
		return NULL;
	}
	if (!get_string_filter_result_value(NULL, i->second, name)) {
		return NULL;
	}
	if ((n = scn->get_node(name)) == NULL) {
		return NULL;
	}
	return new node_filter(n);
}
