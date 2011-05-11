#include <assert.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <set>
#include <armadillo>
#include "lwr.h"

using namespace std;
using namespace arma;

const double RLAMBDA = 0.01;

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

void print_mat(const mat &X) {
	X.print("");
}

void print_vec(const vec &v) {
	v.print("");
}

void print_rowvec(const rowvec &v) {
	v.print("");
}

void log_mat(const mat &m, const string &path) {
	ofstream f(path.c_str());
	m.print(f, "");
	f.close();
}

void log_vec(const rowvec &v, const string &path) {
	ofstream f(path.c_str());
	v.print(f, "");
	f.close();
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
	rowvec Xmean = mean(X, 0);
	mat Xcenter = X - (ones<vec>(X.n_rows, 1) * Xmean);
	rowvec Ymean = mean(Y, 0);
	mat Ycenter = Y - (ones<vec>(Y.n_rows, 1) * Ymean);
	mat W = diagmat(w);
	mat Z = W * Xcenter;
	mat V = W * Ycenter;
	mat Zt = trans(Z);
	mat lambdaeye = RLAMBDA * eye<mat>(X.n_cols, X.n_cols);
	mat C = inv(Zt * Z + lambdaeye) * Zt * V;
	yout = (x - Xmean) * C + Ymean;
}

lwr::lwr(int xdim, int ydim, int nnbrs) : xdim(xdim), ydim(ydim), nnbrs(nnbrs) 
{}
	
void lwr::add(const rowvec &x, const rowvec &y) {
	assert(x.n_cols == xdim && y.n_cols == ydim);
	db.push_back(make_pair(x, y));
}

bool lwr::predict(const rowvec &x, rowvec &y, char method) {
	mat X, Y, Xd, Yd;
	rowvec xd, yd;
	vec d;
	vector<int> xdi, ydi;
	//timer tall, tnn, tslv;
	//tall.start();
	int k = db.size() > nnbrs ? nnbrs : db.size();
	
	if (k == 0) {
		return false;
	}
	
	//tnn.start();
	nearest(k, x, X, Y, d);
	//cout << "NN:  " << tnn.stop() << endl;
	
	vec w = pow(d, -3);
	
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
	
	remove_static(X, Xd, xdi);
	remove_static(Y, Yd, ydi);
	
	if (ydi.size() == 0) {
		// all neighbors are identical, use first as prediction
		y = Y.row(0);
		return true;
	}
	
	xd = ones<rowvec>(1, xdi.size());
	for (int i = 0; i < xdi.size(); ++i) {
		xd(i) = x(xdi[i]);
	}
	
	if (method == 'r') {
		ridge(Xd, Yd, w, xd, yd);
	} else {
		lsqr(Xd, Yd, w, xd, yd);
	}
	
	/* final answer is first (actually any will do) row of
	   Y with dynamic columns changed
	*/
	y = Y.row(0);
	for (int i = 0; i < ydi.size(); ++i) {
		y(ydi[i]) = yd(i);
	}
	
	//cout << "ALL: " << tall.stop() << endl;
	return true;
}

void lwr::nearest(int k, rowvec x, mat &X, mat &Y, vec &d) {
	int i;
	std::list<pair<int, double> > near;
	std::list<pair<int, double> >::iterator j;
	double dist;
	rowvec t;
	
	X.reshape(k, db.front().first.n_cols);
	Y.reshape(k, db.front().second.n_cols);
	d.reshape(k, 1);
	
	for (i = 0; i < db.size(); ++i) {
		t = db[i].first - x;
		dist = dot(t, t);
		for (j = near.begin(); ; ++j) {
			if (j->second > dist || (j == near.end() && near.size() < k)) {
				near.insert(j, make_pair(i, dist));
				if (near.size() > k) {
					near.pop_back();
				}
				break;
			}
			if (j == near.end()) {
				break;
			}
		}
	}
	
	for(i = 0, j = near.begin(); j != near.end(); ++i, ++j) {
		X.row(i) = db[j->first].first;
		Y.row(i) = db[j->first].second;
		d(i) = j->second;
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

int lwr::size() {
	return db.size();
}
