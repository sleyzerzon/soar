#include <assert.h>
#include <set>
#include <fstream>
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

class logger {
public:
	ofstream f;
	bool first;
	
	logger() : f("/tmp/modeldata.log"), first(true) {}
	
	void log(const rowvec &x, const rowvec &y) {
		if (first) {
			f << x.n_cols << " " << y.n_cols << endl;
			first = false;
		}
		for (int i = 0; i < x.n_cols; ++i) {
			f << x(i) << " ";
		}
		for (int i = 0; i < y.n_cols; ++i) {
			f << y(i) << " ";
		}
		f << endl;
	}
};

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

class lwr_model : public learning_model {
public:
	lwr_model() : lastscn(NULL) {}
	
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
	
	/* Add a training example to the model.  The training example
	   is encoded as a pair of vectors (x, y) where
	   x = [prev_state, output], y = [next_state]
	*/
	void add(const env_output &out, const flat_scene &scn) {
		scene_sig last_ssig, curr_ssig;
		env_output_sig osig;
		rowvec x, y;
		map<model_sig, lwr*>::iterator i;
		output_sig::iterator j;
		int k, xdim, ydim;
		lwr* mdl;
		
		if (lastscn == NULL) {
			// incomplete example, wait for next time
			lastscn = new flat_scene(scn);
			return;
		}
		
		lastscn->get_signature(last_ssig);
		scn.get_signature(curr_ssig);
		out.get_signature(osig);
		ydim = scn.vals.size();
		xdim = ydim + osig.size();
		
		if (last_ssig != curr_ssig) {
			// scene structure changed, can't predict this so don't train
			delete lastscn;
			lastscn = new flat_scene(scn);
			return;
		}
		
		model_sig msig = make_pair(last_ssig, osig);
		if ((i = models.find(msig)) == models.end()) {
			mdl = new lwr(xdim, ydim, NNBRS);				
			models[msig] = mdl;
		} else {
			mdl = i->second;
		}

		scene_out_to_vec(*lastscn, out, x);
		scene_to_vec(scn, y);
		mdl->add(x, y);
		lg.log(x, y);
		delete lastscn;
		lastscn = new flat_scene(scn);
	}
	
private:
	map<model_sig, lwr*> models;
	flat_scene *lastscn;
	logger lg;
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
