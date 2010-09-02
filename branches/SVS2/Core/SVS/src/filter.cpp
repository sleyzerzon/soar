#include <iostream>
#include <string>
#include <list>
#include <iterator>
#include <sstream>
#include "linalg.h"
#include "sg_node.h"
#include "scene.h"
#include "filter.h"

using namespace std;

typedef struct filter_cons_entry_struct {
	string name;
	filter* (*cons_func)(const filter_params&);
} filter_cons_entry;

filter* make_node_local_ptlist_filter(const filter_params &params);
filter* make_node_world_ptlist_filter(const filter_params &params);

extern filter* make_bbox_filter(const filter_params&);
extern filter* make_bbox_int_filter(const filter_params&);
extern filter* make_gen_filter(const filter_params&);
extern filter* make_bbox_on_pos_filter(const filter_params&);

static filter_cons_entry filter_cons_tab[] = {
	{ "local",       make_node_local_ptlist_filter },
	{ "world",       make_node_world_ptlist_filter },
	{ "bbox",        make_bbox_filter              },
	{ "bbox_int",    make_bbox_int_filter          },
	{ "gen",         make_gen_filter               },
	{ "bbox_on_pos", make_bbox_on_pos_filter       }
};

filter::~filter() {
	std::list<filter*>::iterator i;
	for (i = childs.begin(); i != childs.end(); ++i) {
		delete *i;
	}
}

void filter::add_child(filter *c) {
	childs.push_back(c);
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

bool bool_filter::get_result_string(std::string &r) {
	bool b;
	if (!get_result(b)) {
		return false;
	}
	r = b ? "t" : "f";
	return true;
}

bool string_filter::get_result_string(std::string &r) {
	if (!get_result(r)) {
		return false;
	}
	return true;
}

bool vec3_filter::get_result_string(std::string &r) {
	stringstream ss;
	vec3 v;
	if (!get_result(v)) {
		return false;
	}
	ss << v;
	r = ss.str();
	return true;
}

bool ptlist_filter::get_result_string(string &r) {
	ptlist pts;
	stringstream ss;
	
	if (!get_result(pts)) {
		return false;
	}
	copy(pts.begin(), pts.end(), ostream_iterator<vec3>(ss, ", "));
	r = ss.str();
	return true;
}

bool node_filter::get_result_string(string &r) {
	sg_node *n;
	if (!get_result(n)) {
		return false;
	}
	r = n->get_name();
	return true;
}

bool bbox_filter::get_result_string(string &r) {
	bbox b;
	stringstream ss;
	if (!get_result(b)) {
		return false;
	}
	ss << b;
	r = ss.str();
	return true;
}

// const string

const_string_filter::const_string_filter(const string &s) 
: s(s) 
{ }

bool const_string_filter::get_result(string &r) {
	r = s;
	return true;
}

string const_string_filter::get_error() {
	return "";
}

// const node

const_node_filter::const_node_filter(sg_node *node)
: node(node)
{}

const_node_filter::~const_node_filter() {
	if (node) {
		node->unlisten(this);
	}
}

string const_node_filter::get_error() {
	if (!node) {
		return "NODE_DELETED";
	}
	return "";
}

void const_node_filter::update(sg_node *n, sg_node::change_type t) {
	if (t == sg_node::DELETED) {
		node = NULL;
	}
	notify();
}

bool const_node_filter::get_result(sg_node* &r) {
	if (node) {
		r = node;
		return true;
	}
	return false;
}

// local points

node_ptlist_filter::node_ptlist_filter(bool local, const_node_filter *nf)
: local(local), node_filter(nf)
{ }

bool node_ptlist_filter::get_result(ptlist &r) {
	sg_node *n;
	if (!node_filter->get_result(n)) {
		errmsg = node_filter->get_error();
		return false;
	}
	if (local) {
		n->get_local_points(r);
	} else {
		n->get_world_points(r);
	}
	return true;
}

void node_ptlist_filter::update(filter *f) {
}

string node_ptlist_filter::get_error() {
	return errmsg;
}

filter* make_node_ptlist_filter(bool local, const filter_params &params) {
	const_node_filter *f;
	
	if (params.empty()) {
		return NULL;
	}
	if (!(f = dynamic_cast<const_node_filter*>(params.begin()->second))) {
		return NULL;
	}
	
	return new node_ptlist_filter(local, f);
}

filter* make_node_local_ptlist_filter(const filter_params &params) {
	return make_node_ptlist_filter(true, params);
}

filter* make_node_world_ptlist_filter(const filter_params &params) {
	return make_node_ptlist_filter(false, params);
}

filter* make_filter(string name, const filter_params &params) {
	int tabsize = sizeof(filter_cons_tab) / sizeof(filter_cons_entry);
	
	cout << "MAKING " << name << endl;
	for(int i = 0; i < tabsize; ++i) {
		if (name == filter_cons_tab[i].name) {
			return filter_cons_tab[i].cons_func(params);
		}
	}
	
	return NULL;
}
