package edu.umich.soar.gridmap2d;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Polygon;
import java.awt.SystemColor;
import java.util.HashMap;

import lcmtypes.pose_t;

import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.core.events.AfterTickEvent;
import edu.umich.soar.gridmap2d.core.events.ResetEvent;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.Player;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomObject;

public class RoomPanel extends GridMapPanel {
	
	private static final long serialVersionUID = -8083633808532173643L;

	//private final Adaptable app;
	private final int cellSize = 16;
	private final Simulation sim;
	private static final int kDotSize = 7;

	public RoomPanel(Adaptable app) {
		//this.app = app;
		
		sim = Adaptables.adapt(app, Simulation.class);
		sim.getEvents().addListener(AfterTickEvent.class, this);
		sim.getEvents().addListener(ResetEvent.class, this);
		
		// FIXME this needs to be called on map event
		setMap((RoomMap)sim.getMap());
	}
	
	private static class IdLabel {
		int [] loc;
		boolean gateway;
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
					label.gateway = rmap.getCell(loc.getLoc()).hasObjectWithProperty(Names.kPropertyGatewayRender);
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
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);
		
		RoomMap map = (RoomMap)sim.getMap();

		loc.reset();
		while (loc.next()) {
			if (walls[loc.getLoc()[0]][loc.getLoc()[1]]) {
			    g.setColor(Color.BLACK);
			} 
			else if (gateways[loc.getLoc()[0]][loc.getLoc()[1]]) {
				g.setColor(Color.WHITE);
			} 
			else {
				g.setColor(SystemColor.control);
			}
			
			g.fillRect(loc.getDraw()[0], loc.getDraw()[1], cellSize, cellSize);
		}
		
		// draw objects
		for (RoomObject ro : map.getRoomObjects()) {
			pose_t pose = ro.getPose();
			if (pose == null) {
				continue;
			}
			g.setColor(ro.getColor());
			g.fillOval((int)pose.pos[0] - 2, cellSize*map.size() - (int)pose.pos[1] - 2, 4, 4);
		}
		
		// draw id labels on top of map
		for (IdLabel label : rmids.values()) {
			if (label.gateway) {
				// TODO:
				//gc.setBackground(WindowManager.white);
			} else {
				//gc.setBackground(WindowManager.widget_background);
			}
			g.setColor(Color.BLACK);
			g.drawString(label.label, label.loc[0], label.loc[1]);
		}
		
		// draw entities now so they appear on top
		for (Player p : sim.getWorld().getPlayers()) {
			Robot player = (Robot)p;
			pose_t pose = player.getState().getPose();
			
			float [] center = new float [] { (float)pose.pos[0], (float)pose.pos[1] };
			float [] offset = new float [] { 0, 0 };
			float [] pathTemp = new float [2];
			
			Polygon poly = new Polygon();
			float heading = (float)player.getState().getYaw();

			// first, move to the point representing the tip of the chevron
			offset[1] = kDotSize * (float)Math.sin(heading);
			offset[0] = kDotSize * (float)Math.cos(heading);
			pathTemp[0] = center[0] + offset[0];
			pathTemp[1] = map.size() * cellSize - (center[1] + offset[1]);
			//float [] original = Arrays.copyOf(pathTemp, pathTemp.length);
			poly.addPoint(Math.round(pathTemp[0]), Math.round(pathTemp[1]));
			//System.out.print(Arrays.toString(pathTemp));
			//System.out.print("-");
			
			int left = (int)pathTemp[0];
			int right = (int)pathTemp[0];
			int top = (int)pathTemp[1];
			int bottom = (int)pathTemp[1];

			// next draw a line to the corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading + (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading + (3*Math.PI)/4);
			pathTemp[0] = center[0] + offset[0];
			pathTemp[1] = map.size() * cellSize - (center[1] + offset[1]);
			poly.addPoint(Math.round(pathTemp[0]), Math.round(pathTemp[1]));
			//System.out.print(Arrays.toString(pathTemp));
			//System.out.print("-");
			left = Math.min((int)Math.floor(pathTemp[0]), left);
			right = Math.max((int)Math.ceil(pathTemp[0]), right);
			top = Math.max((int)Math.ceil(pathTemp[1]), top);
			bottom = Math.min((int)Math.floor(pathTemp[1]), bottom);

			// next draw a line to the other corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading - (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading - (3*Math.PI)/4);
			pathTemp[0] = center[0] + offset[0];
			pathTemp[1] = map.size() * cellSize - (center[1] + offset[1]);
			poly.addPoint(Math.round(pathTemp[0]), Math.round(pathTemp[1]));
			//System.out.print(Arrays.toString(pathTemp));
			//System.out.print("-");
			left = Math.min((int)Math.floor(pathTemp[0]), left);
			right = Math.max((int)Math.ceil(pathTemp[0]), right);
			top = Math.max((int)Math.ceil(pathTemp[1]), top);
			bottom = Math.min((int)Math.floor(pathTemp[1]), bottom);
			
			// finally a line back to the original
			//path.lineTo(original[0], original[1]);
				
			g.setColor(Colors.getColor(player.getColor()));
			g.drawPolygon(poly);
			
			//System.out.println(": lrtb: " + left + "-" + right + "-" + top + "-" + bottom);

//			// clean up more drawing errors
//			right += 1;
//			left -= 1;
//			top += 1;
//			bottom -= 1;
//			
//			map.getCell(getCellAtPixel(new int[] {left, top})).setModified(true);
//			map.getCell(getCellAtPixel(new int[] {left, bottom})).setModified(true);
//			map.getCell(getCellAtPixel(new int[] {right, top})).setModified(true);
//			map.getCell(getCellAtPixel(new int[] {right, bottom})).setModified(true);
//
//			// draw waypoints
//			List<double[]> waypoints = world.getWaypointList(player);
//			for (double[] wp : waypoints) {
//				left = (int)Math.floor(wp[0]) - 2;
//				top = cellSize*map.size() - (int)Math.ceil(wp[1]) - 2;
//				right = left + 4;
//				bottom = top + 4;
//
//				gc.setForeground(WindowManager.getColor(player.getColor()));
//				gc.drawOval(left, top, 4, 4);
//				
//				// set redraw on all four points for next cycle
//				map.getCell(getCellAtPixel(new int[] {left, top})).setModified(true);
//				map.getCell(getCellAtPixel(new int[] {left, bottom})).setModified(true);
//				map.getCell(getCellAtPixel(new int[] {right, top})).setModified(true);
//				map.getCell(getCellAtPixel(new int[] {right, bottom})).setModified(true);
//			}
		}
	}
}
