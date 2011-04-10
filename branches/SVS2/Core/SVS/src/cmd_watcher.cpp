#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <limits>

#include "cmd_watcher.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "ipcsocket.h"
#include "filters/gen_filter.h"

using namespace std;

bool is_reserved_param(const string &name) {
	return name == "result" || name == "parent";
}

/* Remove weird characters from string */
void cleanstring(string &s) {
	string::iterator i;
	for (i = s.begin(); i != s.end();) {
		if (!isalnum(*i) && *i != '.' && *i != '-' && *i != '_') {
			i = s.erase(i);
		} else {
			++i;
		}
	}
}

class cmd_base : public cmd_watcher {
public:
	cmd_base(svs_state *state, sym_hnd cmd_root)
	: state(state), si(state->get_soar_interface()), cmd_root(cmd_root), result_wme(NULL), subtree_size(0), max_time_tag(0)
	{ }
	
	void set_result(const string &r) {
		string current;
		if (result_wme) {
			if (si->get_val(si->get_wme_val(result_wme), current)) {
				if (r == current) {
					return;
				}
			}
			si->remove_wme(result_wme);
		}
		result_wme = si->make_wme(cmd_root, "result", r);
	}
	
	bool cmd_changed() {
		tc_num tc;
		bool changed;
		stack< sym_hnd > to_process;
		wme_list childs;
		wme_list::iterator i;
		sym_hnd parent, v;
		int new_subtree_size = 0, tt;
		string attr;
	
		tc = si->new_tc_num();
		changed = false;
		
		to_process.push(cmd_root);
		while (!to_process.empty()) {
			parent = to_process.top();
			to_process.pop();
			
			si->get_child_wmes(parent, childs);
			for (i = childs.begin(); i != childs.end(); ++i) {
				if (parent == cmd_root) {
					if (si->get_val(si->get_wme_attr(*i), attr) && attr == "result") {
						/* result wmes are added by svs */
						continue;
					}
				}
				v = si->get_wme_val(*i);
				tt = si->get_timetag(*i);
				new_subtree_size++;
				
				if (tt > max_time_tag) {
					changed = true;
					max_time_tag = tt;
				}
	
				if (si->is_identifier(v) && si->get_tc_num(v) != tc) {
					si->set_tc_num(v, tc);
					to_process.push(v);
				}
			}
		}
	
		if (new_subtree_size != subtree_size) {
			changed = true;
			subtree_size = new_subtree_size;
		}
	
		return changed;
	}
	
	bool get_str_param(const string &name, string &val) {
		wme_list children;
		wme_list::iterator i;
		string attr, v;
		
		si->get_child_wmes(cmd_root, children);
		for(i = children.begin(); i != children.end(); ++i) {
			if (si->get_val(si->get_wme_attr(*i), attr)) {
				if (name != attr) {
					continue;
				}
				if (si->get_val(si->get_wme_val(*i), v)) {
					val = v;
					return true;
				}
			}
		}
		return false;
	}
	
	/* Filters are specified in WM like this:
	
	   (<cmd> ^filtername1 <f1>)
	   (<f1> ^_named-param1 <f2> 
	         ^_named-param1 node_name1
	         ^filtername2 <f3>)
	   (<f2> ^filtername3 <f4>)
	   (<f3> ^filtername4 node_name2)
	   ...
	   
	   Parameters can be named or anonymous ("" will be the param name), and
	   filters can either be identifiers, in which case substructure will be
	   parsed recursively, or string constants that specify node filters.
	*/
	filter* make_filter() {
		wme_list children;
		wme_list::iterator i;
		string attr;
		
		si->get_child_wmes(cmd_root, children);
		for (i = children.begin(); i != children.end(); ++i) {
			if (si->get_val(si->get_wme_attr(*i), attr)) {
				if (!is_reserved_param(attr)) {
					return rec_make_filter(*i);
				}
			}
		}
		return NULL;
	}

protected:
	svs_state       *state;
	sym_hnd         cmd_root;
	soar_interface* si;

private:
	filter* rec_make_filter(wme_hnd cmd) {
		string filter_name, node_name;
		filter_params p;
		
		if (!si->get_val(si->get_wme_attr(cmd), filter_name)) {
			return NULL;
		}
		
		if (!get_filter_params(si->get_wme_val(cmd), p)) {
			return NULL;
		}
		
		return ::make_filter(filter_name, p);
	}
	
