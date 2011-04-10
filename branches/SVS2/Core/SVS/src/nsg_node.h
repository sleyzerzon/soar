#ifndef NSG_NODE_H
#define NSG_NODE_H

/* native scene graph implementation */

#include "sg_node.h"
#include "linalg.h"
#include <vector>
#include <set>

class nsg_node : public sg_node {
public:
	nsg_node(std::string name);
	nsg_node(std::string name, const ptlist &points);
	~nsg_node();
	
	sg_node*    copy();

	std::string get_name();
	void        set_name(std::string nm);
	bool        is_group();

	sg_node*    get_parent();
	int         num_children();
	sg_node*    get_child(int i);
	void        walk(std::list<sg_node*> &result);
	bool        attach_child(sg_node *c);
	
	void        set_trans(char type, vec3 trans);
	vec3        get_trans(char type);
	
	void        get_local_points(ptlist &result);
	void        get_world_points(ptlist &result);
	
	void        listen(sg_listener *o);
	void        unlisten(sg_listener *o);
	
private:
	void detach_child(nsg_node *c);
	void set_transform_dirty();
	void update_transform();
	void set_points_dirty();
	void update_points();
	void send_update(sg_node::change_type t, int added=-1);
	
	std::string             name;
	nsg_node*               parent;
	ptlist                  points;
	std::vector<nsg_node*>  children;
	bool                    isgroup;
	vec3                    pos;
	vec3                    rot;
	vec3                    scale;
	transform3              wtransform;
	transform3              ltransform;
	
	bool                    tdirty;       // transforms dirty
	bool                    pdirty;       // convex hull dirty
	
	std::set<sg_listener*>  listeners;
};

#endif
