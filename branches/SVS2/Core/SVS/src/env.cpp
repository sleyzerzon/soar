#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include "env.h"

using namespace std;
using namespace arma;

bool out_dim_desc::operator<(const out_dim_desc &other) const {
	return name < other.name;
}

bool out_dim_desc::operator!=(const out_dim_desc &other) const {
	return name != other.name;
}

bool out_dim_desc::operator==(const out_dim_desc &other) const {
	return name == other.name;
}

output::output() : desc(NULL) {}

output::output(const outdesc *d)
: desc(d), vals(d->size())
{
	reset();
}

output::output(const output &other) 
: vals(other.vals), desc(other.desc)
{ }

double output::get(const string &dim) const {
	outdesc::const_iterator i;
	int j;
	
	for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
		if (i->name == dim) {
			return vals[j];
		}
	}
	assert(false);
}

void output::set(const string &dim, double val) {
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

bool output::next() {
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

void output::reset() {
	outdesc::const_iterator i;
	int j;
	for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
		vals[j] = i->min;
	}
}

string output::serialize() const {
	outdesc::const_iterator i;
	int j;
	stringstream ss;
	
	for (i = desc->begin(), j = 0; i != desc->end(); ++i, ++j) {
		ss << i->name << " " << vals[j] << endl;
	}
	return ss.str();
}

void output::operator=(const output &o) {
	desc = o.desc;
	vals = o.vals;
}

int output::size() const {
	if (!desc) {
		return 0;
	}
	return desc->size();
}

output random_out(const outdesc *d) {
	output r(d);
	outdesc::const_iterator i;
	int j;
	
	for (i = d->begin(), j = 0; i != d->end(); ++i, ++j) {
		r.vals[j] = i->min + ((i->max - i->min) * rand()) / RAND_MAX;
	}
	return r;
}

trajectory::trajectory(const output &out) 
: length(1), desc(out.desc)
{
	t.push_back(out);
}

trajectory::trajectory(int length, const outdesc *d) 
: length(length), desc(d)
{
	output temp(desc);
	t.reserve(length);
	for(int i = 0; i < length; ++i) {
		t.push_back(temp);
	}
}

void trajectory::from_vec(const vec &traj) {
	int i = 0, k;
	vector<output>::iterator j;
	
	assert(traj.n_elem == desc->size() * length);
	t.reserve(traj.n_elem / desc->size());
	for (j = t.begin(); j != t.end(); ++j) {
		for (k = 0; k < j->vals.size(); ++k, ++i) {
			j->vals[k] = traj(i);
		}
	}
}

void trajectory::reset() {
	vector<output>::iterator i;
	for (i = t.begin(); i != t.end(); ++i) {
		i->reset();
	}
}

bool trajectory::next() {
	vector<output>::iterator i;
	for (i = t.begin(); i != t.end(); ++i) {
		if (i->next()) {
			break;
		}
		i->reset();
	}
}

int trajectory::dof() {
	return length * desc->size();
}
