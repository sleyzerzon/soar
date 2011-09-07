#include <math.h>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <list>
#include "command.h"
#include "svs.h"
#include "scene.h"
#include "model.h"
#include "common.h"
#include "bullet_support.h"

using namespace std;

// constants for simplex search
const double RCOEF = 1.0;
const double ECOEF = 2.0;
const double CCOEF = 0.5;
const double SCOEF = 0.5;
const int MAXITERS = 50;
const double INF = numeric_limits<double>::infinity();

void draw_prediction(scene *scn, multi_model *mdl, const floatvec &traj, int stepsize) {
	scene *copy = scn->copy();
	floatvec x, y, u;
	copy->get_properties(y);
	x.resize(y.size() + stepsize);
	for (int offset = 0; offset < traj.size(); offset += stepsize) {
		u = traj.slice(offset, offset + stepsize);
		x.combine(y, u);
		if (!mdl->predict(x, y)) {
			cout << "MODEL ERROR" << endl;
			return;
		}
	}
	copy->set_properties(y);
	copy->draw_all("predict_", 1., 0., 0.);
}

class objective {
public:
	virtual float eval(scene &scn) const = 0;
};

/* Squared Euclidean distance between centroids of two objects */
class euclidean_obj : public objective {
public:
	euclidean_obj(const string &obj1, const string &obj2)
	: obj1(obj1), obj2(obj2) {}
	
	float eval(scene &scn) const {
		sgnode *n1, *n2;
		ptlist p1, p2;
		::vec3 c1, c2;
		
		if (!(n1 = scn.get_node(obj1)) ||
		    !(n2 = scn.get_node(obj2)))
		{
			return INF;
		}
		
		n1->get_world_points(p1);
		n2->get_world_points(p2);
		c1 = calc_centroid(p1);
		c2 = calc_centroid(p2);

		return c1.dist(c2);
	}
	
private:
	string obj1, obj2;
};

/*
 Returns a positive value as long as c is not behind a w.r.t. b, otherwise
 returns 0. Minimized when c is behind a.
*/
class behind_obj : public objective {
public:
	behind_obj(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 u = (cb - ca).unit();
		
		float d = dir_separation(pa, pc, u);
		if (d < 0.) {
			return 0.;
		}
		return d;
	}
	
private:
	string a, b, c;
};

/*
 Returns the shortest distance between the centroid of c and the line
 going through the centroids of a and b. Minimized when the centroids
 of all three objects are collinear.
*/
class collinear_obj : public objective {
public:
	collinear_obj(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		copy(pa.begin(), pa.end(), back_inserter(pc));
		float d = hull_distance(pb, pc);
		if (d < 0) {
			d = 0.;
		}
		/*
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 cc = calc_centroid(pc);
		
		float d = cc.line_dist(ca, cb);
		if (d < 0.001) {
			return 0.;
		}
		*/
		return d;
	}
	
private:
	string a, b, c;
};

class align_facing_objective : public objective {
public:
	align_facing_objective(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		transform3 rot('r', na->get_trans('r'));
		vec3 facing = rot(vec3(1, 0, 0));
		
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 desired = (calc_centroid(pc) - calc_centroid(pb)).unit();
		
		/*
		 Return the negative cosine between the two
		 directions. This will be minimized at -1 if the angle
		 is 0, and maximized at 1 when the angle is 180.
		*/
		float negcos = -(facing.dot(desired));
		//cout << "NEG COS ANGLE " << negcos << endl;
		return negcos;
	}
	
private:
	string a, b, c;
};

class multi_objective {
public:
	multi_objective() {}
	~multi_objective() {
		vector<objective*>::iterator i;
		for (i = objs.begin(); i != objs.end(); ++i) {
			delete *i;
		}
	}
	
	void add(objective *o) {
		objs.push_back(o);
	}
	
	void eval(scene &scn, floatvec &val) const {
		val.resize(objs.size());
		for (int i = 0; i < objs.size(); ++i) {
			val[i] = objs[i]->eval(scn);
		}
	}
private:
	vector<objective*> objs;
};


