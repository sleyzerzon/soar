#ifndef BBOX_FILTER_H
#define BBOX_FILTER_H

#include "bbox.h"
#include "filter.h"
#include <vector>

/* bbox from a bunch of ptlists */
class ptlist_bbox_filter : public bbox_filter, public filter_listener {
public:
	ptlist_bbox_filter(std::vector<ptlist_filter*> inputs);
	~ptlist_bbox_filter();
	
	bool        get_result(bbox &r);
	std::string get_error();
	void        update(filter *u);

private:
	bool                        error;
	std::string                 errmsg;
	bbox*                       box;
	std::vector<ptlist_filter*> in;
};

/* bbox intersection */
class bbox_int_filter : public bool_filter, public filter_listener {
public:
	bbox_int_filter(bbox_filter *a, bbox_filter *b);
	~bbox_int_filter();
	
	bool        get_result(bool &r);
	std::string get_error();
	void        update(filter *u);
	
private:
	bbox_filter *ia, *ib;
	bool result;
	bool dirty;
	bool error;
	std::string errmsg;
};

class bbox_on_pos_filter : public vec3_filter, public filter_listener {
public:
	bbox_on_pos_filter(bbox_filter *bf, bbox_filter *tf);
	
	void   update(filter *f);
	string get_error();
	bool   get_result(vec3 &r);
	
private:
	bbox_filter* bottomfilter;
	bbox_filter* topfilter;
	bool         dirty;
	vec3         pos;
	std::string  errmsg;
};

#endif
