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
	virtual ~filter_listener() {}
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

class string_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(std::string &r) = 0;
};

class vec3_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(vec3 &r) = 0;
};

class ptlist_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(ptlist &r) = 0;
};

class node_filter : public filter {
public:
	virtual bool get_result_string(std::string &r);
	virtual bool get_result(sg_node* &r) = 0;
};

class bbox_filter : public filter {
public:
	bool get_result_string(std::string &r);
	virtual bool get_result(bbox &r) = 0;
};

class const_string_filter : public string_filter {
public:
	const_string_filter(const std::string &s);
	bool        get_result(std::string &r);
	std::string get_error();
private:
	std::string s;
};

class const_node_filter : public node_filter, public sg_listener {
public:
	const_node_filter(sg_node *node);
	virtual ~const_node_filter();
	
	void        update(sg_node *n, sg_node::change_type t);
	bool        get_result(sg_node* &r);
	std::string get_error();

private:
	sg_node *node;
};

class node_ptlist_filter : public ptlist_filter, public filter_listener {
public:
	node_ptlist_filter(bool local, const_node_filter *nf);

	bool        get_result(ptlist &r);
	void        update(filter *f);
	std::string get_error();
	
private:
	bool              local;
	std::string       errmsg;
	const_node_filter *node_filter;
};

typedef std::multimap<std::string,filter*> filter_params;

filter* make_filter(std::string name, const filter_params &params);

#endif
