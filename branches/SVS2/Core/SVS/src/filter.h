#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include "linalg.h"
#include "sg_node.h"
#include "bbox.h"

class filter;

/* A listener that responds to changes in the result of some filter. */

class filter_listener {
public:
	virtual void update(filter *u) = 0;
};

/* A filter in the predicate extraction pipeline. Notifies its listeners
   whenever its calculated result changes. */

class filter {
public:
	virtual ~filter();
	
	virtual bool        get_result_string(std::string &r) = 0;
	virtual std::string get_error() = 0;

	/* all derived classes need to call this for each input filter */
	void add_child(filter *c);
	
	void listen(filter_listener *l);
	void unlisten(filter_listener *l);
	void notify();

private:
	std::list<filter_listener*> listeners;
	std::list<filter*> childs;
};

class bool_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(bool &r) = 0;
};

class ptlist_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(ptlist &r) = 0;
};

class bbox_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(bbox &r) = 0;
};

/* filter interface for scene graph nodes */
class sg_node_filter : public ptlist_filter, public sg_listener {
public:
	sg_node_filter(sg_node *node);
	~sg_node_filter();

	bool        get_result(ptlist &r);
	std::string get_error();
	void        update(sg_node *n, sg_node::change_type t);
	
private:
	sg_node *n;
};

typedef std::multimap<std::string,filter*> filter_params;

filter* make_filter(std::string name, filter_params &params);

#endif
