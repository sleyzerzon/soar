#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sgnode.h"
#include "linalg.h"
#include "common.h"
#include "drawer.h"

class scene : public sgnode_listener {
public:
	scene(std::string name, std::string rootname, bool display);
	~scene();
	
	scene *copy() const;
	
	sgnode *get_root();
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sgnode*> &nodes);
	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
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
	void dump_sgel(std::ostream &os);
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	
	void draw_all(const std::string &prefix, float r, float g, float b);
	void undraw_all(const std::string &prefix);
	
private:
	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
    int  parse_property(std::vector<std::string> &f);
    int  parse_dt(std::vector<std::string> &f);
    bool parse_transforms(std::vector<std::string> &f, int &start, vec3 &pos, vec3 &rot, vec3 &scale);
    
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
	bool         display;
	drawer       draw;
	float        dt;          // time passed since last update (as reported by environment)
};

#endif
