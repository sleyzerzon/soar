package eaters;

import java.util.ArrayList;

public class Food {
	public static final String kTagFood = "food";
	public static final String kTagFoodType = "food-type";
	public static final String kParamFoodName = "food-name";
	public static final String kParamFoodShape = "food-shape";
	public static final String kParamFoodColor = "food-color";
	public static final String kParamFoodValue = "food-value";

	public static final int kCircleFood = 0;
	public static final int kSquareFood = 1;

	public static class FoodType {
		protected String m_Name;
		protected int m_Shape;
		protected int m_Color;
		protected int m_Value;
		
		public FoodType(String name, int shape, int color, int value) {
			m_Name = name;
			m_Shape = shape;
			m_Color = color;
			m_Value = value;
		}
	}
	
	protected static ArrayList m_FoodTypes = new ArrayList();
	
	static {
		FoodType normalFood = new FoodType("normal-food", 0, 0, 5);
		FoodType bonusFood = new FoodType("bonus-food", 1, 1, 10);
		m_FoodTypes.add(normalFood);
		m_FoodTypes.add(bonusFood);
	}
	
	public static void loadSettingsFromXML(JavaElementXML element) throws Exception {
		m_FoodTypes = new ArrayList();
		
		for (int i = 0; i < element.getNumberChildren(); ++i) {
			JavaElementXML child = element.getChild(i);
			
			if (child.getTagName().equalsIgnoreCase(kTagFoodType)) {
				FoodType type = new FoodType(
						child.getAttributeThrows(kParamFoodName), 
						child.getAttributeIntThrows(kParamFoodShape), 
						child.getAttributeIntThrows(kParamFoodColor), 
						child.getAttributeIntThrows(kParamFoodValue));
				m_FoodTypes.add(type);
			} else {
				throw new Exception("Unknown food tag.");
			}
		}
		
		if (m_FoodTypes.size() == 0) {
			throw new Exception("No food types found.");
		}
	}
	
	public static JavaElementXML saveSettingsToXML() {
		JavaElementXML food = new JavaElementXML(kTagFood);
		
		FoodType[] types = (FoodType[]) m_FoodTypes.toArray(new FoodType[0]);
		for (int i = 0; i < types.length; ++i) {

			JavaElementXML type = new JavaElementXML(kTagFoodType);
			type.addAttribute(kParamFoodName, types[i].m_Name);
			type.addAttribute(kParamFoodShape, new Integer(types[i].m_Shape).toString());
			type.addAttribute(kParamFoodColor, new Integer(types[i].m_Color).toString());
			type.addAttribute(kParamFoodValue, new Integer(types[i].m_Value).toString());
			food.addChildElement(type);
		}
		return food;
	}
}
