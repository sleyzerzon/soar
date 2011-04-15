#include <string>
#include "model.h"
#include "soar_interface.h"

using namespace std;

model *_make_null_model_(soar_interface *si, Symbol* root);
model *_make_velocity_model_(soar_interface *si, Symbol *root);
model *_make_lwr_model_(soar_interface *si, Symbol *root);

model *parse_model_struct(soar_interface *si, Symbol *root) {
	wme *type_wme;
	string type;
	
	if (!si->find_child_wme(root, "type", type_wme)) {
		return NULL;
	}
	if (!si->get_val(si->get_wme_val(type_wme), type)) {
		return NULL;
	}
	
	if (type == "null") {
		return _make_null_model_(si, root);
	} else if (type == "velocity") {
		return _make_velocity_model_(si, root);
	} else if (type == "lwr") {
		return _make_lwr_model_(si, root);
	}
	return NULL;
}
