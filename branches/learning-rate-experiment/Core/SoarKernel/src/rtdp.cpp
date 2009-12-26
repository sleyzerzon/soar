#include "rtdp.h"
#include <assert.h>
#include <limits>

#include <iostream>

using namespace std;

rtdp::rtdp() 
: metastate(), model(), tracecounter(0), traces(), qvals(), avals(), svals()
{}

void rtdp::register_rule(production_struct *p, double q) {
	qvals[p] = q;
	//cout << "RTDP: The Q table is now " << qvals.size() << endl;
}

int rtdp::new_trace() {
	//cout << "RTDP: New trace " << tracecounter << endl;
	traces[tracecounter] = pair<rli_state_id, rli_action_id>(0, 0);
	return tracecounter++;
}

void rtdp::end_trace(int trace) {
	map<int, pair<rli_state_id, rli_action_id> >::iterator i;
	//cout << "RTDP: End trace " << trace << endl;
	i = traces.find(trace);
	assert(i != traces.end());
	traces.erase(i);
}

rli_state_id rtdp::get_state(int trace) {
	map<int, pair<rli_state_id, rli_action_id> >::iterator ti;
	ti = traces.find(trace);
	assert(ti != traces.end());
	return ti->second.first;
}

rli_state_id rtdp::update_state(int trace, const std::list<rli_action*> &actions, double r) {
	std::list<rli_action*>::const_iterator i;
	rli_state currstate;
	rli_state_id currstateid;
	
	map<int, pair<rli_state_id, rli_action_id> >::iterator ti;
	rli_state_id laststate;
	rli_action_id lastaction;
	bool isnew;
	
	ti = traces.find(trace);
	assert(ti != traces.end());
	laststate = ti->second.first;
	lastaction = ti->second.second;
	
	for (i = actions.begin(); i != actions.end(); ++i) {
		currstate.insert(metastate.get_action_id(*i));
	}
	
	currstateid = metastate.get_state_id(&currstate, &isnew);
	if (isnew) {
		update_sval(currstateid);
	}
	
	if (laststate != rli_transmodel::INITSTATE && lastaction != rli_transmodel::NULLACTION) {
		model.addexp(laststate, lastaction, currstateid, r);
		//cout << "RTDP " << laststate << "-" << lastaction << "->" << currstateid << endl;
		//cout << "RTDP Model:" << endl;
		//model.print(cout);
	}
	
	ti->second.first = currstateid;
	return currstateid;
}

void rtdp::update_state_terminal(int trace, double r) {
	map<int, pair<rli_state_id, rli_action_id> >::iterator ti;
	rli_state_id laststate;
	rli_action_id lastaction;
		
	ti = traces.find(trace);
	assert(ti != traces.end());
	laststate = ti->second.first;
	lastaction = ti->second.second;
	
	if (laststate != rli_transmodel::INITSTATE && lastaction != rli_transmodel::NULLACTION) {
		model.addexp(laststate, lastaction, rli_transmodel::TERMSTATE, r);
		//cout << "RTDP " << laststate << "-" << lastaction << "->" << rli_transmodel::TERMSTATE << endl;
		//cout << "RTDP Model:" << endl;
		//model.print(cout);
	}
	
	ti->second.first = rli_transmodel::TERMSTATE;
}

void rtdp::set_action(int trace, rli_action* action) {
	map<int, pair<rli_state_id, rli_action_id> >::iterator i;
	i = traces.find(trace);
	assert(i != traces.end());
	i->second.second = metastate.get_action_id(action);
}

void rtdp::set_null_action(int trace) {
	map<int, pair<rli_state_id, rli_action_id> >::iterator i;
	i = traces.find(trace);
	assert(i != traces.end());
	i->second.second = rli_transmodel::NULLACTION;
}

