/********************************************************************
* @file gski_agentperformancemonitor.h
*********************************************************************
* @remarks Copyright (C) 2004 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/2/2004   10:48
*
* purpose: 
*********************************************************************/
#ifndef GSKI_AGENTPERFORMANCEMONITOR_H
#define GSKI_AGENTPERFORMANCEMONITOR_H

#ifdef _WIN32
#pragma warning(disable: 4786)
#endif

#include <string>

#include "IgSKI_AgentPerformanceMonitor.h"

namespace gSKI {
   class Agent;

   class AgentPerformanceMonitor : public IAgentPerformanceMonitor
   {
   public:
      AgentPerformanceMonitor(Agent* pAgent);
      
      virtual ~AgentPerformanceMonitor();

      virtual bool GetStatsString(int argc, char* argv[], 
                                  const char** result);

   private:
      Agent* m_pAgent;
      std::string m_result;

      bool parse_system_stats(int argc, char *argv[]);
      bool parse_memory_stats(int argc, char *argv[]);
      bool parse_rete_stats(int argc, char *argv[]);
      void soar_ecPrintSystemStatistics();
      void soar_ecPrintReteStatistics();
      void soar_ecPrintMemoryStatistics();
      void soar_ecPrintMemoryPoolStatistics();
      void print_null_activation_stats();
      void printTimingInfo();

   };
}

#endif // GSKI_AGENTPERFORMANCEMONITOR_H
