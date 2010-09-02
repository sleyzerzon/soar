#include <sstream>
#include "nsg_node.h"
#include "gen_filter.h"

using namespace std;

gen_filter::gen_filter(std::string name,
                       ptlist_filter *pts,
                       vec3_filter *pos, 
                       vec3_filter *rot,
                       vec3_filter *scale,
                       std::vector<node_filter*> &children)
: name(name), pointf(pts), posf(pos), rotf(rot), scalef(scale), childf(children)
{
	std::vector<node_filter*>::iterator i;
	
	if (pointf) {
		add_child(pointf);
		pointf->listen(this);
	}
	if (posf) {
		add_child(posf);
		posf->listen(this);
	}
	if (rotf) {
		add_child(rotf);
		rotf->listen(this);
	}
	if (scalef) {
		add_child(scalef);
		scalef->listen(this);
	}
	
	for(i = childf.begin(); i != childf.end(); ++i) {
		add_child(*i);
		(**i).listen(this);
	}
}

void gen_filter::update(filter *u) {
	notify();
}

bool gen_filter::get_transform(vec3_filter* f, vec3& t) {
	if (f) {
		if (!f->get_result(t)) {
			errmsg = f->get_error();
			return false;
		}
	}
	return true;
}

bool gen_filter::get_result(sg_node* &r) {
	ptlist pts;
	vec3 pos(0.0,0.0,0.0), rot(0.0,0.0,0.0), scale(1.0,1.0,1.0);
	vector<node_filter*>::iterator i;
	sg_node *n, *c;

	if (pointf && childf.size() > 0) {
		errmsg = "GEOMETRY_NODE_CANNOT_HAVE_CHILDREN";
		return false;
	}
	if (!get_transform(posf, pos)) { return false; }
	if (!get_transform(rotf, rot)) { return false; }
	if (!get_transform(scalef, scale)) { return false; }
	
	if (pointf) {
		if (!pointf->get_result(pts)) {
			errmsg = pointf->get_error();
			return false;
		}
		n = new nsg_node(name, pts);
	} else {
		n = new nsg_node(name);
	}
	n->set_pos(pos);
	n->set_rot(rot);
	n->set_scale(scale);
	
	for (i = childf.begin(); i != childf.end(); ++i) {
		if (!(**i).get_result(c)) {
			errmsg = (**i).get_error();
			delete n;
			return false;
		}
		n->attach_child(c);
	}
	
	r = n;
	return true;
}

string gen_filter::get_error() {
	return errmsg;
}

bool gen_filter::get_result_string(string &r) {
	bool status;
	sg_node *t;
	
	status = get_result(t);
	delete t;
	return status;
}
	

filter* make_gen_filter(const filter_params &p) {
	static int counter = 0;
	stringstream ss;
	filter_params::const_iterator i;
	ptlist_filter *ptsf;
	vec3_filter *posf, *rotf, *scalef;
	node_filter *cf;
	std::vector<node_filter*> childs;

	ptsf = NULL; posf = NULL; rotf = NULL; scalef = NULL;
	for (i = p.begin(); i != p.end(); ++i) {
		if (i->first == "points") {
			if (!(ptsf = dynamic_cast<ptlist_filter*>(i->second))) {
				return NULL;
			}
		} else if (i->first == "pos") {
			if (!(posf = dynamic_cast<vec3_filter*>(i->second))) {
				return NULL;
			}
		} else if (i->first == "rot") {
			if (!(rotf = dynamic_cast<vec3_filter*>(i->second))) {
				return NULL;
			}
		} else if (i->first == "scale") {
			if (!(scalef = dynamic_cast<vec3_filter*>(i->second))) {
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
	return new gen_filter(ss.str(), ptsf, posf, rotf, scalef, childs);
}