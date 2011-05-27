#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sg_node.h"
#include "linalg.h"
#include "common.h"

typedef std::map<std::string, sg_node*> node_map;
typedef std::map<std::string, double> property_map;

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
	
	bool add_node(std::string parent, sg_node *n);
	bool add_node(std::string parent, std::string name);
	bool add_node(std::string parent, std::string name, const ptlist &points);
	bool del_node(std::string name);
	bool set_node_trans(std::string name, char type, vec3 trans);
	void clear();

	double get_property(const std::string &prop) const;
	int num_properties() const;
	// properties will be in alphabetical name order
	void get_all_properties(std::vector<std::pair<std::string, double> > &props) const;
	void set_property(const std::string &prop, double val);
	
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
    bool parse_transforms(std::vector<std::string> &f, int &start);

	std::string   name;
	std::string   rootname;
	sg_node      *root;
	node_map      nodes;
	property_map  properties;
	bool          iscopy;
};

class flat_scene {
public:
	flat_scene();
	flat_scene(const flat_scene &s);
	flat_scene(scene *scn);
	
	bool get_node_trans(const std::string &name, char type, vec3 &t) const;
	bool set_node_trans(const std::string &name, char type, const vec3 &t);
	bool get_property(const std::string &prop, double &val) const;
	bool set_property(const std::string &prop, double val);
	int dof() const;
	
	void get_column_names(std::vector<std::string> &names) const;
	
	void update_scene(scene *scn) const;
	
	bool congruent(const flat_scene &s) const;
	double distance(const flat_scene &s) const;
	
	floatvec vals;
	
private:
	int get_trans_offset(const std::string &name, char type) const;
	
	std::map<std::string, std::pair<std::string, int> > node_info; // node name -> (parent name, index)
	std::map<std::string, int> prop_info; // property name -> index
};

#endif
