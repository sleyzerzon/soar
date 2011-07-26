#include <string>
#include "svs.h"
#include "command.h"
#include "model.h"

using namespace std;

model *_make_null_model_(soar_interface *si, Symbol* root);
model *_make_velocity_model_(soar_interface *si, Symbol *root);
model *_make_lwr_model_(soar_interface *si, Symbol *root);
model *_make_splinter_model_(soar_interface *si, Symbol *root);

model *parse_model_struct(soar_interface *si, Symbol *root, string &name) {
	wme *type_wme, *name_wme;
	string type;
	
	if (!si->find_child_wme(root, "type", type_wme) ||
		!si->get_val(si->get_wme_val(type_wme), type))
	{
		return NULL;
	}
	
	if (!si->find_child_wme(root, "name", name_wme) ||
		!si->get_val(si->get_wme_val(name_wme), name))
	{
		return NULL;
	}
	
	if (type == "null") {
		return _make_null_model_(si, root);
	} else if (type == "velocity") {
		return _make_velocity_model_(si, root);
	} else if (type == "lwr") {
		return _make_lwr_model_(si, root);
	} else if (type == "splinter") {
		return _make_splinter_model_(si, root);
	}
	return NULL;
}

class create_model_command : public command {
public:
	create_model_command(svs_state *state, Symbol *root)
	 : command(state, root), root(root), svsp(state->get_svs())
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	string description() {
		return string("create model");
	}
	
	bool update() {
		string name;
		
		if (!changed()) {
			return true;
		}
		
		model *m = parse_model_struct(si, root, name);
		if (m == NULL) {
			set_status("invalid syntax");
			return false;
		}
		svsp->add_model(name, m);
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
	
private:
	soar_interface *si;
	Symbol         *root;
	svs            *svsp;
};


class assign_model_command : public command {
public:
	assign_model_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), svsp(state->get_svs())
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	~assign_model_command() {
		cout << "DESTRUCTOR " << name << endl;
		svsp->unassign_model(name);
	}
	
	string description() {
		return string("activate model");
	}
	
	bool early() {
		return false;
	}
	
	bool update() {
		int i;
		wme *w;
		wme_list children;
		wme_list::iterator j;
		Symbol *attr, *val;
		string attrstr, valstr;
		map<string, string> inputs;
		map<string, string> outputs;
		
		if (!changed()) {
			return true;
		}
		
		if (!si->find_child_wme(root, "name", w) ||
		    !si->get_val(si->get_wme_val(w), name))
		{
			set_status("need model name");
			return false;
		}
		
		for (i = 0; i < 2; ++i) {
			if (i == 0) {
				if (!si->find_child_wme(root, "inputs", w)) {
					continue;
				}
			} else {
				if (!si->find_child_wme(root, "outputs", w)) {
					continue;
				}
			}
			si->get_child_wmes(si->get_wme_val(w), children);
			for (j = children.begin(); j != children.end(); ++j) {
				attr = si->get_wme_attr(*j);
				if (!si->get_val(attr, attrstr)) {
					continue;
				}
				val = si->get_wme_val(*j);
				if (!si->get_val(val, valstr)) {
					continue;
				}
				if (i == 0) {
					inputs[attrstr] = valstr;
				} else {
					outputs[attrstr] = valstr;
				}
			}
		}
		
		if (!svsp->assign_model(name, inputs, outputs)) {
			set_status("failed to assign model");
			return false;
		}
		
		set_status("success");
		return true;
	}

private:
	soar_interface *si;
	Symbol         *root;
	svs            *svsp;
	string          name;
};

command *_make_create_model_command_(svs_state *state, Symbol *root) {
	return new create_model_command(state, root);
}

command *_make_assign_model_command_(svs_state *state, Symbol *root) {
	return new assign_model_command(state, root);
}
