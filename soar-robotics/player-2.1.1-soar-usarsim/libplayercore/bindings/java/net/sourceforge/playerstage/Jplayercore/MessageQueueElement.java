/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.33
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package net.sourceforge.playerstage.Jplayercore;

public class MessageQueueElement {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected MessageQueueElement(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(MessageQueueElement obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      playercore_javaJNI.delete_MessageQueueElement(swigCPtr);
    }
    swigCPtr = 0;
  }

  public MessageQueueElement() {
    this(playercore_javaJNI.new_MessageQueueElement(), true);
  }

  public void setMsg(Message value) {
    playercore_javaJNI.MessageQueueElement_msg_set(swigCPtr, this, Message.getCPtr(value), value);
  }

  public Message getMsg() {
    long cPtr = playercore_javaJNI.MessageQueueElement_msg_get(swigCPtr, this);
    return (cPtr == 0) ? null : new Message(cPtr, false);
  }

}
