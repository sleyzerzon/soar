#include <iostream>
#include <string>
#include <limits>
#include "command.h"
#include "svs.h"
#include "env.h"
#include "scene.h"
#include "model.h"

using namespace std;

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
bool parse_output_desc_struct(soar_interface *si, Symbol *root, env_output_desc &desc) {
	wme_list dim_wmes;
	wme_list::iterator i;
	string dim_name;
	Symbol *dim_id;
	wme *min_wme, *max_wme, *inc_wme;
	dim_desc d;
	
	if (!si->is_identifier(root)) {
		return false;
	}
	si->get_child_wmes(root, dim_wmes);
	for (i = dim_wmes.begin(); i != dim_wmes.end(); ++i) {
		dim_id = si->get_wme_val(*i);
		if (si->get_val(si->get_wme_attr(*i), dim_name)  &&
		    si->is_identifier(dim_id)                    &&
		    si->find_child_wme(dim_id, "min", min_wme)   &&
		    si->get_val(si->get_wme_val(min_wme), d.min) &&
		    si->find_child_wme(dim_id, "max", max_wme)   &&
		    si->get_val(si->get_wme_val(max_wme), d.max) &&
		    si->find_child_wme(dim_id, "inc", inc_wme)   &&
		    si->get_val(si->get_wme_val(inc_wme), d.inc))
		{
			desc[dim_name] = d;
		}
	}
	return true;
}

class objective {
public:
	virtual double eval(scene *scn) = 0;
};

vec3 calc_centroid(const ptlist &pts) {
	ptlist::const_iterator i;
	int d;
	vec3 c;
	
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
		vec3 c1, c2;
		
		if ((n1 = scn->get_node(obj1)) == NULL ||
		    (n2 = scn->get_node(obj2)) == NULL)
		{
			return numeric_limits<double>::infinity();
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

class controller {
public:
	controller(model *mdl, objective *obj, const env_output_desc &outdesc)
	: mdl(mdl), obj(obj), outdesc(outdesc), step(0) {}

	/* Don't forget to make this a PID controller later */
	bool seek(scene *scn, env_output &bestout) {
		env_output out(outdesc);
		double eval, best = numeric_limits<double>::infinity();
		flat_scene flat(scn);
		vector<double> origvals = flat.vals;
		scene next(*scn);
		bool found = false;
		
		while (true) {
			/* this part is kind of a hack to avoid expensive copying */
			flat.vals = origvals;
			if (!mdl->predict(flat, out)) {
				return false;
			}
			flat.update_scene(&next);
			eval = obj->eval(&next);
			if (eval < best) {
				found = true;
				bestout = out;
				best = eval;
			}
			if (!out.increment()) {
				break;
			}
		}
		step++;
		return found;
	}
	
private:
	model *mdl;
	objective *obj;
	env_output_desc outdesc;
	int step;
};

class control_command : public command {
public:
	control_command(svs_state *state, Symbol *root)
	: state(state), root(root), utils(state, root), 
	  si(state->get_svs()->get_soar_interface()), step(0), 
	  stepwme(NULL), broken(false), 
	  mdl(NULL), ctrl(NULL), obj(NULL)
	{
		update_step();
	}
	
	~control_command() {
		cleanup();
	}
	
	bool update_result() {
		env_output out;

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
		update_step();
		return true;
	}
	
	bool early() { return true; }
	
private:
	/* Assumes this format:
	   C1 ^outputs ( ... )
	      ^objective ( ... )
	*/
	bool parse_cmd() {
		env_output_desc desc;
		wme *outputs_wme, *objective_wme, *model_wme;
		
		cleanup();
		if (!si->find_child_wme(root, "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !parse_output_desc_struct(si, si->get_wme_val(outputs_wme), desc))
		{
			utils.set_result("missing or invalid outputs specification");
			return false;
		}
		if (!si->find_child_wme(root, "objective", objective_wme) ||
			!si->is_identifier(si->get_wme_val(objective_wme)) ||
			(obj = parse_obj_struct(si, si->get_wme_val(objective_wme))) == NULL)
		{
			utils.set_result("missing or invalid objective");
			return false;
		}
		if (!si->find_child_wme(root, "model", model_wme) ||
		    (mdl = parse_model_struct(si, si->get_wme_val(model_wme))) == NULL)
		{
			utils.set_result("missing or invalid model");
			return false;
		}
		state->get_svs()->register_model(mdl);
		ctrl = new controller(mdl, obj, desc);
		return true;
	}

	void cleanup() {
		if (mdl) {
			state->get_svs()->unregister_model(mdl);
			delete mdl;
			mdl = NULL;
		}
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
	model          *mdl;
	objective      *obj;
	wme            *stepwme;
	int             step;
	bool            broken;
};

command *_make_control_command_(svs_state *state, Symbol *root) {
	return new control_command(state, root);
}
