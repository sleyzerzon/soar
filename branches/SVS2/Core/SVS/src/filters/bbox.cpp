#include "filter.h"
#include "bbox.h"

/* bbox of a single node */
class bbox_filter : public map_filter<bbox> {
public:
	bbox_filter(filter_input *input) : map_filter<bbox>(input) {}
	
	bool compute(filter_param_set *params, bbox &b, bool adding) {
		sg_node *n;
		ptlist pts;
		
		if (!get_filter_param(this , params, "node", n)) {
			return false;
		}
		n->get_world_points(pts);
		b = bbox(pts);
		return true;
	}
};

filter* _make_bbox_filter_(scene *scn, filter_input *input) {
	return new bbox_filter(input);
}

/*
 Handles all binary operations between bounding boxes. Currently this
 includes intersection and containment.
*/
class bbox_binary_filter : public map_filter<bool> {
public:
	bbox_binary_filter(filter_input *input, char type) : map_filter<bool>(input), type(type) {}
	
	bool compute(filter_param_set *params, bool &res, bool adding) {
		filter_val *av, *bv;
		bbox a, b;
		
		if (!get_filter_param(this, params, "a", a)) {
			return false;
		}
		if (!get_filter_param(this, params, "b", b)) {
			return false;
		}
		
		switch (type) {
			case 'i':
				res = a.intersects(b);
				return true;
			case 'c':
				res = a.contains(b);
				return true;
			default:
				assert(false);
		}
	}

private:
	char type;
};

filter* _make_bbox_int_filter_(scene *scn, filter_input *input) {
	return new bbox_binary_filter(input, 'i');
}

filter* _make_bbox_contains_filter_(scene *scn, filter_input *input) {
	return new bbox_binary_filter(input, 'c');
}
