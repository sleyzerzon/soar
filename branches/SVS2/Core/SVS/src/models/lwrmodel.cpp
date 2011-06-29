#include <assert.h>
#include <algorithm>
#include <iterator>
#include "model.h"
#include "scene.h"
#include "linalg.h"
#include "lwr.h"

using namespace std;

typedef pair<vector<string>, outdesc> modelsig;
typedef map<modelsig, lwr*> modeltbl;

bool splinter_scene_update(flat_scene &scn, const output &out);

class window {
public:
	window(int n) : w(n), i(0) {}
	
	void insert(float v) {
		w[i++] = v;
		if ( i == w.size() ) {
			i = 0;
		}
	}
	
	float mean() {
		return w.sum() / w.size();
	}
	
private:
	floatvec w;
	int i;
};

class lwr_model : public model {
public:
	lwr_model(int nnbrs) : nnbrs(nnbrs), dtwin(20) {}
	
	~lwr_model() {
		modeltbl::iterator i;
		for (i = mdls.begin(); i != mdls.end(); ++i) {
			delete i->second;
		}
	}
	
	bool predict(flat_scene &scn, const trajectory &trj) {
		vector<string> colnames;
		modeltbl::iterator i;
		lwr *m;
		flat_scene orig(scn);
		//flat_scene ref(scn);
		vector<output>::const_iterator j;
		
		if (trj.t.size() == 0) {
			return true;
		}
		
		scn.get_column_names(colnames);
		modelsig msig = make_pair(colnames, *trj.t.front().desc);
		
		if ((i = mdls.find(msig)) == mdls.end()) {
			return false;
		}
		
		float dt = dtwin.mean();
		m = i->second;
		floatvec x(scn.dof() + trj.t.front().size()), y(scn.vals), dy(scn.vals.size());
		for (j = trj.t.begin(); j != trj.t.end(); ++j) {
			x.combine(y, j->vals);
			if (!m->predict(x, dy, 'r', false)) {
				return false;
			}
			dy *= dt;
			y += dy;
			//splinter_scene_update(ref, *j);
		}
		//cout << sqrt(ref.vals.distsq(y)) << endl;
		scn.vals = y;
		return true;
	}
	
	void learn(const flat_scene &pre, const output &out, const flat_scene &post, float dt) {
		vector<string> colnames;
		modeltbl::iterator i;
		int xdim, ydim;
		lwr* mdl;

		pre.get_column_names(colnames);
		ydim = post.dof();
		xdim = ydim + out.size();
		
		modelsig msig = make_pair(colnames, *out.desc);
		if ((i = mdls.find(msig)) == mdls.end()) {
			mdl = new lwr(xdim, ydim, nnbrs);
			//mdl->load_file("training_data");
			mdls[msig] = mdl;
		} else {
			mdl = i->second;
		}

		floatvec x(xdim), y(post.vals);
		x.combine(pre.vals, out.vals);
		y -= pre.vals;
		y /= dt;
		mdl->add(x, y);
		dtwin.insert(dt);
	}
	
	void printinfo() const {
		modeltbl::const_iterator i;
		cout << "LWR ";
		for (i = mdls.begin(); i != mdls.end(); ++i) {
			cout << i->second->size() << " ";
		}
		cout << endl;
	}
	
private:
	int nnbrs;
	modeltbl mdls;
	window dtwin;
};

model *_make_lwr_model_(soar_interface *si, Symbol *root) {
	wme *w;
	long nnbrs = 50;
	
	if (!si->find_child_wme(root, "num-neighbors", w) ||
	    !si->get_val(si->get_wme_val(w), nnbrs))
	{
		cerr << "Using default 50 neighbors" << endl;
	}
	return new lwr_model(nnbrs);
}
