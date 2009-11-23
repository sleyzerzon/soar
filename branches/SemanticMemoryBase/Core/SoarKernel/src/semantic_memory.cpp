#ifdef SEMANTIC_MEMORY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

// stdafx.cpp : source file that includes just the standard includes
// smem_test.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information
//#include "stdafx.h"
#include "semantic_memory.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


bool LME::operator < (const LME lme) const{
		if(id < lme.id){
			return true;
		}
		else if(lme.id < id){
			return false;
		}
		else if(attr < lme.attr){
			return true;
		}
		else if(lme.attr < attr){
			return false;
		}
		else if(value < lme.value){
			return true;
		}
		else if(lme.value < value){
			return false;
		}
		else if(value_type < lme.value_type){
			return true;
		}
		else{
			return false;
		}
	}

bool CueTriplet::operator < (const CueTriplet t) const{
		if(id < t.id){
			return true;
		}
		else if(t.id < id){
			return false;
		}
		else if(attr<t.attr){
			return true;
		}
		else if(t.attr < attr){
			return false;
		}
		else if(value<t.value){
			return true;
		}
		else if(t.value<value){
			return false;
		}
		else if(value_type<t.value_type){
			return true;
		}
		else{
			return false;
		}
	}


bool is_identifier_type(int value_type){
	if(value_type == 1){
		return true;
	}
	return false;
}

template<class T>
set<T> set_intersect(set<T>& set1, set<T>& set2){
	
	set<T> intersection;
	// the algorithm is nlogm
	set<T>* set1_ptr = NULL;
	set<T>* set2_ptr = NULL;

	if(set1.size() > set2.size()){
		set1_ptr = &set2;
		set2_ptr = &set1;
	}
	else{
		set1_ptr = &set1;
		set2_ptr = &set2;
	}

	for(set<T>::const_iterator itr = set1_ptr->begin(); itr != set1_ptr->end(); ++itr){
		if(set2_ptr->find(*itr) != set2_ptr->end()){
			intersection.insert(*itr);
		}
	}
	return intersection;
}


template <class T>
ostream& operator << (ostream &out, const set<T>& s){
	out << "\nSET" << endl;
	for(set<T>::const_iterator itr = s.begin(); itr != s.end(); ++itr){
		out << *itr << ",";
	}
	out << endl;
	return out;
}


ostream& operator << (ostream &out, const LME& lme){
	out << "\n## Long Term Memory Element ##" << endl;
	out << "<" << lme.id << "," << lme.attr << "," << lme.value << ","<< lme.value_type << ">" << endl;
	out << "[history";
	for(unsigned i=0; i< lme.boost_history.size(); ++i){
		out << lme.boost_history[i] << " ";
	}
	out << "]" << endl;
	return out;
}



ostream& operator << (ostream &out, const set<LME>& s){
	out << "\nSET" << endl;
	for(set<LME>::const_iterator itr = s.begin(); itr != s.end(); ++itr){
		out << *itr << ",";
	}
	out << endl;
	return out;
}

ostream& operator << (ostream &out, const CueTriplet& lme){
	out << "\n## Cue Triplet ##" << endl;
	out << "<" << lme.id << "," << lme.attr << "," << lme.value << ","<< lme.value_type << ">" << endl;
	return out;
}



ostream& operator << (ostream &out, const set<CueTriplet>& s){
	out << "\nSET" << endl;
	for(set<CueTriplet>::const_iterator itr = s.begin(); itr != s.end(); ++itr){
		out << *itr << ",";
	}
	out << endl;
	return out;
}


