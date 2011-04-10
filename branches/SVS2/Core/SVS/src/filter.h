#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <list>
#include <map>
#include "linalg.h"
#include "sg_node.h"

class filter;

typedef std::multimap<std::string,filter*> filter_params;
filter* make_filter(const std::string &name, const filter_params &params);

/* Interface for classes that need to respond to changes in the result
   of some filter. 
*/
class filter_listener {
public:
	virtual ~filter_listener() {}
	virtual void update(filter *u) = 0;
};

/* Wrapper for all filter result types to get around the type system
*/
class filter_result {
public:
	virtual std::string get_string() = 0;
};

class bool_filter_result : public filter_result {
public:
	bool_filter_result(bool r);
	std::string get_string();
	bool get_value();
private:
	bool r;
};

class string_filter_result : public filter_result {
public:
	string_filter_result(const std::string &r);
	std::string get_string();
	std::string get_value();
private:
	std::string r;
};

class vec3_filter_result : public filter_result {
public:
	vec3_filter_result(const vec3 &r);
	std::string get_string();
	vec3 get_value();
private:
	vec3 r;
};

class ptlist_filter_result : public filter_result {
public:
	ptlist_filter_result(const ptlist &r);
	std::string get_string();
	ptlist *get_value();
private:
	ptlist r;
};

class node_filter_result : public filter_result {
public:
	node_filter_result(sg_node *r);
	std::string get_string();
	sg_node *get_value();
private:
	sg_node *r;
};

/* Convenience functions for getting filter results as specific values
   with error checking
 */
bool get_bool_filter_result_value(filter *requester, filter *f, bool &v);
bool get_string_filter_result_value(filter *requester, filter *f, std::string &v);
bool get_vec3_filter_result_value(filter *requester, filter *f, vec3 &v);
bool get_ptlist_filter_result_value(filter *requester, filter *f, ptlist *&v);
bool get_node_filter_result_value(filter *requester, filter *f, sg_node *&v);

/* A filter in the predicate extraction pipeline. Notifies its listeners
   whenever its calculated result changes.
 */
class filter {
public:
	filter();
	virtual ~filter();
	
	void          listen(filter_listener *l);
	void          unlisten(filter_listener *l);
	void          notify();
	void          set_dirty();
	std::string   get_error();
	void          set_error(std::string msg);
	filter_result *get_result();
	
	virtual filter_result *calc_result() = 0;

private:
	filter_result *cached;
	bool dirty;
	std::list<filter_listener*> listeners;
	std::string errmsg;
};

class const_string_filter : public filter {
public:
	const_string_filter(const std::string &s);
	filter_result *calc_result();
private:
	std::string s;
};

class const_node_filter : public filter, public sg_listener {
public:
	const_node_filter(sg_node *node);
	virtual ~const_node_filter();
	
	void update(sg_node *n, sg_node::change_type t, int added);
	filter_result *calc_result();

private:
	sg_node *node;
};

class filter_container : public filter_listener {
public:
	filter_container(filter *owner);
	filter_container(filter *owner, filter* f);
	filter_container(filter *owner, filter* a, filter *b);
	filter_container(filter *owner, const std::vector<filter*> &filters);
	~filter_container();
	
	void   update(filter *f);
	void   add(filter *f);
	filter *get(int i);
	int    size();
	

private:
	filter *owner; 
	std::vector<filter*> filters;
};

#endif
