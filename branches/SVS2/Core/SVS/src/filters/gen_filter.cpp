#include <sstream>
#include <vector>
#include "filter.h"
#include "nsg_node.h"
#include "scene.h"

using namespace std;
int gen_counter = 0;

class gen_filter : public filter {
public:
	gen_filter(filter *nf,
	           filter *ptsf,
	           filter *pos, 
	           filter *rot,
	           filter *scale)
	: namef(nf), pointf(ptsf), posf(pos), rotf(rot), scalef(scale), container(this)
	{
		vector<filter*>::iterator i;
		
		if (namef)  container.add(namef);
		if (pointf) container.add(pointf);
		if (posf)   container.add(posf);
		if (rotf)   container.add(rotf);
		if (scalef) container.add(scalef);
	}

	filter_result *calc_result() {
		string name;
		ptlist *pts;
		vec3 pos(0.0,0.0,0.0), rot(0.0,0.0,0.0), scale(1.0,1.0,1.0);
		sg_node *n;
	
		if (namef) {
			if (!get_string_filter_result_value(this, namef, name)) {
				return NULL;
			}
		} else {
			stringstream ss;
			ss << "gen_node_" << gen_counter++;
			name = ss.str();
		}
		if (posf && !get_vec3_filter_result_value(this, posf, pos)) {
			return NULL;
		}
		if (rotf && !get_vec3_filter_result_value(this, rotf, rot)) {
			return NULL;
		}
		if (scalef && !get_vec3_filter_result_value(this, scalef, scale)) {
			return NULL;
		}
		
		if (pointf) {
			if (!get_ptlist_filter_result_value(this, pointf, pts)) return NULL;
			n = new nsg_node(name, *pts);
		} else {
			n = new nsg_node(name);
		}
		n->set_trans('p', pos);
		n->set_trans('r', rot);
		n->set_trans('s', scale);
		
		if (!attach_children(n)) {
			delete n;
			return NULL;
		}
		return new node_filter_result(n);
	}
	
private:
	bool attach_children(sg_node *n) {
		sg_node *c;
		vector<filter*>::iterator i;
		
		for (i = childf.begin(); i != childf.end(); ++i) {
			if (!get_node_filter_result_value(this, *i, c)) return false;
			n->attach_child(c);
		}
		return true;
	}
	
	filter*              namef;
	filter*              pointf;
	filter*              posf;
	filter*              rotf;
	filter*              scalef;
	std::vector<filter*> childf;
	filter_container     container;
};

filter* _make_gen_filter_(scene *scn, const filter_params &p) {
	filter_params::const_iterator i;
	filter *ptsf = NULL;
	filter *nf = NULL, *posf = NULL, *rotf = NULL, *scalef = NULL;

	ptsf = NULL; posf = NULL; rotf = NULL; scalef = NULL;
	for (i = p.begin(); i != p.end(); ++i) {
		if (i->first == "name") {
			nf = i->second;
		} else if (i->first == "points") {
			ptsf = i->second;
		} else if (i->first == "pos") {
			posf = i->second;
		} else if (i->first == "rot") {
			rotf = i->second;
		} else if (i->first == "scale") {
			scalef = i->second;
		}
	}
	return new gen_filter(nf, ptsf, posf, rotf, scalef);
}
