/* File : sml_ClientInterface.i */
%module sml

// uncomment the following line if you're using Java 1.5/5.0 (or later) and want
//	proper Java enums to be generated for the SML enums
//%include "enums.swg"

%javaconst(1); // strongly recommended by SWIG manual section 19.3.5.1
// the previous line causes problems for some enum values, so we have to set them manually here
// the problem only affects those enums whose values are "calculated" based on other values
%javaconstvalue("smlSystemEventId.smlEVENT_AFTER_RHS_FUNCTION_EXECUTED.swigValue() + 1") smlEVENT_BEFORE_SMALLEST_STEP;
%javaconstvalue("smlProductionEventId.smlEVENT_BEFORE_PRODUCTION_RETRACTED.swigValue() + 1") smlEVENT_AFTER_AGENT_CREATED;
%javaconstvalue("smlPrintEventId.smlEVENT_PRINT.swigValue() + 1") smlEVENT_RHS_USER_FUNCTION;
%javaconstvalue("smlRhsEventId.smlEVENT_RHS_USER_FUNCTION.swigValue() + 1") smlEVENT_XML_TRACE_OUTPUT;
%javaconstvalue("smlXMLEventId.smlEVENT_XML_INPUT_RECEIVED.swigValue() + 1") smlEVENT_AFTER_ALL_OUTPUT_PHASES;
%javaconstvalue("smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT.swigValue() + 1") smlEVENT_EDIT_PRODUCTION;
%javaconstvalue("smlUntypedEventId.smlEVENT_EDIT_PRODUCTION.swigValue() + 1") smlEVENT_LAST;
%javaconstvalue("smlWorkingMemoryEventId.smlEVENT_OUTPUT_PHASE_CALLBACK.swigValue() + 1") smlEVENT_LOG_ERROR;
%javaconstvalue("smlRunEventId.smlEVENT_AFTER_RUNNING.swigValue() + 1") smlEVENT_AFTER_PRODUCTION_ADDED;
%javaconstvalue("smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue() + 1") smlEVENT_OUTPUT_PHASE_CALLBACK;

//
// Doug's custom Java code for registering/unregistering callbacks
//

// in the Java case, we will also provide custom code for unregistering from events
%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Agent::RemoveOutputHandler(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForUntypedEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      throw e ;
    }
  }
  
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6, boolean jarg7);
  public final static native int Agent_RegisterForXMLEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Agent_AddOutputHandler(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForUpdateEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForUntypedEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, Object jarg6);
  public final static native int Kernel_AddRhsFunction(long jarg1, String jarg2, Object jarg3, Object jarg4, Object jarg6);

  public final static native boolean Agent_UnregisterForRunEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForProductionEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForPrintEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForXMLEvent(long jarg1, int jarg2);
  public final static native boolean Agent_RemoveOutputHandler(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForSystemEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForUpdateEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForUntypedEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForAgentEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_RemoveRhsFunction(long jarg1, int jarg2);
%}

%typemap(javacode) sml::Agent %{
  public interface RunEventInterface {
	public void runEventHandler(int eventID, Object data, Agent agent, int phase) ;
  }

  public interface ProductionEventInterface {
     public void productionEventHandler(int eventID, Object data, Agent agent, String prodName, String instantiation) ;
  }
  
  public interface PrintEventInterface { 
  		public void printEventHandler(int eventID, Object data, Agent agent, String message) ;
  }
  
  public interface xmlEventInterface {
  		public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml) ;
  }

  public interface OutputEventInterface {  
  		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) ;
  }

  public int RegisterForRunEvent(smlRunEventId id, RunEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
  
  public int RegisterForProductionEvent(smlProductionEventId id, ProductionEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, true) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, PrintEventInterface handlerObject, Object callbackData, boolean ignoreOwnEchos)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData, ignoreOwnEchos) ; }

  public int RegisterForXMLEvent(smlXMLEventId id, xmlEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_RegisterForXMLEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }
  
  public boolean UnregisterForRunEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForRunEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForProductionEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForPrintEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForXMLEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForXMLEvent(swigCPtr, callbackReturnValue) ;}

  public int AddOutputHandler(String attributeName, OutputEventInterface handlerObject, Object callbackData)
  { return smlJNI.Agent_AddOutputHandler(swigCPtr, attributeName, this, handlerObject, callbackData) ; }

  public boolean RemoveOutputHandler(int callbackReturnValue)
  { return smlJNI.Agent_RemoveOutputHandler(swigCPtr, callbackReturnValue) ;}
%}

%typemap(javacode) sml::Kernel %{
  public interface SystemEventInterface {
     public void systemEventHandler(int eventID, Object data, Kernel kernel) ;
  }

  public interface UpdateEventInterface {  
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) ;
  }

  public interface UntypedEventInterface {  
  	public void untypedEventHandler(int eventID, Object userData, Kernel kernel, Object callbackData) ;
  }

  public interface AgentEventInterface {  
  		public void agentEventHandler(int eventID, Object data, String agentName) ;
  }

  public interface RhsFunctionInterface {  
  		public String rhsFunctionHandler(int eventID, Object data, String agentName, String functionName, String argument) ;
  }
  
  public int RegisterForSystemEvent(smlSystemEventId id, SystemEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
 
  public boolean UnregisterForSystemEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, callbackReturnValue) ;}

  public int RegisterForUpdateEvent(smlUpdateEventId id, UpdateEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForUpdateEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}

  public boolean UnregisterForUpdateEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForUpdateEvent(swigCPtr, callbackReturnValue) ;}

  public int RegisterForUntypedEvent(smlUntypedEventId id, UntypedEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForUntypedEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ;}
 
  public boolean UnregisterForUntypedEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForUntypedEvent(swigCPtr, callbackReturnValue) ;}
  
  public int RegisterForAgentEvent(smlAgentEventId id, AgentEventInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_RegisterForAgentEvent(swigCPtr, id.swigValue(), this, handlerObject, callbackData) ; }

  public boolean UnregisterForAgentEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForAgentEvent(swigCPtr, callbackReturnValue) ;}

  public int AddRhsFunction(String functionName, RhsFunctionInterface handlerObject, Object callbackData)
  { return smlJNI.Kernel_AddRhsFunction(swigCPtr, functionName, this, handlerObject, callbackData) ; }

  public boolean RemoveRhsFunction(int callbackReturnValue)
  { return smlJNI.Kernel_RemoveRhsFunction(swigCPtr, callbackReturnValue) ;}

%}

//
// End custom Java callback code
//

// include stuff common to all languages (i.e. Java and Tcl)
%include "../sml_ClientInterface.i"

// include Doug's custom JNI code for callbacks in the wrapper section
//  so it's in the extern C block
%wrapper %{
#include "JavaCallbackByHand.h"
%}

