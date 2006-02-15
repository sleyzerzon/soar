package eaters;

import utilities.Logger;

public class FoodInfo {
	public static final int Round = 0;
	public static final int Square = 1;
		
	protected String m_Name;
	protected int m_Value;
	protected int m_Shape;
	
	protected Logger m_Logger = Logger.logger;

	public FoodInfo(String name, int value, int shape) {
		m_Name = name;
		m_Value = value;
		m_Shape = shape;
	}
	
	public String getName() {
		return m_Name;
	}
	
	public int getValue() {
		return m_Value;
	}
	
	public int getShape() {
		return m_Shape;
	}
}
	
//	protected static ArrayList m_FoodTypes = new ArrayList();
//	
//	static {
//		FoodType normalFood = new FoodType("normal-food", 0, 0, 5);
//		FoodType bonusFood = new FoodType("bonus-food", 1, 1, 10);
//		m_FoodTypes.add(normalFood);
//		m_FoodTypes.add(bonusFood);
//	}
//	
//	public static void loadSettingsFromXML(JavaElementXML element) throws Exception {
//		m_FoodTypes = new ArrayList();
//		
//		for (int i = 0; i < element.getNumberChildren(); ++i) {
//			JavaElementXML child = element.getChild(i);
//			
//			if (child.getTagName().equalsIgnoreCase(kTagFoodType)) {
//			} else {
//				throw new Exception("Unknown food tag.");
//			}
//		}
//		
//		if (m_FoodTypes.size() == 0) {
//			throw new Exception("No food types found.");
//		}
//	}
//	
//	public static JavaElementXML saveSettingsToXML() {
//		JavaElementXML food = new JavaElementXML(kTagFood);
//		
//		FoodType[] types = (FoodType[]) m_FoodTypes.toArray(new FoodType[0]);
//		for (int i = 0; i < types.length; ++i) {
//
//			JavaElementXML type = new JavaElementXML(kTagFoodType);
//			type.addAttribute(kParamFoodName, types[i].m_Name);
//			type.addAttribute(kParamFoodShape, new Integer(types[i].m_Shape).toString());
//			type.addAttribute(kParamFoodColor, new Integer(types[i].m_Color).toString());
//			type.addAttribute(kParamFoodValue, new Integer(types[i].m_Value).toString());
//			food.addChildElement(type);
//		}
//		return food;
//	}
//	
//	public FoodInfo(String type) {
//		
//	}
//}
