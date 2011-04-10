#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sg_node.h"
#include "linalg.h"

typedef std::map<std::string, sg_node*> node_map;

class scene {
public:
	scene(std::string name, std::string rootname);
	scene(scene &other);

	~scene();
	
	sg_node* get_root();
	sg_node* get_node(std::string name);
	bool     add_node(std::string parent, std::string name);
	bool     add_node(std::string parent, std::string name, const ptlist &points);
	bool     del_node(std::string name);
	bool     set_node_trans(std::string name, char type, vec3 trans);
	void     clear();

	void     parse_sgel(std::string s);
	
private:
	bool add_node(std::string parent, sg_node *n);
	void disp_update_node(sg_node *n);
	void disp_del_node(sg_node *n);

	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
    bool parse_transforms(std::vector<std::string> &f, int &start);

	std::string name;
	std::string rootname;
	sg_node     *root;
	node_map    nodes;
};

#endif
