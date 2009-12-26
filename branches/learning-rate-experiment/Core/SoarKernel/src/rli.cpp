#include "rli.h"

#include <stdlib.h>
#include <vector>
#include <limits>
#include <assert.h>
#include <fstream>

using namespace std;

rli_metastate::rli_metastate()
: idcounter(1), aidmap(), idamap(), sidmap(), idsmap(), ramap(), asmap()
{}

rli_metastate::~rli_metastate() {
	rli_aid_map::iterator i;
	rli_sid_map::iterator j;
	
	for (i = aidmap.begin(); i != aidmap.end(); ++i) {
		delete i->first;
	}
	for (j = sidmap.begin(); j != sidmap.end(); ++j) {
		delete j->first;
	}
}

/* 
 * Will make a copy of the action parameter if it doesn't exist. This is
 * so that users can pass in the pointer to a local copy and not worry
 * about keeping memory around.
 */
rli_action_id rli_metastate::get_action_id(rli_action *action, bool *isnew) {
	rli_aid_map::iterator i;
	rli_action::iterator j;
	rli_action *copy;
	
	i = aidmap.find(action);
	if (i == aidmap.end()) {
		copy = new rli_action(*action);
		aidmap[copy] = idcounter;
		idamap[idcounter] = copy;
		for (j = action->begin(); j != action->end(); ++j) {
			assoc_rule_action(*j, idcounter);
		}
		if (isnew) *isnew = true;
		return idcounter++;
	} else {
		if (isnew) *isnew = false;
		return i->second;
	}
}

rli_state_id rli_metastate::get_state_id(rli_state *state, bool *isnew) {
	rli_sid_map::iterator i;
	rli_state::iterator j;
	rli_state *copy;
	
	i = sidmap.find(state);
	if (i == sidmap.end()) {
		copy = new rli_state(*state);
		sidmap[copy] = idcounter;
		idsmap[idcounter] = copy;
		for (j = state->begin(); j != state->end(); ++j) {
			assoc_action_state(*j, idcounter);
		}
		if (isnew) *isnew = true;
		return idcounter++;
	} else {
		if (isnew) *isnew = false;
		return i->second;
	}
}

rli_action *rli_metastate::get_action(rli_action_id id) {
	map<rli_action_id, rli_action*>::iterator i;
	i = idamap.find(id);
	if (i == idamap.end()) {
		return NULL;
	}
	return i->second;
}

rli_state *rli_metastate::get_state(rli_state_id id) {
	map<rli_state_id, rli_state*>::iterator i;
	i = idsmap.find(id);
	if (i == idsmap.end()) {
		return NULL;
	}
	return i->second;
}

void rli_metastate::assoc_rule_action(production_struct *r, rli_action_id a) {
	map<production_struct*, std::list<rli_action_id> >::iterator i;
	
	i = ramap.find(r);
	if (i == ramap.end()) {
		ramap[r].push_back(a);
	} else {
		i->second.push_back(a);
	}
}

void rli_metastate::assoc_action_state(rli_action_id a, rli_state_id s) {
	map<rli_action_id, std::list<rli_state_id> >::iterator i;
	
	i = asmap.find(a);
	if (i == asmap.end()) {
		asmap[a].push_back(s);
	} else {
		i->second.push_back(s);
	}
}

int rli_metastate::get_relevant_actions(production_struct *r, set<rli_action_id> &actions) {
	map<production_struct*, std::list<rli_action_id> >::iterator i;
	i = ramap.find(r);
	if (i != ramap.end()) {
		actions.insert(i->second.begin(), i->second.end());
		return i->second.size();
	}
	return 0;
}
	
int rli_metastate::get_relevant_states(rli_action_id aid, set<rli_state_id> &states) {
	map<rli_action_id, std::list<rli_state_id> >::iterator i;
	i = asmap.find(aid);
	if (i != asmap.end()) {
		states.insert(i->second.begin(), i->second.end());
		return i->second.size();
	}
	return 0;
}

