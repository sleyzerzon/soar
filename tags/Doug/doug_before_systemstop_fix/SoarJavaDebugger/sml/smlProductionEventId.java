/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public final class smlProductionEventId {
  public final static smlProductionEventId smlEVENT_AFTER_PRODUCTION_ADDED = new smlProductionEventId("smlEVENT_AFTER_PRODUCTION_ADDED", smlRunEventId.smlEVENT_AFTER_RUNNING.swigValue() + 1);
  public final static smlProductionEventId smlEVENT_BEFORE_PRODUCTION_REMOVED = new smlProductionEventId("smlEVENT_BEFORE_PRODUCTION_REMOVED");
  public final static smlProductionEventId smlEVENT_AFTER_PRODUCTION_FIRED = new smlProductionEventId("smlEVENT_AFTER_PRODUCTION_FIRED");
  public final static smlProductionEventId smlEVENT_BEFORE_PRODUCTION_RETRACTED = new smlProductionEventId("smlEVENT_BEFORE_PRODUCTION_RETRACTED");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static smlProductionEventId swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + smlProductionEventId.class + " with value " + swigValue);
  }

  private smlProductionEventId(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private smlProductionEventId(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private static smlProductionEventId[] swigValues = { smlEVENT_AFTER_PRODUCTION_ADDED, smlEVENT_BEFORE_PRODUCTION_REMOVED, smlEVENT_AFTER_PRODUCTION_FIRED, smlEVENT_BEFORE_PRODUCTION_RETRACTED };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

