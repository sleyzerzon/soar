#ifndef ENV_H
#define ENV_H

#include <vector>
#include <string>
#include <map>
#include "ipcsocket.h"

typedef struct dim_desc_struct {
	double min;
	double max;
	double inc;
} dim_desc;

typedef std::map<std::string, dim_desc> env_output_desc;

class env_output {
public:
	env_output(const env_output_desc &d);
	env_output(const env_output &other);
	
	double get(const std::string &dim);
	void   set(const std::string &dim, double val);
	
	bool increment();
	void serialize(std::string &out);
	
private:
	std::map<std::string, double> value;
	env_output_desc desc;
};

class environment {
public:
	environment(std::string path);

	bool output(env_output &out);
	bool input(std::string &sgel);
	
private:
	ipcsocket sock;
};

#endif
