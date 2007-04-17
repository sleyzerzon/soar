/**
 * 
 */
package soar2d.tosca2d;

import tosca.Group;
import tosca.Value;

/**
 * @author Doug
 *
 */
public class EatersOutputStateVariable extends JavaStateVariable {
	protected Value m_Value = new Value() ;

	public EatersOutputStateVariable(String name) {
		super(name) ;
		this.SetLogChanges(true);
	}
	
	public void Initialize() {
		GetCurrentValue().SetFromInteger(2) ;
	}

	/** 
	 * Note: It's ONLY valid to call this within the tickEvent() handler -- which is called by InputFunctionModule while the clock is holding waiting for the function module to respond
	 * Returns 1-4 to make a move.  0 to not go anywhere.
	 * NAG: 5 opens a box with no argument (code) 
	 **/
	public int GetDirection()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		tosca.Integer direction = value.CastToInteger() ;
		int action = direction.GetInteger();
		
		if (action < 5)
			return action;
		else
			return 0;
	}
	
	public Boolean IsOpening()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		if (value.CastToInteger().GetInteger() >= 5) {
			return true;
		} else {
			return false;
		}
	}
	
	public int GetOpenCode()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		tosca.Integer direction = value.CastToInteger() ;
		int action = direction.GetInteger();
		
		if (action == 5)
			return 0;
		else if (action == 6)
			return 1;
		else if (action == 7)
			return 2;
		else
			return -1;
	}
	
	protected Value GetCurrentValue() { return m_Value ; }

}
