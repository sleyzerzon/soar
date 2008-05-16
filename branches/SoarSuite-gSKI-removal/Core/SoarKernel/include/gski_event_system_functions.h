/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_event_system_kernel.h 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#ifndef GSKI_EVENT_SYSTEM_FUNCTIONS_H
#define GSKI_EVENT_SYSTEM_FUNCTIONS_H

/*********************************** INLINE FUNCTIONS *****************************************/
/* Gotta include these after the other stuff to keep circular includes from happening. */
#include "agent.h"
#include "xml.h"

#include <assert.h>

/**
 * @brief Special functions to handle the xml callback
 */

inline void gSKI_MakeAgentCallbackXML(	agent*		soarAgent,
                                        const char*	funcType,
                                        const char*	attOrTag,
										const char*	value=0)
{
   XmlCallbackData xml_data;
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   
   xml_data.funcType = funcType;
   xml_data.attOrTag = attOrTag;
   xml_data.value = value;

   soar_invoke_first_callback(soarAgent, soarAgent, 
	                          XML_GENERATION_CALLBACK, /*(XmlCallbackData)*/ static_cast<void*>(&xml_data));
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void gSKI_MakeAgentCallbackXML(	agent*			soarAgent,
                                        const char*		funcType,
                                        const char*		attOrTag,
										unsigned long	value)
{
	char buf[25];
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   

	SNPRINTF(buf, 24, "%lu", value);
	gSKI_MakeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void gSKI_MakeAgentCallbackXML(	agent*		soarAgent,
                                        const char*	funcType,
                                        const char*	attOrTag,
										double      value)
{
	char buf[25];
	/*
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
	*/	   
	SNPRINTF(buf, 24, "%f", value);
	gSKI_MakeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void GenerateWarningXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagWarning);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagWarning);
}

inline void GenerateErrorXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagError);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagError);
}

inline void GenerateMessageXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagMessage);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagMessage);
}

inline void GenerateVerboseXML(agent* soarAgent, const char* message) {
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagVerbose);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	gSKI_MakeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagVerbose);
}

#endif