void SemanticMemory::merge_LMEs(set<LME>& lmes, long current_cycle){
	hash_map<string, string>merging_hash;
	// this hash mapping all merged id to the unified id
	
	set<string> inserted_ids; // record all chunk ids has been inserted.

	set<string> all_ids;

	HASH_S_HASH_S_HASH_S_LP id_attr_value_hash;
	vector<set<LME>::iterator> all_new_lmes;
	
	// build up indexed hash for new lmes
	// These data structures are for current cycle
	for (set<LME>::iterator itr = lmes.begin(); itr != lmes.end(); ++itr){
		all_new_lmes.push_back(itr);
		int lme_index = all_new_lmes.size()-1;
		insert_LME_hash (id_attr_value_hash, itr->id, itr->attr, itr->value, lme_index, itr->value_type);
		all_ids.insert(itr->id);
	}
	//hash_map::kk
	for (set<string>::iterator itr = all_ids.begin(); itr != all_ids.end(); ++itr){
		string chunk_id = *itr;
		string merged_id;
		// look up the the merged id from merging_hash
		// If found, use the merged it, otherwise, recursively merge the chunk
		hash_map<string, string>::iterator itr2 = merging_hash.find(chunk_id);
		if(itr2 != merging_hash.end()){
			merged_id = itr2->second;
		}
		else{
			set <string> merging_path; // used to detect loop. 
			//If a parent is find as a child later, just don't branch to that anymore, so that it will
			// be evaluated as a 'variable' (since it's a id not in long term memory yet) later
			// Any it'll match with anything.
			// This mechanism does merge chunks with different structure, the later looped chunk is not saved
			// The first looped chunk will be saved correctly
			// The result is that the later looped chunk's immediate node to the loop back to top chunk-id could match
			// with different structures (loops)
			// Smaller loops will be merged with biger loops (super-set loop), though the structure could be different


			merged_id = merge_id(chunk_id, id_attr_value_hash, merging_hash, all_new_lmes, merging_path, current_cycle);
			merging_hash.insert(pair<string,string>(chunk_id, merged_id));
		}

		//string merged_id = merge_id(*itr, id_attr_value_hash, merging_hash, all_new_lmes);
	}
	
}

// merge identical chunks, and update the global structures
// recursively call itself.
// Actual merging operation happens at the end of this sub
string SemanticMemory::merge_id(string& id, HASH_S_HASH_S_HASH_S_LP& id_attr_value_hash, 
								hash_map<string, string >& merging_hash,
								vector<set<LME>::iterator>& all_new_lmes,
								set<string>& merging_path, long& current_cycle)
{
	cout << "Merging " << id << endl;
	merging_path.insert(id);
	if(id_attr_value_hash.find(id) == id_attr_value_hash.end()){
		cout << "No Value Hash" << endl;
		return id;
	}
	HASH_S_HASH_S_LP chunk_attr_value_hash = id_attr_value_hash.find(id)->second;
	cout << "found value hash " << endl;
	for(HASH_S_HASH_S_LP::iterator itr = chunk_attr_value_hash.begin(); itr != chunk_attr_value_hash.end(); ++itr){
		string attr = itr->first;
		cout << "Branching attr " << attr << endl;
		HASH_S_LP value_hash = itr->second;
		for(HASH_S_LP::iterator itr2 = value_hash.begin(); itr2 != value_hash.end(); ++itr2){
			string value = itr2->first;
			int lme_index = itr2->second;
			int value_type = all_new_lmes[lme_index]->value_type;
			if (value_type == IDENTIFIER_SYMBOL_TYPE){ // need to merge this value which is identifier
				cout << "Identifier value " << value << endl;
				string merged_id;
				// first try to find it in merging hash for whether it has been merged already
				hash_map<string, string>::iterator itr3 = merging_hash.find(value);
				if(itr3 != merging_hash.end()){
					merged_id = itr3->second;
					cout << "Already Merged with " << merged_id << endl;
				}
				else{ // call this routine recursively to find identical chunks to merge
					if(merging_path.find(value) != merging_path.end()){ // loop detected
						cout << "Loop structure dected! " << endl;
						merged_id = value;
						// keep the value as it was
						// but don't update the merging_hash yet?
						
						merging_hash.insert(pair<string, string>(value, merged_id));
						// update the hash, so that avoiding every node on the loop need to go a full round!
					}
					else{
						merged_id = merge_id(value, id_attr_value_hash, merging_hash, all_new_lmes, merging_path, current_cycle);
						cout << "Just Merged with " << merged_id << endl;
						merging_hash.insert(pair<string,string>(value, merged_id));
					}
				}

				// update the value of this wme
				all_new_lmes[lme_index]->value = merged_id;
			}
		}
	}

	// After all values have been updated recursively, now identical chunk can be checked for this level

	// If there are loop structures, there could be unresolved ids, and the correct binding need to be determined
	// Any id connecting to a loop structure will be unresolvable locally. 
	// It must be resolve globally, via searching through all potential bindings

	string final_chunk_id;
	bool need_insert = true;
	if(this->test_id(id)){ // existing id, no need to find identical chunk
							// need to insert the new attribute values
		final_chunk_id = id;
		cout << "Updating existing chunk " << final_chunk_id << endl;
	}
	else{ // short term id, find identical chunk to merge.
		
		// This function determines whether current chunk_id have identical matches in long term memory already,
		// given that all values have been recursively merged.
		bool result =  find_identical_chunk(id, id_attr_value_hash, final_chunk_id, all_new_lmes);

		if(result){// If there is the identical chunk already, update the counter
			cout << "Find identical exsiting chunk " << final_chunk_id << endl;
			need_insert = false;
			// Update counter here ...

		}
		
	}
	
	//if(need_insert){// Otherwise insert the new chunk here...
	// Now just always insert the element, if already exist, update the reference history

	//		cout << "Insert new chunk " << final_chunk_id << endl;
			for(HASH_S_HASH_S_LP::iterator itr = chunk_attr_value_hash.begin(); itr != chunk_attr_value_hash.end(); ++itr){
				string attr = itr->first;

				HASH_S_LP value_hash = itr->second;
				for(HASH_S_LP::iterator itr2 = value_hash.begin(); itr2 != value_hash.end(); ++itr2){
					string value = itr2->first;
					int lme_index = itr2->second;
					int value_type = all_new_lmes[lme_index]->value_type;
					cout << " ^" << attr <<" " << value << "("<< value_type<< ")"<< endl;

					vector<int> current_history;
					current_history.push_back(current_cycle);
					// This will be merging with identical elements if the final_chunk_id is merged with some existing id
					// Otherwise, it'll be a new element
					this->insert_LME(final_chunk_id, attr, value, value_type, current_history);
				}
			}

	//	}

	return final_chunk_id;

	
}

