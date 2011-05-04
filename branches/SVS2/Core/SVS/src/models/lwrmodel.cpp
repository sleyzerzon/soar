#include <assert.h>
#include <set>
#include <armadillo>
#include "model.h"
#include "scene.h"
#include "linalg.h"
#include "lwr.h"

using namespace std;
using namespace arma;

const int NNBRS = 30;   // number of nearest neighbors to use

typedef set<string> output_sig;
typedef pair<scene_sig, outdesc> modelsig;
typedef map<modelsig, lwr*> modeltbl;

void scene_to_vec(const flat_scene &scn, rowvec &v) {
	vector<double>::const_iterator i;
	int j;
	v.reshape(1, scn.vals.size());
	for (i = scn.vals.begin(), j = 0; i != scn.vals.end(); ++i, ++j) {
		v(j) = *i;
	}
}

void scene_out_to_vec(const flat_scene &scn, const output &out, rowvec &v) {
	vector<double>::const_iterator i;
	int j;
	scene_to_vec(scn, v);
	j = v.n_elem;
	v.reshape(1, v.n_elem + out.size());
	for (i = out.vals.begin(); i != out.vals.end(); ++i, ++j) {
		v(j) = *i;
	}
}

void vec_to_scene(const rowvec &v, flat_scene &scn) {
	vector<double>::iterator i;
	int j;
	for (i = scn.vals.begin(), j = 0; i != scn.vals.end(); ++i, ++j) {
		*i = v(j);
	}
}

class lwr_model : public model {
public:
	lwr_model() {}
	
	bool predict(flat_scene &scn, const output &out) {
		scene_sig ssig;
		modeltbl::iterator i;
		rowvec x, y;
		
		scn.get_signature(ssig);
		modelsig msig = make_pair(ssig, *out.desc);
		
		if ((i = mdls.find(msig)) == mdls.end()) {
			return false;
		}
		
		scene_out_to_vec(scn, out, x);
		if (!i->second->predict(x, y)) {
			return false;
		}
		vec_to_scene(y, scn);
	}
	
	void learn(const flat_scene &pre, const output &out, const flat_scene &post) {
		scene_sig ssig;
		rowvec x, y;
		modeltbl::iterator i;
		int k, xdim, ydim;
		lwr* mdl;

		post.get_signature(ssig);
		ydim = post.dof();
		xdim = ydim + out.size();
		
		modelsig msig = make_pair(ssig, *out.desc);
		if ((i = mdls.find(msig)) == mdls.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);				
			mdls[msig] =mdl;
		} else {
			mdl = i->second;
		}

		scene_out_to_vec(pre, out, x);
		scene_to_vec(post, y);
		mdl->add(x, y);
	}
	
private:
	modeltbl mdls;
};

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	return new lwr_model();
}
