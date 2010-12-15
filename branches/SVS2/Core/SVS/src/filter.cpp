#include <sstream>
#include <iterator>
#include "filter.h"

using namespace std;

/* filter result stuff */
bool_filter_result::bool_filter_result(bool r) : r(r) {}
	
string bool_filter_result::get_string() {
	return r ? "t" : "f";
}

bool bool_filter_result::get_value() {
	return r;
}

string_filter_result::string_filter_result(const std::string &r) : r(r) {}
	
string string_filter_result::get_string() {
	return r;
}

string string_filter_result::get_value() {
	return r;
}

vec3_filter_result::vec3_filter_result(const vec3 &r) : r(r) {}
	
string vec3_filter_result::get_string() {
	stringstream ss;
	ss << r;
	return ss.str();
}

vec3 vec3_filter_result::get_value() {
	return r;
}

ptlist_filter_result::ptlist_filter_result(const ptlist &r) : r(r) {}

string ptlist_filter_result::get_string() {
	stringstream ss;
	copy(r.begin(), r.end(), ostream_iterator<vec3>(ss, ", "));
	return ss.str();
}

ptlist *ptlist_filter_result::get_value() {
	return &r;
}

node_filter_result::node_filter_result(sg_node *r) : r(r) {}
	
std::string node_filter_result::get_string() {
	return r->get_name();
}

sg_node *node_filter_result::get_value() {
	return r;
}

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


/* filter stuff */
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

const_string_filter::const_string_filter(const std::string &s) : s(s) {}

filter_result *const_string_filter::calc_result() {
	return new string_filter_result(s);
}

const_node_filter::const_node_filter(sg_node *node) : node(node) {
	if (node) {
		node->listen(this);
	}
}

const_node_filter::~const_node_filter() {
	if (node) {
		node->unlisten(this);
	}
}

void const_node_filter::update(sg_node *n, sg_node::change_type t) {
	set_dirty();
	if (t == sg_node::DELETED) {
		node = NULL;
		set_error("node deleted");
	}
}

filter_result *const_node_filter::calc_result() {
	if (node) {
		return new node_filter_result(node);
	}
	return NULL;
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