rli_state_id rli_metastate::random_state() {
	vector<rli_state_id> ids;
	map<rli_state_id, rli_state*>::iterator i;
	for (i = idsmap.begin(); i != idsmap.end(); ++i) {
		ids.push_back(i->first);
	}
	return ids[random() % ids.size()];
}

rli_transmodel::rli_transmodel()
: stats()
{}

rli_transmodel::~rli_transmodel() {
	map<rli_sa_pair, rli_model_stats*>::iterator i;
	for (i = stats.begin(); i != stats.end(); ++i) {
		delete i->second->nstotals;
		delete i->second;
	}
}

void rli_transmodel::addexp(rli_state_id s1, rli_action_id a, rli_state_id s2, double r) {
	map<rli_sa_pair, rli_model_stats*>::iterator i;
	map<rli_action_id, int>::iterator j;
	rli_model_stats *st;

	rli_sa_pair sa(s1, a);
	
	i = stats.find(sa);
	if (i == stats.end()) {
		st = new rli_model_stats();
		st->nstotals = new map<rli_state_id, int>();
		(*st->nstotals)[s2] = 1;
		st->nexps = 1;
		st->reward = r;
		stats[sa] = st;
	} else {
		i->second->nexps++;
		i->second->reward += r;
		j = i->second->nstotals->find(s2);
		if (j == i->second->nstotals->end()) {
			(*i->second->nstotals)[s2] = 1;
		} else {
			j->second++;
		}
	}
}

double rli_transmodel::TDsignal
( rli_state_id s, 
  rli_action_id a, 
  double discount, 
  const map<rli_state_id, double> &svals,
  bool &nostats ) 
{
	map<rli_sa_pair, rli_model_stats*>::iterator i;
	map<rli_state_id, int>::iterator j;
	map<rli_state_id, double>::const_iterator k;
	rli_sa_pair sa(s, a);
	double tdpred;
	double sum;

	i = stats.find(sa);
	if (i == stats.end()) {
		nostats = true;
		return 0.0;
	}
	
	sum = 0.0;
	for (j = i->second->nstotals->begin(); j != i->second->nstotals->end(); ++j) {
		rli_state_id ns = j->first;
		int times = j->second;
		
		if (ns == TERMSTATE) {
			tdpred = i->second->reward / i->second->nexps;
		} else {
			k = svals.find(ns);
			assert( k != svals.end() );
			// (# times visited s') * ( R(s,a,s') + discount * V(s') )
			tdpred = i->second->reward / i->second->nexps + discount * k->second;
		}
		sum += times * tdpred;
	}
	
	nostats = false;
	return sum / i->second->nexps;
}

void rli_transmodel::print(ostream &os) {
	map<rli_sa_pair, rli_model_stats*>::iterator i;
	map<rli_state_id, int >::iterator j;
	double nexps;
	
	for (i = stats.begin(); i != stats.end(); ++i) {
		nexps = (double) i->second->nexps;
		os << i->first.first << " " << i->first.second << " " << i->second->reward / nexps << endl;
		for (j = i->second->nstotals->begin(); j != i->second->nstotals->end(); ++j) {
			os << j->first << " " << j->second /nexps << endl;
		}
		os << endl;
	}
}

/*
 * Print the model, showing actions as a list of production names that
 * make up the action.
 */
void rli_transmodel::print(ostream &os, rli_metastate &ms) {
	map<rli_sa_pair, rli_model_stats*>::iterator i;
	rli_action *a;
	rli_action::iterator j;
	map<rli_state_id, int >::iterator k;
	double nexps;
	
	for (i = stats.begin(); i != stats.end(); ++i) {
		nexps = (double) i->second->nexps;
		os << i->first.first << " "; 
		a = ms.get_action(i->first.second);
		for (j = a->begin(); j != a->end(); ++j) {
			os << (*j)->name->sc.name << ",";
		}
		os << " " << i->second->reward / nexps << endl;
		for (k = i->second->nstotals->begin(); k != i->second->nstotals->end(); ++k) {
			os << k->first << " " << k->second /nexps << endl;
		}
		os << endl;
	}
}