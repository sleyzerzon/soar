/*
 Collision detection using Bullet physics. Currently I'm ignoring the
 broadphase pruning, so this is not as efficient as it can be.
*/
#include <iostream>
#include <fstream>
#include <map>
#include <btBulletCollisionCommon.h>
#include "model.h"
#include "linalg.h"
#include "filter.h"
#include "common.h"

using namespace std;

ostream &operator<<(ostream &os, const btVector3 &v) {
	os << v.x() << " " << v.y() << " " << v.z();
	return os;
}

class debug_drawer : public btIDebugDraw {
public:
	debug_drawer() : fifo("/tmp/dispfifo"), counter(0) { reset(); }
	
	void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
		fifo << "a " << counter++ << " c " << color << " " << from << " " << to << endl;
		fifo.flush();
	}
	
	void drawContactPoint(const btVector3 &pt, const btVector3 &norm, btScalar dist, int time, const btVector3 &color) {
		fifo << "a " << counter++ << " c " << color << " " << pt << endl;
		fifo.flush();
	}
	
	void reportErrorWarning(const char *msg) {
		fifo << "l " << msg << endl;
		fifo.flush();
	}
	
	void draw3dText(const btVector3 &p, const char *text) {
		fifo << "t " << counter++ << " " << p << " " << text << endl;
		fifo.flush();
	}
	/*
	void drawAabb(const btVector3 &min, const btVector3 &max, const btVector3 &color) {
		drawBox(min, max, color);
	}
	*/
	
	void outputBoxVerts(const btVector3 &min, const btVector3 &max) {
		float pt[3];
		for (int i = 0; i < 8; i++) {
			pt[0] = min.x() + ((max.x() - min.x()) * (i & 1));
			pt[1] = min.y() + ((max.y() - min.y()) * ((i >> 1) & 1));
			pt[2] = min.z() + ((max.z() - min.z()) * ((i >> 2) & 1));
			fifo << " ";
			copy(pt, pt + 3, ostream_iterator<float>(fifo, " "));
		}
	}
	
	/*
	void drawBox(const btVector3 &min, const btVector3 &max, const btVector3 &color) {
		fifo << "a " << counter++ << " c " << color;
		outputBoxVerts(min, max);
		fifo << endl;
		fifo.flush();
	}
	
	void drawBox(const btVector3 &min, const btVector3 &max, const btTransform &t, const btVector3 &color) {
		btVector3 o = t.getOrigin();
		btQuaternion q = t.getRotation();
		quaternion q2(q.w(), q.x(), q.y(), q.z());
		vec3 pos(o.x(), o.y(), o.z()), rot = q2.to_rpy();

		fifo << "a " << counter++ << " p " << pos << " r " << rot << " c " << color;
		outputBoxVerts(min, max);
		fifo << endl;
		fifo.flush();
	}
	*/
	
	void setDebugMode(int mode) {
	}
	
	int getDebugMode() const {
		return DBG_DrawWireframe | DBG_DrawContactPoints | DBG_DrawText | DBG_DrawFeaturesText; // | DBG_DrawAabb | DBG_DrawConstraints | DBG_DrawConstraintLimits;
	}
	
	void reset() {
		fifo << "r" << endl;
		fifo.flush();
		counter = 0;
	}
	
private:
	ofstream fifo;
	int counter;
};

struct node_info {
	sgnode *node;
	ptlist vertices;
	btCollisionObject *object;
	btCollisionShape *shape;
};

struct result_info {
	bool oldval;
	bool newval;
	filter_val *fval;
};

typedef map<filter_val*, node_info> input_table_t;
typedef pair<filter_val*, filter_val*> fval_pair;
typedef map<fval_pair, result_info> result_table_t;

btVector3 toBtVec(const vec3 &v) {
	return btVector3(v.a[0], v.a[1], v.a[2]);
}

void update_transforms(node_info &info) {
	vec3 rpy = info.node->get_trans('r');
	btQuaternion q;
	q.setEuler(rpy[0], rpy[1], rpy[2]);
	info.object->getWorldTransform().setOrigin(toBtVec(info.node->get_trans('p')));
	info.object->getWorldTransform().setRotation(q);
	info.shape->setLocalScaling(toBtVec(info.node->get_trans('s')));
}

