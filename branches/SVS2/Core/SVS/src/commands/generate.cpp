#include <string>
#include "command.h"
#include "scene.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class generate_command : public command {
public:
	generate_command(svs_state *state, Symbol *root)
	: utils(state, root), root(root), scn(state->get_scene()), node_filter(NULL), gen_node(NULL), si(state->get_svs()->get_soar_interface())
	{}
	
	~generate_command() {
		reset();
	}
	
	bool update_result() {
		wme *parent_wme, *gen_wme;
		sg_node *gen_node;
		
		if (!utils.cmd_changed()) {
			return true;
		}
		
		reset();
		if (!si->find_child_wme(root, "parent", parent_wme) ||
		    !si->find_child_wme(root, "node", gen_wme))
		{
			utils.set_result("missing parameters");
			return false;
		}
		if (!si->get_val(si->get_wme_val(parent_wme), parent)) {
			utils.set_result("parent name must be a string");
			return false;
		}
		if ((node_filter = parse_filter_struct(si, si->get_wme_val(gen_wme), scn)) == NULL) {
			utils.set_result("incorrect gen filter syntax");
			return false;
		}
		if (!get_node_filter_result_value(NULL, node_filter, gen_node)) {
			utils.set_result("node parameter must be a node filter");
			return false;
		}
		if (!scn->add_node(parent, gen_node)) {
			utils.set_result("error adding node to scene");
			return false;
		}
		return true;
	}
	
	void reset() {
		if (node_filter) {
			delete node_filter;
		}
		if (gen_node) {
			scn->del_node(gen_node->get_name());
			delete gen_node;
		}
	}
	
	bool early() { return false; }
	
private:
	scene          *scn;
	Symbol         *root;
	soar_interface *si;
	cmd_utils       utils;
	string          parent;
	filter         *node_filter;
	sg_node        *gen_node;
	bool            dirty;
};

command *_make_generate_command_(svs_state *state, Symbol *root) {
	return new generate_command(state, root);
}