	filter* make_node_filter(string name) {
		sg_node *n;
		
		n = state->get_scene()->get_node(name);
		if (!n) {
			return NULL;
		}
		return new const_node_filter(n);
	}
	
	bool get_filter_params(sym_hnd id, filter_params &p) {
		wme_list childs, param_childs;
		wme_list::iterator i;
		filter *f;
		string attr, node_name, param_name;
		bool fail;
	
		if (!si->is_identifier(id)) {
			return false;
		}
		
		si->get_child_wmes(id, childs);
		fail = false;
		for (i = childs.begin(); i != childs.end(); ++i) {
			if (!si->get_val(si->get_wme_attr(*i), attr)) {
				fail = true;
				break;
			}
			if (si->get_val(si->get_wme_val(*i), node_name)) {  // scene graph node
				f = make_node_filter(node_name); 
			} else {   // another filter
				si->get_child_wmes(si->get_wme_val(*i), param_childs);
				if (param_childs.size() != 1) {
					fail = true;
					break;
				}
				f = rec_make_filter(param_childs.front());
			}
			if (!f) {
				fail = true;
				break;
			}
			p.insert(pair<string,filter*>(attr, f));
		}
		
		if (fail) {
			filter_params::iterator j;
			
			for (j = p.begin(); j != p.end(); ++j) {
				delete j->second;
			}
			p.clear();
			return false;
		}
		return true;
	}

	wme_hnd result_wme;
	int     subtree_size;
	int     max_time_tag;
};

class extract_cmd_watcher : public cmd_base, public filter_listener {
public:
	extract_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), dirty(true), result_filter(NULL)
	{}
	
	~extract_cmd_watcher() {
		if (result_filter) {
			delete result_filter;
		}
	}
	
	void update(filter *f) {
		dirty = true;
	}
	
	bool update_result() {
		filter_result *r;
		
		if (cmd_changed()) {
			if (result_filter) {
				delete result_filter;
			}
			result_filter = make_filter();
			if (!result_filter) {
				set_result("command error");
				return false;
			}
			result_filter->listen(this);
			dirty = true;
		}
		
		if (result_filter && dirty) {
			dirty = false;
			if (!(r = result_filter->get_result())) {
				set_result(result_filter->get_error());
				return false;
			} else {
				set_result(r->get_string());
			}
		}
		return true;
	}
	
	bool early() { return false; }
	
private:
	filter* result_filter;
	bool    dirty;
};

class gen_cmd_watcher : public cmd_base {
public:
	gen_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root)
	{}
	
	bool update_result() {
		return false;
	}
	
	bool early() { return false; }
	
private:
	string    parent;
	bool      dirty;
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

class ctrl_cmd_watcher : public cmd_base {
public:
	ctrl_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), step(0), stepwme(NULL), broken(false)
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
		
		if (!si->find_child_wme(cmd_root, "outputs", outputs_wme) ||
		    !si->is_identifier(si->get_wme_val(outputs_wme)) ||
		    !si->find_child_wme(cmd_root, "objective", objective_wme) ||
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
		stepwme = si->make_wme(cmd_root, "step", step);
	}

	controller *ctrl;
	wme_hnd   stepwme;
	int       step;
	bool      broken;
};

cmd_watcher* make_cmd_watcher(svs_state *state, wme_hnd w) {
	sym_hnd id;
	string attr;
	soar_interface *si = state->get_soar_interface();
	
	if (!si->get_val(si->get_wme_attr(w), attr)) {
		return NULL;
	}
	id = si->get_wme_val(w);
	if (!si->is_identifier(id)) {
		return NULL;
	}
	
	if (attr == "extract") {
		return new extract_cmd_watcher(state, id);
	} else if (attr == "generate") {
		return new gen_cmd_watcher(state, id);
	} else if (attr == "control") {
		return new ctrl_cmd_watcher(state, id);
	}
	return NULL;
}
