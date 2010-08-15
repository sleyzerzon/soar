#ifndef NSG_NODE_H
#define NSG_NODE_H

/* native scene graph implementation */

#include <vector>
#include "sg_node.h"

class nsg_node : public sg_node {
public:
	nsg_node(std::string nm);
	
	std::string get_name();
	void        set_name(std::string nm);

	sg_node*    get_parent();
	int         get_nchilds();
	sg_node*    get_child(int i);
	bool        add_child(sg_node *c);
	bool        del_child(sg_node *c);

	void        set_pos(Vector3 &xyz);
	Vector3     get_pos();
	void        set_rot(Vector3 &ypr);
	Vector3     get_rot();
	void        set_scale(Vector3 &xyz);
	Vector3     get_scale();
	void        clear_transforms();
	
	Poly3*      get_convex_hull();
	//BBox3*      get_bbox();

protected:
	void       set_transform_dirty();
	void       set_convex_hull_dirty();
	Transform3 get_world_transform();
	
private:
	std::string            name;
	nsg_node*              parent;
	std::vector<nsg_node*> childs;
	bool                   isgroup;
	Vector3                pos;
	Vector3                rot;
	Vector3                scale;
	Transform3             wtransform;
	Transform3             lpos;
	Transform3             lrot;
	Transform3             lscale;
	
	bool                   tdirty;       // transforms dirty
	bool                   chdirty;      // convex hull dirty
	
	ConvexPoly3*           convex_hull;
	//BBox3*               bbox
	
};

#endif
