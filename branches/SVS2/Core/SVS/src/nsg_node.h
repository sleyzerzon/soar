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
	  chdirty(false),
	  isgroup(true),
	  pos(CGAL::NULL_VECTOR),
	  rot(CGAL::NULL_VECTOR),
	  scale(CGAL::NULL_VECTOR),
	  ltransform(CGAL::IDENTITY),
	  wtransform(CGAL::IDENTITY),
	  lhull(),
	  whull()
	{ }

	template<class InputIter>
	nsg_node(std::string nm, InputIter begin, InputIter end)
	: name(nm),
	  parent(NULL),
	  tdirty(false),
	  chdirty(false),
	  isgroup(false),
	  pos(CGAL::NULL_VECTOR),
	  rot(CGAL::NULL_VECTOR),
	  scale(CGAL::NULL_VECTOR),
	  ltransform(CGAL::IDENTITY),
	  wtransform(CGAL::IDENTITY)
	{
		CGAL::convex_hull_3(begin, end, lhull);
		whull = lhull;
	}
	
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
	
	ConvexPoly3 get_convex_hull();

private:
	void       set_transform_dirty();
	void       update_transform();
	void       set_convex_hull_dirty();
	void       update_convex_hull();
	
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
	bool                   chdirty;      // convex hull dirty
	
	ConvexPoly3            whull;
	ConvexPoly3            lhull;
	//BBox3*               bbox
	
};

#endif
