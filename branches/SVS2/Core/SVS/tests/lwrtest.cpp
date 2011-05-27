#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <iterator>
#include <sstream>
#include "lwr.h"
#include "common.h"

using namespace std;

void split(const string &s, const string &delims, vector<string> &result) {
	int i = 0, j = 0;
	while ((j = s.find_first_of(delims, i)) != string::npos) {
		result.push_back(s.substr(i, j - i));
		i = j + 1;
	}
	result.push_back(s.substr(i));
}

void parsefloats(const string &s, vector<float> &v) {
	stringstream ss(s);
	double f;
	
	while (ss) {
		if (!(ss >> f)) {
			break;
		}
		v.push_back(f);
	}
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
		cerr << "couldn't open " << argv[3] << endl;
		exit(1);
	}
	
	while (true) {
		vector<string> parts;
		
		if (!getline(input, line)) {
			break;
		}
		
		if (line[0] == '$') {
			learning = false;
			continue;
		}
		
		split(line, ";", parts);
		assert(parts.size() == 2);
		
		vector<float> x1, y1;
		parsefloats(parts[0], x1);
		parsefloats(parts[1], y1);
		floatvec x2(x1), y2(y1);
		
		if (learning) {
			if (!m) {
				m = new lwr(x2.size(), y2.size(), nnbrs);
			}
			m->add(x2, y2);
		} else {
			floatvec py(y2.size());
			m->predict(x2, py, argv[1][0], false);
			diffs.push_back(sqrt(py.distsq(y2)));
		}
	}
	
	if (diffs.size() == 0) {
		cout << "No predictions" << endl;
		return 0;
	}
	
	double mean = 0.0, std = 0.0;
	for (int i = 0; i < diffs.size(); ++i) {
		mean += diffs[i];
	}
	mean /= diffs.size();
	
	for (int i = 0; i < diffs.size(); ++i) {
		std += pow(diffs[i] - mean, 2);
	}
	std = sqrt(std / diffs.size());
	
	cout << "MEAN: " << mean << endl;
	cout << "STD:  " << std << endl;
	return 0;
}
