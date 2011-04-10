#include <sstream>
#include "env.h"

using namespace std;

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

bool env_output::increment() {
	env_output_desc::iterator i;
	for (i = desc.begin(); i != desc.end(); ++i) {
		if (value[i->first] + i->second.inc <= i->second.max) {
			value[i->first] += i->second.inc;
			return true;
		}
	}
	return false;
}

void env_output::serialize(string &out) {
	map<string, float>::iterator i;
	stringstream ss;
	
	for (i = value.begin(); i != value.end(); ++i) {
		ss << i->first << " " << i->second << endl;
	}
	out = ss.str();
}

environment::environment(string path)
: sock(path)
{}

bool environment::output(env_output &out) {
	string s;
	out.serialize(s);
	return sock.send("output", s);
}

bool environment::input(string &sgel) {
	string type;
	return sock.receive(type, sgel);
}
