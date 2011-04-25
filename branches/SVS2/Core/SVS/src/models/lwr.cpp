#include <assert.h>
#include <iostream>
#include <vector>
#include <set>
#include <armadillo>
#include "model.h"
#include "scene.h"
#include "linalg.h"

using namespace std;
using namespace arma;

const int NNBRS = 30;   // number of nearest neighbors to use

typedef set<string> output_sig;
typedef pair<scene_sig, output_sig> model_sig;

void scene_to_vec(scene *scn, rowvec &v) {
	scene_sig sig;
	scene_sig::iterator i;
	int j, k, l;
	const char *types = "prs";
	::vec3 trans;
	sg_node *n;
	
	v.reshape(1, scn->get_dof());
	scn->get_signature(sig);
	for (i = sig.begin(), j = 0; i != sig.end(); ++i) {
		if (i->second == "PROPERTY") {
			v(j++) = scn->get_property(i->first);
		} else {
			n = scn->get_node(i->first);
			for (k = 0; k < 3; ++k) {
				trans = n->get_trans(types[k]);
				for (l = 0; l < 3; ++l) {
					v(j++) = trans[l];
				}
			}
		}
	}
}

void scene_out_to_vec(scene *scn, const env_output &out, rowvec &v) {
	env_output_sig s;
	env_output_sig::const_iterator i;
	int j;
	scene_to_vec(scn, v);
	out.get_signature(s);
	j = v.n_elem;
	v.reshape(1, v.n_elem + s.size());
	for (i = s.begin(); i != s.end(); ++i, ++j) {
		v(j) = out.get(*i);
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
		int i, k = db.size() > nnbrs ? nnbrs : db.size();
		
		if (k == 0) {
			return false;
		}
		mat X = zeros<mat>(k, xdim);
		mat Y = zeros<mat>(k, ydim);
		vec w = zeros<vec>(k);
		nearest(x, X, Y, w);
		
		/* Any neighbor whose weight is infinity is close
		   enough to provide an exact solution.  If any
		   exist, take their average as the solution.  If we
		   don't do this the solve() will fail due to infinite
		   values in Z and V.
		*/
		rowvec closeavg = zeros<rowvec>(ydim);
		int nclose = 0;
		for (i = 0; i < w.n_elem; ++i) {
			if (w(i) == numeric_limits<double>::infinity()) {
				closeavg += Y.row(i);
				++nclose;
			}
		}
		if (nclose > 0) {
			for(i = 0; i < closeavg.n_elem; ++i) {
				closeavg(i) /= nclose;
			}
			y = closeavg;
			return true;
		}
		
		X.insert_cols(xdim, ones<vec>(k));
		mat W = diagmat(w);
		mat Z = W * X;
		mat V = W * Y;
		mat C = solve(Z, V);
		if (C.n_elem == 0) {
			// solve failed
			w.print("w:");
			Z.print("Z:");
			V.print("V:");
			assert(false);
		}
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
	
	int xdim, ydim, nnbrs;
	vector<pair<rowvec, rowvec> > db;
};

class lwr_model : public learning_model {
public:
	lwr_model() : lastscn(NULL) {}
	
	bool predict(scene *scn, const env_output &out) {
		scene_sig ssig;
		env_output_sig osig;
		map<model_sig, lwr*>::iterator i;
		rowvec x, y;
		
		scn->get_signature(ssig);
		out.get_signature(osig);
		model_sig msig = make_pair(ssig, osig);
		
		if ((i = models.find(msig)) == models.end()) {
			return false;
		}
		scene_out_to_vec(scn, out, x);
		if (!i->second->predict(x, y)) {
			return false;
		}
		vec_to_scene(y, scn);
	}
	
	/* Add a training example to the model.  The training example
	   is encoded as a pair of vectors (x, y) where
	   x = [prev_state, output], y = [next_state]
	*/
	void add(const env_output &out, scene *scn) {
		scene_sig last_ssig, curr_ssig;
		env_output_sig osig;
		rowvec x, y;
		map<model_sig, lwr*>::iterator i;
		output_sig::iterator j;
		int k, xdim, ydim;
		lwr* mdl;
		
		if (lastscn == NULL) {
			// incomplete example, wait for next time
			lastscn = new scene(*scn);
			return;
		}
		
		lastscn->get_signature(last_ssig);
		scn->get_signature(curr_ssig);
		out.get_signature(osig);
		ydim = scn->get_dof();
		xdim = ydim + osig.size();
		
		if (last_ssig != curr_ssig) {
			// scene structure changed, can't predict this so don't train
			delete lastscn;
			lastscn = new scene(*scn);
			return;
		}
		
		model_sig msig = make_pair(last_ssig, osig);
		if ((i = models.find(msig)) == models.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);				
			models[msig] = mdl;
		} else {
			mdl = i->second;
		}

		scene_out_to_vec(lastscn, out, x);
		scene_to_vec(scn, y);
		mdl->add(x, y);
		delete lastscn;
		lastscn = new scene(*scn);
	}
	
private:
	map<model_sig, lwr*> models;
	scene *lastscn;
};

lwr_model *mdl = NULL;

learning_model *make_lwr_model() {
	if (!mdl) {
		mdl = new lwr_model();
	}
	return mdl;
}

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	make_lwr_model();
}
