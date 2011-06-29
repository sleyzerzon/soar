#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
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

lwr::lwr(int xdim, int ydim, int nnbrs) 
: xdim(xdim), ydim(ydim), nnbrs(nnbrs), normalized(false),
  xmin(xdim), xmax(xdim), xrange(xdim)
{
	nn = new brute_nn(&xnorm);
}
	
void lwr::add(const floatvec &x, const floatvec &y) {
	assert(x.size() == xdim && y.size() == ydim);
	examples.push_back(make_pair(x, y));
	
	if (examples.size() == 1) {
		xmin = x;
		xmax = x;
		xrange.zero();
	} else {
		for (int i = 0; i < x.size(); ++i) {
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
		// this looks ugly because I'm trying to avoid unnecessary copying
		xnorm.push_back(x);
		xnorm.back() -= xmin;
		xnorm.back() /= xrange;
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

bool lwr::predict(const floatvec &x, floatvec &y, char method, bool mahalanobis) {
	mat X, Y, Xd, Yd;
	rowvec xd, yd;
	vec d;
	vector<int> xdi, ydi;
	timer tall, tnn, tslv;
	tall.start();
	
	int i, j, k = examples.size() > nnbrs ? nnbrs : examples.size();
	di_queue neighbors;
	
	if (!normalized) {
		normalize();
		normalized = true;
	}
	floatvec xn(x);
	xn = x;
	xn -= xmin;
	xn /= xrange;
	
	tnn.start();
	nn->query(xn, k, neighbors);
	//cout << "NN:  " << tnn.stop() << endl;
	
	X.reshape(k, xdim);
	Y.reshape(k, ydim);
	d.reshape(k, 1);
	for(i = 0; i < k; ++i) {
		d(i) = neighbors.top().first;
		int ind = neighbors.top().second;
		neighbors.pop();
		floatvec &tx = examples[ind].first;
		floatvec &ty = examples[ind].second;
		for(j = 0; j < xdim; ++j) {
			X(i, j) = tx[j];
		}
		for(j = 0; j < ydim; ++j) {
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
	rowvec closeavg = zeros<rowvec>(ydim);
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
		for (i = 0; i < ydim; ++i) {
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
	
	if (method == 'r') {
		ridge(Xd, Yd, w, xd, yd);
	} else {
		lsqr(Xd, Yd, w, xd, yd);
	}
	
	/* final answer is first (actually any will do) row of
	   Y with dynamic columns changed
	*/
	for (i = 0; i < ydim; ++i) {
		y[i] = Y(0, i);
	}
	for (i = 0; i < ydi.size(); ++i) {
		y[ydi[i]] = yd(i);
	}
	
	//cout << "ALL: " << tall.stop() << endl;
	return true;
}

int lwr::size() {
	return examples.size();
}

bool lwr::load_file(const char *file) {
	ifstream f(file);
	string line;
	floatvec x(xdim), y(ydim);
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
		for(int i = 0; i < xdim; ++i) {
			if (!(ss >> x[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		for(int i = 0; i < ydim; ++i) {
			if (!(ss >> y[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		add(x, y);
	}
}
