#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sg_node.h"
#include "linalg.h"
#include "common.h"

class scene {
public:
	scene(std::string name, std::string rootname);
	scene(scene &other);
	~scene();
	
	sg_node *get_root();
	sg_node *get_node(std::string name);
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sg_node*> &nodes);
	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(std::string parent, sg_node *n);
	bool add_node(std::string parent, std::string name);
	bool add_node(std::string parent, std::string name, const ptlist &points);
	bool del_node(std::string name);
	bool set_node_trans(std::string name, char type, vec3 trans);
	void clear();

	void get_property_names(std::vector<std::string> &names) const;
	void get_properties(floatvec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool get_node_properties(const std::string &obj, std::map<std::string, float> &props) const;
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const floatvec &vals);
	
	float get_dt() const;
	
	void parse_sgel(const std::string &s);
	
private:
	void disp_update_node(sg_node *n);
	void disp_del_node(sg_node *n);
	void disp_new_scene(std::string name);
	void disp_del_scene(std::string name);

	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
    int  parse_property(std::vector<std::string> &f);
    int  parse_dt(std::vector<std::string> &f);
    bool parse_transforms(std::vector<std::string> &f, int &start);
	
	typedef std::map<std::string, float> property_map;
	
	struct node_info {
		sg_node *node;
		property_map props;
	};
	
	typedef std::map<std::string, node_info> node_map;

	std::string   name;
	std::string   rootname;
	sg_node      *root;
	node_map      nodes;
	bool          iscopy;
	float         dt;          // time passed since last update (as reported by environment)
};

#endif