// If there is the identical chunk, return true, and new_chunk_id should be different
// Other wise, return false, new_chunk_id is the same as input chunk_id
bool SemanticMemory::find_identical_chunk (string& chunk_id, HASH_S_HASH_S_HASH_S_LP& id_attr_value_hash,
										   string& new_chunk_id, vector<set<LME>::iterator>& all_new_lmes){
	
	set<CueTriplet> chunk_cue;
	HASH_S_HASH_S_LP chunk_attr_value_hash = id_attr_value_hash.find(chunk_id)->second;
	
	for(HASH_S_HASH_S_LP::iterator itr = chunk_attr_value_hash.begin(); itr != chunk_attr_value_hash.end(); ++itr){
		string attr = itr->first;
		HASH_S_LP value_hash = itr->second;
		for(HASH_S_LP::iterator itr2 = value_hash.begin(); itr2 != value_hash.end(); ++itr2){
			string value = itr2->first;
			int lme_index = itr2->second;
			int value_type = all_new_lmes[lme_index]->value_type;
			chunk_cue.insert(CueTriplet(chunk_id, attr, value, value_type));
		}
	}

	set<string> matched_ids = match_retrieve_single_level(chunk_cue);
	// These ids are all super-set chunks
	// Assume currently merge with super set
	if(matched_ids.size() == 0){
		new_chunk_id = chunk_id;
		return false;
	}
	else{
		new_chunk_id =*(matched_ids.begin());
		// the firs super set chunk
		// If a super set chunk is inserted later, there will be separate chunks.
		// And could emerged multiple super-chunks for a later query
		return true;

	}
	//return new_chunk_id;
}

