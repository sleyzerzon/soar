#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <set>
#include "sg_node.h"
#include "linalg.h"

typedef std::map<std::string, sg_node*> node_map;
typedef std::map<std::string, double> property_map;
typedef std::set<std::pair<std::string, std::string> > scene_sig;

class scene {
public:
	scene(std::string name, std::string rootname);
	scene(scene &other);

	~scene();
	
	sg_node *get_root();
	sg_node *get_node(std::string name);
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sg_node*> &nodes);
	bool add_node(std::string parent, sg_node *n);
	bool add_node(std::string parent, std::string name);
	bool add_node(std::string parent, std::string name, const ptlist &points);
	bool del_node(std::string name);
	bool set_node_trans(std::string name, char type, vec3 trans);
	void clear();

	double get_property(const std::string &prop) const;
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
	
	void get_signature(scene_sig &sig) const;
	void update_scene(scene *scn) const;

	std::vector<double> vals;
	
private:
	int get_trans_offset(const std::string &name, char type) const;
	
	std::map<std::string, std::pair<std::string, int> > node_info; // node name -> (parent name, index)
	std::map<std::string, int> prop_info; // property name -> index
};

#endif
