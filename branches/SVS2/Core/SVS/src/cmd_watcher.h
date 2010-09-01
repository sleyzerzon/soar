#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include "filter.h"
#include "soar_int.h"

class cmd_utils {
public:
	cmd_utils(soar_interface *si, sym_hnd cmd_root, scene *scn);
	
	void    set_result(const std::string &r);
	bool    cmd_changed();
	bool    get_str_param(const std::string &name, std::string &val);
	filter* make_filter();

private:
	filter* rec_make_filter(sym_hnd cmd);
	filter* make_node_filter(sym_hnd s);
	bool    get_filter_params(sym_hnd id, filter_params &p);

	sym_hnd         cmd_root;
	scene*          scn;
	wme_hnd         result_wme;
	soar_interface* si;
	int             subtree_size;
	int             max_time_tag;
};

class cmd_watcher {
public:
	virtual bool update_result() = 0;
	virtual ~cmd_watcher() {};
};

class extract_cmd_watcher : public cmd_watcher, public filter_listener {
public:
	extract_cmd_watcher(soar_interface *si, sym_hnd cmd_root, scene *scn);
	~extract_cmd_watcher();
	
	void update(filter *f);
	bool update_result();
	
private:
	cmd_utils utils;
	filter*   result_filter;
	bool      dirty;
};

class gen_cmd_watcher : public cmd_watcher, public filter_listener, public sg_listener {
public:
	gen_cmd_watcher(soar_interface *si, sym_hnd cmd_root, scene *scn);
	~gen_cmd_watcher();
	
	void update(sg_node* n, sg_node::change_type t);
	void update(filter *f);
	bool update_result();
	
private:
	bool get_parent();
	
	cmd_utils    utils;
	sym_hnd      cmd_root;
	scene*       scn;
	sg_node*     parent;
	sg_node*     generated;
	node_filter* result_filter;
	bool         dirty;
};

cmd_watcher* make_cmd_watcher(soar_interface *si, scene *scn, wme_hnd w);

#endif
