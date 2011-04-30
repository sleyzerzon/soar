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
typedef pair<scene_sig, output_sig> model_sig;

void scene_to_vec(const flat_scene &scn, rowvec &v) {
	vector<double>::const_iterator i;
	int j;
	v.reshape(1, scn.vals.size());
	for (i = scn.vals.begin(), j = 0; i != scn.vals.end(); ++i, ++j) {
		v(j) = *i;
	}
}

void scene_out_to_vec(const flat_scene &scn, const env_output &out, rowvec &v) {
	env_output_sig s;
	env_output_sig::const_iterator i;
	int j;
	out.get_signature(s);
	scene_to_vec(scn, v);
	j = v.n_elem;
	v.reshape(1, v.n_elem + s.size());
	for (i = s.begin(); i != s.end(); ++i, ++j) {
		v(j) = out.get(*i);
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
	
	bool predict(flat_scene &scn, const env_output &out) {
		scene_sig ssig;
		env_output_sig osig;
		map<model_sig, lwr*>::iterator i;
		rowvec x, y;
		
		scn.get_signature(ssig);
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
	
	void learn(const flat_scene &pre, const env_output &out, const flat_scene &post) {
		scene_sig ssig;
		env_output_sig osig;
		rowvec x, y;
		map<model_sig, lwr*>::iterator i;
		int k, xdim, ydim;
		lwr* mdl;

		post.get_signature(ssig);
		out.get_signature(osig);
		ydim = post.dof();
		xdim = ydim + osig.size();
		
		model_sig msig = make_pair(ssig, osig);
		if ((i = models.find(msig)) == models.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);				
			models[msig] = mdl;
		} else {
			mdl = i->second;
		}

		scene_out_to_vec(pre, out, x);
		scene_to_vec(post, y);
		mdl->add(x, y);
	}
	
private:
	map<model_sig, lwr*> models;
};

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	return new lwr_model();
}
