#ifndef NSG_NODE_H
#define NSG_NODE_H

/* native scene graph implementation */

#include <vector>
#include "sg_node.h"

class nsg_node : public sg_node {
public:
	nsg_node(std::string nm) 
	: name(nm),
	  parent(NULL),
	  tdirty(false),
	  pdirty(false),
	  isgroup(true),
	  pos(CGAL::NULL_VECTOR),
	  rot(CGAL::NULL_VECTOR),
	  scale(1.0, 1.0, 1.0),
	  ltransform(CGAL::IDENTITY),
	  wtransform(CGAL::IDENTITY)
	{ }

	template<class InputIter>
	nsg_node(std::string nm, InputIter begin, InputIter end)
	: name(nm),
	  parent(NULL),
	  tdirty(false),
	  pdirty(false),
	  isgroup(false),
	  pos(CGAL::NULL_VECTOR),
	  rot(CGAL::NULL_VECTOR),
	  scale(1.0, 1.0, 1.0),
	  ltransform(CGAL::IDENTITY),
	  wtransform(CGAL::IDENTITY)
	{
		copy(begin, end, back_inserter(pts));
	}
	
	std::string get_name();
	void        set_name(std::string nm);

	sg_node*    get_parent();
	int         get_nchilds();
	sg_node*    get_child(int i);
	bool        attach_child(sg_node *c);
	bool        detach_child(sg_node *c);

	void        set_pos(Vector3 xyz);
	Vector3     get_pos();
	void        set_rot(Vector3 ypr);
	Vector3     get_rot();
	void        set_scale(Vector3 xyz);
	Vector3     get_scale();
	void        clear_transforms();
	
	void        get_local_points(std::list<Point3> &result);
	void        get_world_points(std::list<Point3> &result);

private:
	void       set_transform_dirty();
	void       update_transform();
	void       set_points_dirty();
	void       update_points();
	
	std::string            name;
	nsg_node*              parent;
	std::vector<nsg_node*> childs;
	bool                   isgroup;
	Vector3                pos;
	Vector3                rot;
	Vector3                scale;
	Transform3             wtransform;
	Transform3             ltransform;
	
	bool                   tdirty;       // transforms dirty
	bool                   pdirty;       // convex hull dirty
	
	std::list<Point3>      pts;
};

#endif
