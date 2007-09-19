package soar2d.configuration;

import java.io.File;
import java.util.List;

import org.jdom.Element;

public class KitchenConfiguration extends BaseConfiguration implements IConfiguration {

	KitchenConfiguration() {
		super();
		
		this.mapPath += "kitchen" + System.getProperty("file.separator");
		this.agentPath += "kitchen" + System.getProperty("file.separator");
		this.map = new File(this.mapPath + "default.kmap");
	}

	public String getMapExt() {
		return "xml";
	}

	public String getTitle() {
		return "Kitchen";
	}
	
	public void rules(Element rules) throws LoadError {
		List<Element> children = (List<Element>)rules.getChildren();
		if (children.size() < 1) {
			return;
		}
		if (children.size() > 1) {
			throw new LoadError("Only one rules set is allowed");
		}
		Element child = children.get(0);
		if (child.getName().equalsIgnoreCase(getTitle()) == false) {
			throw new LoadError("Expected " + getTitle() + " rules, got " + child.getName());
		}
	}

	public void rulesSave(Element rules) {
		assert false;
	}
	
	public void copy(IConfiguration config) {
		KitchenConfiguration kConfig = (KitchenConfiguration)config;
	}
	
	public void setDefaultTerminals(Configuration configuration) {
		configuration.setTerminalWinningScore(0);
		configuration.setTerminalFoodRemaining(false);
	}

	public boolean getRunTilOutput() {
		return false;
	}
	
	public BaseConfiguration getModule() {
		return this;
	}

	public int getCycleTimeSlice() {
		return 50;
	}

}
