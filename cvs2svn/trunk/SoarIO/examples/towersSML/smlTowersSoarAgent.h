#ifndef SGIO_HANOI_SOAR_AGENT
#define SGIO_HANOI_SOAR_AGENT

#include "sml_Client.h"

using namespace sml ;

/*
#include "sgio_agent.h"
#include "sgio_wmemem.h"
*/

class HanoiWorld;
//namespace sgio_towers
//{

class SoarAgent
{
public:
	SoarAgent(Agent* inAgent, Agent* inWMemory, HanoiWorld* inWorld);

	~SoarAgent();

	void MakeMove();

private:

	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();

	Agent* pAgent;
	Agent* pWMemory;	// In SML there is just an agent, so where SGIO used WorkingMemory we use the Agent pointer again
	HanoiWorld* pWorld;
};


//}// closes namespace

#endif //SGIO_HANOI_SOAR_AGENT