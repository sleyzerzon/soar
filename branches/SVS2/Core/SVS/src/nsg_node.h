#ifndef NSG_NODE_H
#define NSG_NODE_H

/* native scene graph implementation */

#include "sg_node.h"
#include "linalg.h"
#include <vector>

class nsg_node : public sg_node {
public:
	nsg_node(std::string nm) 
	: name(nm), parent(NULL), tdirty(false), pdirty(false), isgroup(true),
	pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
	{}

	template<class InputIter>
	nsg_node(std::string nm, InputIter begin, InputIter end)
	: name(nm), parent(NULL), tdirty(false), pdirty(false), isgroup(false),
	pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
	{
		copy(begin, end, back_inserter(pts));
	}
	
	std::string get_name();
	void        set_name(std::string nm);
	bool        is_group();

	sg_node*    get_parent();
	int         get_nchilds();
	sg_node*    get_child(int i);
	bool        attach_child(sg_node *c);
	void        detach();

	void        set_pos(vec3 xyz);
	vec3        get_pos();
	void        set_rot(vec3 ypr);
	vec3        get_rot();
	void        set_scale(vec3 xyz);
	vec3        get_scale();
	
	void        get_local_points(std::list<vec3> &result);
	void        get_world_points(std::list<vec3> &result);
	
	void        observe(sg_observer *o);
	void        unobserve(sg_observer *o);
	
private:
	void detach_child(nsg_node *c);
	void set_transform_dirty();
	void update_transform();
	void set_points_dirty();
	void update_points();
	void send_update(sg_node::change_type t);
	
	std::string             name;
	nsg_node*               parent;
	std::list<vec3>         pts;
	std::vector<nsg_node*>  childs;
	bool                    isgroup;
	vec3                    pos;
	vec3                    rot;
	vec3                    scale;
	transform3              wtransform;
	transform3              ltransform;
	
	bool                    tdirty;       // transforms dirty
	bool                    pdirty;       // convex hull dirty
	
	std::list<sg_observer*> observers;
};

#endif
