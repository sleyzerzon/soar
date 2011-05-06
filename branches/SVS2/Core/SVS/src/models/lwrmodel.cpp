#include <assert.h>
#include <algorithm>
#include <armadillo>
#include "model.h"
#include "scene.h"
#include "linalg.h"
#include "lwr.h"

using namespace std;
using namespace arma;

const int NNBRS = 30;   // number of nearest neighbors to use

typedef pair<scene_sig, outdesc> modelsig;
typedef map<modelsig, lwr*> modeltbl;

// to compare
bool splinter_scene_update(flat_scene &scn, const output &out);

void scene_to_vec(const flat_scene &scn, rowvec &v) {
	v.reshape(1, scn.dof());
	copy(scn.vals.begin(), scn.vals.end(), v.begin());
	assert(v(0) == scn.vals[0]);
}

void output_to_vec(const output &out, rowvec &v) {
	v.reshape(1, out.vals.size());
	copy(out.vals.begin(), out.vals.end(), v.begin());
}

void scene_out_to_vec(const flat_scene &scn, const output &out, rowvec &v) {
	v.reshape(1, scn.dof() + out.vals.size());
	rowvec::iterator i = v.begin();
	copy(scn.vals.begin(), scn.vals.end(), i);
	copy(out.vals.begin(), out.vals.end(), i);
}

void vec_to_scene(const rowvec &v, flat_scene &scn) {
	assert(v.n_elem == scn.vals.size());
	copy(v.begin(), v.end(), scn.vals.begin());
}

class lwr_model : public model {
public:
	lwr_model() {}
	
	bool predict(flat_scene &scn, const trajectory &trj) {
		scene_sig ssig;
		modeltbl::iterator i;
		rowvec x, y, outvec;
		flat_scene copy(scn);
		vector<output>::const_iterator j;
		
		scn.get_signature(ssig);
		modelsig msig = make_pair(ssig, *trj.t.front().desc);
		
		if ((i = mdls.find(msig)) == mdls.end()) {
			return false;
		}
		
		x.reshape(1, scn.dof() + trj.t.front().size());
		scene_to_vec(scn, y);
		for (j = trj.t.begin(); j != trj.t.end(); ++j) {
			x.subvec(0, y.n_elem - 1) = y;
			output_to_vec(*j, outvec);
			x.subvec(y.n_elem, x.n_elem - 1) = outvec;
			if (!i->second->predict(x, y)) {
				return false;
			}
			splinter_scene_update(copy, *j);
		}
		vec_to_scene(y, scn);
		double error = copy.distance(scn);
		
		cout << "E " << error << endl;
		
		return true;
	}
	
	void learn(const flat_scene &pre, const output &out, const flat_scene &post) {
		scene_sig ssig;
		rowvec x, y;
		modeltbl::iterator i;
		int xdim, ydim;
		lwr* mdl;

		pre.get_signature(ssig);
		ydim = post.dof();
		xdim = ydim + out.size();
		
		modelsig msig = make_pair(ssig, *out.desc);
		if ((i = mdls.find(msig)) == mdls.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);				
			mdls[msig] = mdl;
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