void SemanticMemory::insert_LME(string id, string attr, string value, int value_type, const vector<int>& history){
	
	int lme_index = LME_Array.size();
	

	int inserted_index = insert_LME_hash (memory_id_attr_hash, id, attr, value, lme_index, value_type);
	if(inserted_index == lme_index){//new structure
		// also insert into the other hash
		insert_LME_hash (memory_attr_value_hash, attr, value, id, lme_index, value_type);
		// create a new structure
		LME* new_lme_ptr = new LME(id, attr, value, value_type, history);
		LME_Array.push_back(new_lme_ptr);
	}
	else{ // already exist, don't waste arry storage.
		this->LME_Array[inserted_index]->boost_history.insert(LME_Array[inserted_index]->boost_history.end(), history.begin(), history.end());
	}
	
}



set<LME> SemanticMemory::expand_id(string id, set<CueTriplet>& cue){



	set<LME> attributes;
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.find(id);

	if(itr != memory_id_attr_hash.end()){ // should != end()
		HASH_S_HASH_S_LP& slots = itr->second;

		for(set<CueTriplet>::iterator itr = cue.begin(); itr != cue.end(); ++itr){
			string attr = itr->attr; // for each attribute, find the first value to expand

			HASH_S_HASH_S_LP::iterator result = slots.find(attr);
			if(result != slots.end()){	
				HASH_S_LP values = result->second;
				for(HASH_S_LP::iterator itr3 = values.begin(); itr3 != values.end(); ++ itr3){
					string value = itr3->first;
					int lme_index = itr3->second;
					LME* lme_ptr = LME_Array[lme_index];

					attributes.insert(*lme_ptr);
					break; // only insert one value, the best one based on some metric
				}
			}
		}

	}
	return attributes;
}
/*
// Expand a given id
set<LME> SemanticMemory::expand_id(string id, set<CueTriplet>& cue){
	
	set<LME> attributes;
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.find(id);

	if(itr != memory_id_attr_hash.end()){ // should != end()
		HASH_S_HASH_S_LP& slots = itr->second;
		for(HASH_S_HASH_S_LP::iterator itr2 = slots.begin(); itr2 != slots.end(); ++itr2){
			string attr = itr2->first;
			HASH_S_LP values = itr2->second;

			for(HASH_S_LP::iterator itr3 = values.begin(); itr3 != values.end(); ++ itr3){
				string value = itr3->first;
				int lme_index = itr3->second;
				LME* lme_ptr = LME_Array[lme_index];

				attributes.insert(*lme_ptr);
				break; // only insert one value, the best one based on some metric
			}
		}
	}
	return attributes;
}
*/

// Single level match. Use this for the moment to make the entire system work
//
set<string> SemanticMemory::match_retrieve_single_level(const set<CueTriplet>& cue_set){
	bool start = false;
	set<string> matched_ids_intersection;
	for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
		string id = itr->id;
		string attr = itr->attr;
		string value = itr->value;
		int value_type = itr->value_type;
		set<string> current_matched_ids = this->match_attr_value(attr, value, value_type);
		if(!start){
			matched_ids_intersection.insert(current_matched_ids.begin(), current_matched_ids.end());
		}
		else{
			matched_ids_intersection = set_intersect(matched_ids_intersection, current_matched_ids);
			

			if(matched_ids_intersection.size() == 0){
				break;
			}
		}
		cout << "Current matched ids" << endl;
		cout << matched_ids_intersection << endl;

		start = true;
	}

	return matched_ids_intersection;
}

// This new retrieval function does partial matching using naive Bayesian
// Probabilities are casted into log scale and implemented as 'activations', which have extra functionalities
// 1. Activation retention - better prior probability estimation for target, and task inertia for cue attributes.
// 2. Activation decay - corresponds to effects of unlearning
// Activation is more biological realistic and meanwhile carries its fucntionality in Bayeisan terms.

