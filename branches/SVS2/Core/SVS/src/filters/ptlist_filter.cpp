#include "ptlist_filter.h"

using namespace std;

ptlist_filter::ptlist_filter(bool local, filter *node)
: local(local), container(this, node)
{}

filter_result *ptlist_filter::calc_result() {
	sg_node *n;
	ptlist pts;
	
	if (!get_node_filter_result_value(this, container.get(0), n)) return NULL;
	if (local) {
		n->get_local_points(pts);
	} else {
		n->get_world_points(pts);
	}
	return new ptlist_filter_result(pts);
}

filter* _make_local_filter_(const filter_params &params) {
	if (params.empty()) {
		return NULL;
	}
	return new ptlist_filter(true, params.begin()->second);
}

filter* _make_world_filter_(const filter_params &params) {
	if (params.empty()) {
		return NULL;
	}
	return new ptlist_filter(false, params.begin()->second);
}

