#include <vector>
#include <sstream>
#include "../linalg.h"
#include "../filter.h"
#include "bbox_filter.h"
#include "ptlist_filter.h"

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
: container(this, new ptlist_filter(false, node))
{ }

bbox_filter::bbox_filter(vector<filter*> nodes)
: container(this)
{
	vector<filter*>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		container.add(new ptlist_filter(false, *i));
	}
}

filter_result *bbox_filter::calc_result() {
	int i;
	ptlist *l, allpts;
	filter_result *fr;
	ptlist_filter_result *pfr;
	bbox *b;
	
	if (container.size() == 0) {
		set_error("no inputs to bbox filter");
		return NULL;
	}

	for (i = 0; i < container.size(); i++) {
		if (!get_ptlist_filter_result_value(this, container.get(i), l)) return NULL;
		allpts.insert(allpts.end(), l->begin(), l->end());
	}
	return new bbox_filter_result(bbox(allpts));
}


filter* _make_bbox_filter_(const filter_params &inputs) {
	vector<filter*> filters;
	filter_params::const_iterator i;
	
	ptlist_filter *q;

	filters.reserve(inputs.size());
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		filters.push_back(i->second);
	}
	return new bbox_filter(filters);
}
