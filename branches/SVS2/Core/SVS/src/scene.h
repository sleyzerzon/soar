#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <list>
#include <map>
#include "linalg.h"
#include "sg_node.h"

class scene {
public:
	scene(std::string rootname);
	
	sg_node* get_node(std::string name);
	sg_node* add_group(std::string name, std::string par);
	sg_node* add_geometry(std::string name, std::string par, ptlist &points);
	bool     del_tree(std::string name);
	
private:
	bool add_node(std::string par, sg_node *n);
	
	std::map<std::string, sg_node*> nodes;
};

#endif