/*
 Parse a WME structure and return the appropriate objective function.
 Assumes this format:

 (<o1> ^name <name1>
       ^<param1> <val1>
       ^<param2> <val2>
       ...
       ^next <o2>)
 (<o2> ^name <name2>
       ...
*/
multi_objective *parse_obj_struct(soar_interface *si, Symbol *root) {
	multi_objective *m = new multi_objective();
	wme_list param_wmes;
	
	while (root && si->is_identifier(root) && si->get_child_wmes(root, param_wmes)) {
		wme_list::iterator i;
		string name, attr, val;
		map<string, string> params;
		
		for (i = param_wmes.begin(); i != param_wmes.end(); ++i) {
			if (si->get_val(si->get_wme_attr(*i), attr) &&
			    si->get_val(si->get_wme_val(*i), val))
			{
				params[attr] = val;
			}
		}
		if (!map_get(params, string("name"), name)) {
			break;
		}
		if (name == "euclidean") {
			string a, b;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b))
			{
				break;
			}
			m->add(new euclidean_obj(a, b));
		} else if (name == "behind") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			m->add(new behind_obj(a, b, c));
		} else if (name == "collinear") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			m->add(new collinear_obj(a, b, c));
		} else if (name == "align_facing") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			m->add(new align_facing_objective(a, b, c));
		} else {
			cerr << "skipping unknown objective " << name << endl;
		}
		
		wme *next_wme;
		if (!si->find_child_wme(root, "next", next_wme)) {
			break;
		}
		root = si->get_wme_val(next_wme);
	}
	return m;
}

/*
 This class binds the model and objective function together and is
 responsible for simulating a given trajectory and evaluating the
 objective function on the resulting state.
*/
class traj_eval {
public:
	traj_eval(int stepsize, multi_model *m, multi_objective *obj, const scene &init)
	: mdl(m), stepsize(stepsize), obj(obj), numcalls(0), totaltime(0.)
	{
		scn = init.copy();
		scn->get_properties(initvals);
	}
	
	traj_eval(int stepsize, multi_model *m, multi_objective *obj, const scene &tmp, const floatvec &initvals)
	: mdl(m), stepsize(stepsize), obj(obj), numcalls(0), totaltime(0.), initvals(initvals)
	{
		scn = tmp.copy();
	}

	~traj_eval() {
		delete scn;
	}
	
	bool eval(const floatvec &traj, floatvec &value) {
		timer tm;
		tm.start();
		
		if (traj.size() > 0) {
			floatvec x(initvals.size() + stepsize), y = initvals;
			for (int i = 0; i < traj.size(); i += stepsize) {
				x.combine(y, traj.slice(i, i + stepsize));
				if (!mdl->predict(x, y)) {
					return false;
				}
			}
			scn->set_properties(y);
		}
		obj->eval(*scn, value);
		
		totaltime += tm.stop();
		numcalls++;
		return true;
	}
	
	void eval_curr(floatvec &value) {
		obj->eval(*scn, value);
	}
	
	void print_stats() const {
		cout << "ncall: " << numcalls << endl;
		cout << "ttime: " << totaltime << endl;
		cout << "atime: " << totaltime / numcalls << endl;
	}
	
	
private:
	multi_model      *mdl;
	multi_objective  *obj;
	int               stepsize;  // dimensionality of output
	scene            *scn;       // copy of initial scene to be modified after prediction
	floatvec          initvals;  // flattened values of initial scene
	int               numcalls;
	double            totaltime;
};

void constrain(floatvec &v, const floatvec &min, const floatvec &max) {
	for (int i = 0; i < v.size(); ++i) {
		if (v[i] < min[i]) {
			v[i] = min[i];
		} else if (v[i] > max[i]) {
			v[i] = max[i];
		}
	}
}

void argmin(const vector<floatvec> &v, int &worst, int &nextworst, int &best) {
	worst = 0; nextworst = 0; best = 0;
	for (int i = 1; i < v.size(); ++i) {
		if (v[i] > v[worst]) {
			nextworst = worst;
			worst = i;
		} else if (v[i] > v[nextworst]) {
			nextworst = i;
		} else if (v[i] < v[best]) {
			best = i;
		}
	}
}

