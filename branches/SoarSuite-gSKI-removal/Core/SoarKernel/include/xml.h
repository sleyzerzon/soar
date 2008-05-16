/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                 xml.h
 * =======================================================================
 *  
 * Contains methods for generating XML objects in response to kernel commands.
 *
 * The commands are modelled after the existing kernel functions which are tied to generating
 * string output.  In the past we added code to some of those functions so they'd
 * generate string output and also XML output (as a string: <aaa>...</aaa>).  To capture
 * the XML output a caller would register for the XML callback, generate the XML as a string,
 * parse the XML back to an object and return it.
 *
 * These methods generate XML as an object, so there are no strings being created
 * and subsequently parsed and no need to "intercept" the XML callback channel (which is
 * really for XML trace output to the debugger, not for results from commands).
 * This new approach is more efficient to both create and to subsequently use.

======================================================================= */

#ifndef SOAR_XML_H
#define SOAR_XML_H

#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "callback.h"
#include <stdio.h>

// This is the data that needs to be passed back with the xml callback
// This is defined here because it needs to be included by KernelSML/CLI and the SoarKernel
struct XmlCallbackData {
	const char* funcType;
	const char* attOrTag;
	const char* value;
};

typedef union symbol_union Symbol;
typedef struct wme_struct wme;

extern void xmlBeginTag(char const* pTag) ;
extern void xmlEndTag(char const* pTag) ;
extern void xmlString(char const* pAttribute, char const* pValue) ;
extern void xmlSymbol(agent* thisAgent, char const* pAttribute, Symbol* pSymbol) ;
extern void xmlMoveCurrentToParent() ;
extern void xmlMoveCurrentToChild(int index) ;
extern void xmlMoveCurrentToLastChild() ;
extern void xmlULong(char const* pAttribute, unsigned long value) ;
extern void xmlInt(char const* pAttribute, int value) ;
extern void xmlAddSimpleTag(char const* pTag) ;
extern void xmlAttValue(char const* pTag, char const* pAttribute, char const* pValue) ;
extern void xml_wme (agent* thisAgent, wme *w) ;

/**
 * @brief Special functions to handle the xml callback
 */

inline void makeAgentCallbackXML(	agent*		soarAgent,
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

inline void makeAgentCallbackXML(	agent*			soarAgent,
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
	makeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void makeAgentCallbackXML(	agent*		soarAgent,
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
	makeAgentCallbackXML(soarAgent, funcType, attOrTag, (char*)buf);
	/*
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
	*/
}

inline void GenerateWarningXML(agent* soarAgent, const char* message) {
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagWarning);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagWarning);
}

inline void GenerateErrorXML(agent* soarAgent, const char* message) {
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagError);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagError);
}

inline void GenerateMessageXML(agent* soarAgent, const char* message) {
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagMessage);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagMessage);
}

inline void GenerateVerboseXML(agent* soarAgent, const char* message) {
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionBeginTag, xmlTraceNames::kTagVerbose);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionAddAttribute, xmlTraceNames::kTypeString, message);
	makeAgentCallbackXML(soarAgent, xmlTraceNames::kFunctionEndTag, xmlTraceNames::kTagVerbose);
}

#endif
