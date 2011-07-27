#include <iostream>
#include <map>
#include <set>
#include <SOLID/SOLID.h>
#include "model.h"
#include "linalg.h"
#include "filter.h"
#include "common.h"

using namespace std;

const float MARGIN = 0.1;

DT_Bool collision_callback(void *client_data, void *obj1, void *obj2, const DT_CollData *coll_data);

struct node_info {
	sgnode *node;
	ptlist vertices;
	DT_ShapeHandle shape;
	DT_ObjectHandle obj;
	DT_VertexBaseHandle vertexbase;
};

struct result_info {
	bool oldval;
	bool newval;
	filter_val *fval;
};

typedef map<filter_val*, node_info> input_table_t;
typedef pair<filter_val*, filter_val*> fval_pair;
typedef map<fval_pair, result_info> result_table_t;

void update_transforms(sgnode *n, DT_ObjectHandle obj) {
	vec3 pos = n->get_trans('p');
	quaternion rot = quaternion(n->get_trans('r'));
	vec3 scale = n->get_trans('s');
	DT_SetPosition(obj, pos.a);
	DT_SetOrientation(obj, rot.a);
	DT_SetScaling(obj, scale.a);
	//cout << "Moving " << n->get_name() << " to " << pos << endl;
}


class intersect_filter : public filter {
public:
	intersect_filter(filter_input *input) : filter(input) {
		scene = DT_CreateScene();
		resp_table = DT_CreateRespTable();
		resp_class = DT_GenResponseClass(resp_table);
		DT_AddDefaultResponse(resp_table, collision_callback, DT_SIMPLE_RESPONSE, this);
	}
	
	bool update_results() {
		filter_input::iter i;
		result_table_t::iterator j;
		filter_val *av, *bv;
		sgnode *an, *bn;
		
		for (i = added_input_begin(); i != added_input_end(); ++i) {
			if (!map_get<string, filter_val*>(**i, "a", av) ||
			    !map_get<string, filter_val*>(**i, "b", bv) )
			{
				set_error("missing parameter(s)");
				return false;
			}
			result_info &rp = results[make_pair(av, bv)];
			rp.oldval = false;
			rp.newval = false;
			rp.fval = new filter_val_c<bool>(false);
			add_result(rp.fval, *i);
			add_node(av);
			add_node(bv);
		}
		for (i = removed_input_begin(); i != removed_input_end(); ++i) {
			if (!map_get<string, filter_val*>(**i, "a", av) ||
			    !map_get<string, filter_val*>(**i, "b", bv))
			{
				set_error("missing parameter(s)");
				return false;
			}
			result_info rp;
			if (!map_pop(results, make_pair(av, bv), rp)) {
				assert(false);
			}
			remove_result(rp.fval);
			del_node(av);
			del_node(bv);
		}
		for (i = changed_input_begin(); i != changed_input_end(); ++i) {
			if (!map_get<string, filter_val*>(**i, "a", av) ||
			    !map_get<string, filter_val*>(**i, "b", bv))
			{
				set_error("missing parameter(s)");
				return false;
			}
			change_node(av);
			change_node(bv);
		}
		
		for (j = results.begin(); j != results.end(); ++j) {
			j->second.newval = false;
		}
		DT_Test(scene, resp_table);
		for (j = results.begin(); j != results.end(); ++j) {
			result_info &r = j->second;
			if (r.oldval != r.newval) {
				set_filter_val(r.fval, r.newval);
				change_result(r.fval);
				r.oldval = r.newval;
			}
		}
		return true;
	}
	
	
	void add_collision(filter_val *a, filter_val *b) {
		fval_pair p1 = make_pair(a, b), p2 = make_pair(b, a);
		result_table_t::iterator i;
		
		/* have to check both orderings */
		if ((i = results.find(p1)) == results.end()) {
			i = results.find(p2);
		}
		
		if (i != results.end()) {
			i->second.newval = true;
		}
	}
	
private:
	void add_object(filter_val *v, node_info &info) {
		info.node->get_local_points(info.vertices);
		info.vertexbase = DT_NewVertexBase(&info.vertices[0], 0);
		info.shape = DT_NewComplexShape(info.vertexbase);
		DT_VertexRange(0, info.vertices.size());
		DT_EndComplexShape();
		
		info.obj = DT_CreateObject((void*) v, info.shape);
		update_transforms(info.node, info.obj);
		DT_SetMargin(info.obj, MARGIN);
		DT_AddObject(scene, info.obj);
		DT_SetResponseClass(resp_table, info.obj, resp_class);
	}
	
	void remove_object(node_info &info) {
		DT_RemoveObject(scene, info.obj);
		DT_DestroyObject(info.obj);
		DT_DeleteShape(info.shape);
	}
	
	bool add_node(filter_val *v) {
		sgnode *n;
		
		if (input_table.find(v) != input_table.end()) {
			return true;
		}
		
		if (!get_filter_val(v, n)) {
			return false;
		}
		
		node_info &info = input_table[v];
		info.node = n;
		add_object(v, info);
	}
	
	void del_node(filter_val *v) {
		input_table_t::iterator i = input_table.find(v);
		assert(i != input_table.end());
		
		node_info &info = i->second;
		remove_object(info);
		input_table.erase(v);
	}
	
	bool change_node(filter_val *v) {
		sgnode *newnode;
		
		if (!get_filter_val(v, newnode)) {
			return false;
		}
		
		input_table_t::iterator i = input_table.find(v);
		assert(i != input_table.end());
		node_info &info = i->second;
		if (info.node != newnode) {
			info.node = newnode;
			remove_object(info);
			add_object(v, info);
		} else {
			ptlist newverts;
			info.node->get_local_points(newverts);
			if (info.vertices != newverts) {
				assert(newverts.size() == info.vertices.size());
				info.vertices = newverts;
				DT_ChangeVertexBase(info.vertexbase, &info.vertices[0]);
			}
			update_transforms(info.node, info.obj);
		}
		return true;
	}
	
	DT_SceneHandle     scene;
	DT_RespTableHandle resp_table;
	DT_ResponseClass   resp_class;
	input_table_t      input_table;
	result_table_t     results;
	set<fval_pair>     new_collisions;
	set<fval_pair>     old_collisions;
};

DT_Bool collision_callback(void *client_data, void *obj1, void *obj2, const DT_CollData *coll_data) {
	filter_val *n1 = static_cast<filter_val*>(obj1);
	filter_val *n2 = static_cast<filter_val*>(obj2);
	static_cast<intersect_filter*>(client_data)->add_collision(n1, n2);
	return DT_CONTINUE;
}

filter *_make_intersect_filter_(scene *scn, filter_input *input) {
	return new intersect_filter(input);
}