set<CueTriplet> SemanticMemory::match_retrieve_single_level_2006_1_31(const set<CueTriplet>& cue_set, string& retrieved_id){
	// Cue must have both specified attributes and target attributes
	// This should be enforced !
	set<CueTriplet> probe_set, target_set;
	// First identify probing cue and target attributes respectively
	for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
		string value = itr->value;
		int value_type = itr->value_type;
		if(value_type == IDENTIFIER_SYMBOL_TYPE && !this->test_id(value)){ // target cue
			target_set.insert(*itr);
		}
		else{
			probe_set.insert(*itr);
		}
	}
	// keep a hash of matched LME for each individal attribute in the probe_set
	stdext::hash_map<string, set<int> > attr_matched_LME_index_hash;
	
	// Keep a reference table for attribute-value pairs and target values
	// All that is derived is a 2D table of counts
	stdext::hash_map<string, int> attr_value_to_index;
	stdext::hash_map<string, int> target_value_to_index;

	for(set<CueTriplet>::const_iterator itr = probe_set.begin(); itr != probe_set.end(); ++itr){
		string id = itr->id;
		string attr = itr->attr;
		string value = itr->value;
		int value_type = itr->value_type;
		attr_matched_LME_index_hash[attr] = this->exact_match_attr_value(attr, value, value_type);
	}
	
	bool start = false;
	set<string> matched_ids_intersection;
	for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
		string id = itr->id;
		string attr = itr->attr;
		string value = itr->value;
		int value_type = itr->value_type;
		set<string> current_matched_ids = this->match_attr_value(attr, value, value_type);
		if(!start){
			matched_ids_intersection.insert(current_matched_ids.begin(), current_matched_ids.end());
		}
		else{
			matched_ids_intersection = set_intersect(matched_ids_intersection, current_matched_ids);
			

			if(matched_ids_intersection.size() == 0){
				break;
			}
		}
		cout << "Current matched ids" << endl;
		cout << matched_ids_intersection << endl;

		start = true;
	}
	
	set<CueTriplet> retrieved;
	if(matched_ids_intersection.empty()){ // no matches
		retrieved_id = "F0";
		retrieved.insert(CueTriplet("F0", "status", "failure", 2));
	}
	else{
		retrieved_id = *(matched_ids_intersection.begin());
		for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
			string id = itr->id;
			string attr = itr->attr;
			string value = itr->value;
			int value_type = itr->value_type;
			
			if(value_type == IDENTIFIER_SYMBOL_TYPE && !this->test_id(value)){ // cue field is variable, this is being queried
				set<int> candidate_index = this->match_id_attr(retrieved_id, attr);
				int target_index = *(candidate_index.begin());
				string retrieved_value = (LME_Array[target_index])->value;
				int retrieved_value_type = (LME_Array[target_index])->value_type;
				retrieved.insert(CueTriplet(retrieved_id, attr, retrieved_value, retrieved_value_type));
			}
			else{
				retrieved.insert(CueTriplet(retrieved_id, attr, value, value_type));
			}
		}
	}

	//return matched_ids_intersection;
	return retrieved;
}

