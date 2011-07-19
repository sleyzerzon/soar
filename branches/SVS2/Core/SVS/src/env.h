#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <vector>
#include <sstream>
#include "common.h"

class out_dim_desc {
public:
	std::string name;
	double min;
	double max;
	double inc;
	
	bool operator< (const out_dim_desc &other) const { return name  < other.name; }
	bool operator==(const out_dim_desc &other) const { return name == other.name; }
	bool operator!=(const out_dim_desc &other) const { return name != other.name; }
};

typedef std::vector<out_dim_desc> outdesc;

class output {
public:
	output() : desc(NULL) {}
	
	output(const outdesc *d): desc(d), vals(d->size()) {
		reset();
	}

	output(const output &other): vals(other.vals), desc(other.desc) {}

	
	double get(const std::string &dim) const {
		outdesc::const_iterator i;
		int j;
		
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			if (i->name == dim) {
				return vals[j];
			}
		}
		assert(false);
	}
	
	void set(const std::string &dim, double val) {
		outdesc::const_iterator i;
		int j;
		
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			if (i->name == dim) {
				vals[j] = val;
				return;
			}
		}
		assert(false);
	}
	
	int size() const {
		return vals.size();
	}
	
	void reset() {
		outdesc::const_iterator i;
		int j;
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			vals[j] = i->min;
		}
	}
	
	bool next() {
		outdesc::const_iterator i;
		int j;
		
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			vals[j] += i->inc;
			if (vals[j] <= i->max) {
				return true;
			} else {
				vals[j] = i->min;  // roll over and move on to the next value
			}
		}
		return false;
	}
	
	std::string serialize() const {
		outdesc::const_iterator i;
		int j;
		std::stringstream ss;
		
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			ss << i->name << " " << vals[j] << std::endl;
		}
		return ss.str();
	}
	
	void operator=(const output &o) {
		desc = o.desc;
		vals = o.vals;
	}
	
	void randomize() {
		outdesc::const_iterator i;
		int j;
		
		for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
			vals[j] = i->min + ((i->max - i->min) * rand()) / RAND_MAX;
		}
	}
	
	void get_names(std::vector<std::string> &names) const {
		outdesc::const_iterator i;
		for (i = desc->begin(); i != desc->end(); ++i) {
			names.push_back(i->name);
		}
	}
	
public:
	const outdesc *desc;
	floatvec vals;
};

class trajectory {
public:
	trajectory(const output &out) : length(1), desc(out.desc) {
		t.push_back(out);
	}
	
	trajectory(int length, const outdesc *d) : length(length), desc(d) {
		output temp(desc);
		t.reserve(length);
		for(int i = 0; i < length; ++i) {
			t.push_back(temp);
		}
	}
	
	void from_vec(const floatvec &v) {
		int i = 0;
		std::vector<output>::iterator j;
		
		assert(v.size() == desc->size() * length);
		t.reserve(v.size() / desc->size());
		for (j = t.begin(); j != t.end(); ++j) {
			j->vals = v.slice(i, i + desc->size());
			i += desc->size();
		}
	}
	
	void reset() {
		std::vector<output>::iterator i;
		for (i = t.begin(); i != t.end(); ++i) {
			i->reset();
		}
	}
	
	bool next() {
		std::vector<output>::iterator i;
		for (i = t.begin(); i != t.end(); ++i) {
			if (i->next()) {
				return true;
			}
			i->reset();
		}
		return false;
	}
	
	int dof() {
		return length * desc->size();
	}
	
	const outdesc *desc;
	int length;
	std::vector<output> t;
};

#endif
