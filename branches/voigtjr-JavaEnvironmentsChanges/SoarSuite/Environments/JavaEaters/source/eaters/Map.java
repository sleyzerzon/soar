package eaters;

public class Map {
	public static final String kTagMap = "map";
	public static final String kParamMapWidth = "map-width";
	public static final String kParamMapHeight = "map-height";

	protected static int m_MapWidth = 17;
	protected static int m_MapHeight = 17;

	public static void loadSettingsFromXML(JavaElementXML element) throws Exception {
		m_MapWidth = element.getAttributeIntThrows(kParamMapWidth);
		m_MapHeight = element.getAttributeIntThrows(kParamMapHeight);		
	}
	
	public static JavaElementXML saveSettingsToXML() {
		JavaElementXML map = new JavaElementXML(kTagMap);
		map.addAttribute(kParamMapWidth, new Integer(m_MapWidth).toString());
		map.addAttribute(kParamMapHeight, new Integer(m_MapHeight).toString());
		return map;
	}
}
