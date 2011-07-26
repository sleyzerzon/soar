#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <armadillo>
#include "lwr.h"
#include "nn.h"
#include "common.h"

using namespace std;
using namespace arma;

const double RLAMBDA = 0.0000001;
const double INF = numeric_limits<double>::infinity();

void print_mat(const mat &X) {
	X.print("");
}

void print_vec(const vec &v) {
	v.print("");
}

void print_rowvec(const rowvec &v) {
	v.print("");
}

void lsqr(const mat &X, const mat &Y, const vec &w, const rowvec &x, rowvec &yout) {
	mat X1 = X;
	X1.insert_cols(X.n_cols, ones<vec>(X.n_rows));
	mat W = diagmat(w);
	mat Z = W * X1;
	mat V = W * Y;
	mat C = solve(Z, V);
	if (C.n_elem == 0) {
		// solve failed
		w.print("w:");
		Z.print("Z:");
		V.print("V:");
		assert(false);
	}
	rowvec x1 = ones<rowvec>(1, x.n_cols + 1);
	x1.subvec(0, x.n_cols - 1) = x;
	yout = x1 * C;
}

void ridge(const mat &X, const mat &Y, const vec &w, const rowvec &x, rowvec &yout) {
	int i;
	mat W = diagmat(w);
	mat Z = W * X;
	mat V = W * Y;
	rowvec Zmean = mean(Z, 0);
	mat Zcenter = Z;
	for (i = 0; i < Z.n_rows; ++i) {
		Zcenter.row(i) -= Zmean;
	}
	rowvec Vmean = mean(V, 0);
	mat Vcenter = V;
	for (i = 0; i < Z.n_rows; ++i) {
		Vcenter.row(i) -= Vmean;
	}
	mat Zt = trans(Zcenter);
	mat A = Zt * Zcenter;
	double lambda = RLAMBDA;
	for (i = 0; i < A.n_cols; ++i) {
		double inc = nextafter(A(i, i), INF) - A(i, i);
		lambda = max(lambda, inc);
	}
	for (i = 0; i < A.n_cols; ++i) {
		A(i, i) += lambda;
	}
	mat B = Zt * Vcenter;
	mat C = solve(A, B);
	assert(C.n_elem != 0);
	yout = (x - mean(X, 0)) * C + mean(Y, 0);
}

/* Output a matrix composed only of those columns in the input
   matrix with different values.
*/
void remove_static(mat &X, mat &Xout, vector<int> &dynamic) {
	for (int c = 0; c < X.n_cols; ++c) {
		for (int r = 1; r < X.n_rows; ++r) {
			if (X(r, c) != X(0, c)) {
				dynamic.push_back(c);
				break;
			}
		}
	}
	Xout.reshape(X.n_rows, dynamic.size());
	for (int i = 0; i < dynamic.size(); ++i) {
		Xout.col(i) = X.col(dynamic[i]);
	}
}

lwr::lwr(int nnbrs, const vector<string> &xnames, const vector<string> &ynames)
: nnbrs(nnbrs), xnames(xnames), ynames(ynames), xsize(xnames.size()), ysize(ynames.size()),
  normalized(false), xmin(xnames.size()), xmax(xnames.size()), xrange(xnames.size())
{
	int i;
	nn = new brute_nn(&xnorm);
}

void lwr::learn(const floatvec &x, const floatvec &y, float dt) {
	int i;

	examples.push_back(make_pair(x, y));
	if (examples.size() == 1) {
		xmin = x;
		xmax = x;
		xrange.zero();
	} else {
		for (int i = 0; i < xsize; ++i) {
			if (x[i] < xmin[i]) {
				xmin[i] = x[i];
				normalized = false;
			} else if (x[i] > xmax[i]) {
				xmax[i] = x[i];
				normalized = false;
			}
		}
	}
	
	if (normalized) {
		// otherwise just wait for renormalization
		xnorm.push_back((x - xmin) / xrange);
	}
}

void lwr::normalize() {
	vector<pair<floatvec, floatvec> >::iterator i;
	
	xrange = xmax;
	xrange -= xmin;
	xrange.replace(0.0, 1.0);  // can't have division by 0
	xnorm.clear();
	xnorm.reserve(examples.size());
	for (i = examples.begin(); i != examples.end(); ++i) {
		xnorm.push_back(i->first);
		xnorm.back() -= xmin;
		xnorm.back() /= xrange;
	}
}

