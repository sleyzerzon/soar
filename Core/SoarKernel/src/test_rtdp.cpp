
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <stdlib.h>

#include "rtdp.h"

using namespace std;

typedef struct production_struct {
	production_struct(int s, int a) : state(s), action(a) { }
	int state;
	int action;
} production;

const int nstates = 3;
const int nactions = 2;
const int steps = 1000;

/* transition table */
int transitions[nstates][nactions][2] = {
    /* action       0    |     1        *
     *           ns, r   |  ns, r       *
     * state -------------------------- */
	/* 0 */   { { 1, +1 }, { 2, +2 } },
	/* 1 */   { { 1, +0 }, { 0, +1 } },
	/* 2 */   { { 2, +0 }, { 0, -1 } }
};

production* greedy(int state, map<production*, double> &qvals) {
	map<production*, double>::iterator i;
	double max = -10000.0;
	production *choice;
	
	for (i = qvals.begin(); i != qvals.end(); ++i) {
		if (i->first->state == state && max < i->second) {
			max = i->second;
			choice = i->first;
		}
	}
	return choice;
}

production* rand(int state, map<production*, double> &qvals) {
	map<production*, double>::iterator i;
	vector<production*> choices;
	
	for (i = qvals.begin(); i != qvals.end(); ++i) {
		if (i->first->state == state) {
			choices.push_back(i->first);
		}
	}
	return choices[random() % choices.size()];
}

int main(int argc, char *argv[]) {
	int currstate = 0;
	int nextstate;

	double reward;
	
	map<int, list<production*> > state_prods;
	list<production*>::iterator pli;
	
	map<production*, double> qvals;
	map<production*, double> qvalupdates;
	map<production*, double>::iterator qvi;
	
	rtdp rtdp;
	
	int trace = rtdp.new_trace();

	for (int i = 0; i < nstates; ++i) {
		for (int j = 0; j < nactions; ++j) {
			production *p = new production(i, j);
			qvals[p] = 0.0;
			rtdp.register_rule(p, 0.0);
			state_prods[i].push_back(p);
		}
	}
	
	for (int step = 0; step < steps; ++step) {
		cout << "Current State: " << currstate << endl;
		
		// end of proposal phase
		list<rli_action*> actions;
		for (pli = state_prods[currstate].begin(); pli != state_prods[currstate].end(); ++pli) {
			rli_action *a = new rli_action();
			a->insert(*pli);
			actions.push_back(a);
		}
		
		rtdp.update_state(trace, actions, reward);
		rtdp.dp_update(actions, 0.9, qvalupdates);
		
		cout << "UPDATES" << endl;
		for (qvi = qvalupdates.begin(); qvi != qvalupdates.end(); ++qvi) {
			cout << qvi->first->state << "," << qvi->first->action << "-> " << qvi->second << endl;
			qvals[qvi->first] = qvi->second;
		}
		qvalupdates.clear();
		
		// decision phase
		production *p = rand(currstate, qvals);
		
		// after decision phase
		rli_action a;
		a.insert(p);
		rtdp.set_action(trace, &a);

		// after decision phase
		nextstate = transitions[currstate][p->action][0];
		reward = transitions[currstate][p->action][1];
		currstate = nextstate;
	}
	
	cout << "FINAL Q VALUES" << endl;
	for (qvi = qvals.begin(); qvi != qvals.end(); ++qvi) {
		cout << qvi->first->state << "," << qvi->first->action << " -> " << qvi->second << endl;
	}
	
	cout << "FINAL MODEL" << endl;
	rtdp.get_model().print(cout);
	
	return 0;
}