bool nelder_mead_constrained(const floatvec &min, const floatvec &max, floatvec &best, floatvec &bestval, traj_eval &ev) {
	int ndim = min.size(), i, wi, ni, bi;
	vector<floatvec> eval(ndim+1);
	floatvec reval, eeval, ceval;
	
	floatvec range = max - min;
	vector<floatvec> simplex;
	floatvec centroid(ndim), dir(ndim), reflect(ndim), expand(ndim), 
	         contract(ndim), worst(ndim);
	
	/* random initialization */
	floatvec rtmp(ndim);
	for (i = 0; i < ndim + 1; ++i) {
		rtmp.randomize(min, max);
		if (!ev.eval(rtmp, eval[i])) {
			return false;
		}
		simplex.push_back(rtmp);
	}
	
	for(int iters = 0; iters < MAXITERS; ++iters) {
		argmin(eval, wi, ni, bi);
		worst = simplex[wi];
		best = simplex[bi];
		bestval = eval[bi];
		floatvec sum(ndim);
		
		/*
		 This used to be
		 
		 centroid = (sum(simplex, 1) - worst) / (simplex.n_cols - 1);
		 
		 which I'm pretty sure was wrong.
		*/
		centroid.zero();
		for (i = 0; i < simplex.size(); ++i) {
			if (i != wi) {
				centroid += simplex[i];
			}
		}
		centroid /= (simplex.size() - 1);
		
		dir = centroid - worst;
		reflect = centroid + dir * RCOEF;
		constrain(reflect, min, max);
		
		if (!ev.eval(reflect, reval)) {
			return false;
		}
		if (eval[bi] <= reval && reval < eval[ni]) {
			// reflection
			simplex[wi] = reflect;
			eval[wi] = reval;
			continue;
		}
		
		if (reval < eval[bi]) {
			// expansion
			expand = centroid + dir * ECOEF;
			constrain(expand, min, max);
			if (!ev.eval(expand, eeval)) return false;
			if (eeval < reval) {
				simplex[wi] = expand;
				eval[wi] = eeval;
			} else {
				simplex[wi] = reflect;
				eval[wi] = reval;
			}
			continue;
		}
		
		assert(reval >= eval[ni]);
		
		contract = worst + dir * CCOEF;
		if (!ev.eval(contract, ceval)) {
			return false;
		}
		if (ceval < eval[wi]) {
			// contraction
			simplex[wi] = contract;
			eval[wi] = ceval;
			continue;
		}
		
		// shrink
		for (i = 0; i < simplex.size(); ++i) {
			if (i == bi) {
				continue;
			}
			simplex[i] = best + (simplex[i] - best) * SCOEF;
			if (!ev.eval(simplex[i], eval[i])) return false;
		}
	}
	return true;
}

class tree_search {
private:
	struct common_info {
		multi_objective *obj;
		multi_model *mdl;
		scene *scn;
		output_spec *outspec;
		floatvec min, max, range;
	};
	
	class node {
	public:
		int offset, depth;
		floatvec traj;
		floatvec value;
		floatvec state;
		common_info *ci;
		
		node(const floatvec &state, common_info *ci)
		: offset(0), depth(1), state(state), ci(ci)
		{
			ci->scn->set_properties(state);
			ci->obj->eval(*ci->scn, value);
		}
		
		node(const node &n) 
		: offset(n.offset), depth(n.depth), traj(n.traj), value(n.value), state(n.state), ci(n.ci)
		{}
		
		bool add_step() {
			floatvec step(ci->outspec->size()), newval;
			
			/*
			 First try to seek toward goal with simplex,
			 if that fails, then use random.
			*/
			traj_eval eval(ci->outspec->size(), ci->mdl, ci->obj, *ci->scn, state);
			nelder_mead_constrained(ci->min, ci->max, step, newval, eval);
			if (newval >= value) {
				for (int i = 0; i < ci->outspec->size(); ++i) {
					step[i] = ci->min[i] + (ci->range[i] * rand()) / RAND_MAX;
				}
			}
			floatvec x(state.size() + step.size());
			x.combine(state, step);
			if (!ci->mdl->predict(x, state)) {
				return false;
			}
			ci->scn->set_properties(state);
			ci->obj->eval(*ci->scn, value);
			
			if (offset == traj.size()) {
				if (traj.size() == 0) {
					traj.resize(step.size());
				} else {
					traj.resize(traj.size() * 2);
				}
			}
			for (int i = 0; i < step.size(); ++i) {
				traj[offset + i] = step[i];
			}
			
			offset += step.size();
			depth++;
			
			return true;
		}
	};
	
