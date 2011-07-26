#ifndef LWR_H
#define LWR_H

#include <vector>
#include "model.h"
#include "common.h"
#include "nn.h"

class lwr : public model {
public:
	lwr(int nnbrs, const std::vector<std::string> &inputs, const std::vector<std::string> &outputs);
	void learn(const floatvec &x, const floatvec &y, float dt);
	bool predict(const floatvec &x, floatvec &y);
	std::string get_type() const { return "lwr"; }
	int get_input_size() const { return xnames.size(); }
	int get_output_size() const { return ynames.size(); }
	void get_slots(std::vector<std::string> &ins, std::vector<std::string> &outs) const;

	bool load_file(const char *file);
	int size() const;
	
private:
	void normalize();
	
	int xsize, ysize, nnbrs;
	std::vector<std::string> xnames;
	std::vector<std::string> ynames;

	std::vector<std::pair<floatvec, floatvec> > examples;
	std::vector<floatvec> xnorm;
	floatvec xmin, xmax, xrange;
	bool normalized;
	nearest_neighbor *nn;
};

#endif
