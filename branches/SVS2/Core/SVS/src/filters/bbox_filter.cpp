#include <vector>
#include <sstream>
#include "linalg.h"
#include "filter.h"
#include "scene.h"
#include "bbox_filter.h"

using namespace std;

bool get_bbox_filter_result_value(filter *requester, filter *f, bbox &b) {
	filter_result *fr;
	bbox_filter_result *bfr;
	
	if (!(fr = f->get_result())) {
		requester->set_error(f->get_error());
		return false;
	} 
	if (!(bfr = dynamic_cast<bbox_filter_result*>(fr))) {
		requester->set_error("invalid input");
		return false;
	}
	b = bfr->get_value();
	return true;
}

bbox_filter_result::bbox_filter_result(bbox r) : r(r) {}

string bbox_filter_result::get_string() {
	stringstream ss;
	ss << r;
	return ss.str();
}

bbox bbox_filter_result::get_value() {
	return r;
}

bbox_filter::bbox_filter(filter* node)
: container(this, node)
{ }

bbox_filter::bbox_filter(vector<filter*> nodes)
: container(this)
{
	vector<filter*>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		container.add(*i);
	}
}

filter_result *bbox_filter::calc_result() {
	int i;
	sg_node *n;
	ptlist npts, allpts;
	
	if (container.size() == 0) {
		set_error("no inputs to bbox filter");
		return NULL;
	}

	for (i = 0; i < container.size(); i++) {
		if (!get_node_filter_result_value(this, container.get(i), n)) return NULL;
		npts.clear();
		n->get_world_points(npts);
		allpts.insert(allpts.end(), npts.begin(), npts.end());
	}
	return new bbox_filter_result(bbox(allpts));
}


filter* _make_bbox_filter_(scene *scn, const filter_params &inputs) {
	vector<filter*> filters;
	filter_params::const_iterator i;
	
	filters.reserve(inputs.size());
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		filters.push_back(i->second);
	}
	return new bbox_filter(filters);
}
