#include <iostream>
#include <vector>
#include <utility>
#include <armadillo>
#include "lwr.h"

using namespace std;
using namespace arma;

int main(int argc, char *argv[]) {
	string line;
	int nnbrs, xdim = -1, ydim;
	lwr *m;
	double t;
	rowvec x, y, px, py;
	
	if (argc != 2) {
		cerr << "need one argument" << endl;
		exit(1);
	}
	stringstream ss(argv[1]);
	if (!(ss >> nnbrs)) {
		cerr << "nnbrs parse error" << endl;
		exit(1);
	}
	
	while (true) {
		if (!getline(cin, line)) {
			break;
		}
		stringstream ss2(line);
		
		if (xdim == -1) {
			if (!(ss2 >> xdim >> ydim)) {
				cerr << "dims parse error" << endl;
				exit(1);
			}
			m = new lwr(xdim, ydim, nnbrs);
			x = zeros<rowvec>(xdim);
			y = zeros<rowvec>(ydim);
			continue;
		}
		
		for (int i = 0; i < xdim; ++i) {
			if (!(ss2 >> t)) {
				cerr << "parse error field " << i << " line " << line << endl;
				exit(1);
			}
			x(i) = t;
		}
		
		for (int i = 0; i < ydim; ++i) {
			if (!(ss2 >> t)) {
				cerr << "parse error field " << i + xdim << " line " << line << endl;
				exit(1);
			}
			y(i) = t;
		}
		if (px.n_elem == 0) {
			px = x;
			py = y;
		} else {
			m->add(x, y);
		}
	}

	m->predict(px, y);
	for (int i = 0; i < y.n_elem; ++i) {
		cout << y(i) << " ";
	}
	cout << endl;
}
