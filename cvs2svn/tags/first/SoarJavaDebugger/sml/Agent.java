/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class Agent extends ClientErrors {
  private long swigCPtr;

  protected Agent(long cPtr, boolean cMemoryOwn) {
    super(smlJNI.SWIGAgentUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Agent obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected Agent() {
    this(0, false);
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      throw new UnsupportedOperationException("C++ destructor does not have public access");
    }
    swigCPtr = 0;
    super.delete();
  }

  public int RegisterForRunEvent(smlRunEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForRunEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ;}
  
  public int RegisterForProductionEvent(smlProductionEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForProductionEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }

  public int RegisterForPrintEvent(smlPrintEventId id, Object handlerObject, String handlerMethod, Object callbackData)
  { return smlJNI.Agent_RegisterForPrintEvent(swigCPtr, id.swigValue(), this, handlerObject, handlerMethod, callbackData) ; }
  
  public boolean UnregisterForRunEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForRunEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForProductionEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForProductionEvent(swigCPtr, callbackReturnValue) ;}

  public boolean UnregisterForPrintEvent(int callbackReturnValue)
  { return smlJNI.Agent_UnregisterForPrintEvent(swigCPtr, callbackReturnValue) ;}

  public String GetAgentName() {
    return smlJNI.Agent_GetAgentName(swigCPtr);
  }

  public Kernel GetKernel() {
    long cPtr = smlJNI.Agent_GetKernel(swigCPtr);
    return (cPtr == 0) ? null : new Kernel(cPtr, false);
  }

  public boolean LoadProductions(String pFilename) {
    return smlJNI.Agent_LoadProductions(swigCPtr, pFilename);
  }

  public Identifier GetInputLink() {
    long cPtr = smlJNI.Agent_GetInputLink(swigCPtr);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public Identifier GetILink() {
    long cPtr = smlJNI.Agent_GetILink(swigCPtr);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public Identifier GetOutputLink() {
    long cPtr = smlJNI.Agent_GetOutputLink(swigCPtr);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public Identifier FindIdentifier(String pID, boolean searchInput, boolean searchOutput, int index) {
    long cPtr = smlJNI.Agent_FindIdentifier(swigCPtr, pID, searchInput, searchOutput, index);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public StringElement CreateStringWME(Identifier parent, String pAttribute, String pValue) {
    long cPtr = smlJNI.Agent_CreateStringWME(swigCPtr, Identifier.getCPtr(parent), pAttribute, pValue);
    return (cPtr == 0) ? null : new StringElement(cPtr, false);
  }

  public IntElement CreateIntWME(Identifier parent, String pAttribute, int value) {
    long cPtr = smlJNI.Agent_CreateIntWME(swigCPtr, Identifier.getCPtr(parent), pAttribute, value);
    return (cPtr == 0) ? null : new IntElement(cPtr, false);
  }

  public FloatElement CreateFloatWME(Identifier parent, String pAttribute, double value) {
    long cPtr = smlJNI.Agent_CreateFloatWME(swigCPtr, Identifier.getCPtr(parent), pAttribute, value);
    return (cPtr == 0) ? null : new FloatElement(cPtr, false);
  }

  public Identifier CreateIdWME(Identifier parent, String pAttribute) {
    long cPtr = smlJNI.Agent_CreateIdWME(swigCPtr, Identifier.getCPtr(parent), pAttribute);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public Identifier CreateSharedIdWME(Identifier parent, String pAttribute, Identifier pSharedValue) {
    long cPtr = smlJNI.Agent_CreateSharedIdWME(swigCPtr, Identifier.getCPtr(parent), pAttribute, Identifier.getCPtr(pSharedValue));
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public void Update(StringElement pWME, String pValue) {
    smlJNI.Agent_Update__SWIG_0(swigCPtr, StringElement.getCPtr(pWME), pValue);
  }

  public void Update(IntElement pWME, int value) {
    smlJNI.Agent_Update__SWIG_1(swigCPtr, IntElement.getCPtr(pWME), value);
  }

  public void Update(FloatElement pWME, double value) {
    smlJNI.Agent_Update__SWIG_2(swigCPtr, FloatElement.getCPtr(pWME), value);
  }

  public boolean DestroyWME(WMElement pWME) {
    return smlJNI.Agent_DestroyWME(swigCPtr, WMElement.getCPtr(pWME));
  }

  public String InitSoar() {
    return smlJNI.Agent_InitSoar(swigCPtr);
  }

  public String Stop(boolean stopAllAgents) {
    return smlJNI.Agent_Stop(swigCPtr, stopAllAgents);
  }

  public int GetNumberOutputLinkChanges() {
    return smlJNI.Agent_GetNumberOutputLinkChanges(swigCPtr);
  }

  public WMElement GetOutputLinkChange(int index) {
    long cPtr = smlJNI.Agent_GetOutputLinkChange(swigCPtr, index);
    return (cPtr == 0) ? null : new WMElement(cPtr, false);
  }

  public boolean IsOutputLinkChangeAdd(int index) {
    return smlJNI.Agent_IsOutputLinkChangeAdd(swigCPtr, index);
  }

  public void ClearOutputLinkChanges() {
    smlJNI.Agent_ClearOutputLinkChanges(swigCPtr);
  }

  public int GetNumberCommands() {
    return smlJNI.Agent_GetNumberCommands(swigCPtr);
  }

  public boolean Commands() {
    return smlJNI.Agent_Commands(swigCPtr);
  }

  public Identifier GetCommand(int index) {
    long cPtr = smlJNI.Agent_GetCommand(swigCPtr, index);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public boolean Commit() {
    return smlJNI.Agent_Commit(swigCPtr);
  }

  public String Run(long decisions) {
    return smlJNI.Agent_Run(swigCPtr, decisions);
  }

  public boolean SetStopOnOutput(boolean state) {
    return smlJNI.Agent_SetStopOnOutput(swigCPtr, state);
  }

  public String RunTilOutput(long maxDecisions) {
    return smlJNI.Agent_RunTilOutput(swigCPtr, maxDecisions);
  }

  public void Refresh() {
    smlJNI.Agent_Refresh(swigCPtr);
  }

  public String ExecuteCommandLine(String pCommandLine) {
    return smlJNI.Agent_ExecuteCommandLine(swigCPtr, pCommandLine);
  }

  public boolean ExecuteCommandLineXML(String pCommandLine, AnalyzeXML pResponse) {
    return smlJNI.Agent_ExecuteCommandLineXML(swigCPtr, pCommandLine, AnalyzeXML.getCPtr(pResponse));
  }

  public boolean GetLastCommandLineResult() {
    return smlJNI.Agent_GetLastCommandLineResult(swigCPtr);
  }

}
