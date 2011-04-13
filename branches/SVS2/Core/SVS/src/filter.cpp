#include <sstream>
#include <iterator>
#include <utility>

#include "filter.h"

using namespace std;

bool get_bool_filter_result_value(filter *requester, filter *f, bool &v) {
	filter_result *fr;
	bool_filter_result *cfr;
	
	if (!(fr = f->get_result())) {
		if (requester) requester->set_error(f->get_error());
		return false;
	} 
	if (!(cfr = dynamic_cast<bool_filter_result*>(fr))) {
		if (requester) requester->set_error("invalid input");
		return false;
	}
	v = cfr->get_value();
	return true;
}

bool get_string_filter_result_value(filter *requester, filter *f, string &v) {
	filter_result *fr;
	string_filter_result *cfr;
	
	if (!(fr = f->get_result())) {
		if (requester) requester->set_error(f->get_error());
		return false;
	} 
	if (!(cfr = dynamic_cast<string_filter_result*>(fr))) {
		if (requester) requester->set_error("invalid input");
		return false;
	}
	v = cfr->get_value();
	return true;
}

bool get_vec3_filter_result_value(filter *requester, filter *f, vec3 &v) {
	filter_result *fr;
	vec3_filter_result *cfr;
	
	if (!(fr = f->get_result())) {
		if (requester) requester->set_error(f->get_error());
		return false;
	} 
	if (!(cfr = dynamic_cast<vec3_filter_result*>(fr))) {
		if (requester) requester->set_error("invalid input");
		return false;
	}
	v = cfr->get_value();
	return true;
}

bool get_ptlist_filter_result_value(filter *requester, filter *f, ptlist *&v) {
	filter_result *fr;
	ptlist_filter_result *cfr;
	
	if (!(fr = f->get_result())) {
		if (requester) requester->set_error(f->get_error());
		return false;
	} 
	if (!(cfr = dynamic_cast<ptlist_filter_result*>(fr))) {
		if (requester) requester->set_error("invalid input");
		return false;
	}
	v = cfr->get_value();
	return true;
}

bool get_node_filter_result_value(filter *requester, filter *f, sg_node *&v) {
	filter_result *fr;
	node_filter_result *cfr;
	
	if (!(fr = f->get_result())) {
		if (requester) requester->set_error(f->get_error());
		return false;
	} 
	if (!(cfr = dynamic_cast<node_filter_result*>(fr))) {
		if (requester) requester->set_error("invalid input");
		return false;
	}
	v = cfr->get_value();
	return true;
}


filter::filter() : dirty(true), cached(NULL) {}

filter::~filter() {
	if (cached) {
		delete cached;
	}
}

void filter::listen(filter_listener *l) {
	listeners.push_back(l);
}

void filter::unlisten(filter_listener *l) {
	listeners.remove(l);
}

void filter::notify() {
	std::list<filter_listener*>::iterator i;
	for (i = listeners.begin(); i != listeners.end(); ++i) {
		(**i).update(this);
	}
}

filter_result *filter::get_result() {
	if (dirty) {
		if (cached) {
			delete cached;
		}
		cached = calc_result();
		dirty = false;
	}
	return cached;
}

void filter::set_dirty() {
	dirty = true;
	notify();
}

string filter::get_error() {
	return errmsg;
}

void filter::set_error(string msg) {
	errmsg = msg;
}

filter_container::filter_container(filter *owner) 
: owner(owner)
{}

filter_container::filter_container(filter *owner, filter *f) 
: owner(owner)
{
	filters.push_back(f);
	f->listen(this);
}

filter_container::filter_container(filter *owner, filter *a, filter *b)
: owner(owner)
{
	filters.push_back(a);
	filters.push_back(b);
	a->listen(this);
	b->listen(this);
}

filter_container::filter_container(filter *owner, const vector<filter*> &filters) 
: owner(owner), filters(filters)
{
	vector<filter*>::const_iterator i;
	for(i = filters.begin(); i != filters.end(); ++i) {
		(**i).listen(this);
	}
}

filter_container::~filter_container() {
	vector<filter*>::const_iterator i;
	for(i = filters.begin(); i != filters.end(); ++i) {
		delete *i;
	}
}

void filter_container::update(filter *f) {
	owner->set_dirty();
}

filter *filter_container::get(int i) {
	return filters.at(i);
}

void filter_container::add(filter *f) {
	filters.push_back(f);
	f->listen(this);
}

int filter_container::size() {
	return filters.size();
}

/*
(<c> ^on-top <ot>)
(<ot> ^a <ota> ^b <otb>)
(<ota> ^type node ^name box1)
(<otb> ^type node ^name box2)
*/
filter *parse_filter_struct(soar_interface *si, Symbol *root, scene *scn) {
	wme_list children;
	wme_list::iterator i;
	Symbol* cval;
	string strval, pname, type;
	long intval;
	float floatval;
	filter_params params;
	filter_params::iterator j;
	bool fail;
	filter *f;
	
	fail = false;
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), pname)) {
			continue;
		}
		cval = si->get_wme_val(*i);
		if (pname == "type") {
			if (!si->get_val(cval, type)) {
				fail = true;
				break;
			} else {
				continue;
			}
		}
		
		if (si->get_val(cval, strval)) {
			params.insert(make_pair(pname, new const_string_filter(strval)));
		} else if (si->get_val(cval, intval)) {
			params.insert(make_pair(pname, new const_int_filter(intval)));
		} else if (si->get_val(cval, floatval)) {
			params.insert(make_pair(pname, new const_float_filter(floatval)));
		} else {
			// must be identifier
			params.insert(make_pair(pname, parse_filter_struct(si, cval, scn)));
		}
	}
	
	if (fail || type == "" || !(f = make_filter(type, scn, params))) {
		for (j = params.begin(); j != params.end(); ++j) {
			if (j->second) {
				delete j->second;
			}
		}
		return NULL;
	}
	return f;
}
