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

class flat_scene {
public:
	flat_scene();
	flat_scene(const flat_scene &s);
	flat_scene(scene *scn);
	
	bool get_node_trans(const std::string &name, vec3 &p, vec3 &r, vec3 &s) const;
	bool get_node_trans(const std::string &name, char type, vec3 &t) const;
	bool set_node_trans(const std::string &name, char type, const vec3 &t);
	bool get_property(const std::string &name, const std::string &prop, float &val) const;
	bool set_property(const std::string &name, const std::string &prop, float val);
	int get_dof() const;
	void get_nodes(std::vector<std::string> &nodes) const;
	
	void get_column_names(std::vector<std::string> &names) const;
	
	void update_scene(scene *scn) const;
	
	bool congruent(const flat_scene &s) const;
	float distance(const flat_scene &s) const;
	floatvec get_node_vals(const std::string &name) const;
	
	int get_index(const std::string &name, const std::string &property) const;
	
	floatvec vals;
	
private:
	int get_trans_offset(const std::string &name, char type) const;
	
	struct node_info {
		std::string parent;
		int begin;
		int length;
		
		bool operator==(const node_info &n) const {
			return parent == n.parent && begin == n.begin && length == n.length;
		}
	};
	
	typedef std::map<std::string, node_info> node_table;
	typedef std::map<std::pair<std::string, std::string>, int> prop_table;
	
	node_table nodes;
	prop_table properties;
};

#endif
