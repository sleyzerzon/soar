#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <iterator>
#include <armadillo>
#include "lwr.h"

using namespace std;
using namespace arma;

void split(const string &s, const string &delims, vector<string> &result) {
	int i = 0, j = 0;
	while ((j = s.find_first_of(delims, i)) != string::npos) {
		result.push_back(s.substr(i, j - i));
		i = j + 1;
	}
	result.push_back(s.substr(i));
}

void vectortovec(const vector<double> &v1, rowvec &v2) {
	v2.reshape(1, v1.size());
	copy(v1.begin(), v1.end(), v2.begin());
}

void strtovec(const string &s, rowvec &v) {
	stringstream ss(s);
	vector<double> n;
	double f;
	
	while (ss) {
		if (!(ss >> f)) {
			break;
		}
		n.push_back(f);
	}
	vectortovec(n, v);
}

double dist(const rowvec& a, const rowvec& b) {
	rowvec d = a - b;
	return sqrt(dot(d, d));
}

int main(int argc, char *argv[]) {
	string line;
	int nnbrs;
	lwr *m = NULL;
	vector<double> diffs;
	bool learning = true;
	
	if (argc != 4) {
		cerr << "usage: lwrtest <method> <num neighbors> <data file>" << endl;
		exit(1);
	}
	
	stringstream ss(argv[2]);
	if (!(ss >> nnbrs)) {
		cerr << "nnbrs parse error" << endl;
		exit(1);
	}
	
	ifstream input(argv[3]);
	if (!input) {
		cerr << "couldn't open " << argv[2] << endl;
		exit(1);
	}
	
	while (true) {
		vector<string> parts;
		rowvec x, y;
		
		if (!getline(input, line)) {
			break;
		}
		
		if (line[0] == '$') {
			learning = false;
			continue;
		}
		
		split(line, ";", parts);
		assert(parts.size() == 2);
		strtovec(parts[0], x);
		strtovec(parts[1], y);
		
		if (learning) {
			if (!m) {
				m = new lwr(x.n_elem, y.n_elem, nnbrs);
			}
			m->add(x, y);
		} else {
			rowvec py;
			m->predict(x, py, argv[1][0]);
			diffs.push_back(dist(py, y));
		}
	}
	rowvec diffv;
	vectortovec(diffs, diffv);
	cout << "MEAN: " << mean(diffv) << endl;
	cout << "STD:  " << stddev(diffv) << endl;
	return 0;
}
