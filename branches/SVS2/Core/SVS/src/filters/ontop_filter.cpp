#include "ontop_filter.h"
#include "bbox_filter.h"
#include "scene.h"

ontop_filter::ontop_filter(filter *bottom_node, filter *top_node) 
: container(this)
{
	container.add(new bbox_filter(bottom_node));
	container.add(new bbox_filter(top_node));
}

filter_result *ontop_filter::calc_result() {
	bbox bot, top;
	double tx1, ty1, tz1, tx2, ty2, tz2;
	double bx1, by1, bz1, bx2, by2, bz2;

	if (!get_bbox_filter_result_value(this, container.get(0), bot) ||
	    !get_bbox_filter_result_value(this, container.get(1), top))
	{
		return NULL;
	}
	
	bot.get_vals(bx1, by1, bz1, bx2, by2, bz2);
	top.get_vals(tx1, ty1, tz1, tx2, ty2, tz2);
	return new bool_filter_result(bot.intersects(top) && bz2 == tz1);
}

filter* _make_ontop_filter_(scene *scn, const filter_params &p) {
	filter_params::const_iterator i, j;
	
	if ((i = p.find("bottom")) == p.end()) {
		return NULL;
	}
	if ((j = p.find("top")) == p.end()) {
		return NULL;
	}
	return new ontop_filter(i->second, j->second);
}
