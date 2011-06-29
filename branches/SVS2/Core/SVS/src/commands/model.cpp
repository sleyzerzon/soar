#include <string>
#include "svs.h"
#include "command.h"
#include "model.h"

using namespace std;

class model_command : public command {
public:
	model_command(svs_state *state, Symbol *root) 
	: root(root), utils(state, root), svsp(state->get_svs()), 
	  si(state->get_svs()->get_soar_interface()), m(NULL)
	{}
	
	~model_command() {
		if (m) {
			svsp->unregister_model(name);
			delete m;
		}
	}
	
	string description() {
		return string("model");
	}
	
	bool update_result() {
		if (m != NULL) {
			return true;
		}
		
		m = parse_model_struct(si, root, name);
		if (m == NULL) {
			utils.set_result("invalid syntax");
			return false;
		}
		svsp->register_model(name, m);
		utils.set_result("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	soar_interface *si;
	cmd_utils       utils;
	Symbol         *root;
	svs            *svsp;
	string          name;
	model          *m;
};

command *_make_model_command_(svs_state *state, Symbol *root) {
	return new model_command(state, root);
}
