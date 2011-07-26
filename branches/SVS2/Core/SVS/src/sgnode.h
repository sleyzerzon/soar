#ifndef SGNODE_H
#define SGNODE_H

/* native scene graph implementation */

#include "linalg.h"
#include <vector>
#include <set>

class sgnode_listener;

class sgnode {
public:
	enum change_type {
		CHILD_ADDED,
		DELETED,        // sent from destructor
		POINTS_CHANGED
	};
	
	sgnode(std::string name);
	sgnode(std::string name, const ptlist &points);
	~sgnode();
	
	sgnode*     copy() const;

	std::string get_name() const;
	void        set_name(std::string nm);
	bool        is_group() const;

	sgnode*     get_parent();
	int         num_children() const;
	sgnode*     get_child(int i);
	void        walk(std::list<sg_node*> &result);
	bool        attach_child(sg_node *c);
	
	void        set_trans(char type, vec3 trans);
	vec3        get_trans(char type) const;
	
	void        get_local_points(ptlist &result);
	void        get_world_points(ptlist &result);
	
	void        listen(sg_listener *o);
	void        unlisten(sg_listener *o);
	
private:
	void detach_child(sgnode *c);
	void set_transform_dirty();
	void update_transform();
	void set_points_dirty();
	void update_points();
	void send_update(sg_node::change_type t, int added=-1);
	
	std::string          name;
	sgnode*              parent;
	ptlist               points;
	std::vector<sgnode*> children;
	bool                 isgroup;
	vec3                 pos;
	vec3                 rot;
	vec3                 scale;
	transform3           wtransform;
	transform3           ltransform;
	
	bool                 tdirty;       // transforms dirty
	bool                 pdirty;       // convex hull dirty
	
	std::set<sgnode_listener*> listeners;
};

class sgnode_listener {
public:
	virtual void node_update(sgnode *n, sgnode::change_type t, int added_child) = 0;
};

#endif
