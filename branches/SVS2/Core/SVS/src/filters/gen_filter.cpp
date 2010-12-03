#include <sstream>
#include "../nsg_node.h"
#include "ptlist_filter.h"
#include "gen_filter.h"

using namespace std;

gen_filter::gen_filter(std::string name,
                       filter *pts,
                       filter *pos, 
                       filter *rot,
                       filter *scale,
                       std::vector<filter*> &children)
: name(name), pointf(pts), posf(pos), rotf(rot), scalef(scale), childf(children), container(this)
{
	std::vector<filter*>::iterator i;
	
	if (pointf) container.add(pointf);
	if (posf)   container.add(posf);
	if (rotf)   container.add(rotf);
	if (scalef) container.add(scalef);
	
	for(i = childf.begin(); i != childf.end(); ++i) {
		container.add(*i);
	}
}

bool gen_filter::attach_children(sg_node *n) {
	sg_node *c;
	vector<filter*>::iterator i;
	
	for (i = childf.begin(); i != childf.end(); ++i) {
		if (!get_node_filter_result_value(this, *i, c)) return false;
		n->attach_child(c);
	}
	return true;
}

filter_result *gen_filter::calc_result() {
	ptlist *pts;
	vec3 pos(0.0,0.0,0.0), rot(0.0,0.0,0.0), scale(1.0,1.0,1.0);
	sg_node *n;

	if (pointf && childf.size() > 0) {
		set_error("geometry node cannot have children");
		return NULL;
	}
	if (!get_vec3_filter_result_value(this, posf, pos))     return NULL;
	if (!get_vec3_filter_result_value(this, rotf, rot))     return NULL;
	if (!get_vec3_filter_result_value(this, scalef, scale)) return NULL;
	
	if (pointf) {
		if (!get_ptlist_filter_result_value(this, pointf, pts)) return NULL;
		n = new nsg_node(name, *pts);
	} else {
		n = new nsg_node(name);
	}
	n->set_pos(pos);
	n->set_rot(rot);
	n->set_scale(scale);
	
	if (!attach_children(n)) {
		delete n;
		return NULL;
	}
	return new node_filter_result(n);
}

filter* _make_gen_filter_(const filter_params &p) {
	static int counter = 0;
	stringstream ss;
	filter_params::const_iterator i;
	filter *ptsf;
	filter *posf, *rotf, *scalef;
	filter *cf;
	std::vector<filter*> childs;

	ptsf = NULL; posf = NULL; rotf = NULL; scalef = NULL;
	for (i = p.begin(); i != p.end(); ++i) {
		if (i->first == "points") {
			ptsf = i->second;
		} else if (i->first == "pos") {
			posf = i->second;
		} else if (i->first == "rot") {
			rotf = i->second;
		} else if (i->first == "scale") {
			scalef = i->second;
		} else if (i->first == "child") {
			cf = i->second;
			childs.push_back(cf);
		} 
	}
	ss << "gen_node" << counter++;
	return new gen_filter(ss.str(), ptsf, posf, rotf, scalef, childs);
}
