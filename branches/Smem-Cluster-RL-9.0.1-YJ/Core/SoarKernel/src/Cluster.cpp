// Cluster.cpp : Defines the entry point for the console application.
//
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "portability.h"


//#include "stdafx.h"
#include "Cluster.h"
#include <sstream>
#include <cmath>


float ranf(){
	float  randf = rand()%10000*1.0/10000;
	//cout << "Rand: " << randf << endl;
	return randf;
}
// Generating Gaussian random variables
// Copied from: http://www.taygeta.com/random/gaussian.html
// It's generating 2 random variables at the same time, and chace the second for next use
float box_muller(float m, float s)	/* normal random variate generator */
{				        /* mean m, standard deviation s */
	float x1, x2, w, y1;
	static float y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * ranf() - 1.0;
			x2 = 2.0 * ranf() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( m + y1 * s );
}



Unit::Unit(int dim){
	//this->weights = vector<double>(dim, 0);
	this->weights = this->hypersphere(dim);
	max_dim = dim;
	this->counter = 0;
}

// http://mathworld.wolfram.com/HyperspherePointPicking.html
vector<double> Unit::hypersphere(int dim){
	vector<double> rand_norm;
	float sum_square = 0;
	for(int i=0; i< dim; ++i){
		float rand_num = box_muller(0, 1);
		rand_norm.push_back(rand_num);
		//cout << rand_num << " ";
		sum_square += rand_num * rand_num;
	}
	//cout << endl;
	float normalization = sqrt(sum_square);

	for(int i=0; i< dim; ++i){
		rand_norm[i] /= normalization;
		//cout << rand_norm[i] << " ";
	}
	//cout << endl;
	return rand_norm;
}

double Unit::activation(vector<double> input){
	double total_activation = 0;
	for(int i=0; i<(int)(input.size()); ++i){
		if(i >= this->max_dim){
			cout << "input " << input.size() << " exceeds maximum dimension " << this->max_dim << endl;
			exit(1);
		}
		total_activation += input[i] * this->weights[i];
	}
	return total_activation;
}

double Unit::activation_distance(vector<double> input){
	double total_activation = 0;
	for(int i=0; i<(int)(input.size()); ++i){
		if(i >= this->max_dim){
			cout << "input " << input.size() << " exceeds maximum dimension " << this->max_dim << endl;
			exit(1);
		}
		total_activation += pow((input[i] - this->weights[i]),2);
	}
	return 1/sqrt(total_activation);
}

void Unit::update(vector<double> input, double decay_rate){
	double base_rate = 0.2;
	// 0.5 is default rate, 0.3 is slower decaying - slower convergence, but more robust???
	//double learn_rate = base_rate * pow((float)(this->counter+1), (float)-0.5);
	double learn_rate = base_rate * pow((float)(this->counter+1), (float)decay_rate);
	//cout << learn_rate << endl;
	//for(int i=0; i<(int)(this->max_dim); ++i){
	for(int i=0; i<(int)(input.size()); ++i){
	//	cout << this->weights[i]<<" ";
		double current_bit = 0;
		if(i < input.size()){
			current_bit = input[i]*1.0;
		}
		double delta = (current_bit - this->weights[i]*1.0) * learn_rate;
		this->weights[i] += delta;
		//cout << input[i]<<" "<<delta<<" "<<this->weights[i] << ", ";
	}
	this->counter++;
	//cout << endl;
}

vector<double> Unit::subtract_input(vector<double>& input){
	vector<double> new_input = vector<double>();
	for(int i=0; i<(int)(input.size()); ++i){
		new_input.push_back(input[i] - this->weights[i]);
	}
	return new_input;
}


//#######

NetWork::NetWork(int n_units, int max_dim){
	srand ((unsigned)time( NULL ));
	cout << (unsigned)time( NULL ) << endl;
	//units = vector<Unit>(n_units, Unit(max_dim));
	for(int i=0; i<n_units; ++i){
		units.push_back(Unit(max_dim));
	}
	_n_units = n_units;
	_max_dim = max_dim;
	decay_rate = -0.5;
}

