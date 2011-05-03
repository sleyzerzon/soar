#include <assert.h>
#include <sstream>
#include "env.h"

using namespace std;

env_output::env_output() {}

env_output::env_output(const env_output_desc &d)
: desc(d)
{
	env_output_desc::iterator i;
	for(i = desc.begin(); i != desc.end(); ++i) {
		value[i->first] = i->second.min;
	}
}

env_output::env_output(const env_output &other) 
: value(other.value), desc(other.desc)
{ }

double env_output::get(const string &dim) const {
	map<string, double>::const_iterator i;
	if ((i = value.find(dim)) == value.end()) {
		assert(false);
	}
	return i->second;
}

void env_output::get_signature(env_output_sig &sig) const {
	map<string, double>::const_iterator i;
	for (i = value.begin(); i != value.end(); ++i) {
		sig.insert(i->first);
	}
}

void env_output::set(const string &dim, double val) {
	map<string, double>::iterator i;
	if ((i = value.find(dim)) == value.end()) {
		assert(false);
	}
	i->second = val;
}

bool env_output::increment() {
	env_output_desc::iterator i, j;
	for (i = desc.begin(); i != desc.end(); ++i) {
		if (value[i->first] + i->second.inc <= i->second.max) {
			value[i->first] += i->second.inc;
			for (j = desc.begin(); j != i; ++j) {
				value[j->first] = j->second.min;
			}
			return true;
		}
	}
	return false;
}

string env_output::serialize() const {
	map<string, double>::const_iterator i;
	stringstream ss;
	
	for (i = value.begin(); i != value.end(); ++i) {
		ss << i->first << " " << i->second << endl;
	}
	return ss.str();
}

void env_output::operator=(const env_output &o) {
	desc = o.desc;
	value = o.value;
}

int env_output::size() const {
	return value.size();
}
