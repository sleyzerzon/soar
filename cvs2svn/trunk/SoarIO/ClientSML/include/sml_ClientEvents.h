/////////////////////////////////////////////////////////////////
// List of event ids
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// These are the events that an SML agent can listen for and definitions
// for the handlers that will be called.
//
// ==========================================
// READ THESE NOTES BEFORE CHANGING THIS FILE:
// ==========================================
//
// There are several constraints on the IDs in these event enums.
//
// First, each value must be a unique integer.
//     That's to say the value of the enums in smlSystemEventId
//     must not intersect with any of the values in smlRunEventId (e.g.)
//     This is generally achieved by making the first value of one enum
//     equal the last value of the previous enum + 1.
//     (e.g. smlEVENT_BEFORE_SMALLEST_STEP = smlEVENT_AFTER_RHS_FUNCTION_EXECUTED + 1)
//
// Second, if you change the list of events in an enum, make sure that the
//    related test function (e.g. IsSystemEventID()) is still valid.
//
// Third, the values of these IDs currently are required to match the values
//    in the gSKIEventId enumeration in the kernel.
//    This way there's no need to map to and from ID values, making debugging
//    easier.
//    If we wish to break this requirement later all that's needed is to add
//    a mapping function on the kernel side.
//    There are some run time ASSERTs included in the kernel to help check
//    that the mapping has not been thrown off somehow, but we basically
//    need to be careful when changing either.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EVENT_ID_H
#define SML_EVENT_ID_H

namespace sml {

// Forward declaration
class Agent ;
class Kernel ;

typedef enum {
    smlEVENT_BEFORE_SHUTDOWN            = 1,
    smlEVENT_AFTER_CONNECTION_LOST,
    smlEVENT_BEFORE_RESTART,
    smlEVENT_AFTER_RESTART,
    smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
    smlEVENT_AFTER_RHS_FUNCTION_ADDED,
    smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
    smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
    smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
	smlEVENT_AFTER_RHS_FUNCTION_EXECUTED
} smlSystemEventId ;

static inline bool IsSystemEventID(int id)
{
	return (id >= smlEVENT_BEFORE_SHUTDOWN && id <= smlEVENT_AFTER_RHS_FUNCTION_EXECUTED) ;
}

typedef enum {
    smlEVENT_BEFORE_SMALLEST_STEP = smlEVENT_AFTER_RHS_FUNCTION_EXECUTED + 1,
    smlEVENT_AFTER_SMALLEST_STEP,
    smlEVENT_BEFORE_ELABORATION_CYCLE,
    smlEVENT_AFTER_ELABORATION_CYCLE,
    smlEVENT_BEFORE_PHASE_EXECUTED,
    smlEVENT_AFTER_PHASE_EXECUTED,
    smlEVENT_BEFORE_DECISION_CYCLE,
    smlEVENT_AFTER_DECISION_CYCLE,
    smlEVENT_AFTER_INTERRUPT,
    smlEVENT_BEFORE_RUNNING,
    smlEVENT_AFTER_RUNNING,
} smlRunEventId ;

static inline bool IsRunEventID(int id)
{
	return (id >= smlEVENT_BEFORE_SMALLEST_STEP && id <= smlEVENT_AFTER_RUNNING) ;
}

typedef enum {
    // Production Manager
    smlEVENT_AFTER_PRODUCTION_ADDED = smlEVENT_AFTER_RUNNING + 1,
    smlEVENT_BEFORE_PRODUCTION_REMOVED,
    //smlEVENT_BEFORE_PRODUCTION_FIRED,
    smlEVENT_AFTER_PRODUCTION_FIRED,
    smlEVENT_BEFORE_PRODUCTION_RETRACTED,
} smlProductionEventId ;

static inline bool IsProductionEventID(int id)
{
	return (id >= smlEVENT_AFTER_PRODUCTION_ADDED && id <= smlEVENT_BEFORE_PRODUCTION_RETRACTED) ;
}

typedef enum {
	// Agent manager
    smlEVENT_AFTER_AGENT_CREATED = smlEVENT_BEFORE_PRODUCTION_RETRACTED + 1,
    smlEVENT_BEFORE_AGENT_DESTROYED,
    smlEVENT_BEFORE_AGENT_REINITIALIZED,
    smlEVENT_AFTER_AGENT_REINITIALIZED,
} smlAgentEventId ;

static inline bool IsAgentEventID(int id)
{
	return (id >= smlEVENT_AFTER_AGENT_CREATED && id <= smlEVENT_AFTER_AGENT_REINITIALIZED) ;
}

typedef enum {
	// Working memory changes
	smlEVENT_OUTPUT_PHASE_CALLBACK = smlEVENT_AFTER_AGENT_REINITIALIZED + 1,
} smlWorkingMemoryEventId ;

static inline bool IsWorkingMemoryEventID(int id)
{
	return (id >= smlEVENT_OUTPUT_PHASE_CALLBACK && id <= smlEVENT_OUTPUT_PHASE_CALLBACK) ;
}

typedef enum {
    // Error and print callbacks
	smlEVENT_LOG_ERROR = smlEVENT_OUTPUT_PHASE_CALLBACK + 1,
    smlEVENT_LOG_WARNING,
    smlEVENT_LOG_INFO,
    smlEVENT_LOG_DEBUG,
    smlEVENT_PRINT,
} smlPrintEventId ;

static inline bool IsPrintEventID(int id)
{
	return (id >= smlEVENT_LOG_ERROR && id <= smlEVENT_PRINT) ;
}

typedef enum {
    // Used to indicate an error in some cases
    smlEVENT_INVALID_EVENT              = 0,

	// Marker for end of sml event list
	// Must always be at the end of the enum
	smlEVENT_LAST = smlEVENT_PRINT + 1
} smlGenericEventId ;

typedef enum {
    sml_INPUT_PHASE,
    sml_PROPOSAL_PHASE,
    sml_SELECTION_PHASE,
    sml_APPLY_PHASE,
    sml_OUTPUT_PHASE,
    sml_DECISION_PHASE,
} smlPhase;

// These typedefs all define types of functions.
// For example: typedef void (*X)(type1 arg1, type2 arg2) means we're defining function "X" to take (type1 arg1, type2 arg2) and return void.
// To provide such a handler define a function with this type and pass its address in to the registration function for the event.

// Handler for Run events.
// Passed back the event ID, the agent and the phase together with whatever user data we registered with the client
typedef void (*RunEventHandler)(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);

// Handler for Agent events.
typedef void (*AgentEventHandler)(smlAgentEventId id, void* pUserData, Agent* pAgent) ;

// Handler for Print events.
typedef void (*PrintEventHandler)(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) ;

// Handler for Production manager events.
typedef void (*ProductionEventHandler)(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) ;

// Handler for System events.
typedef void (*SystemEventHandler)(smlSystemEventId id, void* pUserData, Kernel* pKernel) ;

} ;	// End of namespace

#endif	// Header
