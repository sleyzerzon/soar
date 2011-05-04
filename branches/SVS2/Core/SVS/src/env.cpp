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
: desc(d), vals(d->size(), 0.0)
{
	reset();
}

output::output(const output &other) 
: vals(other.vals), desc(other.desc)
{ }

double output::get(const string &dim) const {
	outdesc::const_iterator i;
	vector<double>::const_iterator j;
	
	for (i = desc->begin(), j = vals.begin(); i != desc->end(); ++i, ++j) {
		if (i->name == dim) {
			return *j;
		}
	}
	assert(false);
}

void output::set(const string &dim, double val) {
	outdesc::const_iterator i;
	vector<double>::iterator j;
	
	for (i = desc->begin(), j = vals.begin(); i != desc->end(); ++i, ++j) {
		if (i->name == dim) {
			*j = val;
		}
	}
	assert(false);
}

bool output::next() {
	outdesc::const_iterator i;
	vector<double>::iterator j;
	for (i = desc->begin(), j = vals.begin(); i != desc->end(); ++i, ++j) {
		*j += i->inc;
		if (*j <= i->max) {
			return true;
		} else {
			*j = i->min;  // roll over and move on to the next value
		}
	}
	return false;
}

void output::reset() {
	outdesc::const_iterator i;
	vector<double>::iterator j;
	for (i = desc->begin(), j = vals.begin(); i != desc->end(); ++i, ++j) {
		*j = i->min;
	}
}

string output::serialize() const {
	outdesc::const_iterator i;
	vector<double>::const_iterator j;
	stringstream ss;
	
	for (i = desc->begin(), j = vals.begin(); i != desc->end(); ++i, ++j) {
		ss << i->name << " " << *j << endl;
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
	int i = 0;
	vector<output>::iterator j;
	vector<double>::iterator k;
	
	assert(traj.n_elem == desc->size() * length);
	t.reserve(traj.n_elem / desc->size());
	for (j = t.begin(); j != t.end(); ++j) {
		for (k = j->vals.begin(); k != j->vals.end(); ++k, ++i) {
			*k = traj(i);
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
