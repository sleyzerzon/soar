#include <math.h>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include "command.h"
#include "svs.h"
#include "env.h"
#include "scene.h"
#include "model.h"

using namespace std;

// constants for simplex search
const double RCOEF = 1.0;
const double ECOEF = 2.0;
const double CCOEF = 0.5;
const double SCOEF = 0.5;

const int MAXITERS = 50;
const double INF = numeric_limits<double>::infinity();

/* Parses a WME structure that describes the output the environment expects.
   Assumes this format:

	^outputs (
		^<name1> (
			^min <val>
			^max <val>
			^inc <val>
		)
		^<name2> (
			^min <val>
			^max <val>
			^inc <val>
		)
		...
	)
*/
bool parse_output_desc_struct(soar_interface *si, Symbol *root, outdesc &desc) {
	wme_list dim_wmes;
	wme_list::iterator i;
	Symbol *dim_id;
	wme *min_wme, *max_wme, *inc_wme;
	out_dim_desc d;
	
	if (!si->is_identifier(root)) {
		return false;
	}
	si->get_child_wmes(root, dim_wmes);
	for (i = dim_wmes.begin(); i != dim_wmes.end(); ++i) {
		dim_id = si->get_wme_val(*i);
		if (si->get_val(si->get_wme_attr(*i), d.name)    &&
		    si->is_identifier(dim_id)                    &&
		    si->find_child_wme(dim_id, "min", min_wme)   &&
		    si->get_val(si->get_wme_val(min_wme), d.min) &&
		    si->find_child_wme(dim_id, "max", max_wme)   &&
		    si->get_val(si->get_wme_val(max_wme), d.max) &&
		    si->find_child_wme(dim_id, "inc", inc_wme)   &&
		    si->get_val(si->get_wme_val(inc_wme), d.inc))
		{
			desc.push_back(d);
		}
	}
	sort(desc.begin(), desc.end());
	return true;
}


class objective {
public:
	virtual float eval(scene *scn) = 0;
};

::vec3 calc_centroid(const ptlist &pts) {
	ptlist::const_iterator i;
	int d;
	::vec3 c;
	
	for (i = pts.begin(); i != pts.end(); ++i) {
		for (d = 0; d < 3; ++d) {
			c[d] += (*i)[d];
		}
	}
	for (d = 0; d < 3; ++d) {
		c[d] /= pts.size();
	}
	return c;
}

/* Squared Euclidean distance between centroids of two objects */
class euclidean_obj : public objective {
public:
	euclidean_obj(string obj1, string obj2)
	: obj1(obj1), obj2(obj2) {}
	