void NetWork::reset(){
	
	//units = vector<Unit>(n_units, Unit(max_dim));
	units.clear();
	for(int i=0; i<_n_units; ++i){
		units.push_back(Unit(_max_dim));
	}
}
// Raw input is vector of attribute value pairs
// They will be treated as symbols (assume sensor is binary, with a threshold to translate numerical values) 
// and registered with the system to have a unique index.
// It's possible that highly correlated dimension will be permanently mapped to a single dimension
// This is actually happending at the clustering stage, where each cluster is representing such a component.
vector<double> NetWork::translate_input(vector<pair<string, string> >& attr_val_pairs){
	vector<double> translated_input;
	for (unsigned i=0; i<attr_val_pairs.size(); ++i){
		string attr = (attr_val_pairs[i]).first;
		string val = (attr_val_pairs[i]).second;
		hash_map<string, int> hash;
		int mapped_index = this->index_to_attr_val_pair.size();
		double mapped_value = 1;

		double db_val;
		std::istringstream stream(val);
		if(stream >> db_val && stream.eof()){
			val = "_numeric"; // this should not be the value of any inputs
			mapped_value = db_val;
		}

		HASH_S_S_INT::iterator itr = this->attr_val_pair_to_index.find(attr);
		if(itr != this->attr_val_pair_to_index.end()){
			HASH_S_INT& val_to_index = itr->second;
			//if value is numerical, map all such value to one dimension under that attribute
			
			

			HASH_S_INT::iterator itr2 = val_to_index.find(val);
			if(itr2 != val_to_index.end()){
				mapped_index = itr2->second;
			}
			else{
				val_to_index.insert(pair<string, int>(val, mapped_index));
				mapped_index = this->index_to_attr_val_pair.size();
				this->index_to_attr_val_pair.push_back(pair<string, string> (attr, val));
			}
		}
		else{
			//HASH_S_INT hash;
			//hash.insert(pair<string, int>(val, this->current_index));
			pair<HASH_S_S_INT::iterator, bool> pr;
			pr = this->attr_val_pair_to_index.insert(pair<string, HASH_S_INT>(attr, HASH_S_INT()));
			if(pr.second){
				pr.first->second.insert(pair<string, int>(val, mapped_index));
			}
			else{
				cout << "insert failed\n";
				exit(1);
			}
			this->index_to_attr_val_pair.push_back(pair<string, string> (attr, val));
		}
		// If this index is greater than existing input, fill zeros
		for(int j=translated_input.size(); j<=mapped_index; ++j){
			translated_input.push_back(0);
		}
		translated_input[mapped_index] = mapped_value;
	}
	return translated_input;
}

int NetWork::train_input_one_cycle(vector<double>& input, vector<double>& new_input, vector<int> inhibit, bool update){
	
	// get winner
	int winner_index = this->winner(input, inhibit);

	if(update){
	// update/train winner
		//cout << "updating " << winner_index << endl;
		this->units[winner_index].update(input, decay_rate);
		//for(int i=0; i<input.size(); ++i){
		//	cout << units[winner_index].weights[i] << " ";
		//}
		//cout << endl;
	}

	// subtract winner from input to get new_input
	new_input = this->units[winner_index].subtract_input(input);

	return winner_index;
}

// return the winners
vector<int> NetWork::cluster_input(vector<double> input, bool update){
	int cycles = 3;
	vector<double>  new_input;
	vector<int> winners;
	for(int i=0; i<cycles; ++i){
		int winner = this->train_input_one_cycle(input, new_input, winners, update);
		input = new_input;
		winners.push_back(winner);
	}
	return winners;
}
vector<int> NetWork::cluster_input(vector<pair<string, string> >& attr_val_pairs, bool update){
	vector<double> input = this->translate_input(attr_val_pairs);
	for(int i=0; i<(int)(input.size()); ++i){
		//cout << input[i] << ", ";
	}
	//cout << endl;
	return this->cluster_input(input, update);
}

int NetWork::winner(vector<double> input, vector<int> inhibit){
	
	vector<int> competing_nodes = vector<int>(this->units.size(), 0);
	for(int i=0; i< inhibit.size(); ++i){
		competing_nodes[inhibit[i]] = -1;
	}

	double max_activation = -100;
	int current_winner = -1;
	for(int i=0; i < (int)(this->units.size()); ++i){
		if(competing_nodes[i] == -1){ // being inhibited
			continue;
		}
		double current_activation = this->activation(input, i);

	//	cout << current_activation << " "<<i<<",";
		if(current_activation > max_activation){
			max_activation = current_activation;
			current_winner = i;
		}
	}
	// If max_activation < threshold, then randomly pick a winner from the list of untrained units - with small counts
	// Maybe a list of units ordered by updating counts
	//cout << "winner " << current_winner <<endl;
	return current_winner;
}

