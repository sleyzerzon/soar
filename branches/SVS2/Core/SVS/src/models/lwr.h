#ifndef LWR_H
#define LWR_H

class lwr {
public:
	lwr(int xdim, int ydim, int nnbrs);
	void add(const arma::rowvec &x, const arma::rowvec &y);
	bool predict(const arma::rowvec &x, arma::rowvec &y, char method);
	
	int size();
	
private:
	void nearest(int k, arma::rowvec x, arma::mat &X, arma::mat &Y, arma::vec &d);
	void remove_static(arma::mat &X, arma::mat &Xout, std::vector<int> &dynamic);
	
	int xdim, ydim, nnbrs;
	std::vector<std::pair<arma::rowvec, arma::rowvec> > db;
};

#endif
