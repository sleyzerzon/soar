package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.Simulation;


/**
 * @author voigtjr
 *
 * This object keeps track of the "object templates" or prototypes.
 * These objects are copied to create new objects.
 */
class CellObjectManager {
	private static Logger logger = Logger.getLogger(CellObjectManager.class);

	/**
	 * The templates, mapped by name.
	 */
	private Map<String, CellObject> templates = new HashMap<String, CellObject>();
	
	/**
	 * @param name template object name
	 * @return true if it exists
	 */
	boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	
	/**
	 * expunges all templates
	 */
	void removeAllTemplates() {
		templates.clear();
	}
	
	/**
	 * @param name the template to remove
	 * @return true if removed (false if it didn't exist)
	 */
	boolean removeTemplate(String name) {
		CellObject object = templates.remove(name);
		return object != null;
	}
	
	/**
	 * @param cellObject the new template
	 * @return true if added to list
	 * 
	 * Register a new prototype
	 */
	boolean registerTemplate(CellObject cellObject) {
		if (templates.containsKey(cellObject.getName())) {
			return false;
		}
		if (cellObject == null) {
			return false;
		}
		templates.put(cellObject.getName(), cellObject);
		return true;
	}
	
	Collection<CellObject> getTemplates() {
		return templates.values();
	}
	
	/**
	 * @param name the property name
	 * @return list of cell object templates that have that property
	 */
	List<CellObject> getTemplatesWithProperty(String name) {
		List<CellObject> ret = new ArrayList<CellObject>(templates.values());
		Iterator<CellObject> iter = ret.iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			if (!obj.hasProperty(name)) {
				iter.remove();
			}
		}
		return ret;
	}
	
	/**
	 * @param name the property name
	 * @return true if any templates have that property
	 */
	boolean hasTemplatesWithProperty(String name) {
		List<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return false;
		}
		return true;
	}
	
	List<CellObject> rewardObjects = new ArrayList<CellObject>();
	/**
	 * @param name object name to create
	 * @return the new object
	 * 
	 * Clones an object and returns the new copy
	 */
	CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			CellObject newObject = new CellObject(templates.get(name));
			if (newObject.getBooleanProperty("apply.reward", false)) {
				// assign identification number
				newObject.setProperty("box-id", Integer.toString(rewardObjects.size() + 1));

				// keep track of reward objects
				rewardObjects.add(newObject);
				
				logger.trace("Reward box: " + newObject.getIntProperty("box-id", -1));

			} else if (newObject.getBooleanProperty("apply.reward-info", false)) {
				// assign identification properties
				newObject.setProperty(Names.kPropertyColor, Gridmap2D.simulation.kColors[0]);
				newObject.setProperty("box-id", "0");

				logger.trace("Info box: " + newObject.getIntProperty("box-id", -1));
			}
			return newObject;
		}
		return null;
	}
	
	/**
	 * @param name the property
	 * @return an object with that property
	 * 
	 * creates an object that has the specified property. Many different templates
	 * could possibly have the same property, this randomly picks one to clone.
	 */
	CellObject createRandomObjectWithProperty(String name) {
		List<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return null;
		}
		return new CellObject(all.get(Simulation.random.nextInt(all.size())));
	}
	/**
	 * @param name1 a property
	 * @param name1 another property
	 * @return an object with those properties
	 * 
	 * same as createRandomObjectWithProperty but with two properties
	 */
	CellObject createRandomObjectWithProperties(String name1, String name2) {
		List<CellObject> all = getTemplatesWithProperty(name1);
		if (all.size() <= 0) {
			return null;
		}
		Iterator<CellObject> iter = all.iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			if (!obj.hasProperty(name2)) {
				iter.remove();
			}
		}
		if (all.size() <= 0) {
			return null;
		}
		return new CellObject(all.get(Simulation.random.nextInt(all.size())));
	}
}
