#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sg_node.h"
#include "nsg_node.h"
#include "ipcsocket.h"

typedef std::map<std::string, sg_node*> node_map;

class scene : public sg_listener {
public:
	scene(std::string name, std::string rootname, ipcsocket *disp);
	~scene();
	
	sg_node* get_root();
	sg_node* get_node(std::string name);
	
	void     update(sg_node *n, sg_node::change_type t);
	void     wipe();
	
private:
	void handle_add(sg_node *n);
	void handle_del(sg_node *n);
	void handle_ptschange(sg_node *n);
	void update_object(sg_node *n);

	std::string name;
	std::string rootname;
	sg_node     *root;
	node_map    nodes;
	ipcsocket   *disp;
};

#endif
