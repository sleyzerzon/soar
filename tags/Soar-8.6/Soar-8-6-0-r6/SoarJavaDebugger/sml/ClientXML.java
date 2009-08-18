/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class ClientXML {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected ClientXML(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(ClientXML obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_ClientXML(swigCPtr);
    }
    swigCPtr = 0;
  }

  public ClientXML() {
    this(smlJNI.new_ClientXML__SWIG_0(), true);
  }

  public ClientXML(ClientXML pXML) {
    this(smlJNI.new_ClientXML__SWIG_1(ClientXML.getCPtr(pXML)), true);
  }

  public ClientTraceXML ConvertToTraceXML() {
    long cPtr = smlJNI.ClientXML_ConvertToTraceXML(swigCPtr);
    return (cPtr == 0) ? null : new ClientTraceXML(cPtr, false);
  }

  public String GetTagName() {
    return smlJNI.ClientXML_GetTagName(swigCPtr);
  }

  public boolean IsTag(String pTagName) {
    return smlJNI.ClientXML_IsTag(swigCPtr, pTagName);
  }

  public int GetNumberChildren() {
    return smlJNI.ClientXML_GetNumberChildren(swigCPtr);
  }

  public boolean GetChild(ClientXML pChild, int index) {
    return smlJNI.ClientXML_GetChild(swigCPtr, ClientXML.getCPtr(pChild), index);
  }

  public int GetNumberAttributes() {
    return smlJNI.ClientXML_GetNumberAttributes(swigCPtr);
  }

  public String GetAttributeName(int index) {
    return smlJNI.ClientXML_GetAttributeName(swigCPtr, index);
  }

  public String GetAttributeValue(int index) {
    return smlJNI.ClientXML_GetAttributeValue(swigCPtr, index);
  }

  public String GetAttribute(String attName) {
    return smlJNI.ClientXML_GetAttribute(swigCPtr, attName);
  }

  public String GetCharacterData() {
    return smlJNI.ClientXML_GetCharacterData(swigCPtr);
  }

  public boolean IsCharacterDataBinary() {
    return smlJNI.ClientXML_IsCharacterDataBinary(swigCPtr);
  }

  public boolean ConvertCharacterDataToBinary() {
    return smlJNI.ClientXML_ConvertCharacterDataToBinary(swigCPtr);
  }

  public int GetCharacterDataLength() {
    return smlJNI.ClientXML_GetCharacterDataLength(swigCPtr);
  }

  public String GenerateXMLString(boolean includeChildren) {
    return smlJNI.ClientXML_GenerateXMLString(swigCPtr, includeChildren);
  }

  public int DetermineXMLStringLength(boolean includeChildren) {
    return smlJNI.ClientXML_DetermineXMLStringLength(swigCPtr, includeChildren);
  }

  public static String AllocateString(int length) {
    return smlJNI.ClientXML_AllocateString(length);
  }

  public static void DeleteString(String string) {
    smlJNI.ClientXML_DeleteString(string);
  }

  public static String CopyString(String original) {
    return smlJNI.ClientXML_CopyString(original);
  }

  public static String CopyBuffer(String original, int length) {
    return smlJNI.ClientXML_CopyBuffer(original, length);
  }

}