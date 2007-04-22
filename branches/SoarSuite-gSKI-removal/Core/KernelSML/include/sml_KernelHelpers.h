/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_donottouch.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_DONOTTOUCH_H
#define GSKI_DONOTTOUCH_H

typedef struct production_struct production;
typedef unsigned char wme_trace_type;
typedef struct rete_node_struct rete_node;
typedef unsigned char ms_trace_type;
typedef struct agent_struct agent;

typedef void * soar_callback_agent;

#include <string>

namespace sml
{
  class AgentSML ;
  class KernelSML ;

  class KernelHelpers
  {
  public:

     /**
     * @brief
     */
     virtual ~KernelHelpers(){}

     /**
     * @brief
     */
     KernelHelpers(){}

     /**
     * @brief
     */
	 void SetSysparam (AgentSML* agent, int param_number, long new_value);
     long GetSysparam(AgentSML* agent, int param_number);
	 const long* GetSysparams(AgentSML* agent);

     rete_node* NameToProduction(AgentSML* agent, char* string_to_test);
     void PrintPartialMatchInformation(AgentSML* thisAgent, 
     struct rete_node_struct *p_node, wme_trace_type wtt);
     void PrintMatchSet(AgentSML* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
	 virtual void XMLPartialMatchInformation(AgentSML* thisAgent, 
                                               struct rete_node_struct *p_node,
                                               wme_trace_type wtt) ;
    virtual void XMLMatchSet(AgentSML* thisAgent, wme_trace_type wtt, ms_trace_type  mst) ;

     void PrintStackTrace(AgentSML* thisAgent, bool print_states, bool print_operators);
     void PrintSymbol(AgentSML*     thisAgent, 
                      char*       arg, 
                      bool        name_only, 
                      bool        print_filename, 
                      bool        internal,
                      bool        full_prod,
                      int         depth);
     void PrintUser(AgentSML*       thisAgent,
                    char*         arg,
                    bool          internal,
                    bool          print_filename,
                    bool          full_prod,
                    unsigned int  productionType);
                 
     bool Preferences(AgentSML* thisAgent, int detail, bool object, const char* idString, const char* attrString);

     bool ProductionFind(AgentSML*     thisAgent,
                         agent*      agnt,
                         KernelSML*    kernel,
                         bool        lhs,
                         bool        rhs,
                         char*       arg,
                         bool        show_bindings,
                         bool        just_chunks,
                         bool        no_chunks);

     bool GDSPrint(AgentSML* thisAgent);

	 void GetForceLearnStates(AgentSML* pIAgent, std::string& res);
	 void GetDontLearnStates(AgentSML* pIAgent, std::string& res);

	 void SetVerbosity(AgentSML* pIAgent, bool setting);
	 bool GetVerbosity(AgentSML* pIAgent);

	 bool BeginTracingProduction(AgentSML* pIAgent, const char* pProductionName);
	 bool StopTracingProduction(AgentSML* pIAgent, const char* pProductionName);

	 long AddWme(AgentSML* pIAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool acceptable);
	 int RemoveWmeByTimetag(AgentSML* pIAgent, int num);

	 void PrintInternalSymbols(AgentSML* pIAgent);

	 int AddWMEFilter(AgentSML* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
	 int RemoveWMEFilter(AgentSML* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
	 bool ResetWMEFilters(AgentSML* pIAgent, bool adds, bool removes);
	 void ListWMEFilters(AgentSML* pIAgent, bool adds, bool removes);

	 void ExplainListChunks(AgentSML* pIAgent);
	 bool ExplainChunks(AgentSML* pIAgent, const char* pProduction, int mode);

	 const char* GetChunkNamePrefix(AgentSML* pIAgent);
	 bool SetChunkNamePrefix(AgentSML* pIAgent, const char* pPrefix);

	 unsigned long GetChunkCount(AgentSML* pIAgent);
	 void SetChunkCount(AgentSML* pIAgent, unsigned long count);

	 void SeedRandomNumberGenerator(unsigned long int* pSeed);
  };
}
#endif

