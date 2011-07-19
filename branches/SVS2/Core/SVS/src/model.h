#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include "scene.h"
#include "env.h"
#include "soar_interface.h"
#include "common.h"

class model {
public:
	virtual bool predict(const floatvec &x, floatvec &y) = 0;
	
	/* Add a training example to a learning model. The training
	   example is encoded as a pair of vectors (x, y) where
	   x = [prev_state, output], y = [next_state]
	*/
	virtual void learn(const floatvec &x, const floatvec &y, float dt) {}

	virtual void get_slots(std::vector<std::string> &inputs, std::vector<std::string> &outputs) const = 0;
	
	virtual void printinfo() const = 0;
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
	
	void printinfo() const {
		std::cout << "multi model" << std::endl;
	}
	
	void get_slots(std::vector<std::string> &inputs, std::vector<std::string> &outputs) const {}

	bool assign_model(const std::string &name, 
	                  const slot_prop_map &in_slot_props, 
	                  const slot_prop_map &out_slot_props) 
	{
		model_config config;
		std::vector<std::string>::iterator i;
		
		if (!map_get(model_db, name, config.mdl)) {
			return false;
		}
		config.in_slot_props = in_slot_props;
		config.out_slot_props = out_slot_props;
		active_models.push_back(config);
		
		return true;
	}

	void add_model(const std::string &name, model *m) {
		model_db[name] = m;
	}
	
	void set_indexes(const std::vector<std::string> &props) {
		std::list<model_config>::iterator i;
		std::vector<std::string> in_slots, out_slots;
		
		for (i = active_models.begin(); i != active_models.end(); ++i) {
			int index = -1;
			i->mdl->get_slots(in_slots, out_slots);
			i->in_indexes.clear();
			if (!assign_slot_indexes(props, in_slots, i->in_slot_props, i->in_indexes)) {
				i->error = true;
				continue;
			}
			if (!assign_slot_indexes(props, out_slots, i->out_slot_props, i->out_indexes)) {
				i->error = true;
				continue;
			}
			i->error = false;
		}
	}
	
private:
	bool assign_slot_indexes(const std::vector<std::string> props,
							 const std::vector<std::string> &slots, 
							 const slot_prop_map &assignments,
							 std::vector<int> &indexes)
	{
		std::vector<std::string> fields;
		std::string name;
		std::vector<std::string>::const_iterator i;
		int index;

		for (i = slots.begin(); i != slots.end(); ++i) {
			if (!map_get(assignments, *i, name)) {
				return false;
			}
			index = -1;
			for (int j = 0; j < props.size(); ++j) {
				if (props[j] == name) {
					index = j;
					break;
				}
			}
			if (index < 0) {
				return false;
			}
			indexes.push_back(index);
		}
		return true;
	}

	struct model_config {
		slot_prop_map in_slot_props;
		slot_prop_map out_slot_props;
		std::vector<int> in_indexes;
		std::vector<int> out_indexes;
		bool error;
		model *mdl;
	};
	
	std::list<model_config> active_models;
	
	std::map<std::string, model*> model_db;
};

model *parse_model_struct(soar_interface *si, Symbol *root, std::string &name);


#endif