// Dot-production activation calculation
double NetWork::activation(vector<double> input, int index){
	//return this->units[index].activation(input);
	// Distance based activation
	return this->units[index].activation_distance(input);
}


void NetWork::output_attr_val_pairs(){
	vector<pair<string, string> > attr_val_pairs = vector<pair<string, string> >(attr_val_pair_to_index.size());
	//cout << "SIZE " << attr_val_pair_to_index.size() << endl;

	for(HASH_S_S_INT::iterator itr = attr_val_pair_to_index.begin(); itr != attr_val_pair_to_index.end(); ++itr){
		string attr = itr->first;
		HASH_S_INT val_to_index = itr->second;
		for(HASH_S_INT::iterator itr2 = val_to_index.begin(); itr2 != val_to_index.end(); ++itr2){
			string val = itr2->first;
			int index = itr2->second;
			//cout << index << endl;
			if(index >= attr_val_pairs.size()){
				attr_val_pairs.insert(attr_val_pairs.end(), index+1-attr_val_pairs.size(), pair<string,string>());
			}
			attr_val_pairs[index] = pair<string, string>(attr, val);
		}
	}

	for(int i=0; i<attr_val_pairs.size(); ++i){
		cout << "(" << attr_val_pairs[i].first << "," << attr_val_pairs[i].second << ") ";
	}
	cout << endl;
}

//
void NetWork::cluster_export(string filename){
	std::ofstream SoarFile(filename.c_str());
	//SoarFile.open(filename.c_str());
	int used_dim = this->attr_val_pair_to_index.size();
	if(SoarFile){
		// Eventually, there should be a way to directly dump/load binary memory
		
		SoarFile << this->units.size() << " " << this->index_to_attr_val_pair.size() <<endl;
		for(int i = 0; i < this->index_to_attr_val_pair.size(); ++i){
			string attr = this->index_to_attr_val_pair[i].first;
			string value = this->index_to_attr_val_pair[i].second;
			SoarFile << attr << " " << value << endl;
		}
		//SoarFile << ">END OF ATTR VALUE PAIR" << endl;

		/*for(HASH_S_S_INT::iterator itr = thisAgent->clusterNet->attr_val_pair_to_index.begin(); itr != thisAgent->clusterNet->attr_val_pair_to_index.end(); ++itr){
		string attr = itr->first;
		HASH_S_INT value_to_index = itr->second;
		for(HASH_S_INT::iterator itr2 = value_to_index.begin(); itr2 != value_to_index.end(); ++itr2){
		string value = itr2->first;
		int index = itr2->second;
		}
		}*/

		for(int i=0; i< this->units.size(); ++i){
			for(int j=0; j<used_dim; ++j){
				//print(thisAgent, "%f ", thisAgent->clusterNet->units[i].weights[j]);
				SoarFile << this->units[i].weights[j] << " ";
			}
			SoarFile << this->units[i].counter << "\n";
		}
	}
}

void NetWork::cluster_import(string filename){
	std::ifstream SoarFile;
	SoarFile.open(filename.c_str());
	if(SoarFile){
		
		string line;
		float input_number;
		int i = 0;
		getline(SoarFile, line);
		std::istringstream isstr_first(line);
		int n_units, n_dim;
		isstr_first >> n_units >> n_dim;

		for(int i = 0; i<n_dim; i++ ){
			getline(SoarFile, line);
			std::istringstream isstr(line);
			string attr, val;
			isstr >> attr >> val;
			this->index_to_attr_val_pair.push_back(pair<string, string> (attr, val));

			pair<HASH_S_S_INT::iterator, bool> pr;
			pr = this->attr_val_pair_to_index.insert(pair<string, HASH_S_INT>(attr, HASH_S_INT()));
			if(pr.second){
				pr.first->second.insert(pair<string, int>(val, i));
			}

		}
		while (getline(SoarFile, line)) {
			std::istringstream isstr(line);
			int j = 0;
			while(isstr >> input_number){
				if(i >= this->units.size() || j>= this->units[i].weights.size()){
					break;
					// outof bounds
				}
				if(isstr.eof()){
					this->units[i].counter = input_number;
					//cout << input_number;
				}
				else{
					this->units[i].weights[j] = input_number;
				}
				++ j;
			}
			++ i;
		}
	}
}
