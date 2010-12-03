#ifndef GEN_FILTER_H
#define GEN_FILTER_H

#include <vector>
#include "../filter.h"
#include "sg_node.h"

class gen_filter : public filter {
public:
	gen_filter(std::string name,
	           filter *pts,
	           filter *pos, 
	           filter *rot,
	           filter *scale,
	           std::vector<filter*> &children);

	filter_result *calc_result();
	
private:
	bool attach_children(sg_node *n);
	
	std::string          name;
	filter*              pointf;
	filter*              posf;
	filter*              rotf;
	filter*              scalef;
	std::vector<filter*> childf;
	filter_container     container;
};

filter* make_gen_filter(const filter_params &p);

#endif

