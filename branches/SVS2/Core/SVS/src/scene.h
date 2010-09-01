#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sg_node.h"
#include "nsg_node.h"

typedef std::map<std::string, sg_node*> node_map;

class scene : public sg_listener {
public:
	scene(std::string rootname);
	scene(scene *c);
	~scene();
	
	sg_node* get_root();
	sg_node* get_node(std::string name);
	
	void     update(sg_node *n, sg_node::change_type t);
	
private:
	void update_names(sg_node *n);
	
	sg_node* root;
	node_map nodes;
};

#endif