// Still exact match, and everything else is equivalent to previous versions.
// Just changed the way failure chunk is returned
// Need to work with the new version of caller routine in kernal (retrieve function in smem.cpp)
set<CueTriplet> SemanticMemory::match_retrieve_single_level_2006_1_22(const set<CueTriplet>& cue_set, string& retrieved_id){
	// Cue must have both specified attributes and target attributes
	// This should be enforced !
	bool start = false;
	set<string> matched_ids_intersection;
	for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
		string id = itr->id;
		string attr = itr->attr;
		string value = itr->value;
		int value_type = itr->value_type;
		set<string> current_matched_ids = this->match_attr_value(attr, value, value_type);
		if(!start){
			matched_ids_intersection.insert(current_matched_ids.begin(), current_matched_ids.end());
		}
		else{
			matched_ids_intersection = set_intersect(matched_ids_intersection, current_matched_ids);
			

			if(matched_ids_intersection.size() == 0){
				break;
			}
		}
		cout << "Current matched ids" << endl;
		cout << matched_ids_intersection << endl;

		start = true;
	}
	
	set<CueTriplet> retrieved;
	if(matched_ids_intersection.empty()){ // no matches
		retrieved_id = "F0";
		retrieved.insert(CueTriplet("F0", "status", "failure", 2));
	}
	else{
		retrieved_id = *(matched_ids_intersection.begin());
		for(set<CueTriplet>::const_iterator itr = cue_set.begin(); itr != cue_set.end(); ++itr){
			string id = itr->id;
			string attr = itr->attr;
			string value = itr->value;
			int value_type = itr->value_type;
			
			if(value_type == IDENTIFIER_SYMBOL_TYPE && !this->test_id(value)){ // cue field is variable, this is being queried
				set<int> candidate_index = this->match_id_attr(retrieved_id, attr);
				int target_index = *(candidate_index.begin());
				string retrieved_value = (LME_Array[target_index])->value;
				int retrieved_value_type = (LME_Array[target_index])->value_type;
				retrieved.insert(CueTriplet(retrieved_id, attr, retrieved_value, retrieved_value_type));
			}
			else{
				retrieved.insert(CueTriplet(retrieved_id, attr, value, value_type));
			}
		}
	}

	//return matched_ids_intersection;
	return retrieved;
}
// given id and attribute, retrieve the index for corresponding LMEs
set<int> SemanticMemory::match_id_attr(const string id, const string attr){
	set<int> index;
	
	HASH_S_HASH_S_HASH_S_LP::iterator itr = this->memory_id_attr_hash.find(id);
	if(itr == memory_id_attr_hash.end()){// no such id
		return index;
	}
	
	HASH_S_HASH_S_LP& attr_hash = itr->second;
	HASH_S_HASH_S_LP::iterator itr2 = attr_hash.find(attr);
	if(itr2 == attr_hash.end()){//no such attr for the given id, return empty set
		return index;
	}
	else{
		HASH_S_LP& value_hash = itr2->second;
		for(HASH_S_LP::iterator itr3 = value_hash.begin(); itr3 != value_hash.end(); ++itr3){
			//cout << "Attr-Value matched: " << itr3->first << endl;
			//returned_ids_set.insert(itr3->first);
			index.insert(itr3->second);
		}
	}
	
	return index;
}


int SemanticMemory::insert_LME_hash (HASH_S_HASH_S_HASH_S_LP& hash, string key1, string key2, string key3, int lme_index, int value_type){
	
	// find the identifier if already exists
	HASH_S_HASH_S_HASH_S_LP::iterator itr = hash.find(key1);
	// otherwise, create a new identifier
	if(itr == hash.end()){
		pair<HASH_S_HASH_S_HASH_S_LP::iterator, bool> result = 
			hash.insert(pair<string, HASH_S_HASH_S_LP> (key1, HASH_S_HASH_S_LP()));
		itr = result.first;
	}
	// For primary structure, value of the corresponds identifier is a hash of attribute-value hash - slots_hash
	HASH_S_HASH_S_LP& slots_hash = itr->second;

	// find the slot if already exists
	HASH_S_HASH_S_LP::iterator itr2 = slots_hash.find(key2);
	// otherwise, create a new slot
	if(itr2 == slots_hash.end()){
		pair<HASH_S_HASH_S_LP::iterator, bool> result = 
			slots_hash.insert(pair<string, HASH_S_LP>(key2, HASH_S_LP() ));
		itr2 = result.first;
	}
	
	HASH_S_LP& value_hash = itr2->second;
	HASH_S_LP::iterator itr3 = value_hash.find(key3);
	if(itr3 == value_hash.end()){
		pair<HASH_S_LP::iterator, bool> result = value_hash.insert(pair<string, int>(key3, lme_index));
		return lme_index;
	}
	else{// already exsist, update activation...
		cout << "Already exisits!" << endl;
		cout << key1 << ", " << key2 << ", " << key3 << endl;
		//cout << *(LME_Array[lme_index]) << endl;
		return itr3->second;
	}
	return lme_index;
}



