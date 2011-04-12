#ifndef ONTOP_H
#define ONTOP_H

#include "filter.h"

class ontop_filter : public filter {
public:
	ontop_filter(filter *bottom_node, filter *top_node);
	filter_result *calc_result();
private:
	filter *bottom_bbox;
	filter *top_bbox;
	filter_container container;
};

#endif
