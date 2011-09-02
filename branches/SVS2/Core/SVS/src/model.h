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
	
	bool predict(const floatvec &x, floatvec &y) {
		std::list<model_config>::const_iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			if (i->error) {
				return false;
			}
			floatvec xp(x.slice(i->in_indexes)), yp(y.slice(i->out_indexes));
			if (!i->mdl->predict(xp, yp)) {
				return false;
			}
			y.set_indices(i->out_indexes, yp);
		}
		return true;
	}
	
	void learn(const floatvec &x, const floatvec &y, float dt) {
		std::list<model_config>::iterator i;
		int j;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			if (i->error) {
				continue;
			}
			floatvec xp = x.slice(i->in_indexes), yp = y.slice(i->out_indexes);
			float error = i->mdl->test(xp, yp);
			if (error < 0. || error > 1.0e-8) {
				i->mdl->learn(xp, yp, dt);
			}
		}
	}
	
	float test(const floatvec &x, const floatvec &y) {
		float s = 0.0;
		std::list<model_config>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			std::cerr << i->name << "(" << i->mdl->get_type() << "): ";
			if (i->error) {
				std::cerr << "NP (assign error)" << std::endl;
				continue;
			}
			floatvec xp = x.slice(i->in_indexes), yp = y.slice(i->out_indexes);
			float d = i->mdl->test(xp, yp);
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
		model_config config;
		std::vector<std::string> in_slots, out_slots;
		std::vector<std::string>::iterator i;
		
		config.name = name;
		if (!map_get(model_db, name, config.mdl)) {
			std::cerr << "ERROR (assign_model): no model " << name << std::endl;
			return false;
		}
		config.mdl->get_slots(in_slots, out_slots);
		
		for (i = in_slots.begin(); i != in_slots.end(); ++i) {
			if (in_slot_props.find(*i) == in_slot_props.end()) {
				std::cerr << "ERROR (assign_model): input slot " << *i << std::endl;
				return false;
			}
		}
		for (i = out_slots.begin(); i != out_slots.end(); ++i) {
			if (out_slot_props.find(*i) == out_slot_props.end()) {
				std::cout << "ERROR (assign_model): output slot " << std::endl;
				return false;
			}
		}
		
		config.in_slot_props = in_slot_props;
		config.out_slot_props = out_slot_props;
		
		if (!assign_slot_indexes(in_slots, in_slot_props, config.in_indexes, false) ||
		    !assign_slot_indexes(out_slots, out_slot_props, config.out_indexes, true))
		{
			return false;
		}
		config.error = false;
		active_models.push_back(config);
		return true;
	}

	void unassign_model(const std::string &name) {
		std::list<model_config>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			if (i->name == name) {
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
		reassign_indexes();
	}
	
	
private:
	void reassign_indexes() {
		std::list<model_config>::iterator i;
		
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			std::vector<std::string> in_slots, out_slots;
			int index = -1;
			i->mdl->get_slots(in_slots, out_slots);
			i->in_indexes.clear();
			if (!assign_slot_indexes(in_slots, i->in_slot_props, i->in_indexes, false)) {
				i->error = true;
				continue;
			}
			i->out_indexes.clear();
			if (!assign_slot_indexes(out_slots, i->out_slot_props, i->out_indexes, true)) {
				i->error = true;
				continue;
			}
			i->error = false;
		}
	}
	
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
		bool error;
		model *mdl;
	};
	
	std::list<model_config>       active_models;
	std::map<std::string, model*> model_db;
	std::vector<std::string>      prop_vec;
};

#endif
