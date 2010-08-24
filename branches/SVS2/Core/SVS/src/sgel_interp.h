#ifndef SGEL_INTERP_H
#define SGEL_INTERP_H

/* Scene graph editing language interpreter */

#include <istream>
#include <map>
#include <vector>
#include "sg_node.h"

class sgel_interp {
public:
	sgel_interp(sg_node *root);
	
	/* stops at first error and returns position, or -1 for success */
	int parse_line(std::string s);

private:
	int parse_attach(std::vector<std::string> &f);
	int parse_detach(std::vector<std::string> &f);
	int parse_change(std::vector<std::string> &f);

	std::map<std::string, sg_node*> nodes;
};

#endif
