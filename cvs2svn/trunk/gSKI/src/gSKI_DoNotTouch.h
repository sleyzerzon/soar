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

#include "IgSKI_DoNotTouch.h"

#include <string>

namespace gSKI
{
   class IAgent;
   namespace EvilBackDoor 
   {
      class TgDWorkArounds : public ITgDWorkArounds
      {
      public:

         /**
         * @brief
         */
         virtual ~TgDWorkArounds(){}

         /**
         * @brief
         */
         TgDWorkArounds(){}

         /**
         * @brief
         */
         void SetSysparam (IAgent* agent, int param_number, long new_value);
         long GetSysparam(IAgent* agent, int param_number);
		 const long* GetSysparams(IAgent* agent);

         rete_node* NameToProduction(IAgent* agent, char* string_to_test);
         void PrintPartialMatchInformation(IAgent* thisAgent, 
         struct rete_node_struct *p_node, wme_trace_type wtt);
         void PrintMatchSet(IAgent* thisAgent, wme_trace_type wtt, ms_trace_type  mst);
         void PrintStackTrace(IAgent* thisAgent, bool print_states, bool print_operators);
         void PrintSymbol(IAgent*     thisAgent, 
                          char*       arg, 
                          bool        name_only, 
                          bool        print_filename, 
                          bool        internal,
                          bool        full_prod,
                          int         depth);
         void PrintUser(IAgent*       thisAgent,
                        char*         arg,
                        bool          internal,
                        bool          print_filename,
                        bool          full_prod,
                        unsigned int  productionType);
                     
         bool Preferences(IAgent*     thisAgent,
                          int         argc,
                          char*       argv[]);


         bool ProductionFind(IAgent*     thisAgent,
                             agent*      agnt,
                             IKernel*    kernel,
                             bool        lhs,
                             bool        rhs,
                             char*       arg,
                             bool        show_bindings,
                             bool        just_chunks,
                             bool        no_chunks);

         bool GDSPrint(IAgent* thisAgent);

		 void GetForceLearnStates(IAgent* pIAgent, std::string& res);
		 void GetDontLearnStates(IAgent* pIAgent, std::string& res);

		 void SetVerbosity(IAgent* pIAgent, bool setting);
		 bool GetVerbosity(IAgent* pIAgent);

		 bool BeginTracingProduction(IAgent* pIAgent, const char* pProductionName);
		 bool StopTracingProduction(IAgent* pIAgent, const char* pProductionName);

		 long AddWme(IAgent* pIAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool acceptable);
		 int RemoveWmeByTimetag(IAgent* pIAgent, int num);

		 void PrintInternalSymbols(IAgent* pIAgent);

		 int AddWMEFilter(IAgent* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes);
	  };
   }
}
#endif

