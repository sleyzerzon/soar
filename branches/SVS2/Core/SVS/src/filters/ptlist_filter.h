#ifndef PTLIST_FILTER_H
#define PTLIST_FILTER_H

#include "../sg_node.h"
#include "../linalg.h"
#include "../filter.h"

class ptlist_filter : public filter {
public:
	ptlist_filter(bool local, filter *node);
	filter_result *calc_result();
private:
	bool              local;
	filter_container  container;
};

#endif
