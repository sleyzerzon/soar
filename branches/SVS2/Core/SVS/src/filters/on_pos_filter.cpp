#include "filter.h"
#include "scene.h"
#include "on_pos_filter.h"

on_pos_filter::on_pos_filter(filter *bottom_node, filter *top_node) 
: container(this)
{
	container.add(new bbox_filter(bottom_node));
	container.add(new bbox_filter(top_node));
}

filter_result *on_pos_filter::calc_result() {
	bbox bot, top;
	double tx1, ty1, tz1, tx2, ty2, tz2, tcx, tcy;
	double bx1, by1, bz1, bx2, by2, bz2, bcx, bcy;
	
	if (container.size() != 2) {
		set_error("incorrect number of inputs");
		return NULL;
	}
	
	if (!get_bbox_filter_result_value(this, container.get(0), bot) ||
	    !get_bbox_filter_result_value(this, container.get(1), top))
	{
		return NULL;
	}
	
	bot.get_vals(bx1, by1, bz1, bx2, by2, bz2);
	bcx = (bx1+bx2)/2;
	bcy = (by1+by2)/2;
	
	top.get_vals(tx1, ty1, tz1, tx2, ty2, tz2);
	tcx = (tx1+tx2)/2;
	tcy = (ty1+ty2)/2;
	
	return new vec3_filter_result(vec3(bcx - tcx, bcy - tcy, bz2 - tz1));
}

filter* _make_on_pos_filter_(scene *scn, const filter_params &p) {
	filter_params::const_iterator i, j;
	
	if ((i = p.find("bottom")) == p.end()) {
		return NULL;
	}

	if ((i = p.find("top")) == p.end()) {
		return NULL;
	}
	
	return new on_pos_filter(i->second, j->second);
}
