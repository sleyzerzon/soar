#include <assert.h>
#include <iostream>
#include <vector>
#include <list>
#include <armadillo>
#include "model.h"
#include "scene.h"
#include "linalg.h"

using namespace std;
using namespace arma;

void scene_to_vec(scene *scn, rowvec &v) {
	scene_sig sig;
	scene_sig::iterator i;
	int j, k, l;
	const char *types = "prs";
	::vec3 trans;
	sg_node *n;
	
	scn->get_signature(sig);
	v.reshape(1, sig.size() * 9);
	for (i = sig.begin(), j = 0; i != sig.end(); ++i) {
		n = scn->get_node(i->first);
		for (k = 0; k < 3; ++k) {
			trans = n->get_trans(types[k]);
			for (l = 0; l < 3; ++l) {
				v(j++) = trans[l];
			}
		}
	}
}

void vec_to_scene(const rowvec &v, scene *scn) {
	scene_sig sig;
	scene_sig::iterator i;
	int j, k, l;
	::vec3 trans;
	const char *types = "prs";
	
	scn->get_signature(sig);
	for (i = sig.begin(), j = 0; i != sig.end(); ++i) {
		for (k = 0; k < 3; ++k) {
			for (l = 0; l < 3; ++l) {
				trans[l] = v(j++);
			}
			scn->set_node_trans(i->first, types[k], trans);
		}
	}
}

class lwr {
public:
	lwr(int xdim, int ydim, int nnbrs) : xdim(xdim), ydim(ydim), nnbrs(nnbrs) {}
	
	void add(const rowvec &x, const rowvec &y) {
		assert(x.n_cols == xdim && y.n_cols == ydim);
		db.push_back(make_pair(x, y));
	}

	bool predict(rowvec x, rowvec &y) {
		if (db.size() < nnbrs) {
			return false;
		}
		mat X = zeros<mat>(nnbrs, xdim);
		mat Y = zeros<mat>(nnbrs, ydim);
		vec w = zeros<vec>(nnbrs);
		nearest(x, X, Y, w);
		X.insert_cols(xdim, ones<vec>(nnbrs));
		mat W = diagmat(w);
		mat Z = W * X;
		mat V = W * Y;
		mat C = solve(Z, V);
		x.insert_cols(xdim, ones<rowvec>(1,1));
		y = x * C;
		return true;
	}
	
private:
	void nearest(rowvec x, mat &X, mat &Y, vec &w) {
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
				if (*k > d || (j == inds.end() && inds.size() < nnbrs)) {
					inds.insert(j, i);
					dists.insert(k, d);
					if (inds.size() > nnbrs) {
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
		
		for(i = 0, j = inds.begin(), k = dists.begin(); i < nnbrs; ++i, ++j, ++k) {
			X.row(i) = db[*j].first;
			Y.row(i) = db[*j].second;
			w(i) = ::pow(*k, -3);
		}
	}
	
	int xdim, ydim, nnbrs;
	vector<pair<rowvec, rowvec> > db;
};

class lwr_model : public model {
public:
	lwr_model() {}
	
	bool predict(scene *scn, const env_output &out) {
		scene_sig sig;
		map<scene_sig, lwr*>::iterator i;
		rowvec x, y;
		
		scn->get_signature(sig);
		if ((i = models.find(sig)) == models.end()) {
			return false;
		}
		scene_to_vec(scn, x);
		if (!i->second->predict(x, y)) {
			return false;
		}
		vec_to_scene(y, scn);
	}
	
	void add(scene *scn) {
		scene_sig sig;
		rowvec p;
		int vecsize;
		
		scn->get_signature(sig);
		vecsize = sig.size() * 9;
		if (lastsig.size() == 0) {
			lastsig = sig;
		} else if (lastsig != sig) {
			return;
		}
		
		scene_to_vec(scn, p);
		if (lastpoint.n_elem > 0) {
			assert(lastpoint.n_elem == p.n_elem);
			models[sig]->add(lastpoint, p);
		} else {
			models[sig] = new lwr(vecsize, vecsize, 30);
		}
		lastpoint = p;
	}
	
private:
	map<scene_sig, lwr*> models;
	rowvec lastpoint;
	scene_sig lastsig;
};

lwr_model *mdl = NULL;

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	if (!mdl) {
		mdl = new lwr_model();
	}
	return mdl;
}
