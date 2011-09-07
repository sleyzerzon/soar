#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <map>
#include <vector>
#include "scene.h"
#include "soar_interface.h"
#include "common.h"

class model {
public:
	virtual bool predict(const floatvec &x, floatvec &y) = 0;
	virtual void get_slots(std::vector<std::string> &inputs, std::vector<std::string> &outputs) const = 0;
	virtual std::string get_type() const = 0;
	virtual int get_input_size() const = 0;
	virtual int get_output_size() const = 0;
	
	virtual void learn(const floatvec &x, const floatvec &y, float dt) {}
	
	virtual float test(const floatvec &x, const floatvec &y) {
		floatvec py(y.size());
		if (!predict(x, py)) {
			return -1.0;
		} else {
			return py.dist(y);
		}
	}
};

/*
 This class keeps track of how to combine several distinct models to make
 a single prediction.  Its main responsibility is to map the values from
 a single scene vector to the vectors that the individual models expect
 as input, and then map the values of the output vectors from individual
 models back into a single output vector for the entire scene. The mapping
 is specified by the Soar agent at runtime using the assign-model command.
 
 SVS uses a single instance of this class to make all its predictions. I
 may turn this into a singleton in the future.
*/
class multi_model {
public:
	typedef std::map<std::string, std::string> slot_prop_map;

	multi_model() {}
	
	~multi_model() {
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			delete *i;
		}
	}
	
	bool predict(const floatvec &x, floatvec &y) {
		std::list<model_config*>::const_iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			floatvec xp(x.slice(cfg->in_indexes)), yp(y.slice(cfg->out_indexes));
			if (!cfg->mdl->predict(xp, yp)) {
				return false;
			}
			y.set_indices(cfg->out_indexes, yp);
		}
		return true;
	}
	
	void learn(const floatvec &x, const floatvec &y, float dt) {
		std::list<model_config*>::iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			floatvec xp = x.slice(cfg->in_indexes), yp = y.slice(cfg->out_indexes);
			/*
			float error = cfg->mdl->test(xp, yp);
			if (error < 0. || error > 1.0e-8) {
				cfg->mdl->learn(xp, yp, dt);
			}
			*/
			cfg->mdl->learn(xp, yp, dt);
		}
	}
	
	float test(const floatvec &x, const floatvec &y) {
		float s = 0.0;
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			model_config *cfg = *i;
			std::cerr << cfg->name << "(" << cfg->mdl->get_type() << "): ";
			floatvec xp = x.slice(cfg->in_indexes), yp = y.slice(cfg->out_indexes);
			float d = cfg->mdl->test(xp, yp);
			if (d < 0.) {
				std::cerr << "NP" << std::endl;
				s = -1.;
			} else if (s >= 0.) {
				std::cerr << d << std::endl;
				s += d;
			}
		}
		if (s >= 0.) {
			return s / active_models.size();
		}
		return -1.;
	}
	
	bool assign_model(const std::string &name, 
	                  const slot_prop_map &in_slot_props, 
	                  const slot_prop_map &out_slot_props) 
	{
		std::vector<std::string> in_slots, out_slots;
		std::vector<std::string>::iterator i;
		model_config *cfg = new model_config();
		
		cfg->name = name;
		if (!map_get(model_db, name, cfg->mdl)) {
			std::cerr << "ERROR (assign_model): no model " << name << std::endl;
			delete cfg;
			return false;
		}
		cfg->mdl->get_slots(in_slots, out_slots);
		cfg->in_slot_props = in_slot_props;
		cfg->out_slot_props = out_slot_props;
		
		if (!assign_slot_indexes(in_slots, in_slot_props, cfg->in_indexes, false) ||
		    !assign_slot_indexes(out_slots, out_slot_props, cfg->out_indexes, true))
		{
			delete cfg;
			return false;
		}
		active_models.push_back(cfg);
		return true;
	}

	void unassign_model(const std::string &name) {
		std::list<model_config*>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			if ((**i).name == name) {
				active_models.erase(i);
				return;
			}
		}
	}
	
	void add_model(const std::string &name, model *m) {
		model_db[name] = m;
	}
	
	void set_property_vector(const std::vector<std::string> &props) {
		prop_vec = props;
	}
	
private:
	bool assign_slot_indexes(const std::vector<std::string> &slots, 
							 const slot_prop_map &assignments,
							 std::vector<int> &indexes,
							 bool allow_unassigned)
	{
		std::vector<std::string> fields;
		std::string name;
		std::vector<std::string>::const_iterator i;
		int index;

		for (i = slots.begin(); i != slots.end(); ++i) {
			if (!map_get(assignments, *i, name)) {
				if (allow_unassigned) {
					continue;
				} else {
					std::cerr << "SLOT NOT ASSIGNED " << *i << std::endl;
					return false;
				}
			}
			index = -1;
			for (int j = 0; j < prop_vec.size(); ++j) {
				if (prop_vec[j] == name) {
					index = j;
					break;
				}
			}
			if (index < 0) {
				std::cerr << "PROPERTY NOT FOUND " << name << std::endl;
				return false;
			}
			indexes.push_back(index);
		}
		return true;
	}

	struct model_config {
		std::string name;
		slot_prop_map in_slot_props;
		slot_prop_map out_slot_props;
		std::vector<int> in_indexes;
		std::vector<int> out_indexes;
		model *mdl;
	};
	
	std::list<model_config*>      active_models;
	std::map<std::string, model*> model_db;
	std::vector<std::string>      prop_vec;
};

#endif