	struct node_compare {
		bool operator()(const node *lhs, const node *rhs) const {
			/*
			 Since the priority queue keeps the largest items,
			 we have to reverse the comparison.
			*/
			return lhs->value > rhs->value;
		}
	};
	
public:
	tree_search(scene *scn, multi_model *mdl, multi_objective *obj, output_spec *outspec, float thresh)
	: outspec(outspec), thresh(thresh)
	{
		ci.scn = scn->copy();
		ci.obj = obj;
		ci.mdl = mdl;
		ci.outspec = outspec;
		ci.min.resize(outspec->size());
		ci.max.resize(outspec->size());
		ci.range.resize(outspec->size());
		for (int i = 0; i < outspec->size(); ++i) {
			ci.min[i] = (*outspec)[i].min;
			ci.max[i] = (*outspec)[i].max;
			ci.range[i] = ci.max[i] - ci.min[i];
		}
		
		floatvec initstate;
		scn->get_properties(initstate);
		bestnode = new node(initstate, &ci);
		leafs.push_back(bestnode);
		num_nodes = 1;
		total_depth = 1.0;
		avg_depth = 1.0;
		avg_bf = 1.0;
 	}
	
	~tree_search() {
		std::list<node*>::iterator i;
		for (i = leafs.begin(); i != leafs.end(); ++i) {
			delete *i;
		}
		for (i = nonleafs.begin(); i != nonleafs.end(); ++i) {
			delete *i;
		}
	}
	
	void expand() {
		/*
		 Since the underlying container for the queue is a vector,
		 we know that &nodes.top() points to the first position
		 of an internal array of nodes.
		*/
		bool isleaf;
		node *newnode;
		std::list<node*>::iterator selected;
		
		if (nonleafs.size() > 0 && avg_depth / avg_bf > thresh) {
			int r = rand() % nonleafs.size();
			selected = nonleafs.begin();
			advance(selected, r);
			isleaf = false;
		} else {
			int r = rand() % leafs.size();
			selected = leafs.begin();
			advance(selected, r);
			isleaf = true;
		}
		
		newnode = new node(**selected);
		
		if (!newnode->add_step()) {
			cout << "TREE SEARCH ERROR" << endl;
			delete newnode;
			return;
		}
		
		if (isleaf) {
			nonleafs.push_back(*selected);
			leafs.erase(selected);
		}
		
		leafs.push_back(newnode);
		num_nodes++;
		total_depth += newnode->depth;
		avg_depth = total_depth / num_nodes;
		avg_bf = ((float) num_nodes) / nonleafs.size();
		
		if (newnode->value < bestnode->value) {
			bestnode = newnode;
		}
	}
	
	void search(int iterations, floatvec &besttraj, floatvec &bestval) {
		std::list<node*>::iterator i;
		int j;
		for (j = 0; j < iterations; ++j) {
			expand();
		}
		
		besttraj = bestnode->traj;
		bestval = bestnode->value;
		cout << "BEST TRAJ LENGTH " << bestnode->offset / outspec->size() << endl;
		cout << "AVG DEPTH " << avg_depth << endl;
		cout << "AVG BF " << avg_bf << endl;
		
		/*
		floatvec lengths(leafs.size());
		for (i = leafs.begin(), j = 0; i != leafs.end(); ++i, ++j) {
			lengths[j] = (**i).depth;
		}
		histogram(lengths, 10);
		*/
	}
	
private:
	//priority_queue<node*, vector<node*>, node_compare> nodes;
	std::list<node*> leafs;
	std::list<node*> nonleafs;
	common_info ci;
	output_spec *outspec;
	int num_nodes;
	float total_depth, avg_depth, avg_bf, thresh;
	node *bestnode;
};

class controller {
public:
	controller(multi_model *mmdl, multi_objective *obj, output_spec *outspec, int depth, string type)
	: mmdl(mmdl), obj(obj), outspec(outspec), depth(depth), type(type), incr(depth, outspec)
	{
		int i, j;
		
		stepsize = outspec->size();
		min.resize(depth * outspec->size());
		max.resize(depth * outspec->size());
		for (i = 0; i < depth; ++i) {
			for (j = 0; j < stepsize; ++j) {
				min[i * stepsize + j] = (*outspec)[j].min;
				max[i * stepsize + j] = (*outspec)[j].max;
			}
		}
	}

