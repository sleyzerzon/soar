#ifndef LWR_H
#define LWR_H

#include <vector>
#include "common.h"

class lwr {
public:
	lwr(int xdim, int ydim, int nnbrs);
	void add(const floatvec &x, const floatvec &y);
	bool load_file(const char *file);
	bool predict(const floatvec &x, floatvec &y, char method, bool mahalanobis);
	int size();
	
private:
	void normalize();
	
	int xdim, ydim, nnbrs;
	std::vector<std::pair<floatvec, floatvec> > examples;
	std::vector<floatvec> xnorm;
	floatvec xmin, xmax, xrange;
	bool normalized;
};

#endif
