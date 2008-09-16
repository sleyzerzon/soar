package soar2d.player;

import java.io.File;
import java.io.IOException;
import java.util.*;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import sml.*;
import soar2d.Soar2D;

public class InputLinkMetadata {
	
	private ArrayList<File> files = new ArrayList<File>(1);
	private Agent agent;
	private Identifier metadataWME;
	private HashMap<Integer, Identifier> sharedIdentifierMap = new HashMap<Integer, Identifier>();
	
	public InputLinkMetadata(Agent agent) {
		assert agent != null;
		this.agent = agent;
	}
	
	public void load(File metadataFile) throws Exception {
		if (files.contains(metadataFile)) {
			Soar2D.logger.fine("Ignoring duplicate metadata file " + metadataFile.getAbsolutePath());
			return;
		}
		
		try {
			if (metadataWME == null) {
				metadataWME = agent.CreateIdWME(agent.GetInputLink(), "metadata");
				if (agent.HadError()) {
					throw new Exception(agent.GetLastErrorDescription());
				}
			}
			
			if (!metadataFile.exists()) {
				throw new Exception("Metadata file doesn't exist: " + metadataFile.getAbsolutePath());
			}
			
			try {
				SAXBuilder builder = new SAXBuilder();
				Document doc = builder.build(metadataFile);
				Element root = doc.getRootElement();
				if (root == null || !root.getName().equalsIgnoreCase("metadata")) {
					throw new Exception("Couldn't find metadata tag in metadata file.");
				}
				
				iterateChildren(root, metadataWME);
				
			} catch (IOException e) {
				throw new Exception("Metadata: I/O exception: " + e.getMessage());
			} catch (JDOMException e) {
				throw new Exception("Metadata: Error during parsing: " + e.getMessage());
			} catch (IllegalStateException e) {
				throw new Exception("Metadata: Illegal state: " + e.getMessage());
			}
					
		} catch (Exception e) {
			String message = "Metadata: " + e.getMessage();
			Soar2D.logger.severe(message);
			throw new Exception(message);
		}
		
		files.add(metadataFile);
	}
	
	private void iterateChildren(Element element, Identifier parentWME) throws Exception {
		assert element != null;
		
		List<Element> children = (List<Element>)element.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();

			String attributeString = child.getAttributeValue("attribute");
			if (attributeString == null) {
				throw new Exception(element.getName() + " under " + element.getName() + " missing attribute parameter.");
			}
			String valueString = child.getAttributeValue("value");
			String sharedIdString = child.getAttributeValue("shared-id");

			if (child.getName().equalsIgnoreCase("identifier")) {
				if (valueString != null) 
					Soar2D.logger.info("Ignoring identifier value parameter");

				int sharedId = 0; // only valid if sharedIdString not null
				
				if (sharedIdString != null) {
					sharedId = Integer.parseInt(sharedIdString); // throws
					
					Identifier sharedIdentifier = sharedIdentifierMap.get(sharedId);
					
					if (sharedIdentifier != null) {
						// shared WME
						agent.CreateSharedIdWME(parentWME, attributeString, sharedIdentifier);
						if (agent.HadError()) {
							throw new Exception(agent.GetLastErrorDescription());
						}
						
						assert child.getChildren().size() == 0;
						continue;
					}
				}
				
				Identifier newIdentifier = agent.CreateIdWME(parentWME, attributeString);
				if (agent.HadError()) {
					throw new Exception(agent.GetLastErrorDescription());
				}
				assert newIdentifier != null;
				
				if (sharedIdString != null) {
					sharedIdentifierMap.put(sharedId, newIdentifier);
				}
				
				if (child.getChildren().size() > 0)
					iterateChildren(child, newIdentifier);
				
			} else if (child.getName().equalsIgnoreCase("string-element")) {
				assert child.getChildren().size() == 0;

				if (sharedIdString != null) 
					Soar2D.logger.info("Ignoring string-element sharedId parameter");
				if (valueString == null) {
					throw new Exception(element.getName() + " under " + element.getName() + " missing value parameter.");
				}
				
				agent.CreateStringWME(parentWME, attributeString, valueString);
				if (agent.HadError()) {
					throw new Exception(agent.GetLastErrorDescription());
				}
				
			} else if (child.getName().equalsIgnoreCase("int-element")) {
				assert child.getChildren().size() == 0;

				if (sharedIdString != null) 
					Soar2D.logger.info("Ignoring int-element sharedId parameter");
				if (valueString == null) {
					throw new Exception(element.getName() + " under " + element.getName() + " missing value parameter.");
				}

				int value = Integer.parseInt(valueString); // throws
				
				agent.CreateIntWME(parentWME, attributeString, value);
				if (agent.HadError()) {
					throw new Exception(agent.GetLastErrorDescription());
				}
				
			} else if (child.getName().equalsIgnoreCase("float-element")) {
				assert child.getChildren().size() == 0;

				if (sharedIdString != null) 
					Soar2D.logger.info("Ignoring float-element sharedId parameter");
				if (valueString == null) {
					throw new Exception(element.getName() + " under " + element.getName() + " missing value parameter.");
				}

				float value = Float.parseFloat(valueString); // throws
				
				agent.CreateFloatWME(parentWME, attributeString, value);
				if (agent.HadError()) {
					throw new Exception(agent.GetLastErrorDescription());
				}
			}
		}
	}
	
	public void destroy() {
		files.clear();
		sharedIdentifierMap.clear();
		
		if (metadataWME == null) {
			Soar2D.logger.fine("No metadata WME to destroy.");
			return;
		}
		
		agent.DestroyWME(metadataWME);
		metadataWME = null;
	}
}
