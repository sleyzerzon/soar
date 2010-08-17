#ifndef SG_NODE_H
#define SG_NODE_H

/* Implementation neutral scene graph interface */

#include <string>
#include "cgalsupport.h"

class sg_node {
public:
	virtual void        set_name(std::string nm) = 0;
	virtual std::string get_name() = 0;

	virtual sg_node*    get_parent() = 0;
	virtual int         get_nchilds() = 0;
	virtual sg_node*    get_child(int i) = 0;
	virtual bool        attach_child(sg_node *c) = 0;
	virtual bool        detach_child(sg_node *c) = 0;

	virtual void        set_pos(Vector3 xyz) = 0;
	virtual Vector3     get_pos() = 0;
	virtual void        set_rot(Vector3 ypr) = 0;   // yaw-pitch-roll
	virtual Vector3     get_rot() = 0;
	virtual void        set_scale(Vector3 xyz) = 0;
	virtual Vector3     get_scale() = 0;
	virtual void        clear_transforms() = 0;
	
	virtual void        get_local_points(std::list<Point3> &result) = 0;
	virtual void        get_world_points(std::list<Point3> &result) = 0;
	//virtual BBox3      get_bbox() = 0;
};

#endif
