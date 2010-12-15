#include <iostream>
#include "../filter.h"
#include "bbox_filter.h"

using namespace std;

class bbox_int_filter : public filter {
public:
	bbox_int_filter(filter *node_a, filter *node_b) 
	: container(this, new bbox_filter(node_a), new bbox_filter(node_b))
	{}
	
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