	float eval(scene *scn) {
		sg_node *n1, *n2;
		ptlist p1, p2;
		::vec3 c1, c2;
		
		if ((n1 = scn->get_node(obj1)) == NULL ||
		    (n2 = scn->get_node(obj2)) == NULL)
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

/* Parse a WME structure and return the appropriate objective function.
   Assumes this format:

   ^objective (
       ^name <name>
       ^<param1> <val1>
       ^<param2> <val2>
	   ...
   )
*/
objective *parse_obj_struct(soar_interface *si, Symbol *root) {
	wme_list param_wmes;
	wme_list::iterator i;
	map<string, string> params;
	string name, attr, val;
	
	if (!si->is_identifier(root)) {
		return NULL;
	}
	if (!si->get_child_wmes(root, param_wmes)) {
		return NULL;
	}
	for (i = param_wmes.begin(); i != param_wmes.end(); ++i) {
		if (si->get_val(si->get_wme_attr(*i), attr) &&
		    si->get_val(si->get_wme_val(*i), val))
		{
			params[attr] = val;
		}
	}
	name = params["name"];
	if (name == "euclidean") {
		if (params.find("a") == params.end() || params.find("b") == params.end()) {
			return NULL;
		}
		return new euclidean_obj(params["a"], params["b"]);
	}
	return NULL;
}

/* this class is a little dirty to avoid as much allocation and copying as possible */
class traj_eval {
public:
	traj_eval(outdesc *desc, int length, model *m, objective *obj, scene *init)
	: cachedtrj(length, desc), obj(obj), next(*init)
	{
		outdesc::const_iterator i;
		mdl = dynamic_cast<multi_model*>(m);
		assert(mdl);
		
		init->get_property_names(names);
		for (i = desc->begin(); i != desc->end(); ++i) {
			names.push_back(i->name);
		}
		init->get_properties(initvals);
	}

	/* version to be used in incremental search */
	bool eval(const trajectory &traj, float &value) {
		vector<output>::const_iterator i;
		
		mdl->set_indexes(names);
		
		floatvec x(names.size()), y = initvals;
		
		if (traj.t.size() > 0) {
			for (i = traj.t.begin(); i != traj.t.end(); ++i) {
				x.combine(y, i->vals);
				if (!mdl->predict(x, y)) {
					return false;
				}
			}
			next.set_properties(y);
		}
		value = obj->eval(&next);
		return true;
	}
	
	/* version to be used in Nelder-Mead search */
	bool eval(const floatvec &v, float &value) {
		cachedtrj.from_vec(v);
		return eval(cachedtrj, value);
	}
	
private:
	multi_model   *mdl;
	objective     *obj;
	trajectory     cachedtrj; // keep around to avoid reallocation
	scene          next;      // copy of initial scene to be modified after prediction
	floatvec       initvals;  // flattened values of initial scene for resetting flat
	vector<string> names;
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

void argmin(const floatvec &v, int &worst, int &nextworst, int &best) {
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

void calc_centroid(const vector<floatvec> &simplex, floatvec &sum, int exclude) {
	for (int i = 0; i < simplex.size(); ++i) {
		if (i != exclude) {
			sum += simplex[i];
		}
	}
	sum /= (simplex.size() - 1);
}

bool nelder_mead_constrained(trajectory &t, traj_eval &ev) {
	int ndim = t.dof(), i, wi, ni, bi;
	outdesc::const_iterator j;
	floatvec min(t.dof()), max(t.dof());
	floatvec eval(ndim+1);
	for (i = 0; i < min.size(); ) {
		for (j = t.desc->begin(); j != t.desc->end(); ++j) {
			min[i] = j->min;
			max[i] = j->max;
			++i;
		}
	}
	
	floatvec range = max - min;
	vector<floatvec> simplex;
	float reval, eeval, ceval;
	floatvec centroid(ndim), dir(ndim), reflect(ndim), expand(ndim), 
	         contract(ndim), best(ndim), worst(ndim);
	
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
		floatvec sum(ndim);
		
		/*
		 This used to be
		 
		 centroid = (sum(simplex, 1) - worst) / (simplex.n_cols - 1);
		 
		 which I'm pretty sure was wrong.
		*/
		calc_centroid(simplex, centroid, wi);
		
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
	t.from_vec(best);
	return true;
}

class controller {
public:
	controller(svs *svsp, objective *obj, const outdesc &odesc, int depth, string type)
	: svsp(svsp), obj(obj), odesc(odesc), step(0), depth(depth), type(type) 
	{}

	/* Don't forget to make this a PID controller later */
	bool seek(scene *scn, output &bestout) {
		if (type == "simplex") {
			return simplex_seek(scn, bestout);
		} else if (type == "random") {
			return random_seek(scn, bestout);
		}
		return naive_seek(scn, bestout);
	}
	
	bool random_seek(scene *scn, output &bestout) {
		bestout = output(&odesc);
		bestout.randomize();
		return true;
	}
	
	bool naive_seek(scene *scn, output &bestout) {
		float val, best;
		bool found = false;
		model *mdl = svsp->get_model();
		
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(&odesc, depth, mdl, obj, scn);
		trajectory trj(depth, &odesc);
		
		while (true) {
			if (!evaluator.eval(trj, val)) {
				cout << "WARNING: Using random output" << endl;
				bestout = output(&odesc);
				bestout.randomize();
				return true;
			}
			if (!found || val < best) {
				found = true;
				bestout = trj.t.front();
				best = val;
			}
			if (!trj.next()) {
				break;
			}
		}
		step++;
		return found;
	}
	
	bool simplex_seek(scene *scn, output &bestout) {
		model *mdl = svsp->get_model();
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(&odesc, depth, mdl, obj, scn);
		trajectory trj(depth, &odesc);
		if (!nelder_mead_constrained(trj, evaluator)) {
			cout << "WARNING: Using random output" << endl;
			bestout = output(&odesc);
			bestout.randomize();
			return true;
		}
		bestout = trj.t.front();
		return true;
	}
	
private:
	svs       *svsp;
	objective *obj;
	outdesc    odesc;
	int        step;
	int        depth;
	string     type;
};

class control_command : public command {
public:
	control_command(svs_state *state, Symbol *root)
	: command(state, root), state(state), root(root),
	  si(state->get_svs()->get_soar_interface()), step(0),
	  stepwme(NULL), broken(false), ctrl(NULL), obj(NULL)
	{
		//update_step();
	}
	
	~control_command() {
		cleanup();
	}
	
	string description() {
		return string("control");
	}
	
	bool update() {
		output out;

		if (changed()) {
			broken = !parse_cmd();
		}
		if (broken) {
			return false;
		}
		
		if (!ctrl->seek(state->get_scene(), out)) {
			set_status("no valid output found");
			return false;
		}
		if (state->get_level() == 0) {
			state->get_svs()->set_next_output(out);
		}
		// need to update scene with model otherwise
		
		set_status("success");
		step++;
		//update_step();
		return true;
	}
	
	bool early() { return true; }
	
private:
	/* Assumes this format:
	   C1 ^outputs ( ... )
	      ^objective ( ... )
	*/
	bool parse_cmd() {
		outdesc desc;
		wme *outputs_wme, *objective_wme, *model_wme, *depth_wme, *type_wme;
		long depth;
		string type;
		
		cleanup();
		if (!si->find_child_wme(root, "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !parse_output_desc_struct(si, si->get_wme_val(outputs_wme), desc))
		{
			set_status("missing or invalid outputs specification");
			return false;
		}
		if (!si->find_child_wme(root, "type", type_wme) ||
			!si->get_val(si->get_wme_val(type_wme), type))
		{
			set_status("missing or invalid type");
			return false;
		}
		if (type == "random") {
			obj = NULL;
			depth = 0;
		} else {
			if (!si->find_child_wme(root, "objective", objective_wme) ||
				!si->is_identifier(si->get_wme_val(objective_wme)) ||
				(obj = parse_obj_struct(si, si->get_wme_val(objective_wme))) == NULL)
			{
				set_status("missing or invalid objective");
				return false;
			}
			
			if (!si->find_child_wme(root, "depth", depth_wme) ||
				!si->get_val(si->get_wme_val(depth_wme), depth))
			{
				set_status("missing or invalid depth");
				return false;
			}
		}
		ctrl = new controller(state->get_svs(), obj, desc, depth, type);
		return true;
	}

	void cleanup() {
		delete obj; obj = NULL;
		delete ctrl; ctrl = NULL;
	}
	
	void update_step() {
		if (stepwme)
			si->remove_wme(stepwme);
		stepwme = si->make_wme(root, "step", step);
	}

	soar_interface *si;
	svs_state      *state;
	Symbol         *root;
	controller     *ctrl;
	objective      *obj;
	wme            *stepwme;
	int             step;
	bool            broken;
};

command *_make_control_command_(svs_state *state, Symbol *root) {
	return new control_command(state, root);
}
