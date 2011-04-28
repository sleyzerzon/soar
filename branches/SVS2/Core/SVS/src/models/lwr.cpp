#include <assert.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <armadillo>
#include "lwr.h"

using namespace std;
using namespace arma;

class timer {
public:
	timer() {}
	
	void start() {
		gettimeofday(&t1, NULL);
	}
	
	double stop() {
		timeval t2, t3;
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
		return t3.tv_sec + t3.tv_usec / 1000000.0;
	}
	
	timeval t1;
};

lwr::lwr(int xdim, int ydim, int nnbrs) : xdim(xdim), ydim(ydim), nnbrs(nnbrs) 
{}
	
void lwr::add(const rowvec &x, const rowvec &y) {
	assert(x.n_cols == xdim && y.n_cols == ydim);
	db.push_back(make_pair(x, y));
}

bool lwr::predict(const rowvec &x, rowvec &y) {
	//timer tall, tnn, tslv;
	//tall.start();
	int k = db.size() > nnbrs ? nnbrs : db.size();
	
	if (k == 0) {
		return false;
	}
	
	mat X = zeros<mat>(k, xdim);
	mat Y = zeros<mat>(k, ydim);
	vec w = zeros<vec>(k);
	
	//tnn.start();
	nearest(x, X, Y, w);
	//cout << "NN:  " << tnn.stop() << endl;
	
	/* Any neighbor whose weight is infinity is close
	   enough to provide an exact solution.  If any
	   exist, take their average as the solution.  If we
	   don't do this the solve() will fail due to infinite
	   values in Z and V.
	*/
	rowvec closeavg = zeros<rowvec>(ydim);
	int nclose = 0;
	for (int i = 0; i < w.n_elem; ++i) {
		if (w(i) == math::inf()) {
			closeavg += Y.row(i);
			++nclose;
		}
	}
	if (nclose > 0) {
		for(int i = 0; i < closeavg.n_elem; ++i) {
			closeavg(i) /= nclose;
		}
		y = closeavg;
		return true;
	}
	
	mat Xt, Yt;
	vector<int> xd, yd;
	remove_static(X, Xt, xd);
	remove_static(Y, Yt, yd);
	
	if (yd.size() == 0) {
		// all neighbors are identical, use first as prediction
		y = Y.row(0);
		return true;
	}
	
	Xt.insert_cols(Xt.n_cols, ones<vec>(k));
	mat W = diagmat(w);
	mat Z = W * Xt;
	mat V = W * Yt;
	//tslv.start();
	mat C = solve(Z, V);
	//cout << "SLV: " << tslv.stop() << endl;
	if (C.n_elem == 0) {
		// solve failed
		w.print("w:");
		Z.print("Z:");
		V.print("V:");
		assert(false);
	}
	
	// xt is input x without static columns 
	rowvec xt = ones<rowvec>(xd.size() + 1);
	for (int i = 0; i < xd.size(); ++i) {
		xt(i) = x(xd[i]);
	}
	// yt is output y without static columns
	rowvec yt = xt * C;
	
	/* final answer is first (actually any will do) row of
	   Y with dynamic columns changed
	*/
	y = Y.row(0);
	for (int i = 0; i < yd.size(); ++i) {
		y(yd[i]) = yt(i);
	}
	
	//cout << "ALL: " << tall.stop() << endl;
	return true;
}

void lwr::nearest(rowvec x, mat &X, mat &Y, vec &w) {
	int i;
	std::list<int> inds;
	std::list<int>::iterator j;
	std::list<double> dists;
	std::list<double>::iterator k;
	double d;
	rowvec t;
	
	for (i = 0; i < db.size(); ++i) {
		t = db[i].first - x;
		d = dot(t, t);
		for (j = inds.begin(), k = dists.begin(); ; ++j, ++k) {
			if (*k > d || (j == inds.end() && inds.size() < X.n_rows)) {
				inds.insert(j, i);
				dists.insert(k, d);
				if (inds.size() > X.n_rows) {
					inds.pop_back();
					dists.pop_back();
				}
				break;
			}
			if (j == inds.end()) {
				break;
			}
		}
	}
	
	for(i = 0, j = inds.begin(), k = dists.begin(); i < X.n_rows; ++i, ++j, ++k) {
		X.row(i) = db[*j].first;
		Y.row(i) = db[*j].second;
		w(i) = ::pow(*k, -3);
	}
}

/* Output a matrix composed only of those columns in the input
   matrix with different values.
*/
void lwr::remove_static(mat &X, mat &Xout, vector<int> &dynamic) {
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
