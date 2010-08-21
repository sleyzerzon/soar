#ifndef SGEL_INTERP_H
#define SGEL_INTERP_H

/* Scene graph editing language interpreter */

#include <istream>
#include <map>
#include "sg_node.h"

class sgel_interp {
public:
	sgel_interp(sg_node *w);
	
	/* stops at first error and returns position, or -1 for success */
	int parse(std::string s);

	int parse_add(std::vector<std::string> &f);

private:
	std::map<std::string, sg_node*> nodes;
};

#endif
