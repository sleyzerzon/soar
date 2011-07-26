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

class output_incr {
public:
	output_incr(const output_info_vec &dims) : dims(dims), current(dims.size()) {
		reset();
	}

	output_incr(const output_incr &other): current(other.current), dims(other.dims) {}
	

	
	void reset() {
		output_info_vec::const_iterator i;
		int j;
		for (i = dims.begin(), j = 0; i != dims.end(); ++i, ++j) {
			current[j] = i->min;
		}
	}
	
	bool next() {
		output_info_vec::const_iterator i;
		int j;
		
		for (i = dims.begin(), j = 0; i != dims.end(); ++i, ++j) {
			current[j] += i->inc;
			if (current[j] <= i->max) {
				return true;
			} else {
				current[j] = i->min;  // roll over and move on to the next value
			}
		}
		return false;
	}
	
	void get_current(floatvec &v) const {
		v = current;
	}
	
	int size() const {
		return current.size();
	}
	
private:
	output_info_vec dims;
	floatvec current;
};

typedef vector<floatvec> trajectory;

class traj_incr {
public:
	traj_incr() : len(0) {}
	
	traj_incr(int len, const output_incr &prototype)
	: len(len), current(len, prototype)
	{
		reset();
	}
	
	void reset() {
		std::vector<output_incr>::iterator i;
		for (i = current.begin(); i != current.end(); ++i) {
			i->reset();
		}
	}
	
	bool next() {
		std::vector<output_incr>::iterator i;
		for (i = current.begin(); i != current.end(); ++i) {
			if (i->next()) {
				return true;
			}
			i->reset();
		}
		return false;
	}
	
	void get_current(trajectory &t) const {
		std::vector<output_incr>::const_iterator i;
		trajectory::iterator j;
		
		if (len == 0) {
			t.clear();
			return;
		}
		t.resize(current.size());
		for (i = current.begin(), j = t.begin(); i != current.end(); ++i, ++j) {
			i->get_current(*j);
		}
	}
	
	int dof() {
		if (len == 0) {
			return 0;
		}
		return len * current.front().size();
	}

private:
	int len;
	std::vector<output_incr> current;
};

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


class objective {
public:
	virtual float eval(scene &scn) const = 0;
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

/* this class is a little dirty to avoid as much allocation and copying as possible */
class traj_eval {
public:
	traj_eval(int outsize, model *m, objective *obj, const scene &init)
	: outsize(outsize), obj(obj), scn(init)
	{
		mdl = dynamic_cast<multi_model*>(m);
		assert(mdl);
		scn.get_properties(initvals);
	}

	/* version to be used in incremental search */
	bool eval(const trajectory &traj, float &value) {
		if (traj.size() > 0) {
			trajectory::const_iterator i;
			floatvec x(initvals.size() + outsize), y = initvals;
			for (i = traj.begin(); i != traj.end(); ++i) {
				x.combine(y, *i);
				if (!mdl->predict(x, y)) {
					return false;
				}
			}
			scn.set_properties(y);
		}
		value = obj->eval(scn);
		return true;
	}
	
	/* version to be used in Nelder-Mead search */
	bool eval(const floatvec &v, float &value) {
		assert(v.size() % outsize == 0);
		if (t.size() * outsize != v.size()) {
			t.resize(v.size() / outsize, floatvec(outsize));
		}
		
		int offset = 0;
		trajectory::iterator i;
		for (i = t.begin(); i != t.end(); ++i) {
			for (int j = 0; j < outsize; ++j) {
				(*i)[j] = v[offset + j];
			}
			offset += outsize;
		}
		return eval(t, value);
	}
	
private:
	multi_model   *mdl;
	objective     *obj;
	int            outsize;   // dimensionality of output
	trajectory     t;         // cached to prevent repeated memory allocation
	scene          scn;       // copy of initial scene to be modified after prediction
	floatvec       initvals;  // flattened values of initial scene
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
	return true;
}

class controller {
public:
	controller(svs *svsp, objective *obj, const output_info_vec &out_info, int depth, string type)
	: svsp(svsp), obj(obj), out_info(out_info), step(0), depth(depth), type(type) 
	{
		int i, j;
		min.resize(depth * out_info.size());
		max.resize(depth * out_info.size());
		for (i = 0; i < depth; ++i) {
			for (j = 0; j < out_info.size(); ++j) {
				min[i * depth + j] = out_info[j].min;
				max[i * depth + j] = out_info[j].max;
			}
		}
	}

	/* Don't forget to make this a PID controller later */
	bool seek(scene *scn, floatvec &bestout) {
		if (type == "simplex") {
			return simplex_seek(scn, bestout);
		}
		return naive_seek(scn, bestout);
	}
	
	bool naive_seek(scene *scn, floatvec &bestout) {
		float val, best;
		bool found = false;
		model *mdl = svsp->get_model();
		
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(out_info.size(), mdl, obj, *scn);
		traj_incr incr(depth, output_incr(out_info));
		trajectory t;
		
		while (true) {
			incr.get_current(t);
			if (!evaluator.eval(t, val)) {
				return false;
			}
			if (!found || val < best) {
				found = true;
				bestout = t.front();
				best = val;
			}
			if (!incr.next()) {
				break;
			}
		}
		step++;
		return found;
	}
	
	bool simplex_seek(scene *scn, floatvec &bestout) {
		model *mdl = svsp->get_model();
		if (!mdl) {
			return false;
		}
		
		traj_eval evaluator(out_info.size(), mdl, obj, *scn);
		floatvec best(min.size());
		if (!nelder_mead_constrained(min, max, best, evaluator)) {
			return false;
		}
		bestout = best.slice(0, out_info.size());
		return true;
	}
	
private:
	svs             *svsp;
	objective       *obj;
	output_info_vec  out_info;
	floatvec         min, max;   // for Nelder-Mead
	int              step;
	int              depth;
	string           type;
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
		
		if (!ctrl->seek(state->get_scene(), out.vals)) {
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
		sock = new ipcsocket("ctrl", false);
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
