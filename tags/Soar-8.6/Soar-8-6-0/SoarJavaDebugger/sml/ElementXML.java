/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class ElementXML {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected ElementXML(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(ElementXML obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_ElementXML(swigCPtr);
    }
    swigCPtr = 0;
  }

  public ElementXML() {
    this(smlJNI.new_ElementXML__SWIG_0(), true);
  }

  public ElementXML(SWIGTYPE_p_ElementXML_InterfaceStructTag hXML) {
    this(smlJNI.new_ElementXML__SWIG_1(SWIGTYPE_p_ElementXML_InterfaceStructTag.getCPtr(hXML)), true);
  }

  public int ReleaseRefOnHandle() {
    return smlJNI.ElementXML_ReleaseRefOnHandle(swigCPtr);
  }

  public int AddRefOnHandle() {
    return smlJNI.ElementXML_AddRefOnHandle(swigCPtr);
  }

  public int GetRefCount() {
    return smlJNI.ElementXML_GetRefCount(swigCPtr);
  }

  public void Attach(SWIGTYPE_p_ElementXML_InterfaceStructTag hXML) {
    smlJNI.ElementXML_Attach(swigCPtr, SWIGTYPE_p_ElementXML_InterfaceStructTag.getCPtr(hXML));
  }

  public SWIGTYPE_p_ElementXML_InterfaceStructTag Detach() {
    long cPtr = smlJNI.ElementXML_Detach(swigCPtr);
    return (cPtr == 0) ? null : new SWIGTYPE_p_ElementXML_InterfaceStructTag(cPtr, false);
  }

  public SWIGTYPE_p_ElementXML_InterfaceStructTag GetXMLHandle() {
    long cPtr = smlJNI.ElementXML_GetXMLHandle(swigCPtr);
    return (cPtr == 0) ? null : new SWIGTYPE_p_ElementXML_InterfaceStructTag(cPtr, false);
  }

  public boolean SetTagName(String tagName, boolean copyName) {
    return smlJNI.ElementXML_SetTagName__SWIG_0(swigCPtr, tagName, copyName);
  }

  public boolean SetTagName(String tagName) {
    return smlJNI.ElementXML_SetTagName__SWIG_1(swigCPtr, tagName);
  }

  public boolean SetTagNameConst(String tagName) {
    return smlJNI.ElementXML_SetTagNameConst(swigCPtr, tagName);
  }

  public String GetTagName() {
    return smlJNI.ElementXML_GetTagName(swigCPtr);
  }

  public boolean IsTag(String pTagName) {
    return smlJNI.ElementXML_IsTag(swigCPtr, pTagName);
  }

  public void AddChild(ElementXML pChild) {
    smlJNI.ElementXML_AddChild(swigCPtr, ElementXML.getCPtr(pChild));
  }

  public int GetNumberChildren() {
    return smlJNI.ElementXML_GetNumberChildren(swigCPtr);
  }

  public boolean GetChild(ElementXML pChild, int index) {
    return smlJNI.ElementXML_GetChild(swigCPtr, ElementXML.getCPtr(pChild), index);
  }

  public boolean AddAttribute(String attributeName, String attributeValue, boolean copyName, boolean copyValue) {
    return smlJNI.ElementXML_AddAttribute__SWIG_0(swigCPtr, attributeName, attributeValue, copyName, copyValue);
  }

  public boolean AddAttribute(String attributeName, String attributeValue, boolean copyName) {
    return smlJNI.ElementXML_AddAttribute__SWIG_1(swigCPtr, attributeName, attributeValue, copyName);
  }

  public boolean AddAttribute(String attributeName, String attributeValue) {
    return smlJNI.ElementXML_AddAttribute__SWIG_2(swigCPtr, attributeName, attributeValue);
  }

  public boolean AddAttributeConst(String attributeName, String attributeValue) {
    return smlJNI.ElementXML_AddAttributeConst(swigCPtr, attributeName, attributeValue);
  }

  public boolean AddAttributeConstConst(String attributeName, String attributeValue) {
    return smlJNI.ElementXML_AddAttributeConstConst(swigCPtr, attributeName, attributeValue);
  }

  public int GetNumberAttributes() {
    return smlJNI.ElementXML_GetNumberAttributes(swigCPtr);
  }

  public String GetAttributeName(int index) {
    return smlJNI.ElementXML_GetAttributeName(swigCPtr, index);
  }

  public String GetAttributeValue(int index) {
    return smlJNI.ElementXML_GetAttributeValue(swigCPtr, index);
  }

  public String GetAttribute(String attName) {
    return smlJNI.ElementXML_GetAttribute(swigCPtr, attName);
  }

  public void SetCharacterData(String characterData, boolean copyData) {
    smlJNI.ElementXML_SetCharacterData__SWIG_0(swigCPtr, characterData, copyData);
  }

  public void SetCharacterData(String characterData) {
    smlJNI.ElementXML_SetCharacterData__SWIG_1(swigCPtr, characterData);
  }

  public void SetCharacterDataConst(String characterData) {
    smlJNI.ElementXML_SetCharacterDataConst(swigCPtr, characterData);
  }

  public void SetBinaryCharacterData(String characterData, int length, boolean copyData) {
    smlJNI.ElementXML_SetBinaryCharacterData__SWIG_0(swigCPtr, characterData, length, copyData);
  }

  public void SetBinaryCharacterData(String characterData, int length) {
    smlJNI.ElementXML_SetBinaryCharacterData__SWIG_1(swigCPtr, characterData, length);
  }

  public void SetBinaryCharacterDataConst(String characterData, int length) {
    smlJNI.ElementXML_SetBinaryCharacterDataConst(swigCPtr, characterData, length);
  }

  public String GetCharacterData() {
    return smlJNI.ElementXML_GetCharacterData(swigCPtr);
  }

  public boolean IsCharacterDataBinary() {
    return smlJNI.ElementXML_IsCharacterDataBinary(swigCPtr);
  }

  public boolean ConvertCharacterDataToBinary() {
    return smlJNI.ElementXML_ConvertCharacterDataToBinary(swigCPtr);
  }

  public int GetCharacterDataLength() {
    return smlJNI.ElementXML_GetCharacterDataLength(swigCPtr);
  }

  public void SetUseCData(boolean useCData) {
    smlJNI.ElementXML_SetUseCData(swigCPtr, useCData);
  }

  public boolean GetUseCData() {
    return smlJNI.ElementXML_GetUseCData(swigCPtr);
  }

  public String GenerateXMLString(boolean includeChildren) {
    return smlJNI.ElementXML_GenerateXMLString(swigCPtr, includeChildren);
  }

  public int DetermineXMLStringLength(boolean includeChildren) {
    return smlJNI.ElementXML_DetermineXMLStringLength(swigCPtr, includeChildren);
  }

  public static String AllocateString(int length) {
    return smlJNI.ElementXML_AllocateString(length);
  }

  public static void DeleteString(String string) {
    smlJNI.ElementXML_DeleteString(string);
  }

  public static String CopyString(String original) {
    return smlJNI.ElementXML_CopyString(original);
  }

  public static String CopyBuffer(String original, int length) {
    return smlJNI.ElementXML_CopyBuffer(original, length);
  }

  public static ElementXML ParseXMLFromString(String pString) {
    long cPtr = smlJNI.ElementXML_ParseXMLFromString(pString);
    return (cPtr == 0) ? null : new ElementXML(cPtr, false);
  }

  public static ElementXML ParseXMLFromStringSequence(String pString, long startPos, SWIGTYPE_p_size_t endPos) {
    long cPtr = smlJNI.ElementXML_ParseXMLFromStringSequence(pString, startPos, SWIGTYPE_p_size_t.getCPtr(endPos));
    return (cPtr == 0) ? null : new ElementXML(cPtr, false);
  }

  public static ElementXML ParseXMLFromFile(String pFilename) {
    long cPtr = smlJNI.ElementXML_ParseXMLFromFile(pFilename);
    return (cPtr == 0) ? null : new ElementXML(cPtr, false);
  }

  public static String GetLastParseErrorDescription() {
    return smlJNI.ElementXML_GetLastParseErrorDescription();
  }

}