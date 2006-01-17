/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class Kernel extends ClientErrors {
  private long swigCPtr;

  protected Kernel(long cPtr, boolean cMemoryOwn) {
    super(smlJNI.SWIGKernelUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Kernel obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected Kernel() {
    this(0, false);
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_Kernel(swigCPtr);
    }
    swigCPtr = 0;
    super.delete();
  }

  public int RegisterForSystemEvent(smlSystemEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForSystemEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ;}
 
  public boolean UnregisterForSystemEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForSystemEvent(swigCPtr, callbackReturnValue) ;}
  
  public int RegisterForAgentEvent(smlAgentEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_RegisterForAgentEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }

  public boolean UnregisterForAgentEvent(int callbackReturnValue)
  { return smlJNI.Kernel_UnregisterForAgentEvent(swigCPtr, callbackReturnValue) ;}

  public int AddRhsFunction(String functionName, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Kernel_AddRhsFunction(swigCPtr, functionName, this, handlerObject, handlerMethod, callbackData) ; }

  public boolean RemoveRhsFunction(int callbackReturnValue)
  { return smlJNI.Kernel_RemoveRhsFunction(swigCPtr, callbackReturnValue) ;}


  public static Kernel CreateKernelInCurrentThread(String pLibraryName, boolean optimized, int portToListenOn) {
    long cPtr = smlJNI.Kernel_CreateKernelInCurrentThread__SWIG_0(pLibraryName, optimized, portToListenOn);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateKernelInCurrentThread(String pLibraryName, boolean optimized) {
    long cPtr = smlJNI.Kernel_CreateKernelInCurrentThread__SWIG_1(pLibraryName, optimized);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateKernelInCurrentThread(String pLibraryName) {
    long cPtr = smlJNI.Kernel_CreateKernelInCurrentThread__SWIG_2(pLibraryName);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateKernelInNewThread(String pLibraryName, int portToListenOn) {
    long cPtr = smlJNI.Kernel_CreateKernelInNewThread__SWIG_0(pLibraryName, portToListenOn);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateKernelInNewThread(String pLibraryName) {
    long cPtr = smlJNI.Kernel_CreateKernelInNewThread__SWIG_1(pLibraryName);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateRemoteConnection(boolean sharedFileSystem, String pIPaddress, int port) {
    long cPtr = smlJNI.Kernel_CreateRemoteConnection__SWIG_0(sharedFileSystem, pIPaddress, port);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static Kernel CreateRemoteConnection(boolean sharedFileSystem, String pIPaddress) {
    long cPtr = smlJNI.Kernel_CreateRemoteConnection__SWIG_1(sharedFileSystem, pIPaddress);
    return (cPtr == 0) ? null : new Kernel(cPtr, true);
  }

  public static int GetDefaultPort() {
    return smlJNI.Kernel_GetDefaultPort();
  }

  public void SetTraceCommunications(boolean state) {
    smlJNI.Kernel_SetTraceCommunications(swigCPtr, state);
  }

  public boolean IsTracingCommunications() {
    return smlJNI.Kernel_IsTracingCommunications(swigCPtr);
  }

  public Agent CreateAgent(String pAgentName) {
    long cPtr = smlJNI.Kernel_CreateAgent(swigCPtr, pAgentName);
    return (cPtr == 0) ? null : new Agent(cPtr, false);
  }

  public void UpdateAgentList() {
    smlJNI.Kernel_UpdateAgentList(swigCPtr);
  }

  public int GetNumberAgents() {
    return smlJNI.Kernel_GetNumberAgents(swigCPtr);
  }

  public boolean DestroyAgent(Agent pAgent) {
    return smlJNI.Kernel_DestroyAgent(swigCPtr, Agent.getCPtr(pAgent));
  }

  public Agent GetAgent(String pAgentName) {
    long cPtr = smlJNI.Kernel_GetAgent(swigCPtr, pAgentName);
    return (cPtr == 0) ? null : new Agent(cPtr, false);
  }

  public Agent GetAgentByIndex(int index) {
    long cPtr = smlJNI.Kernel_GetAgentByIndex(swigCPtr, index);
    return (cPtr == 0) ? null : new Agent(cPtr, false);
  }

  public boolean IsAgentValid(Agent pAgent) {
    return smlJNI.Kernel_IsAgentValid(swigCPtr, Agent.getCPtr(pAgent));
  }

  public String ExecuteCommandLine(String pCommandLine, String pAgentName) {
    return smlJNI.Kernel_ExecuteCommandLine(swigCPtr, pCommandLine, pAgentName);
  }

  public boolean ExecuteCommandLineXML(String pCommandLine, String pAgentName, AnalyzeXML pResponse) {
    return smlJNI.Kernel_ExecuteCommandLineXML(swigCPtr, pCommandLine, pAgentName, AnalyzeXML.getCPtr(pResponse));
  }

  public String RunAllAgents(long numberSteps, smlRunStepSize stepSize) {
    return smlJNI.Kernel_RunAllAgents__SWIG_0(swigCPtr, numberSteps, stepSize.swigValue());
  }

  public String RunAllAgents(long numberSteps) {
    return smlJNI.Kernel_RunAllAgents__SWIG_1(swigCPtr, numberSteps);
  }

  public String RunAllAgentsForever() {
    return smlJNI.Kernel_RunAllAgentsForever(swigCPtr);
  }

  public String RunAllTilOutput(long maxDecisions) {
    return smlJNI.Kernel_RunAllTilOutput__SWIG_0(swigCPtr, maxDecisions);
  }

  public String RunAllTilOutput() {
    return smlJNI.Kernel_RunAllTilOutput__SWIG_1(swigCPtr);
  }

  public String StopAllAgents() {
    return smlJNI.Kernel_StopAllAgents(swigCPtr);
  }

  public boolean FireStartSimulationEvent() {
    return smlJNI.Kernel_FireStartSimulationEvent(swigCPtr);
  }

  public boolean FireStopSimulationEvent() {
    return smlJNI.Kernel_FireStopSimulationEvent(swigCPtr);
  }

  public String ExpandCommandLine(String pCommandLine) {
    return smlJNI.Kernel_ExpandCommandLine(swigCPtr, pCommandLine);
  }

  public boolean GetLastCommandLineResult() {
    return smlJNI.Kernel_GetLastCommandLineResult(swigCPtr);
  }

  public boolean IsRunCommand(String pCommandLine) {
    return smlJNI.Kernel_IsRunCommand(swigCPtr, pCommandLine);
  }

  public boolean CheckForIncomingCommands() {
    return smlJNI.Kernel_CheckForIncomingCommands(swigCPtr);
  }

  public boolean StartEventThread() {
    return smlJNI.Kernel_StartEventThread(swigCPtr);
  }

  public boolean StopEventThread() {
    return smlJNI.Kernel_StopEventThread(swigCPtr);
  }

  public void Sleep(int milliseconds) {
    smlJNI.Kernel_Sleep(swigCPtr, milliseconds);
  }

  public boolean SetInterruptCheckRate(int newRate) {
    return smlJNI.Kernel_SetInterruptCheckRate(swigCPtr, newRate);
  }

  public String GetLibraryLocation() {
    return smlJNI.Kernel_GetLibraryLocation(swigCPtr);
  }

  public final static int kDefaultSMLPort = 12121;

}
