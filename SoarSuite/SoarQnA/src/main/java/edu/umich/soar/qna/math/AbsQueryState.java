package edu.umich.soar.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class AbsQueryState extends UnaryMathQueryState {

	@Override
	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			if ((operand1 instanceof Long)) {
				newList.add(new Long(Math.abs(((Long) operand1).longValue())));
			} else {
				newList.add(new Double(Math.abs(((Number) operand1).doubleValue())));
			}
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}

}
