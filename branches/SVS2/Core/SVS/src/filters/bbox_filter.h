#ifndef BBOX_FILTER_H
#define BBOX_FILTER_H

#include <vector>
#include "bbox.h"
#include "filter.h"

class bbox_filter_result : public filter_result {
public:
	bbox_filter_result(bbox r);
	std::string get_string();
	bbox get_value();
private:
	bbox r;
};

bool get_bbox_filter_result_value(filter *requester, filter *f, bbox &b);

/* bbox of the world points of a set of nodes */
class bbox_filter : public filter {
public:
	bbox_filter(filter* node);
	bbox_filter(std::vector<filter*> nodes);
	filter_result* calc_result();
private:
	filter_container container;
};

#endif
