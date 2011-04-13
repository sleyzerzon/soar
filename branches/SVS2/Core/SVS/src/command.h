#ifndef CMD_WATCHER_H
#define CMD_WATCHER_H

#include "soar_interface.h"

class svs_state;

class command {
public:
	virtual bool update_result() = 0;
	virtual bool early() = 0;
	virtual ~command() {};
};

class cmd_utils {
public:
	cmd_utils(svs_state *state, Symbol *cmd_root);
	
	/* create or change value of a (C1 ^result <msg>) wme */
	void set_result(const std::string &r);
	
	/* check if any substructure in the command changed */
	bool cmd_changed();
	
	/* get the value of a string wme */
	bool get_str_param(const std::string &name, std::string &val);
	
private:
	svs_state      *state;
	Symbol         *cmd_root;
	soar_interface *si;
	wme            *result_wme;
	int             subtree_size;
	int             max_time_tag;
};

command *make_command(svs_state *state, wme *w);

#endif
