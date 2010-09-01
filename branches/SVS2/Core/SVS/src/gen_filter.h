#ifndef GEN_FILTER_H
#define GEN_FILTER_H

#include <sstream>
#include <vector>
#include "filter.h"
#include "sg_node.h"
#include "nsg_node.h"

class transform_filter : public filter {
public:
	virtual bool get_result(vec3 &p, vec3 &r, vec3 &s) = 0;
};

class gen_filter : public node_filter, public filter_listener {
public:
	gen_filter(std::string name, ptlist_filter *pts, transform_filter *trans, std::vector<node_filter*> &childs)
	:name(name), pf(pts), tf(trans), cf(childs) 
	{
		std::vector<node_filter*>::iterator i;
		
		if (pf) { pf->listen(this); }
		if (tf) { tf->listen(this); }
		for(i = cf.begin(); i != cf.end(); ++i) {
			(**i).listen(this);
		}
	}

	void update(filter *u) {
	}
	
	bool changed() {
		return true;
	}
	
	bool get_result(sg_node* &r) {
		ptlist pts;
		vec3 pos, rot, scale;
		std::vector<node_filter*>::iterator i;
		sg_node *n, *c;

		if (error) {
			return false;
		}
		
		if (pf && cf.size() > 0) {
			errmsg = "GEOMETRY_NODE_CANNOT_HAVE_CHILDREN";
			error = true;
			return false;
		}
		if (tf) {
			if (!tf->get_result(pos, rot, scale)) {
				errmsg = tf->get_error();
				error = true;
				return false;
			}
		}
		if (pf) {
			if (!pf->get_result(pts)) {
				errmsg = pf->get_error();
				error = true;
				return false;
			}
			n = new nsg_node(name, pts);
		} else {
			n = new nsg_node(name);
		}
		for (i = cf.begin(); i != cf.end(); ++i) {
			if (!(**i).get_result(c)) {
				errmsg = (**i).get_error();
				error = true;
				delete n;
				return false;
			}
			n->attach_child(c);
		}
		
		r = n;
		return true;
	}

	std::string get_error() {
		return errmsg;
	}
	
	bool get_result_string(std::string &r) {
		bool status;
		sg_node *t;
		
		status = get_result(t);
		delete t;
		return status;
	}
	
private:
	std::string name, errmsg;
	bool error;
	ptlist_filter *pf;
	transform_filter *tf;
	std::vector<node_filter*> cf;
};

inline filter* make_gen_filter(filter_params &p) {
	static int counter = 0;
	std::stringstream ss;
	filter_params::iterator i;
	ptlist_filter *pf;
	transform_filter *tf;
	node_filter *cf;
	std::vector<node_filter*> childs;

	for (i = p.begin(); i != p.end(); ++i) {
		if (i->first == "points") {
			if (!(pf = dynamic_cast<ptlist_filter*>(i->second))) {
				return NULL;
			}
		} else if (i->first == "transform") {
			if (!(tf = dynamic_cast<transform_filter*>(i->second))) {
				return NULL;
			}
		} else if (i->first == "child") {
			if (!(cf = dynamic_cast<node_filter*>(i->second))) {
				return NULL;
			}
			childs.push_back(cf);
		} 
	}
	ss << "gen_node" << counter++;
	return new gen_filter(ss.str(), pf, tf, childs);
}

#endif

