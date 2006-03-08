#ifndef _CLUSTER_
#define _CLUSTER_

//#include <map>
#include <vector>
#include <hash_map>
#include <string>
#include <iostream>

using std::vector;
using std::string;
using std::cout;
using std::ostream;
using std::endl;
using std::pair;
using stdext::hash_map;
using stdext::hash_compare;
using std::less;
using std::allocator;

// This is a single unit in the network
// This class should support unit based operations: activation = unit*input, input2=input1-unit
class Unit
{
private:
	
	int max_dim;
	int counter;
public:
	vector<double> weights;
	Unit(int max_dim);
	double activation(vector<double> input);
	void update(vector<double> input); // learning rate is determined within the unit
	vector<double> subtract_input(vector<double>&);
	// http://mathworld.wolfram.com/HyperspherePointPicking.html
	// Uniform distribution from a hypersphere
	vector<double> hypersphere(int dim);

};

typedef stdext::hash_map<string, int> HASH_S_INT;
typedef stdext::hash_map<string, HASH_S_INT> HASH_S_S_INT;

class NetWork
{
private:
	
	HASH_S_S_INT attr_val_pair_to_index;
	int winner(vector<double> input, vector<int> inhibit=vector<int>());
	double activation(vector<double> input, int index);
	

public:
	vector<Unit> units;
	vector<pair<string, string> > index_to_attr_val_pair;

	NetWork(int n_units, int max_dim);
	// raw input
	vector<double> translate_input(vector<pair<string, string> >& attr_val_pairs);
	// returned the winner index and subtracted input
	int train_input_one_cycle(vector<double>& input, vector<double>& new_input, vector<int> inhibit=vector<int>(), bool update=true);
	
	vector<int> cluster_input(vector<double> input, bool update=true);

	//void Cluster(vector<pair<string, string>& attr_val_pairs);
	
};




#endif