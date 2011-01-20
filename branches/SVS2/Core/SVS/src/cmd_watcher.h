#ifndef CMD_WATCHER_H
#define CMD_WATCHER_H

#include "filter.h"
#include "soar_interface.h"

class soar_interface;
class svs_state;
class ipcsocket;

class cmd_utils {
public:
	cmd_utils(svs_state *state, sym_hnd cmd_root);
	
	void    set_result(const std::string &r);
	bool    cmd_changed();
	bool    get_str_param(const std::string &name, std::string &val);
	filter* make_filter();

private:
	filter* rec_make_filter(wme_hnd cmd);
	filter* make_node_filter(std::string name);
	bool    get_filter_params(sym_hnd id, filter_params &p);

	svs_state       *state;
	sym_hnd         cmd_root;
	wme_hnd         result_wme;
	soar_interface* si;
	int             subtree_size;
	int             max_time_tag;
};

class cmd_watcher {
public:
	virtual bool update_result() = 0;
	virtual bool early() = 0;
	virtual ~cmd_watcher() {};
};

cmd_watcher* make_cmd_watcher(svs_state *state, wme_hnd w);

class extract_cmd_watcher : public cmd_watcher, public filter_listener {
public:
	extract_cmd_watcher(svs_state *state, sym_hnd cmd_root);
	~extract_cmd_watcher();
	
	void update(filter *f);
	bool update_result();
	bool early() { return false; }
	
private:
	svs_state *state;
	cmd_utils utils;
	filter*   result_filter;
	bool      dirty;
};

class gen_cmd_watcher : public cmd_watcher, public filter_listener, public sg_listener {
public:
	gen_cmd_watcher(svs_state *state, sym_hnd cmd_root);
	~gen_cmd_watcher();
	
	void update(sg_node* n, sg_node::change_type t);
	void update(filter *f);
	bool update_result();
	bool early() { return false; }
	
private:
	bool get_parent();
	
	svs_state *state;
	cmd_utils utils;
	sym_hnd   cmd_root;
	sg_node   *parent;
	sg_node   *generated;
	filter    *result_filter;
	bool      dirty;
};

class recall_cmd_watcher : public cmd_watcher {
public:
	recall_cmd_watcher(svs_state *state, sym_hnd cmd_root);
	bool update_result();
	bool early() { return true; }

private:
	svs_state *state;
	cmd_utils utils;
};

class ctrl_cmd_watcher : public cmd_watcher {
public:
	ctrl_cmd_watcher(svs_state *state, sym_hnd cmd_root);
	
	bool update_result();
	
	bool early() { return true; }
	
private:
	std::string parse(std::string &msg);
	void        update_step();
	
	svs_state      *state;
	ipcsocket      *ipc;
	soar_interface *si;
	sym_hnd        cmd_root;
	wme_hnd        stepwme;
	cmd_utils      utils;
	int            id;
	int            step;
	bool           broken;
};

class model_cmd_watcher : public cmd_watcher {
public:
	model_cmd_watcher(svs_state *state, sym_hnd cmd_root);
	
	bool update_result();
	
	bool early() { return true; }
	
private:
	svs_state      *state;
	sym_hnd        cmd_root;
	ipcsocket      *ipc;
	soar_interface *si;
	cmd_utils      utils;
};

#endif