bool lwr::predict(const floatvec &x, floatvec &y) {
	mat X, Y, Xd, Yd;
	rowvec xd, yd;
	vec d;
	vector<int> xdi, ydi;
	di_queue neighbors;
	timer tall, tnn, tslv;
	int i, j, k;

	tall.start();
	k = examples.size() > nnbrs ? nnbrs : examples.size();
	if (k == 0) {
		return false;
	}
	
	if (!normalized) {
		normalize();
		normalized = true;
	}
	
	floatvec xn((x - xmin) / xrange);
	
	tnn.start();
	nn->query(xn, k, neighbors);
	//cout << "NN:  " << tnn.stop() << endl;
	
	X.reshape(k, xsize);
	Y.reshape(k, ysize);
	d.reshape(k, 1);
	for(i = 0; i < k; ++i) {
		d(i) = neighbors.top().first;
		int ind = neighbors.top().second;
		neighbors.pop();
		floatvec &tx = examples[ind].first;
		floatvec &ty = examples[ind].second;
		for(j = 0; j < xsize; ++j) {
			X(i, j) = tx[j];
		}
		for(j = 0; j < ysize; ++j) {
			Y(i, j) = ty[j];
		}
	}
	
	vec w = sqrt(pow(d, -3));
	
	/* Any neighbor whose weight is infinity is close
	   enough to provide an exact solution.  If any
	   exist, take their average as the solution.  If we
	   don't do this the solve() will fail due to infinite
	   values in Z and V.
	*/
	rowvec closeavg = zeros<rowvec>(ysize);
	int nclose = 0;
	for (i = 0; i < w.n_elem; ++i) {
		if (w(i) == INF) {
			closeavg += Y.row(i);
			++nclose;
		}
	}
	if (nclose > 0) {
		for(i = 0; i < closeavg.n_elem; ++i) {
			y[i] = closeavg(i) / nclose;
		}
		return true;
	}
	
	remove_static(X, Xd, xdi);
	remove_static(Y, Yd, ydi);
	
	if (ydi.size() == 0) {
		// all neighbors are identical, use first as prediction
		for (i = 0; i < ysize; ++i) {
			y[i] = Y(0, i);
		}
		return true;
	}
	
	if (X.n_rows < xdi.size()) {
		// would result in underconstrained system
		return false;
	}
	
	xd = ones<rowvec>(1, xdi.size());
	for (i = 0; i < xdi.size(); ++i) {
		xd(i) = x[xdi[i]];
	}
	
	ridge(Xd, Yd, w, xd, yd);
	
	/* final answer is first (actually any will do) row of
	   Y with dynamic columns changed
	*/
	for (i = 0; i < ysize; ++i) {
		y[i] = Y(0, i);
	}
	for (i = 0; i < ydi.size(); ++i) {
		y[ydi[i]] = yd(i);
	}
	
	//cout << "ALL: " << tall.stop() << endl;
	return true;
}

int lwr::size() const {
	return examples.size();
}

bool lwr::load_file(const char *file) {
	ifstream f(file);
	string line;
	floatvec x(xsize), y(ysize);
	float dt;
	int linenum = 0;
	size_t p;
	while (getline(f, line)) {
		++linenum;
		if ((p = line.find('#')) != string::npos) {
			line.erase(p);
		}
		if (line.find_first_not_of(" \t\n") == string::npos) {
			continue;
		}
		stringstream ss(line);
		for(int i = 0; i < xsize; ++i) {
			if (!(ss >> x[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		for(int i = 0; i < ysize; ++i) {
			if (!(ss >> y[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		if (!(ss >> dt)) {
			cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
			return false;
		}
		
		learn(x, y, dt);
	}
}

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	Symbol *input_root = NULL, *output_root = NULL, *attr;
	wme_list children;
	wme_list::iterator i;
	long nnbrs = 50;
	string attrstr;
	vector<string> inputs, outputs;
	
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		attr = si->get_wme_attr(*i);
		if (!si->get_val(attr, attrstr)) {
			continue;
		}
		if (attrstr == "num-neighbors") {
			if (!si->get_val(si->get_wme_val(*i), nnbrs)) {
				cerr << "WARNING: attribute num-neighbors does not have integer value, using default 50 neighbors" << endl;
			}
		} else if (attrstr == "inputs") {
			input_root = si->get_wme_val(*i);
		} else if (attrstr == "outputs") {
			output_root = si->get_wme_val(*i);
		}
			
	}
	if (input_root == NULL || output_root == NULL) {
		return NULL;
	}
	children.clear();
	si->get_child_wmes(input_root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		attr = si->get_wme_attr(*i);
		if (!si->get_val(attr, attrstr)) {
			continue;
		}
		inputs.push_back(attrstr);
	}
	children.clear();
	si->get_child_wmes(output_root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		attr = si->get_wme_attr(*i);
		if (!si->get_val(attr, attrstr)) {
			continue;
		}
		outputs.push_back(attrstr);
	}
	return new lwr(nnbrs, inputs, outputs);
}

void lwr::get_slots(vector<string> &in_slots, vector<string> &out_slots) const {
	copy(xnames.begin(), xnames.end(), back_inserter(in_slots));
	copy(ynames.begin(), ynames.end(), back_inserter(out_slots));
}

