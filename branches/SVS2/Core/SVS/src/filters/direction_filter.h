#ifndef DIRECTION_FILTER_H
#define DIRECTION_FILTER_H

#include "filter.h"

class direction_filter : public filter {
public:
	direction_filter(char dir, filter *a, filter *b);
	bool get_direction(vec3 &result);
	filter_result *calc_result();
private:
	filter_container container;
	char dir;
};

#endif
