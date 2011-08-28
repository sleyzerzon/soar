#include <stdlib.h>
#include <string>
#include <vector>
#include "common.h"

using namespace std;

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	fields.clear();
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

string getnamespace() {
	char *s;
	if ((s = getenv("SVSNAMESPACE")) == NULL) {
		return "";
	}
	
	string ns(s);
	if (ns.size() > 0 && *ns.rbegin() != '/') {
		ns.push_back('/');
	}
	return ns;
}

ostream &operator<<(ostream &os, const floatvec &v) {
	copy(v.mem, v.mem + v.sz, ostream_iterator<float>(os, ","));
	return os;
}

ostream &operator<<(ostream &os, const namedvec &v) {
	string name;
	for (int i = 0; i < v.size(); ++i) {
		if (!v.get_name(i, name)) {
			assert(false);
		}
		os << name << " " << v.vals[i] << endl;
	}
	return os;
}

vec3 calc_centroid(const ptlist &pts) {
	ptlist::const_iterator i;
	int d;
	vec3 c;
	
	for (i = pts.begin(); i != pts.end(); ++i) {
		for (d = 0; d < 3; ++d) {
			c[d] += (*i)[d];
		}
	}
	for (d = 0; d < 3; ++d) {
		c[d] /= pts.size();
	}
	return c;
}
