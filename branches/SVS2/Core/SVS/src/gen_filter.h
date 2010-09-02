#ifndef GEN_FILTER_H
#define GEN_FILTER_H

#include <vector>
#include "filter.h"
#include "sg_node.h"

class transform_filter : public filter {
public:
	virtual bool get_result(vec3 &p, vec3 &r, vec3 &s) = 0;
};

class gen_filter : public node_filter, public filter_listener {
public:
	gen_filter(std::string name,
	           ptlist_filter *pts,
	           vec3_filter *pos, 
	           vec3_filter *rot,
	           vec3_filter *scale,
	           std::vector<node_filter*> &children);

	void update(filter *u);
	bool get_result(sg_node* &r);
	bool get_result_string(std::string &r);
	std::string get_error();
	
private:
	bool get_transform(vec3_filter* f, vec3& t);
	
	std::string               name;
	std::string               errmsg;
	ptlist_filter*            pointf;
	vec3_filter*              posf;
	vec3_filter*              rotf;
	vec3_filter*              scalef;
	std::vector<node_filter*> childf;
};

filter* make_gen_filter(const filter_params &p);

#endif

