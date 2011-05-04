#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <vector>
#include <armadillo>

class out_dim_desc {
public:
	std::string name;
	double min;
	double max;
	double inc;
	
	bool operator<(const out_dim_desc &other) const;
	bool operator==(const out_dim_desc &other) const;
	bool operator!=(const out_dim_desc &other) const;
};

typedef std::vector<out_dim_desc> outdesc;

class output {
public:
	output();
	output(const outdesc *d);
	output(const output &other);
	
	double get(const std::string &dim) const;
	void   set(const std::string &dim, double val);
	int    size() const;
	bool   valid() const;
	
	void reset();
	bool next();
	std::string serialize() const;
	
	void operator=(const output &o);
	
	const outdesc *desc;
	std::vector<double> vals;
};

class trajectory {
public:
	trajectory(int length, const outdesc *d);
	
	void from_vec(const arma::vec &traj);
	void reset();
	bool next();
	int dof();
	
	const outdesc *desc;
	int length;
	std::vector<output> t;
};

#endif