void rtdp::dp_update(rli_state_id sid, double discount, map<production_struct*,double> &changes) {
	rli_state *state;
	rli_state::iterator j;
	rli_action *a;
	rli_action::iterator k;
	double q;
	bool nostats;

	if (sid == rli_transmodel::INITSTATE) {
		return;
	}
	
	state = metastate.get_state(sid);
	
	for ( j = state->begin(); j != state->end(); ++j ) {
		q = model.TDsignal(sid, *j, discount, svals, nostats);
		if (!nostats) {
			a = metastate.get_action(*j);
			for (k = a->begin(); k != a->end(); ++k) {
				qvals[*k] = q / a->size();
				changes[*k] = q / a->size();
				update_vals(*k);
			}
		} else {
			//cout << "RTDP: NO STATS!" << endl;
		}
	}
}

void rtdp::random_dp_update(double discount, map<production_struct*, double> &changes) {
	rli_state_id randstate = metastate.random_state();
	dp_update(randstate, discount, changes);
}

rli_transmodel &rtdp::get_model() {
	return model;
}

rli_metastate &rtdp::get_metastate() {
	return metastate;
}

/*
 * The value of a state is the maximum Q value of the legal actions in
 * that state
 */
double rtdp::update_sval(rli_state_id sid) {
	rli_state *s;
	rli_state::iterator i;
	map<rli_action_id, double>::iterator j;
	map<rli_state_id, double>::iterator k;
	double max, q;
	
	s = metastate.get_state(sid);
	assert(s);
		
	max = -numeric_limits<double>::max();
	for (i = s->begin(); i != s->end(); ++i) {
		j = avals.find(*i);
		if (j == avals.end()) {
			q = update_aval(*i);
		} else {
			q = j->second;
		}
		if (q > max) {
			max = q;
		}
	}
	
	k = svals.find(sid);
	if (k == svals.end()) {
		svals[sid] = max;
	} else {
		k->second = max;
	}
	
	return max;
}

/*
 * The value for an action is the sum of the Q values of the RL rules
 * that contribute to the action.
 */
double rtdp::update_aval(rli_action_id aid) {
	rli_action *a;
	rli_action::iterator i;
	map<rli_action_id, double>::iterator j;
	double sum;
	
	a = metastate.get_action(aid);
	assert(a);
		
	sum = 0.0;
	for (i = a->begin(); i != a->end(); ++i) {
		sum += qvals.find(*i)->second;
	}
	
	j = avals.find(aid);
	if (j == avals.end()) {
		avals[aid] = sum;
	} else {
		j->second = sum;
	}
	
	return sum;
}

/*
 * Update the necessary action and state values given that RL rule r
 * changed value.
 *
 * The current implementation is pretty inefficient as every update
 * to an RL rule will result in the recomputation of all action and
 * state values that are affected. A faster implementation would just
 * mark which actions and states need value updates and update lazily,
 * potentially taking care of many changes to RL rule values in one
 * update.
 */
void rtdp::update_vals(production_struct *r) {
	set<rli_action_id> relactions;
	set<rli_action_id>::iterator ai;
	set<rli_state_id> relstates;
	set<rli_state_id>::iterator si;
	
	metastate.get_relevant_actions(r, relactions);
	
	// update Q value for all affected rli_actions
	for (ai = relactions.begin(); ai != relactions.end(); ++ai) {
		update_aval(*ai);
		metastate.get_relevant_states(*ai, relstates);
	}
	
	for (si = relstates.begin(); si != relstates.end(); ++si) {
		update_sval(*si);
	}
}

void rtdp::print_svals(ostream &os) {
	map<rli_state_id, double>::iterator i;
	for (i = svals.begin(); i != svals.end(); ++i) {
		os << i->first << " " << i->second << endl;
	}
}

void rtdp::print_avals(ostream &os) {
	map<rli_action_id, double>::iterator i;
	for (i = avals.begin(); i != avals.end(); ++i) {
		os << i->first << " " << i->second << endl;
	}
}

void rtdp::print_model(ostream &os) {
	model.print(os, metastate);
}