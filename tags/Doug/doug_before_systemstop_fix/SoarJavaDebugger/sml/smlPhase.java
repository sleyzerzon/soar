/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public final class smlPhase {
  public final static smlPhase sml_INPUT_PHASE = new smlPhase("sml_INPUT_PHASE");
  public final static smlPhase sml_PROPOSAL_PHASE = new smlPhase("sml_PROPOSAL_PHASE");
  public final static smlPhase sml_SELECTION_PHASE = new smlPhase("sml_SELECTION_PHASE");
  public final static smlPhase sml_APPLY_PHASE = new smlPhase("sml_APPLY_PHASE");
  public final static smlPhase sml_OUTPUT_PHASE = new smlPhase("sml_OUTPUT_PHASE");
  public final static smlPhase sml_DECISION_PHASE = new smlPhase("sml_DECISION_PHASE");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static smlPhase swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + smlPhase.class + " with value " + swigValue);
  }

  private smlPhase(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private smlPhase(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private static smlPhase[] swigValues = { sml_INPUT_PHASE, sml_PROPOSAL_PHASE, sml_SELECTION_PHASE, sml_APPLY_PHASE, sml_OUTPUT_PHASE, sml_DECISION_PHASE };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

