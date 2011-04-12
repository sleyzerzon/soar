#include "cmd_watcher.h"

class generate_command : public command {
public:
	generate_command(svs_state *state, sym_hnd cmd_root)
	: utils(state, cmd_root)
	{}
	
	bool update_result() {
		return false;
	}
	
	bool early() { return false; }
	
private:
	cmd_utils utils;
	string    parent;
	bool      dirty;
};

