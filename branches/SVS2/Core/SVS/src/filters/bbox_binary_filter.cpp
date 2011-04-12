/* A single class is sufficient to handle all binary operations between
   bounding boxes. Currently this includes intersection and containment.
*/

#include <iostream>
#include "filter.h"
#include "scene.h"
#include "bbox_filter.h"

using namespace std;

class bbox_binary_filter : public filter {
public:
	bbox_binary_filter(filter *node_a, filter *node_b, char type) 
	: container(this, new bbox_filter(node_a), new bbox_filter(node_b)), type(type)
	{}
	
	filter_result* calc_result() {
		bbox a, b;
		
		if (!get_bbox_filter_result_value(this, container.get(0), a) ||
		    !get_bbox_filter_result_value(this, container.get(1), b))
		{
			return NULL;
		}
		
		bool result;
		switch (type) {
			case 'i':
				result = a.intersects(b);
				break;
			case 'c':
				result = a.contains(b);
				break;
		}
		return new bool_filter_result(result);
	}
private:
	char type;
	filter_container container;
};

static filter* make_bbox_binary_filter(const filter_params &inputs, char type) {
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
	return new bbox_binary_filter(a, b, type);
}

filter* _make_bbox_int_filter_(scene *scn, const filter_params &inputs) {
	return make_bbox_binary_filter(inputs, 'i');
}

filter* _make_bbox_contains_filter_(scene *scn, const filter_params &inputs) {
	return make_bbox_binary_filter(inputs, 'c');
}
