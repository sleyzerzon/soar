#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <iterator>

#include "linalg.h"
#include "sg_node.h"
#include "scene.h"
#include "soar_interface.h"

class filter;

typedef std::multimap<std::string,filter*> filter_params;
filter* make_filter(const std::string &name, scene *scn, const filter_params &params);

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
	bool_filter_result(bool v) : v(v) {}
	std::string get_string() { return v ? "true" : "false"; }
	bool get_value() { return v; }
private:
	bool v;
};

class string_filter_result : public filter_result {
public:
	string_filter_result(const std::string &v) : v(v) {}
	std::string get_string() { return v; }
	std::string get_value() { return v; }
private:
	std::string v;
};

class int_filter_result : public filter_result {
public:
	int_filter_result(int v) : v(v) {}
	std::string get_string() {
		std::stringstream ss;
		ss << v;
		return ss.str();
	}
	int get_value() { return v; }
private:
	int v;
};

class double_filter_result : public filter_result {
public:
	double_filter_result(double v) : v(v) {}
	std::string get_string() {
		std::stringstream ss;
		ss << v;
		return ss.str();
	}
	double get_value() { return v; }
private:
	double v;
};

class vec3_filter_result : public filter_result {
public:
	vec3_filter_result(const vec3 &v) : v(v) {}
	std::string get_string() {
		std::stringstream ss;
		ss << v;
		return ss.str();
	}
	vec3 get_value() { return v; }
private:
	vec3 v;
};

class ptlist_filter_result : public filter_result {
public:
	ptlist_filter_result(const ptlist &v) : v(v) {}
	std::string get_string() {
		std::stringstream ss;
		copy(v.begin(), v.end(), std::ostream_iterator<vec3>(ss, ", "));
		return ss.str();
	}
	ptlist *get_value() { return &v; }
private:
	ptlist v;
};

class node_filter_result : public filter_result {
public:
	node_filter_result(sg_node *v) : v(v) {}
	std::string get_string() { return v->get_name(); }
	sg_node *get_value() { return v; }
private:
	sg_node *v;
};

/* Convenience functions for getting filter results as specific values
   with error checking
 */
bool get_bool_filter_result_value(filter *requester, filter *f, bool &v);
bool get_string_filter_result_value(filter *requester, filter *f, std::string &v);
bool get_double_filter_result_value(filter *requester, filter *f, double &v);
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
	const_string_filter(const std::string &v) : v(v) {}
	filter_result *calc_result() { return new string_filter_result(v); }
private:
	std::string v;
};

class const_int_filter : public filter {
public:
	const_int_filter(int v) : v(v) {}
	filter_result *calc_result() { return new int_filter_result(v); }
private:
	int v;
};

class const_double_filter : public filter {
public:
	const_double_filter(double v) : v(v) {}
	filter_result *calc_result() { return new double_filter_result(v); }
private:
	double v;
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

/* Create a filter from a WM structure. Recursive. */
filter *parse_filter_struct(soar_interface *si, Symbol *root, scene *scn);

#endif