bool SemanticMemory::test_id(const string id){
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.find(id);
	if(itr == memory_id_attr_hash.end()){
		cout << "ID "<< id << " Not found";
		return false;
	}
	else{
		cout << "ID "<< id << " found as Long term" << endl;
		return true;
	}
}
void SemanticMemory::print(){
	cout << "Test" << endl;
}
void SemanticMemory::dump(ostream& out){
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.begin();
	
	out << "\nNow Dumping Semantic Memory content" << endl;

	while(itr != memory_id_attr_hash.end()){
		
		HASH_S_HASH_S_LP slots_hash = itr->second;
		HASH_S_HASH_S_LP::iterator itr2 = slots_hash.begin();
		
		while(itr2 != slots_hash.end()){
			HASH_S_LP value_hash = itr2->second;
			HASH_S_LP::iterator itr3 = value_hash.begin();

			while(itr3 != value_hash.end()){
				int lme_index = itr3->second;
				LME* lme_ptr = LME_Array[lme_index]; 
				cout << *lme_ptr<<endl;
				++itr3;
			}

			++itr2;
		}

		++itr;
	}
	out << "#####Finish dumping Semantic Memory content#####" << endl;
}

void SemanticMemory::dump(vector<LME*>& out){
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.begin();
	

	while(itr != memory_id_attr_hash.end()){
		
		HASH_S_HASH_S_LP slots_hash = itr->second;
		HASH_S_HASH_S_LP::iterator itr2 = slots_hash.begin();
		
		while(itr2 != slots_hash.end()){
			HASH_S_LP value_hash = itr2->second;
			HASH_S_LP::iterator itr3 = value_hash.begin();

			while(itr3 != value_hash.end()){
				int lme_index = itr3->second;
				LME* lme_ptr = LME_Array[lme_index];
				//cout << lme_ptr<<endl;
				out.push_back(lme_ptr);
				++itr3;
			}

			++itr2;
		}

		++itr;
	}
	
}







//##############################
// Private
//
//##############################

set<int> SemanticMemory::exact_match_attr_value(const string attr, const string value, int value_type){
	
	set<int> returned_index_set;
	HASH_S_HASH_S_HASH_S_LP::iterator itr = this->memory_attr_value_hash.find(attr);
	if(itr == memory_attr_value_hash.end()){// no such attribute, return empty set
		return set<int>();
	}
	else if(value_type == IDENTIFIER_SYMBOL_TYPE){ //temp identifier represent variable, which match with all values, identifier or not
		if(!this->test_id(value)){ // short term temp identifier
			//************IMPORTANT******************
			// May need to keep the binding of variables consistent. Therefore, need additional hash keep track of
			// bindings, return the bindings and find a consistent match at the upper level
			HASH_S_HASH_S_LP& value_hash = itr->second;
			for(HASH_S_HASH_S_LP::iterator itr2 = value_hash.begin(); itr2 != value_hash.end(); ++itr2){
				string current_value = itr2->first; // No matter what this value is, it matches with the variable
						// The problem is to make the variable binding consistent accross all matched sets
						// i.e. if the same varaible are specified with multiple attr_value pairs, the variable need to
						// match with the same value
						// Need to associate with each returned id a binding-list, the final matched id need to have
						// a consistent binding list, this can be done on the fly.
				// There are some complications when multi-valued attributes are to be matched
				// By default, different varibale name means the matched actual value should be different or could be the same?
				// Since there is no way to specify that via the cue link.
				HASH_S_LP& id_hash = itr2->second;
				for(HASH_S_LP::iterator itr3 = id_hash.begin(); itr3 != id_hash.end(); ++itr3){
					cout << "Attr-Value matched: " << itr3->first << endl;
					returned_index_set.insert(itr3->second);
				}

			}
		}
	}
	else{ // This is the actual working part
		HASH_S_HASH_S_LP& value_hash = itr->second;
		HASH_S_HASH_S_LP::iterator itr2 = value_hash.find(value);
		if(itr2 == value_hash.end()){//no such value for the given attribute, return empty set
			return set<int>();
		}
		else{
			HASH_S_LP& id_hash = itr2->second;
			for(HASH_S_LP::iterator itr3 = id_hash.begin(); itr3 != id_hash.end(); ++itr3){
				cout << "Attr-Value matched: " << itr3->first << endl;
				returned_index_set.insert(itr3->second);
			}
		}
	}
	return returned_index_set;

}


