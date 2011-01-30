#include <stdlib.h>
#include <string>
#include <vector>

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
	string ns = getenv("SVSNAMESPACE");
	if (ns.size() > 0 && *ns.rbegin() != '/') {
		ns.push_back('/');
	}
	return ns;
}
