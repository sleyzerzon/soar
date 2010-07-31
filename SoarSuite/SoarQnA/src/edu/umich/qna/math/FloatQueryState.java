package edu.umich.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class FloatQueryState extends MathQueryState {
	Double convertedValue;
	
	FloatQueryState() {
		convertedValue = null;
	}

	@Override
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if ((queryParameters.size() == 1) && (queryParameters.containsKey("operand1"))) {
			List<Object> tempList;
			
			tempList = queryParameters.get("operand1");
			if ((tempList.size() == 1)) {
				Object tempObject = tempList.iterator().next();
				
				if (tempObject instanceof Number) {
					convertedValue = new Double(((Number) tempObject).doubleValue());
				} else if (tempObject instanceof String) {
					try {
						convertedValue = new Double(Double.parseDouble(((String) tempObject)));
					} catch (NumberFormatException e) {
					}
				}
				
				if (convertedValue != null) {
					returnVal = true;
					hasComputed = false;
				}
			}
		}
		
		return returnVal;
	}

	@Override
	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			newList.add(convertedValue);
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}
}
