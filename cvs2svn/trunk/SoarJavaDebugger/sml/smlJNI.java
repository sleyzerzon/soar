/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

class smlJNI {

  static {
    try {
        System.loadLibrary("Java_sml_ClientInterface");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
  
  public final static native int Agent_RegisterForRunEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForProductionEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Agent_RegisterForPrintEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForSystemEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_RegisterForAgentEvent(long jarg1, int jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);
  public final static native int Kernel_AddRhsFunction(long jarg1, String jarg2, Object jarg3, Object jarg4, String jarg5, Object jarg6);

  public final static native boolean Agent_UnregisterForRunEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForProductionEvent(long jarg1, int jarg2);
  public final static native boolean Agent_UnregisterForPrintEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForSystemEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_UnregisterForAgentEvent(long jarg1, int jarg2);
  public final static native boolean Kernel_RemoveRhsFunction(long jarg1, int jarg2);

  public final static native String get_sml_Names_kTagSML();
  public final static native String get_sml_Names_kID();
  public final static native String get_sml_Names_kAck();
  public final static native String get_sml_Names_kDocType();
  public final static native String get_sml_Names_kDocType_Call();
  public final static native String get_sml_Names_kDocType_Response();
  public final static native String get_sml_Names_kDocType_Notify();
  public final static native String get_sml_Names_kSoarVersion();
  public final static native String get_sml_Names_kSMLVersion();
  public final static native String get_sml_Names_kSMLVersionValue();
  public final static native String get_sml_Names_kSoarVersionValue();
  public final static native String get_sml_Names_kOutputLinkName();
  public final static native String get_sml_Names_kTagCommand();
  public final static native String get_sml_Names_kCommandName();
  public final static native String get_sml_Names_kCommandOutput();
  public final static native String get_sml_Names_kRawOutput();
  public final static native String get_sml_Names_kStructuredOutput();
  public final static native String get_sml_Names_kTagArg();
  public final static native String get_sml_Names_kArgParam();
  public final static native String get_sml_Names_kArgType();
  public final static native String get_sml_Names_kTagError();
  public final static native String get_sml_Names_kErrorCode();
  public final static native String get_sml_Names_kTagName();
  public final static native String get_sml_Names_kTagResult();
  public final static native String get_sml_Names_kValueDelta();
  public final static native String get_sml_Names_kValueFull();
  public final static native String get_sml_Names_kTagWME();
  public final static native String get_sml_Names_kWME_TimeTag();
  public final static native String get_sml_Names_kWME_Id();
  public final static native String get_sml_Names_kWME_Attribute();
  public final static native String get_sml_Names_kWME_Value();
  public final static native String get_sml_Names_kWME_ValueType();
  public final static native String get_sml_Names_kWME_Action();
  public final static native String get_sml_Names_kValueAdd();
  public final static native String get_sml_Names_kValueRemove();
  public final static native String get_sml_Names_kTypeString();
  public final static native String get_sml_Names_kTypeInt();
  public final static native String get_sml_Names_kTypeDouble();
  public final static native String get_sml_Names_kTypeChar();
  public final static native String get_sml_Names_kTypeBoolean();
  public final static native String get_sml_Names_kTypeID();
  public final static native String get_sml_Names_kTypeVariable();
  public final static native String get_sml_Names_kParamAgent();
  public final static native String get_sml_Names_kParamKernel();
  public final static native String get_sml_Names_kParamThis();
  public final static native String get_sml_Names_kParamName();
  public final static native String get_sml_Names_kParamFilename();
  public final static native String get_sml_Names_kParamLearning();
  public final static native String get_sml_Names_kParamOSupportMode();
  public final static native String get_sml_Names_kParamValue();
  public final static native String get_sml_Names_kParamWme();
  public final static native String get_sml_Names_kParamWmeObject();
  public final static native String get_sml_Names_kParamAttribute();
  public final static native String get_sml_Names_kParamCount();
  public final static native String get_sml_Names_kParamLength();
  public final static native String get_sml_Names_kParamThread();
  public final static native String get_sml_Names_kParamProcess();
  public final static native String get_sml_Names_kParamLine();
  public final static native String get_sml_Names_kParamLocation();
  public final static native String get_sml_Names_kParamLogLocation();
  public final static native String get_sml_Names_kParamLogLevel();
  public final static native String get_sml_Names_kParamInputProducer();
  public final static native String get_sml_Names_kParamOutputProcessor();
  public final static native String get_sml_Names_kParamWorkingMemory();
  public final static native String get_sml_Names_kParamAttributePath();
  public final static native String get_sml_Names_kParamUpdate();
  public final static native String get_sml_Names_kParamEventID();
  public final static native String get_sml_Names_kParamLearnSetting();
  public final static native String get_sml_Names_kParamLearnOnlySetting();
  public final static native String get_sml_Names_kParamLearnExceptSetting();
  public final static native String get_sml_Names_kParamLearnAllLevelsSetting();
  public final static native String get_sml_Names_kParamLearnForceLearnStates();
  public final static native String get_sml_Names_kParamLearnDontLearnStates();
  public final static native String get_sml_Names_kParamLogSetting();
  public final static native String get_sml_Names_kParamDirectory();
  public final static native String get_sml_Names_kParamSeconds();
  public final static native String get_sml_Names_kParamWarningsSetting();
  public final static native String get_sml_Names_kParamPhase();
  public final static native String get_sml_Names_kParamInstance();
  public final static native String get_sml_Names_kParamTimers();
  public final static native String get_sml_Names_kParamMessage();
  public final static native String get_sml_Names_kParamAlias();
  public final static native String get_sml_Names_kParamAliasedCommand();
  public final static native String get_sml_Names_kParamIndifferentSelectionMode();
  public final static native String get_sml_Names_kParamNumericIndifferentMode();
  public final static native String get_sml_Names_kParamRunResult();
  public final static native String get_sml_Names_kParamVersionMajor();
  public final static native String get_sml_Names_kParamVersionMinor();
  public final static native String get_sml_Names_kParamVersionMicro();
  public final static native String get_sml_Names_kParamWaitSNC();
  public final static native String get_sml_Names_kParamFunction();
  public final static native String get_sml_Names_kParamChunkNamePrefix();
  public final static native String get_sml_Names_kParamChunkCount();
  public final static native String get_sml_Names_kParamChunkLongFormat();
  public final static native String get_sml_Names_kTrue();
  public final static native String get_sml_Names_kFalse();
  public final static native String get_sml_Names_kCommand_CreateAgent();
  public final static native String get_sml_Names_kCommand_DestroyAgent();
  public final static native String get_sml_Names_kCommand_GetAgentList();
  public final static native String get_sml_Names_kCommand_LoadProductions();
  public final static native String get_sml_Names_kCommand_GetInputLink();
  public final static native String get_sml_Names_kCommand_GetOutputLink();
  public final static native String get_sml_Names_kCommand_Run();
  public final static native String get_sml_Names_kCommand_Input();
  public final static native String get_sml_Names_kCommand_Output();
  public final static native String get_sml_Names_kCommand_CheckForIncomingCommands();
  public final static native String get_sml_Names_kCommand_StopOnOutput();
  public final static native String get_sml_Names_kCommand_RegisterForEvent();
  public final static native String get_sml_Names_kCommand_UnregisterForEvent();
  public final static native String get_sml_Names_kCommand_Event();
  public final static native String get_sml_Names_kCommand_CommandLine();
  public final static native String get_sml_Names_kCommand_ExpandCommandLine();
  public final static native long new_sml_Names();
  public final static native void delete_sml_Names(long jarg1);
  public final static native long new_ElementXML__SWIG_0();
  public final static native long new_ElementXML__SWIG_1(long jarg1);
  public final static native void delete_ElementXML(long jarg1);
  public final static native int ElementXML_ReleaseRefOnHandle(long jarg1);
  public final static native int ElementXML_AddRefOnHandle(long jarg1);
  public final static native int ElementXML_GetRefCount(long jarg1);
  public final static native void ElementXML_Attach(long jarg1, long jarg2);
  public final static native long ElementXML_Detach(long jarg1);
  public final static native long ElementXML_GetXMLHandle(long jarg1);
  public final static native boolean ElementXML_SetTagName(long jarg1, String jarg2, boolean jarg3);
  public final static native boolean ElementXML_SetTagNameConst(long jarg1, String jarg2);
  public final static native String ElementXML_GetTagName(long jarg1);
  public final static native boolean ElementXML_IsTag(long jarg1, String jarg2);
  public final static native void ElementXML_AddChild(long jarg1, long jarg2);
  public final static native int ElementXML_GetNumberChildren(long jarg1);
  public final static native boolean ElementXML_GetChild(long jarg1, long jarg2, int jarg3);
  public final static native boolean ElementXML_AddAttribute(long jarg1, String jarg2, String jarg3, boolean jarg4, boolean jarg5);
  public final static native boolean ElementXML_AddAttributeConst(long jarg1, String jarg2, String jarg3);
  public final static native boolean ElementXML_AddAttributeConstConst(long jarg1, String jarg2, String jarg3);
  public final static native int ElementXML_GetNumberAttributes(long jarg1);
  public final static native String ElementXML_GetAttributeName(long jarg1, int jarg2);
  public final static native String ElementXML_GetAttributeValue(long jarg1, int jarg2);
  public final static native String ElementXML_GetAttribute(long jarg1, String jarg2);
  public final static native void ElementXML_SetCharacterData(long jarg1, String jarg2, boolean jarg3);
  public final static native void ElementXML_SetCharacterDataConst(long jarg1, String jarg2);
  public final static native void ElementXML_SetBinaryCharacterData(long jarg1, String jarg2, int jarg3, boolean jarg4);
  public final static native void ElementXML_SetBinaryCharacterDataConst(long jarg1, String jarg2, int jarg3);
  public final static native String ElementXML_GetCharacterData(long jarg1);
  public final static native boolean ElementXML_IsCharacterDataBinary(long jarg1);
  public final static native boolean ElementXML_ConvertCharacterDataToBinary(long jarg1);
  public final static native int ElementXML_GetCharacterDataLength(long jarg1);
  public final static native void ElementXML_SetUseCData(long jarg1, boolean jarg2);
  public final static native boolean ElementXML_GetUseCData(long jarg1);
  public final static native String ElementXML_GenerateXMLString(long jarg1, boolean jarg2);
  public final static native int ElementXML_DetermineXMLStringLength(long jarg1, boolean jarg2);
  public final static native String ElementXML_AllocateString(int jarg1);
  public final static native void ElementXML_DeleteString(String jarg1);
  public final static native String ElementXML_CopyString(String jarg1);
  public final static native String ElementXML_CopyBuffer(String jarg1, int jarg2);
  public final static native long ElementXML_ParseXMLFromString(String jarg1);
  public final static native long ElementXML_ParseXMLFromFile(String jarg1);
  public final static native String ElementXML_GetLastParseErrorDescription();
  public final static native long new_AnalyzeXML();
  public final static native void delete_AnalyzeXML(long jarg1);
  public final static native void AnalyzeXML_Analyze(long jarg1, long jarg2);
  public final static native long AnalyzeXML_GetCommandTag(long jarg1);
  public final static native long AnalyzeXML_GetResultTag(long jarg1);
  public final static native long AnalyzeXML_GetErrorTag(long jarg1);
  public final static native boolean AnalyzeXML_IsSML(long jarg1);
  public final static native String AnalyzeXML_GetCommandName(long jarg1);
  public final static native String AnalyzeXML_GetResultString(long jarg1);
  public final static native int AnalyzeXML_GetResultInt(long jarg1, int jarg2);
  public final static native boolean AnalyzeXML_GetResultBool(long jarg1, boolean jarg2);
  public final static native String AnalyzeXML_GetArgValue(long jarg1, String jarg2);
  public final static native boolean AnalyzeXML_GetArgBool(long jarg1, String jarg2, boolean jarg3);
  public final static native int AnalyzeXML_GetArgInt(long jarg1, String jarg2, int jarg3);
  public final static native boolean ClientErrors_HadError(long jarg1);
  public final static native String ClientErrors_GetLastErrorDescription(long jarg1);
  public final static native void delete_ClientErrors(long jarg1);
  public final static native long new_ClientErrors();
  public final static native boolean IsSystemEventID(int jarg1);
  public final static native boolean IsRunEventID(int jarg1);
  public final static native boolean IsProductionEventID(int jarg1);
  public final static native boolean IsAgentEventID(int jarg1);
  public final static native boolean IsWorkingMemoryEventID(int jarg1);
  public final static native boolean IsPrintEventID(int jarg1);
  public final static native boolean IsRhsEventID(int jarg1);
  public final static native void set_EventHandlerPlusData_m_UserData(long jarg1, long jarg2);
  public final static native long get_EventHandlerPlusData_m_UserData(long jarg1);
  public final static native void set_EventHandlerPlusData_m_CallbackID(long jarg1, int jarg2);
  public final static native int get_EventHandlerPlusData_m_CallbackID(long jarg1);
  public final static native long new_EventHandlerPlusData(long jarg1, int jarg2);
  public final static native int EventHandlerPlusData_getCallbackID(long jarg1);
  public final static native long EventHandlerPlusData_getUserData(long jarg1);
  public final static native void delete_EventHandlerPlusData(long jarg1);
  public final static native boolean WMElement_IsJustAdded(long jarg1);
  public final static native long WMElement_GetParent(long jarg1);
  public final static native long WMElement_GetIdentifier(long jarg1);
  public final static native String WMElement_GetIdentifierName(long jarg1);
  public final static native String WMElement_GetAttribute(long jarg1);
  public final static native String WMElement_GetValueType(long jarg1);
  public final static native String WMElement_GetValueAsString(long jarg1);
  public final static native int WMElement_GetTimeTag(long jarg1);
  public final static native boolean WMElement_IsIdentifier(long jarg1);
  public final static native long WMElement_ConvertToIdentifier(long jarg1);
  public final static native long WMElement_ConvertToIntElement(long jarg1);
  public final static native long WMElement_ConvertToFloatElement(long jarg1);
  public final static native long WMElement_ConvertToStringElement(long jarg1);
  public final static native String IntElement_GetValueType(long jarg1);
  public final static native String IntElement_GetValueAsString(long jarg1);
  public final static native int IntElement_GetValue(long jarg1);
  public final static native long IntElement_ConvertToIntElement(long jarg1);
  public final static native String FloatElement_GetValueType(long jarg1);
  public final static native String FloatElement_GetValueAsString(long jarg1);
  public final static native double FloatElement_GetValue(long jarg1);
  public final static native long FloatElement_ConvertToFloatElement(long jarg1);
  public final static native String StringElement_GetValueType(long jarg1);
  public final static native String StringElement_GetValueAsString(long jarg1);
  public final static native String StringElement_GetValue(long jarg1);
  public final static native long StringElement_ConvertToStringElement(long jarg1);
  public final static native long new_IdentifierSymbol(long jarg1);
  public final static native void delete_IdentifierSymbol(long jarg1);
  public final static native String IdentifierSymbol_GetIdentifierSymbol(long jarg1);
  public final static native void IdentifierSymbol_SetIdentifierSymbol(long jarg1, String jarg2);
  public final static native boolean IdentifierSymbol_AreChildrenModified(long jarg1);
  public final static native void IdentifierSymbol_SetAreChildrenModified(long jarg1, boolean jarg2);
  public final static native void IdentifierSymbol_NoLongerUsedBy(long jarg1, long jarg2);
  public final static native int IdentifierSymbol_GetNumberUsing(long jarg1);
  public final static native void IdentifierSymbol_AddChild(long jarg1, long jarg2);
  public final static native void IdentifierSymbol_RemoveChild(long jarg1, long jarg2);
  public final static native String Identifier_GetValueType(long jarg1);
  public final static native String Identifier_GetValueAsString(long jarg1);
  public final static native boolean Identifier_IsIdentifier(long jarg1);
  public final static native long Identifier_ConvertToIdentifier(long jarg1);
  public final static native long Identifier_FindIdentifier(long jarg1, String jarg2, int jarg3);
  public final static native long Identifier_FindTimeTag(long jarg1, int jarg2);
  public final static native long Identifier_FindByAttribute(long jarg1, String jarg2, int jarg3);
  public final static native String Identifier_GetParameterValue(long jarg1, String jarg2);
  public final static native String Identifier_GetCommandName(long jarg1);
  public final static native void Identifier_AddStatusComplete(long jarg1);
  public final static native void Identifier_AddStatusError(long jarg1);
  public final static native void Identifier_AddErrorCode(long jarg1, int jarg2);
  public final static native int Identifier_GetNumberChildren(long jarg1);
  public final static native long Identifier_GetChild(long jarg1, int jarg2);
  public final static native boolean Identifier_AreChildrenModified(long jarg1);
  public final static native void set_SystemEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_SystemEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_SystemEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_SystemEventHandlerPlusData(long jarg1);
  public final static native void set_AgentEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_AgentEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_AgentEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_AgentEventHandlerPlusData(long jarg1);
  public final static native void set_RhsEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_RhsEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_RhsEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_RhsEventHandlerPlusData(long jarg1);
  public final static native long Kernel_CreateKernelInCurrentThread(String jarg1, boolean jarg2, int jarg3);
  public final static native long Kernel_CreateKernelInNewThread(String jarg1, int jarg2);
  public final static native long Kernel_CreateRemoteConnection(boolean jarg1, String jarg2, int jarg3);
  public final static native int Kernel_GetDefaultPort();
  public final static native void Kernel_SetTraceCommunications(long jarg1, boolean jarg2);
  public final static native void delete_Kernel(long jarg1);
  public final static native long Kernel_CreateAgent(long jarg1, String jarg2);
  public final static native void Kernel_UpdateAgentList(long jarg1);
  public final static native int Kernel_GetNumberAgents(long jarg1);
  public final static native boolean Kernel_DestroyAgent(long jarg1, long jarg2);
  public final static native long Kernel_GetAgent(long jarg1, String jarg2);
  public final static native long Kernel_GetAgentByIndex(long jarg1, int jarg2);
  public final static native String Kernel_ExecuteCommandLine(long jarg1, String jarg2, String jarg3);
  public final static native boolean Kernel_ExecuteCommandLineXML(long jarg1, String jarg2, String jarg3, long jarg4);
  public final static native String Kernel_ExpandCommandLine(long jarg1, String jarg2);
  public final static native boolean Kernel_GetLastCommandLineResult(long jarg1);
  public final static native boolean Kernel_IsRunCommand(long jarg1, String jarg2);
  public final static native boolean Kernel_CheckForIncomingCommands(long jarg1);
  public final static native boolean Kernel_StartEventThread(long jarg1);
  public final static native boolean Kernel_StopEventThread(long jarg1);
  public final static native void Kernel_Sleep(long jarg1, int jarg2);
  public final static native void set_RunEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_RunEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_RunEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_RunEventHandlerPlusData(long jarg1);
  public final static native void set_ProductionEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_ProductionEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_ProductionEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_ProductionEventHandlerPlusData(long jarg1);
  public final static native void set_PrintEventHandlerPlusData_m_Handler(long jarg1, long jarg2);
  public final static native long get_PrintEventHandlerPlusData_m_Handler(long jarg1);
  public final static native long new_PrintEventHandlerPlusData(long jarg1, long jarg2, int jarg3);
  public final static native void delete_PrintEventHandlerPlusData(long jarg1);
  public final static native String Agent_GetAgentName(long jarg1);
  public final static native long Agent_GetKernel(long jarg1);
  public final static native boolean Agent_LoadProductions(long jarg1, String jarg2);
  public final static native long Agent_GetInputLink(long jarg1);
  public final static native long Agent_GetILink(long jarg1);
  public final static native long Agent_GetOutputLink(long jarg1);
  public final static native long Agent_FindIdentifier(long jarg1, String jarg2, boolean jarg3, boolean jarg4, int jarg5);
  public final static native long Agent_CreateStringWME(long jarg1, long jarg2, String jarg3, String jarg4);
  public final static native long Agent_CreateIntWME(long jarg1, long jarg2, String jarg3, int jarg4);
  public final static native long Agent_CreateFloatWME(long jarg1, long jarg2, String jarg3, double jarg4);
  public final static native long Agent_CreateIdWME(long jarg1, long jarg2, String jarg3);
  public final static native long Agent_CreateSharedIdWME(long jarg1, long jarg2, String jarg3, long jarg4);
  public final static native void Agent_Update__SWIG_0(long jarg1, long jarg2, String jarg3);
  public final static native void Agent_Update__SWIG_1(long jarg1, long jarg2, int jarg3);
  public final static native void Agent_Update__SWIG_2(long jarg1, long jarg2, double jarg3);
  public final static native boolean Agent_DestroyWME(long jarg1, long jarg2);
  public final static native String Agent_InitSoar(long jarg1);
  public final static native String Agent_Stop(long jarg1, boolean jarg2);
  public final static native int Agent_GetNumberOutputLinkChanges(long jarg1);
  public final static native long Agent_GetOutputLinkChange(long jarg1, int jarg2);
  public final static native boolean Agent_IsOutputLinkChangeAdd(long jarg1, int jarg2);
  public final static native void Agent_ClearOutputLinkChanges(long jarg1);
  public final static native int Agent_GetNumberCommands(long jarg1);
  public final static native boolean Agent_Commands(long jarg1);
  public final static native long Agent_GetCommand(long jarg1, int jarg2);
  public final static native boolean Agent_Commit(long jarg1);
  public final static native String Agent_Run(long jarg1, long jarg2);
  public final static native boolean Agent_SetStopOnOutput(long jarg1, boolean jarg2);
  public final static native String Agent_RunTilOutput(long jarg1, long jarg2);
  public final static native void Agent_Refresh(long jarg1);
  public final static native String Agent_ExecuteCommandLine(long jarg1, String jarg2);
  public final static native boolean Agent_ExecuteCommandLineXML(long jarg1, String jarg2, long jarg3);
  public final static native boolean Agent_GetLastCommandLineResult(long jarg1);
  public final static native long SWIGIntElementUpcast(long jarg1);
  public final static native long SWIGFloatElementUpcast(long jarg1);
  public final static native long SWIGStringElementUpcast(long jarg1);
  public final static native long SWIGIdentifierUpcast(long jarg1);
  public final static native long SWIGSystemEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGAgentEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGRhsEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGKernelUpcast(long jarg1);
  public final static native long SWIGRunEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGProductionEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGPrintEventHandlerPlusDataUpcast(long jarg1);
  public final static native long SWIGAgentUpcast(long jarg1);
}
