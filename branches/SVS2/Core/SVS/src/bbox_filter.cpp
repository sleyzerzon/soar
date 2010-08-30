#include "bbox_filter.h"
#include "filter.h"
#include "linalg.h"
#include <vector>

using namespace std;

ptlist_bbox_filter::ptlist_bbox_filter(vector<ptlist_filter*> inputs)
: in(inputs), box(NULL)
{
	vector<ptlist_filter*>::iterator i;
	for (i = in.begin(); i != in.end(); ++i) {
		add_child(*i);
		(**i).listen(this);
	}
}

ptlist_bbox_filter::~ptlist_bbox_filter() {
	vector<ptlist_filter*>::iterator i;
	for (i = in.begin(); i != in.end(); ++i) {
		(**i).unlisten(this);
	}
}

bool ptlist_bbox_filter::get_result(bbox &r) {
	vector<ptlist_filter*>::iterator i;
	ptlist l;
	
	if (error) {
		return false;
	}
	if (!box) {
		i = in.begin();
		if (i == in.end()) {
			error = true;
			errmsg = "No inputs";
			return false;
		}
		if (!(**i).get_result(l)) {
			error = true;
			errmsg = (**i).get_error();
			return false;
		}
		box = new bbox(l);
		for (++i; i != in.end(); ++i) {
			if (!(**i).get_result(l)) {
				error = true;
				errmsg = (**i).get_error();
				return false;
			}
			box->include(l);
		}
	}
	r = *box;
	return true;
}

string ptlist_bbox_filter::get_error() {
	return errmsg;
}

void ptlist_bbox_filter::update(filter *u) {
	delete box;
	box = NULL;
	notify();
}

bbox_int_filter::bbox_int_filter(bbox_filter *a, bbox_filter *b)
: qa(a), qb(b), dirty(true), error(false)
{
	add_child(qa);
	add_child(qb);
	qa->listen(this);
	qb->listen(this);
}

bbox_int_filter::~bbox_int_filter() {
	qa->unlisten(this);
	qb->unlisten(this);
}

void bbox_int_filter::update(filter *u) {
	dirty = true;
	notify();
}

bool bbox_int_filter::get_result(bool &r) {
	if (error) { return false; }
	if (dirty) {
		bbox a, b;

		if (!qa->get_result(a)) {
			error = true;
			errmsg = qa->get_error();
			return false;
		}
		
		if (!qb->get_result(b)) {
			error = true;
			errmsg = qb->get_error();
			return false;
		}
		result = a.intersects(b);
		dirty = false;
	}
	r = result;
	return true;
}
string bbox_int_filter::get_error() {
	return errmsg;
}

filter* make_bbox_filter(filter_params &inputs) {
	vector<ptlist_filter*> casted;
	filter_params::iterator i;
	
	ptlist_filter *q;

	casted.reserve(inputs.size());
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		q = dynamic_cast<ptlist_filter*>(i->second);
		if (!q) {
			return NULL;
		}
		casted.push_back(q);
	}
	return new ptlist_bbox_filter(casted);
}

filter* make_bbox_int_filter(filter_params &inputs) {
	filter_params::iterator i;
	bbox_filter *a, *b;

	if (inputs.size() < 2) {
		return NULL;
	}
	
	i = inputs.begin(); 
	a = dynamic_cast<bbox_filter*>((i++)->second);
	b = dynamic_cast<bbox_filter*>(i->second);
	if (!a || !b) {
		return NULL;
	}
	return new bbox_int_filter(a, b);
}
