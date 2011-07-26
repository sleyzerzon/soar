#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sgnode.h"
#include "linalg.h"
#include "common.h"
#include "ipcsocket.h"

class scene : public ipc_listener {
public:
	scene(std::string name, std::string rootname);
	scene(const scene &other);
	~scene();
	
	sgnode *get_root();
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sgnode*> &nodes);
	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(std::string parent, sgnode *n);
	bool add_node(std::string parent, std::string name);
	bool add_node(std::string parent, std::string name, const ptlist &points);
	bool del_node(std::string name);
	bool set_node_trans(std::string name, char type, vec3 trans);
	void clear();

	void get_property_names(std::vector<std::string> &names) const;
	void get_properties(floatvec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool add_property(const std::string &obj, const std::string &prop, float val);
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const floatvec &vals);
	bool remove_property(const std::string &obj, const std::string &prop);
	
	float get_dt() const;
	
	void parse_sgel(const std::string &s);
	
	void ipc_connect(ipcsocket *sock);
	void ipc_disconnect(ipcsocket *sock);
	
private:
	void disp_update_node(sgnode *n);
	void disp_del_node(sgnode *n);
	void disp_new_scene();
	void disp_del_scene();

	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
    int  parse_property(std::vector<std::string> &f);
    int  parse_dt(std::vector<std::string> &f);
    bool parse_transforms(std::vector<std::string> &f, int &start);
	
	typedef std::map<std::string, float> property_map;
	
	struct node_info {
		sgnode *node;
		property_map props;
	};
	
	typedef std::map<std::string, node_info> node_map;

	std::string  name;
	std::string  rootname;
	sgnode      *root;
	node_map     nodes;
	bool         iscopy;
	float        dt;          // time passed since last update (as reported by environment)
};

#endif
