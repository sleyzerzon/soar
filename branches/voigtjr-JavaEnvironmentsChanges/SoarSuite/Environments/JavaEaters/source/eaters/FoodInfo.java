package eaters;

import org.eclipse.swt.graphics.Color;

import utilities.Logger;

public class FoodInfo {
	public static final String kRound = "round";
	public static final String kSquare = "square";
		
	protected String m_Name;
	protected int m_Value;
	protected String m_Shape;
	protected String m_ColorString;
	protected Color m_Color;
	
	protected Logger m_Logger = Logger.logger;

	public FoodInfo(String name, int value, String shape, String color) {
		m_Name = name;
		m_Value = value;
		m_Shape = shape;
		m_ColorString = color;
	}
	
	public String getName() {
		return m_Name;
	}
	
	public int getValue() {
		return m_Value;
	}
	
	public String getShape() {
		return m_Shape;
	}
	
	public Color getColor() {
		if (m_Color == null) {
			m_Color = EatersWindowManager.getColor(m_ColorString);
		}
		return m_Color;
	}
}
