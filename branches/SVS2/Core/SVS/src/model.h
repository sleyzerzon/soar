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
	virtual void learn(const floatvec &x, const floatvec &y, float dt) {}
	virtual void get_slots(std::vector<std::string> &inputs, std::vector<std::string> &outputs) const = 0;
	virtual std::string get_type() const = 0;
	virtual int get_input_size() const = 0;
	virtual int get_output_size() const = 0;
	
	virtual float test(const floatvec &x, const floatvec &y) {
		floatvec py(y.size());
		if (!predict(x, py)) {
			return -1.0;
		} else {
			return py.dist(y);
		}
	}
};

class multi_model : public model {
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
			y.multi_set(i->out_indexes, yp);
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
			i->mdl->learn(xp, yp, dt);
		}
	}
	
	std::string get_type() const {
		return "multi-model";
	}
	
	float test(const floatvec &x, const floatvec &y) {
		float s = 0.0;
		std::list<model_config>::iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			std::cout << i->name << "(" << i->mdl->get_type() << "): ";
			if (i->error) {
				std::cout << "NP (assign error)" << std::endl;
				continue;
			}
			floatvec xp = x.slice(i->in_indexes), yp = y.slice(i->out_indexes);
			float d = i->mdl->test(xp, yp);
			if (d < 0.) {
				std::cout << "NP" << std::endl;
				s = -1.;
			} else if (s >= 0.) {
				std::cout << d << std::endl;
				s += d;
			}
		}
		if (s >= 0.) {
			return s / active_models.size();
		}
		return -1.;
	}
	
	/*
	 Yes, these are slow. But they're used very infrequently, if at all.
	*/
	int get_input_size() const {
		std::vector<std::string> in, out;
		get_slots(in, out);
		return in.size();
	}
	
	int get_output_size() const {
		std::vector<std::string> in, out;
		get_slots(in, out);
		return out.size();
	}
	
	void get_slots(std::vector<std::string> &inputs, std::vector<std::string> &outputs) const {
		std::set<std::string> all_in, all_out;
		std::list<model_config>::const_iterator i;
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			std::vector<std::string> in, out;
			i->mdl->get_slots(in, out);
			copy(in.begin(), in.end(), std::inserter(all_in, all_in.begin()));
			copy(out.begin(), out.end(), std::inserter(all_out, all_out.begin()));
		}
		copy(all_in.begin(), all_in.end(), back_inserter(inputs));
		copy(all_out.begin(), all_out.end(), back_inserter(outputs));
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
			return false;
		}
		config.mdl->get_slots(in_slots, out_slots);
		
		for (i = in_slots.begin(); i != in_slots.end(); ++i) {
			if (in_slot_props.find(*i) == in_slot_props.end()) {
				return false;
			}
		}
		for (i = out_slots.begin(); i != out_slots.end(); ++i) {
			if (out_slot_props.find(*i) == out_slot_props.end()) {
				return false;
			}
		}
		
		config.in_slot_props = in_slot_props;
		config.out_slot_props = out_slot_props;
		
		if (!assign_slot_indexes(in_slots, in_slot_props, config.in_indexes) ||
		    !assign_slot_indexes(out_slots, out_slot_props, config.out_indexes))
		{
			config.error = true;
		} else {
			config.error = false;
		}
		
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
			if (!assign_slot_indexes(in_slots, i->in_slot_props, i->in_indexes)) {
				i->error = true;
				continue;
			}
			i->out_indexes.clear();
			if (!assign_slot_indexes(out_slots, i->out_slot_props, i->out_indexes)) {
				i->error = true;
				continue;
			}
			i->error = false;
		}
	}
	
	bool assign_slot_indexes(const std::vector<std::string> &slots, 
							 const slot_prop_map &assignments,
							 std::vector<int> &indexes)
	{
		std::vector<std::string> fields;
		std::string name;
		std::vector<std::string>::const_iterator i;
		int index;

		for (i = slots.begin(); i != slots.end(); ++i) {
			if (!map_get(assignments, *i, name)) {
				std::cout << "SLOT NOT ASSIGNED " << *i << std::endl;
				return false;
			}
			index = -1;
			for (int j = 0; j < prop_vec.size(); ++j) {
				if (prop_vec[j] == name) {
					index = j;
					break;
				}
			}
			if (index < 0) {
				std::cout << "PROPERTY NOT FOUND " << name << std::endl;
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

model *parse_model_struct(soar_interface *si, Symbol *root, std::string &name);


#endif
