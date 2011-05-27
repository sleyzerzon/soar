#include <math.h>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <armadillo>
#include "command.h"
#include "svs.h"
#include "env.h"
#include "scene.h"
#include "model.h"

using namespace std;
using namespace arma;

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
	virtual double eval(scene *scn) = 0;
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
	
	double eval(scene *scn) {
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
	traj_eval(outdesc *desc, int length, model *mdl, objective *obj, scene *init)
	: cachedtrj(length, desc), mdl(mdl), obj(obj), next(*init), flat(init)
	{
		initvals = flat.vals;
	}

	/* version to be used in incremental search */
	bool eval(const trajectory &traj, double &value) {
		vector<output>::const_iterator i;
		
		flat.vals = initvals;
		if (!mdl->predict(flat, traj)) {
			return false;
		}
		flat.update_scene(&next);
		value = obj->eval(&next);
		return true;
	}
	
	/* version to be used in Nelder-Mead search */
	bool eval(vec traj, double &value) {
		cachedtrj.from_vec(traj);
		return eval(cachedtrj, value);
	}
	
private:
	model       *mdl;
	objective   *obj;
	trajectory   cachedtrj; // keep around to avoid reallocation
	scene        next;      // copy of initial scene to be modified after prediction
	flat_scene   flat;      // keep around to avoid reallocation
	floatvec     initvals;  // flattened values of initial scene for resetting flat
};

void constrain(vec &v, const vec &min, const vec &max) {
	for (int i = 0; i < v.n_elem; ++i) {
		if (v(i) < min(i)) {
			v(i) = min(i);
		} else if (v(i) > max(i)) {
			v(i) = max(i);
		}
	}
}

void argmin(const vec &v, int &worst, int &nextworst, int &best) {
	worst = 0; nextworst = 0; best = 0;
	for (int i = 1; i < v.n_elem; ++i) {
		if (v(i) > v(worst)) {
			nextworst = worst;
			worst = i;
		} else if (v(i) > v(nextworst)) {
			nextworst = i;
		} else if (v(i) < v(best)) {
			best = i;
		}
	}
}

bool nelder_mead_constrained(trajectory &t, traj_eval &ev) {
	int ndim = t.dof(), i, wi, ni, bi;
	outdesc::const_iterator j;
	vec min = zeros(t.dof(), 1), max = zeros(t.dof(), 1);
	for (i = 0; i < min.n_elem; ) {
		for (j = t.desc->begin(); j != t.desc->end(); ++j) {
			min(i) = j->min;
			max(i) = j->max;
			++i;
		}
	}
	
	vec range = max - min;
	mat simplex = zeros<mat>(ndim, ndim+1);
	vec eval = zeros<vec>(ndim+1, 1);
	double reval, eeval, ceval;
	vec centroid, dir, reflect, expand, contract, best, worst;
	
	/* random initialization */
	for (i = 0; i < simplex.n_cols; ++i) {
		simplex.col(i) = min + randu<vec>(ndim, 1) % range;
		if (!ev.eval(simplex.col(i), eval(i))) return false;
	}
	
	for(int iters = 0; iters < MAXITERS; ++iters) {
		argmin(eval, wi, ni, bi);
		worst = simplex.col(wi);
		best = simplex.col(bi);
		centroid = (sum(simplex, 1) - worst) / (simplex.n_cols - 1);
		dir = centroid - worst;
		reflect = centroid + RCOEF * dir;
		constrain(reflect, min, max);
		
		if (!ev.eval(reflect, reval)) return false;
		if (eval(bi) <= reval && reval < eval(ni)) {
			// reflection
			simplex.col(wi) = reflect;
			eval(wi) = reval;
			continue;
		}
		
		if (reval < eval(bi)) {
			// expansion
			expand = centroid + ECOEF * dir;
			constrain(expand, min, max);
			if (!ev.eval(expand, eeval)) return false;
			if (eeval < reval) {
				simplex.col(wi) = expand;
				eval(wi) = eeval;
			} else {
				simplex.col(wi) = reflect;
				eval(wi) = reval;
			}
			continue;
		}
		
		assert(reval >= eval(ni));
		
		contract = worst + CCOEF * dir;
		if (!ev.eval(contract, ceval)) return false;
		if (ceval < eval(wi)) {
			// contraction
			simplex.col(wi) = contract;
			eval(wi) = ceval;
			continue;
		}
		
		// shrink
		for (i = 0; i < simplex.n_cols; ++i) {
			if (i == bi) {
				continue;
			}
			simplex.col(i) = best + SCOEF * (simplex.col(i) - best);
			if (!ev.eval(simplex.col(i), eval(i))) return false;
		}
	}
	t.from_vec(best);
	return true;
}

class controller {
public:
	controller(svs *svsp, string modelname, objective *obj, const outdesc &odesc, int depth, string type)
	: svsp(svsp), modelname(modelname), obj(obj), odesc(odesc), step(0), depth(depth), type(type) 
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
		bestout = random_out(&odesc);
		return true;
	}
	
	bool naive_seek(scene *scn, output &bestout) {
		double val, best = INF;
		bool found = false;
		model *mdl = svsp->get_model(modelname);
		
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(&odesc, depth, mdl, obj, scn);
		trajectory trj(depth, &odesc);
		
		while (true) {
			if (!evaluator.eval(trj, val)) {
				cout << "WARNING: Using random output" << endl;
				bestout = random_out(&odesc);
				return true;
			}
			if (val < best) {
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
		model *mdl = svsp->get_model(modelname);
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(&odesc, depth, mdl, obj, scn);
		trajectory trj(depth, &odesc);
		if (!nelder_mead_constrained(trj, evaluator)) {
			cout << "WARNING: Using random output" << endl;
			bestout = random_out(&odesc);
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
	string     modelname;
};

class control_command : public command {
public:
	control_command(svs_state *state, Symbol *root)
	: state(state), root(root), utils(state, root), 
	  si(state->get_svs()->get_soar_interface()), step(0), 
	  stepwme(NULL), broken(false), ctrl(NULL), obj(NULL)
	{
		//update_step();
	}
	
	~control_command() {
		cleanup();
	}
	
	bool update_result() {
		output out;

		if (utils.cmd_changed()) {
			broken = !parse_cmd();
		}
		if (broken) {
			return false;
		}
		
		if (!ctrl->seek(state->get_scene(), out)) {
			utils.set_result("no valid output found");
			return false;
		}
		if (state->get_level() == 0) {
			state->get_svs()->set_next_output(out);
		}
		// need to update scene with model otherwise
		
		utils.set_result("success");
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
		string type, modelname;
		
		cleanup();
		if (!si->find_child_wme(root, "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !parse_output_desc_struct(si, si->get_wme_val(outputs_wme), desc))
		{
			utils.set_result("missing or invalid outputs specification");
			return false;
		}
		if (!si->find_child_wme(root, "type", type_wme) ||
			!si->get_val(si->get_wme_val(type_wme), type))
		{
			utils.set_result("missing or invalid type");
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
				utils.set_result("missing or invalid objective");
				return false;
			}
			
			if (!si->find_child_wme(root, "model", model_wme) ||
			    !si->get_val(si->get_wme_val(model_wme), modelname))
			{
				utils.set_result("missing model name");
				return false;
			}
			if (!si->find_child_wme(root, "depth", depth_wme) ||
				!si->get_val(si->get_wme_val(depth_wme), depth))
			{
				utils.set_result("missing or invalid depth");
				return false;
			}
		}
		ctrl = new controller(state->get_svs(), modelname, obj, desc, depth, type);
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
	cmd_utils       utils;
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