set<string> SemanticMemory::match_attr_value(const string attr, const string value, int value_type){
	
	set<string> returned_ids_set;
	HASH_S_HASH_S_HASH_S_LP::iterator itr = this->memory_attr_value_hash.find(attr);
	if(itr == memory_attr_value_hash.end()){// no such attribute, return empty set
		return set<string>();
	}
	else if(value_type == IDENTIFIER_SYMBOL_TYPE){ //temp identifier represent variable, which match with all values, identifier or not
		if(!this->test_id(value)){ // short term temp identifier
			//************IMPORTANT******************
			// May need to keep the binding of variables consistent. Therefore, need additional hash keep track of
			// bindings, return the bindings and find a consistent match at the upper level
			HASH_S_HASH_S_LP& value_hash = itr->second;
			for(HASH_S_HASH_S_LP::iterator itr2 = value_hash.begin(); itr2 != value_hash.end(); ++itr2){
				string current_value = itr2->first; // No matter what this value is, it matches with the variable
						// The problem is to make the variable binding consistent accross all matched sets
						// i.e. if the same varaible are specified with multiple attr_value pairs, the variable need to
						// match with the same value
						// Need to associate with each returned id a binding-list, the final matched id need to have
						// a consistent binding list, this can be done on the fly.
				// There are some complications when multi-valued attributes are to be matched
				// By default, different varibale name means the matched actual value should be different or could be the same?
				// Since there is no way to specify that via the cue link.
				HASH_S_LP& id_hash = itr2->second;
				for(HASH_S_LP::iterator itr3 = id_hash.begin(); itr3 != id_hash.end(); ++itr3){
					cout << "Attr-Value matched: " << itr3->first << endl;
					returned_ids_set.insert(itr3->first);
				}

			}
		}
	}
	else{
		HASH_S_HASH_S_LP& value_hash = itr->second;
		HASH_S_HASH_S_LP::iterator itr2 = value_hash.find(value);
		if(itr2 == value_hash.end()){//no such value for the given attribute, return empty set
			return set<string>();
		}
		else{
			HASH_S_LP& id_hash = itr2->second;
			for(HASH_S_LP::iterator itr3 = id_hash.begin(); itr3 != id_hash.end(); ++itr3){
				cout << "Attr-Value matched: " << itr3->first << endl;
				returned_ids_set.insert(itr3->first);
			}
		}
	}
	return returned_ids_set;

}


int SemanticMemory::clear(){
	unsigned size = LME_Array.size();
	
	for (unsigned i=0; i<size; ++i){
		delete LME_Array[i];
	}
	LME_Array.clear();
	memory_id_attr_hash.clear();
	memory_attr_value_hash.clear();

	return size;

}
int SemanticMemory::reset_history(){
	unsigned size = LME_Array.size();
	
	for (unsigned i=0; i<size; ++i){
		LME_Array[i]->boost_history.clear();
	}


	return size;

}


SemanticMemory::~SemanticMemory(){
	cout << "Calling SemanticMemory destructor" <<endl;
	this->clear();
/*	
	HASH_S_HASH_S_HASH_S_LP::iterator itr = memory_id_attr_hash.begin();
	while(itr != memory_id_attr_hash.end()){
		
		HASH_S_HASH_S_LP slots_hash = itr->second;
		HASH_S_HASH_S_LP::iterator itr2 = slots_hash.begin();
		

		while(itr2 != slots_hash.end()){
			HASH_S_LP value_hash = itr2->second;
			HASH_S_LP::iterator itr3 = value_hash.begin();

			while(itr3 != value_hash.end()){
				int lme_index = itr3->second;
				LME* lme_ptr = LME_Array[lme_index];
				cout << *lme_ptr<<endl;
				//delete lme_ptr;
				++itr3;
			}


			++itr2;
		}

		++itr;
	}
	*/
}
#endif SEMANTIC_MEMORY