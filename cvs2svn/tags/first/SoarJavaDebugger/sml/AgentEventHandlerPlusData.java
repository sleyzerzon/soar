/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class AgentEventHandlerPlusData extends EventHandlerPlusData {
  private long swigCPtr;

  protected AgentEventHandlerPlusData(long cPtr, boolean cMemoryOwn) {
    super(smlJNI.SWIGAgentEventHandlerPlusDataUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(AgentEventHandlerPlusData obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected AgentEventHandlerPlusData() {
    this(0, false);
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_AgentEventHandlerPlusData(swigCPtr);
    }
    swigCPtr = 0;
    super.delete();
  }

  public void setM_Handler(SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void m_Handler) {
    smlJNI.set_AgentEventHandlerPlusData_m_Handler(swigCPtr, SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void.getCPtr(m_Handler));
  }

  public SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void getM_Handler() {
    long cPtr = smlJNI.get_AgentEventHandlerPlusData_m_Handler(swigCPtr);
    return (cPtr == 0) ? null : new SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void(cPtr, false);
  }

  public AgentEventHandlerPlusData(SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void handler, SWIGTYPE_p_void userData, int callbackID) {
    this(smlJNI.new_AgentEventHandlerPlusData(SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void.getCPtr(handler), SWIGTYPE_p_void.getCPtr(userData), callbackID), true);
  }

}
