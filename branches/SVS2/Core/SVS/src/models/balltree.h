#ifndef BALLTREE_H
#define BALLTREE_H

#include <vector>
#include <queue>
#include <utility>
#include <armadillo>

typedef std::pair<double, int> di_pair;
typedef std::priority_queue<di_pair> di_queue;

class balltree {
public:
	balltree(int ndim, int leafsize, std::vector<arma::rowvec> *pts, const std::vector<int> &inds);
	~balltree();
	
	// returns number of points pruned
	int query(const arma::rowvec &q, int k, di_queue &nn);
	
private:
	void distsq_to(const arma::rowvec &q, std::vector<double> &dout);
	void update_ball();
	void split();
	void linear_scan(const arma::rowvec &q, int k, di_queue &nn);
	
	balltree *left, *right, *parent;
	arma::rowvec center;
	double radius;
	std::vector<int> inds;
	std::vector<arma::rowvec> *pts;
	int ndim;
	int leafsize;
};

#endif
