#include <string>
#include "model.h"
#include "soar_interface.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model(string obj, string xdim, string ydim, string zdim)
	: obj(obj), xdim(xdim), ydim(ydim), zdim(zdim) {}
	
	bool predict(flat_scene &scn, const trajectory &trj) {
		vec3 v, p;
		vector<output>::const_iterator i;
		
		if (!scn.get_node_trans(obj, 'p', p)) {
			return false;
		}
		for (i = trj.t.begin(); i != trj.t.end(); ++i) {
			v.x = i->get(xdim);
			v.y = i->get(ydim);
			v.z = i->get(zdim);
			p += v;
		}
		scn.set_node_trans(obj, 'p', p);
		return true;
	}
	
private:
	string obj, xdim, ydim, zdim;
};

model *_make_velocity_model_(soar_interface *si, Symbol *root) {
	wme_list children;
	wme_list::iterator i;
	string attr, obj, xdim, ydim, zdim;
	
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), attr)) {
			continue;
		}
		if (attr == "obj") {
			if (!si->get_val(si->get_wme_val(*i), obj)) return NULL;
		} else if (attr == "x") {
			if (!si->get_val(si->get_wme_val(*i), xdim)) return NULL;
		} else if (attr == "y") {
			if (!si->get_val(si->get_wme_val(*i), ydim)) return NULL;
		} else if (attr == "z") {
			if (!si->get_val(si->get_wme_val(*i), zdim)) return NULL;
		}
	}
	return new velocity_model(obj, xdim, ydim, zdim);
}
