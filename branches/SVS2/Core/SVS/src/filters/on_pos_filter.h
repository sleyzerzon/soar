#ifndef ON_POS_FILTER_H
#define ON_POS_FILTER_H

#include "bbox_filter.h"

class on_pos_filter : public filter {
public:
	on_pos_filter(filter *bottom_node, filter *top_node);
	filter_result *calc_result();
private:
	filter_container container;
};

#endif