	int seek(scene *scn, floatvec &bestout) {
		floatvec besttraj, currval, bestval;
		bool result;
		if (type == "tree") {
			tree_search t(scn, mmdl, obj, outspec, 10);
			t.search(depth, besttraj, bestval);
		} else {
			traj_eval evaluator(stepsize, mmdl, obj, *scn);
			if (type == "simplex") {
				result = nelder_mead_constrained(min, max, besttraj, bestval, evaluator);
			} else {
				result = naive_seek(evaluator, besttraj, bestval);
			}
			evaluator.print_stats();
			if (!result) {
				return 0;
			}
		}
		obj->eval(*scn, currval);
		if (besttraj.size() < stepsize || currval <= bestval) {
			return 1;
		}
		bestout = besttraj.slice(0, stepsize);
		draw_prediction(scn, mmdl, besttraj, stepsize);
		cout << "BEST VAL " << bestval << endl;
		cout << "BEST OUT " << bestout << endl;
		return 2;
	}
	
	bool naive_seek(traj_eval &evaluator, floatvec &besttraj, floatvec &bestval) {
		floatvec val;
		bool found = false;
		
		incr.reset();
		while (true) {
			if (!evaluator.eval(incr.traj, val)) {
				return false;
			}
			if (!found || val < bestval) {
				found = true;
				besttraj = incr.traj;
				bestval = val;
			}
			if (!incr.next()) {
				break;
			}
		}
		return found;
	}
	
private:
	/*
	 Incrementer for a single step within a trajectory
	*/
	class step_incr {
	public:
		step_incr(output_spec *outspec, floatvec *traj, int divisions, int start) 
		: outspec(outspec), traj(traj), start(start), divisions(divisions), inc(outspec->size())
		{
			for (int i = 0; i < outspec->size(); ++i) {
				inc[i] = ((*outspec)[i].max - (*outspec)[i].min) / divisions;
			}
			reset();
		}
	
		void reset() {
			for (int i = 0; i < outspec->size(); ++i) {
				(*traj)[start + i] = (*outspec)[i].min;
			}
		}
		
		bool next() {
			for (int i = 0; i < outspec->size(); ++i) {
				(*traj)[start + i] += inc[i];
				if ((*traj)[start + i] <= (*outspec)[i].max) {
					return true;
				} else {
					(*traj)[start + i] = (*outspec)[i].min;  // roll over and move on to the next value
				}
			}
			return false;
		}
		
	private:
		output_spec *outspec;
		int start, divisions;
		floatvec *traj;
		floatvec inc;
	};
	
	/*
	 Incrementer for a trajectory, used with naive search
	*/
	class traj_incr {
	public:
		traj_incr() : len(0) {}
		
		traj_incr(int len, output_spec *outspec)
		: len(len)
		{
			int stepsize = outspec->size();
			traj.resize(len * stepsize);
			for (int i = 0; i < len; i++) {
				steps.push_back(step_incr(outspec, &traj, 3, i * stepsize));
			}
			reset();
		}
		
		void reset() {
			std::vector<step_incr>::iterator i;
			for (i = steps.begin(); i != steps.end(); ++i) {
				i->reset();
			}
		}
		
		bool next() {
			std::vector<step_incr>::iterator i;
			for (i = steps.begin(); i != steps.end(); ++i) {
				if (i->next()) {
					return true;
				}
				i->reset();
			}
			return false;
		}
		
		floatvec traj;
	
	private:
		vector<step_incr> steps;
		int len;
	};
	
	multi_model     *mmdl;
	multi_objective *obj;
	output_spec     *outspec;
	floatvec         min, max;   // for Nelder-Mead
	int              depth;
	int              stepsize;
	string           type;
	traj_incr        incr;
};

class seek_command : public command {
public:
	seek_command(svs_state *state, Symbol *root)
	: command(state, root), state(state), step(0),
	  stepwme(NULL), broken(false), ctrl(NULL), obj(NULL)
	{
		si = state->get_svs()->get_soar_interface();
		//update_step();
	}
	
	~seek_command() {
		cleanup();
	}
	
