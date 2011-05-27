#include <assert.h>
#include <iostream>
#include <algorithm>
#include "balltree.h"

using namespace std;
using namespace arma;

namespace {
	inline double distsq(const rowvec &a, const rowvec &b) {
		return accu(pow(a - b, 2));
	}
	
	inline double dist(const rowvec &a, const rowvec &b) {
		return sqrt(distsq(a, b));
	}
	
	inline bool possibly_better(double d, int k, di_queue &nn) {
		return nn.size() < k || (nn.size() > 0 && d < nn.top().first);
	}
	
	inline int argmax(const vector<double> &v) {
		int maxi = 0;
		for(int i = 1; i < v.size(); ++i) {
			if (v[i] > v[maxi]) {
				maxi = i;
			}
		}
		return maxi;
	}
}

balltree::balltree(int ndim, int leafsize, vector<rowvec> *pts, const vector<int> &inds)
: ndim(ndim), leafsize(leafsize), pts(pts), inds(inds), left(NULL), right(NULL), parent(NULL)
{
	if (this->inds.size() == 0) {
		for (int i = 0; i < pts->size(); ++i) {
			this->inds.push_back(i);
		}
	}
	if (this->inds.size() > leafsize) {
		split();
	} else {
		update_ball();
	}
}

balltree::~balltree() {
	delete left;
	delete right;
}

void balltree::distsq_to(const rowvec &q, vector<double> &dout) {
	vector<int>::const_iterator i;
	dout.clear();
	for (i = inds.begin(); i != inds.end(); ++i) {
		dout.push_back(distsq(q, (*pts)[*i]));
	}
}

void balltree::update_ball() {
	if (left != NULL) {
		rowvec dir = left->center - right->center;
		rowvec unit = dir / sqrt(dot(dir, dir));
		rowvec p1 = left->center + unit * left->radius;
		rowvec p2 = right->center - unit * right->radius;
		center = (p1 + p2) / 2.0;
		radius = dist(center, p1);
	} else {
		vector<int>::const_iterator i;
		vector<double> dists;
		int furthest;
		center = zeros<rowvec>(1, ndim);
		for (i = inds.begin(); i != inds.end(); ++i) {
			center += (*pts)[*i];
		}
		center /= inds.size();
		distsq_to(center, dists);
		radius = sqrt(*max_element(dists.begin(), dists.end()));
	}
}

void balltree::split() {
	vector<double> dc, dl, dr;
	vector<int> linds, rinds;
	int i, li, ri;
	
	assert(left == NULL && right == NULL);
	
	update_ball();
	distsq_to(center, dc);
	li = argmax(dc);                  // furthest from center
	distsq_to((*pts)[inds[li]], dl);
	ri = argmax(dl);                  // furthest from li
	distsq_to((*pts)[inds[ri]], dr);
	for (i = 0; i < dl.size(); ++i) {
		if (dl[i] < dr[i]) {
			linds.push_back(inds[i]);
		} else {
			rinds.push_back(inds[i]);
		}
	}
	left = new balltree(ndim, leafsize, pts, linds);
	left->parent = this;
	right = new balltree(ndim, leafsize, pts, rinds);
	right->parent = this;
	update_ball();
}

void balltree::linear_scan(const rowvec &q, int k, di_queue &nn) {
	vector<int>::const_iterator i;
	double d;
	
	for (i = inds.begin(); i != inds.end(); ++i) {
		d = distsq((*pts)[*i], q);
		if (possibly_better(d, k, nn)) {
			nn.push(make_pair(d, *i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
}

int balltree::query(const rowvec &q, int k, di_queue &nn) {
	double dmin = pow(max(dist(center, q) - radius, 0.0), 2);
	if (nn.size() >= k && dmin >= nn.top().first) {
		return inds.size();
	}
	
	if (inds.size() <= leafsize) { // right == NULL for sure
		linear_scan(q, k, nn);
		return 0;
	}
	
	int pruned = 0;
	pruned += left->query(q, k, nn);
	pruned += right->query(q, k, nn);
	return pruned;
}
