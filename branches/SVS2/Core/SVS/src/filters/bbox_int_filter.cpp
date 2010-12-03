#include "../filter.h"
#include "bbox_filter.h"

class bbox_int_filter : public filter {
public:
	bbox_int_filter(filter *a, filter *b) 
	: container(this)
	{
		bbox_filter *ab, *bb;
		ab = new bbox_filter(a);
		bb = new bbox_filter(b);
		container.add(ab);
		container.add(bb);
	}
	
	filter_result* calc_result() {
		bbox a, b;
		
		if (!get_bbox_filter_result_value(this, container.get(0), a) ||
		    !get_bbox_filter_result_value(this, container.get(1), b))
		{
			return NULL;
		}
		
		return new bool_filter_result(a.intersects(b));
	}
private:
	filter_container container;
};

filter* _make_bbox_int_filter_(const filter_params &inputs) {
	filter_params::const_iterator i;
	filter *a, *b;

	if (inputs.size() != 2) {
		return NULL;
	}
	
	i = inputs.begin(); 
	a = (i++)->second;
	b = i->second;
	if (!a || !b) {
		return NULL;
	}
	return new bbox_int_filter(a, b);
}
