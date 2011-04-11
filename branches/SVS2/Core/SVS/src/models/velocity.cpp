#include <string>
#include "model.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model(string object, string vxn, string vyn, string vzn)
	obj(obj), vxn(vxn), vyn(vyn), vzn(vzn) {}
	
	void predict(scene *scn, const env_output &out) {
		vec3 v;
		sg_node n;
		n = scn->get_node(obj);
		v.x = out.get(vxn);
		v.y = out.get(vyn);
		v.z = out.get(vzn);
		n->set_trans('p', n->get_trans('p') + v);
	}
	
private:
	string obj, vxn, vyn, vzn;
};

model* _make_velocity_model_(Symbol *id) {
	return new velocity_model("", "", "", "");
}
