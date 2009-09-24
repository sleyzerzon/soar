package edu.umich.soar.gridmap2d;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.SystemColor;
import java.util.HashMap;

import com.commsen.stopwatch.Stopwatch;

import lcmtypes.pose_t;

import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Cell;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomObject;
import edu.umich.soar.gridmap2d.map.RoomWorld;

public class RoomPanel extends GridMapPanel {
	
	private static final long serialVersionUID = -8083633808532173643L;

	private final int cellSize = 16;
	private final Simulation sim;
	private static final int DOT_SIZE = 7;
	private static final Polygon TRIANGLE = new Polygon();
	
	static {
		float x = DOT_SIZE;
		float y = 0;
		TRIANGLE.addPoint(Math.round(x), Math.round(y));
		
		// next draw a line to the corner
		x = DOT_SIZE/2.0f * (float)Math.cos((3*Math.PI)/4);
		y = DOT_SIZE/2.0f * (float)Math.sin((3*Math.PI)/4);
		TRIANGLE.addPoint(Math.round(x), Math.round(y));

		// next draw a line to the other corner
		x = DOT_SIZE/2.0f * (float)Math.cos((-3*Math.PI)/4);
		y = DOT_SIZE/2.0f * (float)Math.sin((-3*Math.PI)/4);
		TRIANGLE.addPoint(Math.round(x), Math.round(y));
	}
	
	public RoomPanel(Adaptable app) {
		sim = Adaptables.adapt(app, Simulation.class);
		
		this.setDoubleBuffered(true);
		
		// FIXME this needs to be called on map event
		setMap((RoomMap)sim.getMap());
	}
	
	private static class IdLabel {
		int [] loc;
		String label;
	}
	
	private final HashMap<Integer, IdLabel> rmids = new HashMap<Integer, IdLabel>();
	
	private static class Location {
		private final int CELL_SIZE;
		private final int MAP_SIZE;
		private final int [] loc = new int [2];
		private final int [] dloc = new int [2];
		private boolean first;
		
		Location(int cellSize, int mapSize) {
			this.CELL_SIZE = cellSize;
			this.MAP_SIZE = mapSize;
			reset();
		}
		
		void reset() {
			loc[0] = 0;
			loc[1] = 0;
			dloc[0] = 0;
			dloc[1] = CELL_SIZE*(MAP_SIZE - 1);
			first = true;
		}
		
		boolean next() {
			if (first) {
				first = false;
				return true;
			}
			
			if (++loc[0] < MAP_SIZE) {
				dloc[0] += CELL_SIZE;
				return true;
			}
			loc[0] = 0;
			dloc[0] = 0;
			if (++loc[1] < MAP_SIZE) {
				dloc[1] -= CELL_SIZE;
				return true;
			}
			return false;
		}
		
		int[] getLoc() {
			return loc;
		}
		
		int[] getDraw() {
			return dloc;
		}
	}
	
	private Location loc;
	private boolean walls[][];
	private boolean gateways[][];
	
	void setMap(RoomMap map) {
		rmids.clear();
		loc = new Location(cellSize, map.size());
		
		walls = new boolean[map.size()][];
		for (int i = 0; i < walls.length; ++i) {
			walls[i] = new boolean[map.size()];
		}

		gateways = new boolean[map.size()][];
		for (int i = 0; i < gateways.length; ++i) {
			gateways[i] = new boolean[map.size()];
		}
		
		RoomMap rmap = (RoomMap)map;
		while (loc.next()) {
			CellObject number = map.getCell(loc.getLoc()).getFirstObjectWithProperty(Names.kPropertyNumber);
			if (number != null) {
				if (!rmids.containsKey(number.getProperty(Names.kPropertyNumber, -1, Integer.class))) {
					IdLabel label = new IdLabel();
					label.label = number.getProperty(Names.kPropertyNumber);
					label.loc = new int [] { loc.getDraw()[0] + 1, loc.getDraw()[1] };
					rmids.put(number.getProperty(Names.kPropertyNumber, -1, Integer.class), label);
				}
			}
			
			walls[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.getCell(loc.getLoc()).hasObjectWithProperty(Names.kPropertyBlock);
			gateways[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.getCell(loc.getLoc()).hasObjectWithProperty(Names.kPropertyGatewayRender);
		}
	}
	
	@Override
	int getCellSize() {
		return cellSize;
	}
	
	@Override
	protected void paintComponent(Graphics gr) {
		long id = Stopwatch.start("RoomPanel", "paintComponent");
		try {
			super.paintComponent(gr);
			RoomMap map = (RoomMap)sim.getMap();
			RoomWorld world = (RoomWorld)sim.getWorld();
			Graphics2D g2d = (Graphics2D)gr;
			
			loc.reset();
			while (loc.next()) {
				Cell cell = map.getCell(loc.getLoc());
				if (sim.isRunning()) {
					if (!cell.isModified()) {
						//continue;
					}
					cell.setModified(false);
				}
				
				if (walls[loc.getLoc()[0]][loc.getLoc()[1]]) {
					g2d.setColor(Color.BLACK);
				} 
				else if (gateways[loc.getLoc()[0]][loc.getLoc()[1]]) {
					g2d.setColor(Color.WHITE);
				} 
				else {
					g2d.setColor(SystemColor.control);
				}
				
				g2d.fillRect(loc.getDraw()[0], loc.getDraw()[1], cellSize, cellSize);
			}
			
			// draw objects
			for (RoomObject ro : map.getRoomObjects()) {
				pose_t pose = ro.getPose();
				if (pose == null) {
					continue;
				}
				g2d.setColor(ro.getColor());
				g2d.fillOval((int)pose.pos[0] - 2, cellSize*map.size() - (int)pose.pos[1] - 2, 4, 4);
			}
	
			// draw id labels on top of map
			for (IdLabel label : rmids.values()) {
				g2d.setColor(Color.BLACK);
				g2d.drawString(label.label, label.loc[0], label.loc[1] + cellSize - 2);
			}
			
			// draw entities now so they appear on top
			for (Robot p : world.getPlayers()) {
				Robot player = (Robot)p;
				pose_t pose = player.getState().getPose();
				
				double x = pose.pos[0];
				double y = map.size() * cellSize - pose.pos[1];
				
				g2d.translate(x, y);
				g2d.rotate(-player.getState().getYaw());
				g2d.setColor(Colors.getColor(player.getColor()));
				g2d.drawPolygon(TRIANGLE);
				g2d.rotate(player.getState().getYaw());
				g2d.translate(-x, -y);
			}
		} finally {
			Stopwatch.stop(id);
		}
	}

	@Override
	int [] getCellAtPixel(int x, int y) {
		RoomMap map = (RoomMap)sim.getMap();
		y = map.size() * cellSize - y;
		return super.getCellAtPixel(x, y);
	}

}
