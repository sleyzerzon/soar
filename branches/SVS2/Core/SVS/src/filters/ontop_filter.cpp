#include "filter.h"
#include "bbox.h"
#include "scene.h"

class ontop_filter : public filter {
public:
	ontop_filter(filter *bottom_node, filter *top_node)
	: container(this)
	{
		container.add(bottom_node);
		container.add(top_node);
	}

	filter_result *calc_result() {
		sg_node *tn, *bn;
		ptlist tp, bp;
		double tx1, ty1, tz1, tx2, ty2, tz2;
		double bx1, by1, bz1, bx2, by2, bz2;
		
		if (!get_node_filter_result_value(this, container.get(0), tn) ||
		    !get_node_filter_result_value(this, container.get(1), bn))
		{
			return NULL;
		}
		
		tn->get_world_points(tp);
		bn->get_world_points(bp);
		
		bbox tbox(tp), bbox(bp);
		bbox.get_vals(bx1, by1, bz1, bx2, by2, bz2);
		tbox.get_vals(tx1, ty1, tz1, tx2, ty2, tz2);
		return new bool_filter_result(bbox.intersects(tbox) && bz2 == tz1);
	}
	
private:
	filter_container container;
};


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
