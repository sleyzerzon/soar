/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class sml implements smlConstants {
  public static boolean IsSystemEventID(int id) {
    return smlJNI.IsSystemEventID(id);
  }

  public static boolean IsRunEventID(int id) {
    return smlJNI.IsRunEventID(id);
  }

  public static boolean IsProductionEventID(int id) {
    return smlJNI.IsProductionEventID(id);
  }

  public static boolean IsAgentEventID(int id) {
    return smlJNI.IsAgentEventID(id);
  }

  public static boolean IsWorkingMemoryEventID(int id) {
    return smlJNI.IsWorkingMemoryEventID(id);
  }

  public static boolean IsPrintEventID(int id) {
    return smlJNI.IsPrintEventID(id);
  }

  public static boolean IsRhsEventID(int id) {
    return smlJNI.IsRhsEventID(id);
  }

  public static boolean IsXMLEventID(int id) {
    return smlJNI.IsXMLEventID(id);
  }

}