#ifndef SG_NODE_H
#define SG_NODE_H

/* Implementation neutral scene graph interface */

#include <string>
#include "linalg.h"

class sg_listener;

class sg_node {
public:
	enum change_type {
		CHILD_ADDED,    // get_child(get_nchilds()-1) is the added child
		DETACHED,
		DELETED,        // sent from destructor
		POINTS_CHANGED
	};
	
	virtual ~sg_node() {}
	
	virtual sg_node*    copy() = 0;  // deep copy, copy doesn't inherit listeners
	virtual void        set_name(std::string nm) = 0;
	virtual std::string get_name() = 0;
	virtual bool        is_group() = 0;

	virtual sg_node*    get_parent() = 0;
	virtual int         num_children() = 0;
	virtual sg_node*    get_child(int i) = 0;
	virtual bool        attach_child(sg_node *c) = 0;

	virtual void        set_pos(vec3 xyz) = 0;
	virtual vec3        get_pos() = 0;
	virtual void        set_rot(vec3 ypr) = 0;   // yaw-pitch-roll
	virtual vec3        get_rot() = 0;
	virtual void        set_scale(vec3 xyz) = 0;
	virtual vec3        get_scale() = 0;
	
	virtual void        get_local_points(ptlist &result) = 0;
	virtual void        get_world_points(ptlist &result) = 0;
	
	virtual void        listen(sg_listener *o) = 0;
	virtual void        unlisten(sg_listener *o) = 0;
};

class sg_listener {
public:
	virtual void update(sg_node *n, sg_node::change_type t) = 0;
};

#endif
