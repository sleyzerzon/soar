#include <iostream>
#include <sstream>
#include <iterator>
#include <map>
#include "../filter.h"
#include "../bbox_filter.h"

using namespace std;

map<string, filter*> ftable;

class ptlist_src : public ptlist_filter {
public:
	ptlist_src(ptlist _pts) : pts(_pts) {}
	
	bool get_result(ptlist &r) {
		r = pts;
		return true;
	}
	
	string get_error() { return ""; }
	
	ptlist pts;
};

ptlist_src* make_ptlist_src(stringstream &ss) {
	ptlist pts;
	double x, y, z;
	
	while (!ss.eof()) {
		ss >> x >> y >> z;
		if (ss.fail()) {
			cerr << "bad input" << endl;
			return NULL;
		}
		pts.push_back(vec3(x,y,z));
	}
	
	return new ptlist_src(pts);
}

bool get_param_filters(stringstream &ss, filter_params &params) {
	string p, fname;
	map<string, filter*>::iterator i;
	
	params.clear();
	while(!ss.eof()) {
		ss >> p >> fname;
		if (ss.fail()) {
			cerr << "wrong number of arguments" << endl;
			return false;
		}
		if ((i = ftable.find(fname)) == ftable.end()) {
			cerr << fname << " not defined" << endl;
			return false;
		}
		params.insert(pair<string,filter*>(p, i->second));
	}
	
	return true;
}

bool add_filter(stringstream &ss) {
	string ftype, fname;
	filter *f;
	filter_params params;
	
	ss >> ftype >> fname;
	if (ftype == "src") {
		f = make_ptlist_src(ss);
	} else {
		if (!get_param_filters(ss, params)) {
			return false;
		}
		f = make_filter(ftype, params);
	}
	if (!f) {
		cerr << "failed to make " << fname << endl;
		return false;
	}
	ftable[fname] = f;
	return true;
}

bool print_val(stringstream &ss) {
	bool_filter   *bf; bool   bval;
	bbox_filter   *xf; bbox   xval;
	ptlist_filter *pf; ptlist pval;

	string name;
	char type;
	map<string, filter*>::iterator i;
	
	ss >> name >> type;
	if ((i = ftable.find(name)) == ftable.end()) {
		cerr << name << " not defined" << endl;
		return false;
	}
	
	switch (type) {
		case 'b':
			if(!(bf = dynamic_cast<bool_filter*>(i->second))) {
				cerr << name << " is not of type " << type << endl;
				return false;
			}
			if (!bf->get_result(bval)) {
				cerr << name << " error: " << bf->get_error() << endl;
				return false;
			}
			cout << bval << endl;
			return true;
		case 'x':
			if(!(xf = dynamic_cast<bbox_filter*>(i->second))) {
				cerr << name << " is not of type " << type << endl;
				return false;
			}
			if (!xf->get_result(xval)) {
				cerr << name << " error: " << xf->get_error() << endl;
				return false;
			}
			cout << xval << endl;
			return true;
		case 'p':
			if(!(pf = dynamic_cast<ptlist_filter*>(i->second))) {
				cerr << name << " is not of type " << type << endl;
				return false;
			}
			if (!pf->get_result(pval)) {
				cerr << name << " error: " << pf->get_error() << endl;
				return false;
			}
			copy(pval.begin(), pval.end(), ostream_iterator<vec3>(cout, " "));
			cout << endl;
			return true;
		default:
			cerr << "unsupported type " << type << endl;
			return false;
	}
	return false;
}

int main() {
	stringstream ss;
	string line;
	char cmd;
	
	while (!cin.eof()) {
		getline(cin, line);
		if (line.size() == 0) {
			return 0;
		}
		ss.clear();
		ss.str(line);
		ss >> cmd;
		switch (cmd) {
			case 'a':
				if (!add_filter(ss)) {
					return 1;
				}
				break;
			case 'p':
				if (!print_val(ss)) {
					return 1;
				}
				break;
			default:
				cerr << "invalid command" << endl;
				return 1;
		}
	}
}
