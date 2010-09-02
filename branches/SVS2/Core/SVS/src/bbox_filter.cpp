#include "bbox_filter.h"
#include "filter.h"
#include "linalg.h"
#include <vector>

using namespace std;

ptlist_bbox_filter::ptlist_bbox_filter(vector<ptlist_filter*> inputs)
: in(inputs), box(NULL), error(false)
{
	vector<ptlist_filter*>::iterator i;
	for (i = in.begin(); i != in.end(); ++i) {
		add_child(*i);
		(**i).listen(this);
	}
}

ptlist_bbox_filter::~ptlist_bbox_filter() {
	vector<ptlist_filter*>::iterator i;
	for (i = in.begin(); i != in.end(); ++i) {
		(**i).unlisten(this);
	}
}

bool ptlist_bbox_filter::get_result(bbox &r) {
	vector<ptlist_filter*>::iterator i;
	ptlist l;
	
	if (error) {
		return false;
	}
	if (!box) {
		i = in.begin();
		if (i == in.end()) {
			error = true;
			errmsg = "BBOX_NO_INPUTS";
			return false;
		}
		if (!(**i).get_result(l)) {
			error = true;
			errmsg = (**i).get_error();
			return false;
		}
		box = new bbox(l);
		for (++i; i != in.end(); ++i) {
			if (!(**i).get_result(l)) {
				error = true;
				errmsg = (**i).get_error();
				return false;
			}
			box->include(l);
		}
	}
	r = *box;
	return true;
}

string ptlist_bbox_filter::get_error() {
	return errmsg;
}

void ptlist_bbox_filter::update(filter *u) {
	delete box;
	box = NULL;
	notify();
}

bbox_int_filter::bbox_int_filter(bbox_filter *a, bbox_filter *b)
: ia(a), ib(b), dirty(true), error(false)
{
	add_child(ia);
	add_child(ib);
	ia->listen(this);
	ib->listen(this);
}

bbox_int_filter::~bbox_int_filter() {
	ia->unlisten(this);
	ib->unlisten(this);
}

void bbox_int_filter::update(filter *u) {
	dirty = true;
	notify();
}

bool bbox_int_filter::get_result(bool &r) {
	if (error) { return false; }
	if (dirty) {
		bbox a, b;

		if (!ia->get_result(a)) {
			error = true;
			errmsg = ia->get_error();
			return false;
		}
		
		if (!ib->get_result(b)) {
			error = true;
			errmsg = ib->get_error();
			return false;
		}
		result = a.intersects(b);
		dirty = false;
	}
	r = result;
	return true;
}

string bbox_int_filter::get_error() {
	return errmsg;
}

bbox_on_pos_filter::bbox_on_pos_filter(bbox_filter *bf, bbox_filter *tf) 
: bottomfilter(bf), topfilter(tf), dirty(true)
{
	add_child(bottomfilter);
	add_child(topfilter);
	bottomfilter->listen(this);
	topfilter->listen(this);

}

void bbox_on_pos_filter::update(filter *f) {
	dirty = true;
}

string bbox_on_pos_filter::get_error() {
	return errmsg;
}

bool bbox_on_pos_filter::get_result(vec3 &r) {
	bbox bot, top;
	double tx1, ty1, tz1, tx2, ty2, tz2, tcx, tcy;
	double bx1, by1, bz1, bx2, by2, bz2, bcx, bcy;
	
	if (dirty) {
		if (!bottomfilter->get_result(bot)) {
			errmsg = bottomfilter->get_error();
			return false;
		}
		bot.get_vals(bx1, by1, bz1, bx2, by2, bz2);
		bcx = (bx1+bx2)/2;
		bcy = (by1+by2)/2;
		
		if (!topfilter->get_result(top)) {
			errmsg = topfilter->get_error();
			return false;
		}
		top.get_vals(tx1, ty1, tz1, tx2, ty2, tz2);
		tcx = (tx1+tx2)/2;
		tcy = (ty1+ty2)/2;
		
		pos = vec3(bcx - tcx, bcy - tcy, bz2 - tz1);
	}
	r = pos;
	return true;
}

bbox_ontop_filter::bbox_ontop_filter(bbox_filter *bf, bbox_filter *tf) 
: bottomfilter(bf), topfilter(tf), dirty(true)
{
	add_child(bottomfilter);
	add_child(topfilter);
	bottomfilter->listen(this);
	topfilter->listen(this);

}

void bbox_ontop_filter::update(filter *f) {
	dirty = true;
}

string bbox_ontop_filter::get_error() {
	return errmsg;
}

bool bbox_ontop_filter::get_result(bool &r) {
	bbox bot, top;
	double tx1, ty1, tz1, tx2, ty2, tz2;
	double bx1, by1, bz1, bx2, by2, bz2;
	
	if (dirty) {
		if (!bottomfilter->get_result(bot)) {
			errmsg = bottomfilter->get_error();
			return false;
		}
		bot.get_vals(bx1, by1, bz1, bx2, by2, bz2);
		
		if (!topfilter->get_result(top)) {
			errmsg = topfilter->get_error();
			return false;
		}
		top.get_vals(tx1, ty1, tz1, tx2, ty2, tz2);
		
		ontop = bot.intersects(top) && bz2 == tz1;
	}
	r = ontop;
	return true;
}

filter* make_bbox_filter(const filter_params &inputs) {
	vector<ptlist_filter*> casted;
	filter_params::const_iterator i;
	
	ptlist_filter *q;

	casted.reserve(inputs.size());
	for (i = inputs.begin(); i != inputs.end(); ++i) {
		q = dynamic_cast<ptlist_filter*>(i->second);
		if (!q) {
			return NULL;
		}
		casted.push_back(q);
	}
	return new ptlist_bbox_filter(casted);
}

filter* make_bbox_int_filter(const filter_params &inputs) {
	filter_params::const_iterator i;
	bbox_filter *a, *b;

	if (inputs.size() < 2) {
		return NULL;
	}
	
	i = inputs.begin(); 
	a = dynamic_cast<bbox_filter*>((i++)->second);
	b = dynamic_cast<bbox_filter*>(i->second);
	if (!a || !b) {
		return NULL;
	}
	return new bbox_int_filter(a, b);
}

filter* make_bbox_on_pos_filter(const filter_params &p) {
	filter_params::const_iterator i;
	bbox_filter *bf, *tf;
	
	if ((i = p.find("bottom")) == p.end()) {
		return NULL;
	}
	bf = dynamic_cast<bbox_filter*>(i->second);
	if (!bf) {
		return NULL;
	}
	if ((i = p.find("top")) == p.end()) {
		return NULL;
	}
	tf = dynamic_cast<bbox_filter*>(i->second);
	if (!tf) {
		return NULL;
	}
	
	return new bbox_on_pos_filter(bf, tf);
}

filter* make_bbox_ontop_filter(const filter_params &p) {
	filter_params::const_iterator i;
	bbox_filter *bf, *tf;
	
	if ((i = p.find("bottom")) == p.end()) {
		return NULL;
	}
	bf = dynamic_cast<bbox_filter*>(i->second);
	if (!bf) {
		return NULL;
	}
	if ((i = p.find("top")) == p.end()) {
		return NULL;
	}
	tf = dynamic_cast<bbox_filter*>(i->second);
	if (!tf) {
		return NULL;
	}
	
	return new bbox_ontop_filter(bf, tf);
}
