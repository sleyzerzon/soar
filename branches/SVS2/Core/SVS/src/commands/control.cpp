#include <math.h>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include "command.h"
#include "svs.h"
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

/*
 Description of a single output dimension.
*/
struct output_dim_info {
	string name;
	float min;
	float max;
	float inc;
};

bool output_comp(const output_dim_info &a, const output_dim_info &b) {
	return a.name < b.name;
}

typedef std::vector<output_dim_info> output_info_vec;

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
bool parse_wm_output_struct(soar_interface *si, Symbol *root, output_info_vec &out_info) {
	
	wme_list dim_wmes;
	wme_list::iterator i;
	Symbol *dim_id;
	wme *min_wme, *max_wme, *inc_wme;
	output_dim_info d;
	
	out_info.clear();
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
			out_info.push_back(d);
		}
	}
	sort(out_info.begin(), out_info.end(), output_comp);
	return true;
}



class objective {
public:
	virtual float eval(scene &scn) const = 0;
};

/* Squared Euclidean distance between centroids of two objects */
class euclidean_obj : public objective {
public:
	euclidean_obj(string obj1, string obj2)
	: obj1(obj1), obj2(obj2) {}
	
	float eval(scene &scn) const {
		sgnode *n1, *n2;
		ptlist p1, p2;
		::vec3 c1, c2;
		
		if ((n1 = scn.get_node(obj1)) == NULL ||
		    (n2 = scn.get_node(obj2)) == NULL)
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

/*
 This class binds the model and objective function together and is
 responsible for simulating a given trajectory and evaluating the
 objective function on the resulting state.
*/
class traj_eval {
public:
	traj_eval(int stepsize, multi_model *m, objective *obj, const scene &init)
	: mdl(m), stepsize(stepsize), obj(obj), numcalls(0), totaltime(0.)
	{
		scn = init.copy();
		scn->get_properties(initvals);
	}

	~traj_eval() {
		delete scn;
	}
	
	bool eval(const floatvec &traj, float &value) {
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
		value = obj->eval(*scn);
		
		totaltime += tm.stop();
		numcalls++;
		return true;
	}
	
	void print_stats() const {
		cout << "ncall: " << numcalls << endl;
		cout << "ttime: " << totaltime << endl;
		cout << "atime: " << totaltime / numcalls << endl;
	}
	
	void draw() {
		scn->draw_all("predict_", 1., 0., 0.);
	}
	
private:
	multi_model   *mdl;
	objective     *obj;
	int            stepsize;  // dimensionality of output
	scene         *scn;       // copy of initial scene to be modified after prediction
	floatvec       initvals;  // flattened values of initial scene
	int            numcalls;
	double         totaltime;
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

bool nelder_mead_constrained(const floatvec &min, const floatvec &max, floatvec &best, traj_eval &ev) {
	int ndim = min.size(), i, wi, ni, bi;
	floatvec eval(ndim+1);
	float reval, eeval, ceval;
	
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

class controller {
public:
	controller(svs *svsp, objective *obj, const output_info_vec &stepinfo, int depth, string type)
	: svsp(svsp), obj(obj), stepinfo(stepinfo), depth(depth), type(type), incr(depth, stepinfo)
	{
		int i, j;
		
		stepsize = stepinfo.size();
		min.resize(depth * stepinfo.size());
		max.resize(depth * stepinfo.size());
		for (i = 0; i < depth; ++i) {
			for (j = 0; j < stepsize; ++j) {
				min[i * stepsize + j] = stepinfo[j].min;
				max[i * stepsize + j] = stepinfo[j].max;
			}
		}
	}

	bool seek(scene *scn, floatvec &bestout) {
		if (type == "simplex") {
			return simplex_seek(scn, bestout);
		}
		return naive_seek(scn, bestout);
	}
	
	bool naive_seek(scene *scn, floatvec &bestout) {
		float val, best;
		bool found = false;
		
		traj_eval evaluator(stepsize, svsp->get_model(), obj, *scn);
		incr.reset();
		while (true) {
			if (!evaluator.eval(incr.traj, val)) {
				return false;
			}
			if (!found || val < best) {
				found = true;
				bestout = incr.traj.slice(0, stepsize);
				best = val;
			}
			if (!incr.next()) {
				break;
			}
		}
		return found;
	}
	
	bool simplex_seek(scene *scn, floatvec &bestout) {
		traj_eval evaluator(stepsize, svsp->get_model(), obj, *scn);
		floatvec best(min.size());
		if (!nelder_mead_constrained(min, max, best, evaluator)) {
			return false;
		}
		evaluator.print_stats();
		float bestval;
		evaluator.eval(best, bestval);
		//evaluator.draw();
		bestout = best.slice(0, stepsize);
		return true;
	}
	
private:
	/*
	 Incrementer for a single step within a trajectory
	*/
	class step_incr {
	public:
		step_incr(const output_info_vec &dims, floatvec *traj, int start) 
		: dims(dims), traj(traj), start(start)
		{
			reset();
		}
	
		void reset() {
			output_info_vec::const_iterator i;
			int j;
			for (i = dims.begin(), j = 0; i != dims.end(); ++i, ++j) {
				(*traj)[start + j] = i->min;
			}
		}
		
		bool next() {
			output_info_vec::const_iterator i;
			int j;
			
			for (i = dims.begin(), j = 0; i != dims.end(); ++i, ++j) {
				(*traj)[start + j] += i->inc;
				if ((*traj)[start + j] <= i->max) {
					return true;
				} else {
					(*traj)[start + j] = i->min;  // roll over and move on to the next value
				}
			}
			return false;
		}
		
	private:
		output_info_vec dims;
		int start;
		floatvec *traj;
	};
	
	/*
	 Incrementer for a trajectory, used with naive search
	*/
	class traj_incr {
	public:
		traj_incr() : len(0) {}
		
		traj_incr(int len, const output_info_vec &stepinfo)
		: len(len)
		{
			int stepsize = stepinfo.size();
			traj.resize(len * stepsize);
			for (int i = 0; i < len; i++) {
				steps.push_back(step_incr(stepinfo, &traj, i * stepsize));
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
	
	svs             *svsp;
	objective       *obj;
	output_info_vec  stepinfo;
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
		if (changed()) {
			broken = !parse_cmd();
		}
		if (broken) {
			return false;
		}
		
		timer t1;
		t1.start();
		if (!ctrl->seek(state->get_scene(), out.vals)) {
			set_status("no valid output found");
			return false;
		}
		cout << "SEEK " << t1.stop() << endl;
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
		output_info_vec out_info;
		output_info_vec::iterator i;
		wme *outputs_wme, *objective_wme, *model_wme, *depth_wme, *type_wme;
		long depth;
		string type;
		
		cleanup();
		if (!si->find_child_wme(get_root(), "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !parse_wm_output_struct(si, si->get_wme_val(outputs_wme), out_info))
		{
			set_status("missing or invalid outputs specification");
			return false;
		}
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
		ctrl = new controller(state->get_svs(), obj, out_info, depth, type);
		out.clear();
		for (i = out_info.begin(); i != out_info.end(); ++i) {
			out.add_name(i->name, 0.0);
		}
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

	soar_interface *si;
	svs_state      *state;
	controller     *ctrl;
	objective      *obj;
	wme            *stepwme;
	int             step;
	bool            broken;
	namedvec        out;
};

command *_make_seek_command_(svs_state *state, Symbol *root) {
	return new seek_command(state, root);
}

class random_control_command : public command {
public:
	random_control_command(svs_state *state, Symbol *root)
	: command(state, root), state(state)
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	string description() {
		return string("random control");
	}
	
	bool update() {
		if (changed()) {
			wme *outputs_wme;
			output_info_vec out_info;
			if (!si->find_child_wme(get_root(), "outputs", outputs_wme) ||
			    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
			    !parse_wm_output_struct(si, si->get_wme_val(outputs_wme), out_info))
			{
				set_status("missing or invalid outputs specification");
				return false;
			}
			min.resize(out_info.size());
			max.resize(out_info.size());
			out.clear();
			for (int i = 0; i < out_info.size(); ++i) {
				min[i] = out_info[i].min;
				max[i] = out_info[i].max;
				out.add_name(out_info[i].name, 0.0);
			}
		}
		
		out.vals.randomize(min, max);
		if (state->get_level() == 0) {
			state->get_svs()->set_next_output(out);
		}
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	soar_interface  *si;
	svs_state       *state;
	namedvec         out;
	floatvec         min, max;
};

command *_make_random_control_command_(svs_state *state, Symbol *root) {
	return new random_control_command(state, root);
}

class manual_control_command : public command {
public:
	manual_control_command(svs_state *state, Symbol *root) : command(state, root), state(state) {
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
		
		namedvec out;
		while (getline(ss1, line)) {
			stringstream ss2(line);
			string label;
			float val;
			if (!(ss2 >> label) || !(ss2 >> val)) {
				set_status("input error");
				return false;
			}
			out.add_name(label, val);
		}
		
		if (state->get_level() == 0) {
			state->get_svs()->set_next_output(out);
		}
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	svs_state *state;
	ipcsocket *sock;
};

command *_make_manual_control_command_(svs_state *state, Symbol *root) {
	return new manual_control_command(state, root);
}
