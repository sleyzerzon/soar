#ifndef SG_NODE_H
#define SG_NODE_H

/* Implementation neutral scene graph interface */

#include <string>
#include <list>
#include "linalg.h"

class sg_observer;

class sg_node {
public:
	enum change_type {
		ADDCHILD,  // get_child(get_nchilds()-1) is the added child
		DETACH, 
		PTSCHANGE
	};
	
	virtual void        set_name(std::string nm) = 0;
	virtual std::string get_name() = 0;
	virtual bool        is_group() = 0;

	virtual sg_node*    get_parent() = 0;
	virtual int         get_nchilds() = 0;
	virtual sg_node*    get_child(int i) = 0;
	virtual bool        attach_child(sg_node *c) = 0;
	virtual void        detach() = 0;

	virtual void        set_pos(vec3 xyz) = 0;
	virtual vec3        get_pos() = 0;
	virtual void        set_rot(vec3 ypr) = 0;   // yaw-pitch-roll
	virtual vec3        get_rot() = 0;
	virtual void        set_scale(vec3 xyz) = 0;
	virtual vec3        get_scale() = 0;
	
	virtual void        get_local_points(std::list<vec3> &result) = 0;
	virtual void        get_world_points(std::list<vec3> &result) = 0;
	
	virtual void        observe(sg_observer *o) = 0;
	virtual void        unobserve(sg_observer *o) = 0;
};

class sg_observer {
public:
	virtual void update(sg_node *n, sg_node::change_type t) = 0;
};

#endif