btCollisionShape *ptlist_to_btshape(const ptlist &pts) {
	vector<btVector3> pts1;
	ptlist::const_iterator i;
	int j;
	
	/*
	for (i = pts.begin(); i != pts.end(); ++i) {
		pts1.push_back(btVector3(i->a[0], i->a[1], i->a[2]));
	}
	btConvexHullShape *s = new btConvexHullShape(reinterpret_cast<const btScalar*>(&pts[0]), pts.size());
	btConvexHullShape *s = new btConvexHullShape(reinterpret_cast<const btScalar*>(&pts1[0]), pts1.size());
	*/
	
	btConvexHullShape *s = new btConvexHullShape();
	
	for (i = pts.begin(); i != pts.end(); ++i) {
		s->addPoint(toBtVec(*i));
	}
	
	/*
	vec3 halfextents = pts[0];
	for (i = pts.begin(); i != pts.end(); ++i) {
		for (j = 0; j < 3; ++j) {
			halfextents[j] = halfextents[j] < i->a[j] ? i->a[j] : halfextents[j];
		}
	}
	btCollisionShape *s = new btBoxShape(toBtVec(halfextents));
	*/
	
	s->setMargin(0.0001);
	return s;
}

class intersect_filter : public filter {
public:
	intersect_filter(filter_input *input) : filter(input), callback(this) {
		btVector3 worldAabbMin(-1000,-1000,-1000);
		btVector3 worldAabbMax(1000,1000,1000);
		
		config = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(config);
		broadphase = new btSimpleBroadphase();
		//broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax);
		cworld = new btCollisionWorld(dispatcher, broadphase, config);
		cworld->setDebugDrawer(&drawer);
	}
	
	~intersect_filter() {
		delete dispatcher;
		delete broadphase;
		delete cworld;
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
			btCollisionObject *o1 = input_table[j->first.first].object;
			btCollisionObject *o2 = input_table[j->first.second].object;
			j->second.oldval = j->second.newval;
			j->second.newval = false;
			cworld->contactPairTest(o1, o2, callback);
		}
		
		/*
		cworld->performDiscreteCollisionDetection();
		int num_manifolds = dispatcher->getNumManifolds();
		cout << "NUM MANIFOLDS: " << num_manifolds << endl;
		for (int k = 0; k < num_manifolds; ++k) {
			btPersistentManifold *m = dispatcher->getManifoldByIndexInternal(k);
			int numcontacts = m->getNumContacts();
			cout << "NUM CONTACTS: " << numcontacts << endl;
			if (numcontacts == 0) {
				continue;
			}
			btCollisionObject *a = static_cast<btCollisionObject*>(m->getBody0());
			btCollisionObject *b = static_cast<btCollisionObject*>(m->getBody1());
			filter_val *af = static_cast<filter_val*>(a->getUserPointer());
			filter_val *bf = static_cast<filter_val*>(b->getUserPointer());
			add_collision(af, bf);
		}
		*/
		
		for (j = results.begin(); j != results.end(); ++j) {
			result_info &r = j->second;
			if (r.oldval != r.newval) {
				set_filter_val(r.fval, r.newval);
				change_result(r.fval);
			}
		}
		drawer.reset();
		cworld->debugDrawWorld();
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

	class collision_callback : public btCollisionWorld::ContactResultCallback {
	public:
		collision_callback(intersect_filter *f) : f(f) {}
		
		virtual	btScalar addSingleResult(btManifoldPoint& cp, 
		                                 const btCollisionObject* obj1,
		                                 int partId1,
		                                 int index1,
		                                 const btCollisionObject* obj2,
		                                 int partId2,
		                                 int index2)
		{
			filter_val *v1, *v2;
			v1 = static_cast<filter_val*>(obj1->getUserPointer());
			v2 = static_cast<filter_val*>(obj2->getUserPointer());
			f->add_collision(v1, v2);
			return 0.f;
		}
		
	private:
		intersect_filter *f;
	};
	
	void add_object(filter_val *v, node_info &info) {
		info.node->get_local_points(info.vertices);
		info.shape = ptlist_to_btshape(info.vertices);
		info.object = new btCollisionObject();
		info.object->setUserPointer(static_cast<void*>(v));
		info.object->setCollisionShape(info.shape);
		update_transforms(info);
		cworld->addCollisionObject(info.object);
	}
	
	void remove_object(node_info &info) {
		cworld->removeCollisionObject(info.object);
		delete info.shape;
		delete info.object;
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
		return true;
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
				cout << "SETTING NEW VERTICES FOR " << info.node->get_name() << endl;
				copy(newverts.begin(), newverts.end(), ostream_iterator<vec3>(cout, " / "));
				cout << endl;
				
				assert(newverts.size() == info.vertices.size());
				info.vertices = newverts;
				delete info.shape;
				info.shape = ptlist_to_btshape(info.vertices);
				info.object->setCollisionShape(info.shape);
			}
			update_transforms(info);
		}
		return true;
	}
	
	btCollisionConfiguration *config;
	btCollisionDispatcher    *dispatcher;
	btBroadphaseInterface    *broadphase;
	btCollisionWorld         *cworld;
	debug_drawer             drawer;
	collision_callback       callback;
	
	input_table_t      input_table;
	result_table_t     results;
};

filter *_make_intersect_filter_(scene *scn, filter_input *input) {
	return new intersect_filter(input);
}
