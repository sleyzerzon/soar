package eaters;

import org.eclipse.swt.graphics.Point;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import utilities.Logger;

public class Eater {
	public final static int kEaterVision = 2;
	
	public final static String kEaterID = "eater";
	public final static String kDirectionID = "direction";
	public final static String kNameID = "name";
	public final static String kScoreID = "score";
	public final static String kxID = "x";
	public final static String kyID = "y";
	public final static String kMyLocationID = "my-location";
	public final static String kContentID = "content";
	public final static String kMoveID = "move";
	public final static String kJumpID = "jump";

	public final static String kNorth = "north";
	public final static String kEast = "east";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	protected Logger m_Logger = Logger.logger;
	protected Agent m_Agent;
	protected World m_World;
	protected String m_Name;
	protected int m_Score = 0;
	protected Point m_Location;
	
	protected StringElement m_DirectionWME;
	protected StringElement m_NameWME;
	protected IntElement m_ScoreWME;
	protected IntElement m_xWME;
	protected IntElement m_yWME;
	protected Cell[][] m_Cells = new Cell[(kEaterVision * 2 ) + 1][(kEaterVision * 2 ) + 1];
	
	private class Cell {
		Identifier me;
		StringElement content;

		Identifier north;
		Identifier south;
		Identifier east;
		Identifier west;
		
		boolean iterated = false;
	}
	
	public Eater(Agent agent, World world, Point location) {
		m_Agent = agent;
		m_World = world;
		m_Location = location;
		
		m_Name = m_Agent.GetAgentName();
		m_Logger.log("Created eater: " + m_Name);
		
		Identifier eater = m_Agent.CreateIdWME(m_Agent.GetInputLink(), kEaterID);
		
		m_DirectionWME = m_Agent.CreateStringWME(eater, kDirectionID, kNorth);
		m_NameWME = m_Agent.CreateStringWME(eater, kNameID, m_Name);
		m_ScoreWME = m_Agent.CreateIntWME(eater, kScoreID, m_Score);
		m_xWME = m_Agent.CreateIntWME(eater, kxID, m_Location.x);
		m_yWME = m_Agent.CreateIntWME(eater, kyID, m_Location.y);
		
		for (int i = 0; i < m_Cells.length; ++i) {
			for (int j = 0; j < m_Cells.length; ++j) {
				m_Cells[i][j] = new Cell();
			}
		}
		
		m_Cells[kEaterVision][kEaterVision].me = m_Agent.CreateIdWME(m_Agent.GetInputLink(), kMyLocationID);
		createView(kEaterVision, kEaterVision);
		
		m_Agent.Commit();
	}
	
	public String getName() {
		return m_Name;
	}
	
	public Agent getAgent() {
		return m_Agent;
	}
	
	protected void createView(int x, int y) {
		if (x >= 0 && x <= 4 && y >=0 && y <= 4 && !m_Cells[x][y].iterated) {
			m_Cells[x][y].iterated = true;
			m_Cells[x][y].content = m_Agent.CreateStringWME(m_Cells[x][y].me, kContentID, World.kTypeEmpty);

			if (x > 0) {
				if (m_Cells[x - 1][y].me == null)
					m_Cells[x - 1][y].me = m_Agent.CreateIdWME(m_Cells[x][y].me,"west");
				else
					m_Cells[x][y].west = m_Agent.CreateSharedIdWME(m_Cells[x][y].me,"west",m_Cells[x - 1][y].me);
			}
			
			if (x < 4) {
				if (m_Cells[x + 1][y].me == null)
					m_Cells[x + 1][y].me = m_Agent.CreateIdWME(m_Cells[x][y].me,"east");
				else
					m_Cells[x][y].east = m_Agent.CreateSharedIdWME(m_Cells[x][y].me,"east",m_Cells[x + 1][y].me);
			}
			
			if (y > 0) {
				if (m_Cells[x][y - 1].me == null)
					m_Cells[x][y - 1].me = m_Agent.CreateIdWME(m_Cells[x][y].me,"north");
				else
					m_Cells[x][y].north = m_Agent.CreateSharedIdWME(m_Cells[x][y].me,"north",m_Cells[x][y - 1].me);
			}
			
			if (y < 4) {
				if (m_Cells[x][y + 1].me == null)
					m_Cells[x][y + 1].me = m_Agent.CreateIdWME(m_Cells[x][y].me,"south");
				else
					m_Cells[x][y].south = m_Agent.CreateSharedIdWME(m_Cells[x][y].me,"south",m_Cells[x][y + 1].me);
			}
			
			createView(x - 1,y);
			createView(x + 1,y);
			createView(x,y - 1);
			createView(x,y + 1);
		}	
	}
	
	public void updateInput() {
		int xView, yView;
		for (int x = 0; x < m_Cells.length; ++x) {
			xView = x - Eater.kEaterVision + m_Location.x;
			for (int y = 0; y < m_Cells[x].length; ++y) {
				yView = y - Eater.kEaterVision + m_Location.y;
				String content = m_World.getContentNameByLocation(xView, yView);
				m_Agent.Update(m_Cells[x][y].content, content);
			}
		}
		m_Agent.Commit();
	}
	
	public class MoveInfo {
		Eater eater;
		String direction;
		boolean jump;
	}
	
	public void processOutput() {
		final int kNumCommands = m_Agent.GetNumberCommands();

		MoveInfo move = new MoveInfo();
		move.eater = this;
		
		for (int i = 0; i < kNumCommands; ++i) {
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(kMoveID)) {
				move.jump = false;
				move.direction = commandId.GetParameterValue(kDirectionID);
				if (move.direction != null) {
					if (m_World.moveEater(move)) {
						commandId.AddStatusComplete();
					}
				} else {
					m_Logger.log("Ignoring improperly formatted command: " + kMoveID);
					continue;
				}
			} else if (commandName.equalsIgnoreCase(kJumpID)) {
				move.jump = true;
				move.direction = commandId.GetParameterValue(kDirectionID);
				if (move.direction != null) {
					if (m_World.moveEater(move)) {
						commandId.AddStatusComplete();
					}
				} else {
					m_Logger.log("Ignoring improperly formatted command: " + kDirectionID);
					continue;
				}
			} else {
				m_Logger.log("Ignoring unknown command: " + commandName);
				continue;
			}
		}
	}
	
	public Point getLocation() {
		return m_Location;
	}
	
	public void adjustScore(int delta) {
		m_Score += delta;
	}
	
	public void setLocation(Point location) {
		m_Location = location;
	}
}






















