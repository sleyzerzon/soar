#ifndef SGEL_INTERP_H
#define SGEL_INTERP_H

/* Scene graph editing language interpreter */

#include <map>
#include <vector>
#include "scene.h"

class sgel_interp {
public:
	sgel_interp(scene *_scn);
	
	/* stops at first error and returns position, or -1 for success */
	int parse_line(std::string s);

private:
	int parse_attach(std::vector<std::string> &f);
	int parse_detach(std::vector<std::string> &f);
	int parse_change(std::vector<std::string> &f);
	
	scene *scn;
};

#endif
