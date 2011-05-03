#ifndef ENV_H
#define ENV_H

#include <string>
#include <map>
#include <set>

typedef struct dim_desc_struct {
	double min;
	double max;
	double inc;
} dim_desc;

typedef std::map<std::string, dim_desc> env_output_desc;
typedef std::set<std::string> env_output_sig;
class env_output {
public:
	env_output();
	env_output(const env_output_desc &d);
	env_output(const env_output &other);
	
	double get(const std::string &dim) const;
	void   set(const std::string &dim, double val);
	void   get_signature(env_output_sig &sig) const;
	int    size() const;
	
	bool increment();
	std::string serialize() const;
	
	void operator=(const env_output &o);
	
private:
	std::map<std::string, double> value;
	env_output_desc desc;
};

#endif