	string description() {
		return string("control");
	}
	
	bool update() {
		floatvec out;
		
		if (changed()) {
			broken = !parse_cmd();
		}
		if (broken) {
			return false;
		}
		
		timer t1;
		t1.start();
		int result = ctrl->seek(state->get_scene(), out);
		switch (result) {
			case 0:
				set_status("no valid output found");
				break;
			case 1:
				set_status("local minimum");
				break;
			case 2:
				state->set_output(out);
				set_status("success");
				step++;
				//update_step();
				break;
		}
		cout << "SEEK " << t1.stop() << endl;
		return true;
	}
	
	bool early() { return true; }
	
private:
	/* Assumes this format:
	   C1 ^outputs ( ... )
	      ^objective ( ... )
	*/
	bool parse_cmd() {
		wme *objective_wme, *model_wme, *depth_wme, *type_wme;
		long depth;
		string type;
		
		cleanup();
		if (!si->find_child_wme(get_root(), "type", type_wme) ||
			!si->get_val(si->get_wme_val(type_wme), type))
		{
			set_status("missing or invalid type");
			return false;
		}
		if (!si->find_child_wme(get_root(), "objective", objective_wme) ||
			!si->is_identifier(si->get_wme_val(objective_wme)) ||
			(obj = parse_obj_struct(si, si->get_wme_val(objective_wme))) == NULL)
		{
			set_status("missing or invalid objective");
			return false;
		}
		
		if (!si->find_child_wme(get_root(), "depth", depth_wme) ||
			!si->get_val(si->get_wme_val(depth_wme), depth))
		{
			set_status("missing or invalid depth");
			return false;
		}
		ctrl = new controller(state->get_model(), obj, state->get_output_spec(), depth, type);
		return true;
	}

	void cleanup() {
		delete obj; obj = NULL;
		delete ctrl; ctrl = NULL;
	}
	
	void update_step() {
		if (stepwme)
			si->remove_wme(stepwme);
		stepwme = si->make_wme(get_root(), "step", step);
	}

	soar_interface  *si;
	svs_state       *state;
	controller      *ctrl;
	multi_objective *obj;
	wme             *stepwme;
	int              step;
	bool             broken;
};

command *_make_seek_command_(svs_state *state, Symbol *root) {
	return new seek_command(state, root);
}

class random_control_command : public command {
public:
	random_control_command(svs_state *state, Symbol *root)
	: command(state, root), state(state)
	{ }
	
	string description() {
		return string("random control");
	}
	
	bool update() {
		if (changed()) {
			wme *outputs_wme;
			output_spec *outspec = state->get_output_spec();
			out.resize(outspec->size());
			min.resize(outspec->size());
			max.resize(outspec->size());
			for (int i = 0; i < outspec->size(); ++i) {
				min[i] = (*outspec)[i].min;
				max[i] = (*outspec)[i].max;
			}
		}
		
		out.randomize(min, max);
		state->set_output(out);
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	svs_state *state;
	floatvec   out, min, max;
};

command *_make_random_control_command_(svs_state *state, Symbol *root) {
	return new random_control_command(state, root);
}

class manual_control_command : public command {
public:
	manual_control_command(svs_state *state, Symbol *root) 
	: command(state, root), state(state), outspec(state->get_output_spec())
	{
		sock = new ipcsocket('s', "ctrl", false);
	}
	
	~manual_control_command() {
		delete sock;
	}
	
	string description() {
		return string("manual control");
	}
	
	bool update() {
		string msg, line;
		stringstream ss1;

		sock->send("dummy");
		sock->receive(msg);
		ss1.str(msg);
		
		floatvec out(outspec->size());
		while (getline(ss1, line)) {
			stringstream ss2(line);
			string name;
			float val;
			if (!(ss2 >> name) || !(ss2 >> val)) {
				set_status("input error");
				return false;
			}
			for (int i = 0; i < outspec->size(); ++i) {
				if ((*outspec)[i].name == name) {
					out[i] = val;
					break;
				}
			}
		}
		
		state->set_output(out);
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	output_spec *outspec;
	svs_state *state;
	ipcsocket *sock;
};

command *_make_manual_control_command_(svs_state *state, Symbol *root) {
	return new manual_control_command(state, root);
}
