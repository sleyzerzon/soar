#include "cmd_watcher.h"

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
bool parse_output_desc_struct(soar_interface *si, sym_hnd root, env_output_desc &desc) {
	wme_list dim_wmes;
	wme_list::iterator i;
	string dim_name;
	sym_hnd dim_id;
	wme_hnd min_wme, max_wme, inc_wme;
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
		    si->find_child_wme(dim_id, "min", inc_wme)   &&
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
	double c[3];
	
	for (i = pts.begin(); i != pts.end(); ++i) {
		for (d = 0; d < 3; ++d) {
			c[d] += (*i)[d];
		}
		c[d] /= pts.size();
	}
	return vec3(c[0], c[1], c[2]);
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

		return pow(c1[0]-c2[0], 2) + pow(c1[1]-c2[1], 2) + pow(c1[2]-c2[2], 2);
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
objective *parse_obj_struct(soar_interface *si, sym_hnd root) {
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
	env_output *seek(scene *scn) {
		env_output curr(outdesc);
		env_output *bestout = NULL;
		double eval, best;
		scene *next;
		
		while (curr.increment()) {
			next = new scene(*scn);
			mdl->predict(next, curr);
			eval = obj->eval(next);
			if (!bestout || eval > best) {
				if (!bestout) {
					delete bestout;
				}
				bestout = new env_output(curr);
				best = eval;
			}
			delete next;
		}
		step++;
		return bestout;
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
	: state(state), utils(state, root), step(0), stepwme(NULL), broken(false)
	{
		wme_hnd w;
		int r;
		
		if (!parse_cmd()) {
			broken = true;
			return;
		}
		update_step();
	}
	
	bool update_result() {
		env_output *out;
		if (broken) {
			return false;
		}
		out = ctrl->seek(state->get_scene());
		if (state->get_level() == 0) {
			state->get_svs()->get_env()->output(*out);
		}
		// need to update scene with model otherwise
		delete out;
		
		set_result("success");
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
		objective *obj;
		wme_hnd outputs_wme, objective_wme;
		
		if (!si->find_child_wme(root, "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !si->find_child_wme(root, "objective", objective_wme) ||
			!si->is_identifier(si->get_wme_val(objective_wme)))
		{
			return false;
		}
		if ((obj = parse_obj_struct(si, si->get_wme_val(objective_wme))) == NULL) {
			return false;
		}
		if (!parse_output_desc_struct(si, si->get_wme_val(outputs_wme), desc)) {
			return false;
		}
		ctrl = new controller(state->get_svs()->get_model(), obj, desc);
	}

	void update_step() {
		if (stepwme)
			si->remove_wme(stepwme);
		stepwme = si->make_wme(root, "step", step);
	}

private:
	cmd_utils   utils;
	svs_state  *state;
	Symbol     *root;
	controller *ctrl;
	wme_hnd     stepwme;
	int         step;
	bool        broken;
};
