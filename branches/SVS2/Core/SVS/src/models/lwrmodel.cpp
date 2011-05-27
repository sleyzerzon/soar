#include <assert.h>
#include <algorithm>
#include <iterator>
#include <armadillo>
#include "model.h"
#include "scene.h"
#include "linalg.h"
#include "lwr.h"

using namespace std;
using namespace arma;

const int NNBRS = 50;   // number of nearest neighbors to use

typedef pair<vector<string>, outdesc> modelsig;
typedef map<modelsig, lwr*> modeltbl;

// to compare
bool splinter_scene_update(flat_scene &scn, const output &out);

class lwr_model : public model {
public:
	lwr_model() {}
	
	bool predict(flat_scene &scn, const trajectory &trj) {
		vector<string> colnames;
		modeltbl::iterator i;
		lwr *m;
		flat_scene ref(scn), orig(scn);
		vector<output>::const_iterator j;
		
		if (trj.t.size() == 0) {
			return true;
		}
		
		scn.get_column_names(colnames);
		modelsig msig = make_pair(colnames, *trj.t.front().desc);
		
		if ((i = mdls.find(msig)) == mdls.end()) {
			return false;
		}
		
		m = i->second;
		floatvec x(scn.dof() + trj.t.front().size()), y(scn.vals);
		for (j = trj.t.begin(); j != trj.t.end(); ++j) {
			x.combine(y, j->vals);
			if (!m->predict(x, y, 'r', false)) {
				return false;
			}
			splinter_scene_update(ref, *j);
		}
		scn.vals = y;
		
		//rowvec refvec;
		//scene_to_vec(ref, refvec);
		//rowvec err = abs(y - refvec);
		//err.print("error");
		
		return true;
	}
	
	void learn(const flat_scene &pre, const output &out, const flat_scene &post) {
		vector<string> colnames;
		modeltbl::iterator i;
		int xdim, ydim;
		lwr* mdl;

		pre.get_column_names(colnames);
		ydim = post.dof();
		xdim = ydim + out.size();
		
		modelsig msig = make_pair(colnames, *out.desc);
		if ((i = mdls.find(msig)) == mdls.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);
			mdl->load_file("training_data");
			mdls[msig] = mdl;
		} else {
			mdl = i->second;
		}

		floatvec x(xdim);
		x.combine(pre.vals, out.vals);
		mdl->add(x, post.vals);
	}
	
private:
	modeltbl mdls;
};

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	return new lwr_model();
}
