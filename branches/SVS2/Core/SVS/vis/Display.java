import april.vis.*;
import april.jmat.*;
import april.util.*;

import java.io.*;
import java.util.*;
import java.awt.*;
import javax.swing.*;

import quickhull3d.*;

class Display {
	class DispObj {
		static final double PT_RADIUS = 0.1;
		
		private double[] pos = {0.0, 0.0, 0.0};
		private double[] rot = {0.0, 0.0, 0.0};
		private double[] scl = {1.0, 1.0, 1.0};
		private float[] color = { 1.0f, 0.0f, 0.0f};
		
		private ArrayList<double[]> pts = new ArrayList<double[]>();
		
		VisObject shape = null;
		VisChain chain = null;
		
		void setPos(double[] p) {
			pos = p;
			chain = null;
		}
		
		void setRot(double[] r) {
			rot = r;
			chain = null;
		}
		
		void setScale(double[] s) {
			scl = s;
			chain = null;
		}
		
		void setColor(double[] c) {
			for (int i = 0; i < 3; i++) {
				color[i] = (float) c[i];
			}
		}
		
		void clearPoints() {
			pts.clear();
			shape = null;
			chain = null;
		}
		
		void addPoint(double[] v) {
			double[] v1 = Arrays.copyOf(v, 3);
			pts.add(v1);
			shape = null;
		}
		
		void calcShape() {
			int i;
			Color col = new Color(color[0], color[1], color[2]);
			
			switch (pts.size()) {
				case 0:
					shape = new VisSphere(PT_RADIUS, col);
					break;
				case 1:
					shape = new VisChain(LinAlg.translate(pts.get(0)), new VisSphere(PT_RADIUS, col));
					break;
				case 2:
					shape = new VisLineSegment(pts.get(0), pts.get(1), col);
					break;
				case 3:
					shape = new VisData(pts, new VisDataFillStyle(col));
					break;
				default:
					Point3d[] pts2 = new Point3d[pts.size()];
					for (i = 0; i < pts.size(); i++) {
						double[] p1 = pts.get(i);
						pts2[i] = new Point3d(p1[0], p1[1], p1[2]);
					}
					
					QuickHull3D hull = new QuickHull3D();
					hull.build(pts2);
					hull.triangulate();
					Point3d[] verts = hull.getVertices();
					int[][] faces = hull.getFaces();
					
					ArrayList<double[]> visVerts = new ArrayList<double[]>();
					ArrayList<int[]> visTriangles = new ArrayList<int[]>();
					
					for (i = 0; i < verts.length; i++) {
						System.err.println(String.format("%f %f %f", verts[i].x, verts[i].y, verts[i].z));
						double[] v = {verts[i].x, verts[i].y, verts[i].z};
						visVerts.add(v);
					}
					for (i = 0; i < faces.length; i++) {
						assert faces[i].length == 3;
						System.err.println(String.format("%d %d %d", faces[i][0], faces[i][1], faces[i][2]));
						visTriangles.add(faces[i]);
					}
					
					int col2 = (255 << 24) + 
					           (((int) (color[0] * 255)) << 16) + 
					           (((int) (color[1] * 255)) << 8) + 
					           ((int) (color[2] * 255));
					shape = new VisTriangles(new ConstantColorizer(col2), visVerts, visTriangles);
					break;
			}
		}
		
		VisChain getChain() {
			if (shape == null) {
				calcShape();
				chain = null;
			}
			if (chain == null) {
				chain = new VisChain();
				chain.add(LinAlg.translate(pos));
				chain.add(LinAlg.rollPitchYawToMatrix(rot));
				chain.add(LinAlg.scale(scl[0], scl[1], scl[2]));
				chain.add(shape);
			}
			return chain;
		}
	}
	
	class DispText {
		String str;
		double[] pos;
		VisText text;
		
		DispText(String s, double[] p) {
			str = s;
			pos = p;
		}
		
		VisText getText() {
			if (text == null) {
				text = new VisText(pos, VisText.ANCHOR.CENTER, str);
			}
			return text;
		}
	}
	
	VisWorld vw;
	VisCanvas vc;
	VisWorld.Buffer buf;
	VisWorld.Buffer textbuf;

	JFrame frame;
	JSplitPane splitpane;
	JTextArea logtext;
	
	Map<String, DispObj> objects;
	Map<String, DispText> texts;
	
	public Display() {
		objects = new HashMap<String, DispObj>();
		texts = new HashMap<String, DispText>();
		
		vw = new VisWorld();
		vc = new VisCanvas(vw);
		buf = vw.getBuffer("buf");
		textbuf = vw.getBuffer("textbuf");
		
		frame = new JFrame("Display");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		logtext = new JTextArea();
		splitpane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, vc, logtext);
		frame.setLayout(new BorderLayout());
		frame.add(splitpane, BorderLayout.CENTER);
		frame.setSize(800, 600);
		frame.setVisible(true);
		splitpane.setDividerLocation(0.80);
		
		RenderThread renderer = new RenderThread();
		renderer.start();
	}
	
	void parseLine(String line) {
		String[] fields = line.split("[ \t]+");
		String[] fields2 = line.split("[ \t]+", 2);
		synchronized (objects) {
			switch (fields[0].charAt(0)) {
				case 'a':
					parseAdd(fields);
					break;
				case 'd':
					parseDel(fields);
					break;
				case 'c':
					parseChange(fields);
					break;
				case 't':
					parseText(fields);
					break;
				case 'l':
					logtext.append(fields2[1] + "\n");
					break;
				case 'r':
					reset();
					break;
				default:
					System.err.println("unknown command " + fields[0]);
					break;
			}
		}
	}
	
	void reset() {
		objects.clear();
		texts.clear();
	}
	
	/*
	 The format is
	 a <name> [p <px> <py> <pz>] [r <rx> <ry> <rz>] [s <sx> <sy> <sz>] [<v1x> <v1y> <v1z> ... ]
	*/
	void parseAdd(String fields[]) {
		
		if (fields.length < 2) {
			System.err.println("invalid add command");
			return;
		}
		
		String name = fields[1];
		DispObj obj = new DispObj();
		if (objects.containsKey(name)) {
			System.err.println(String.format("object %s already exists", name));
			return;
		}
		
		double vals[] = new double[3];
		int i = 2;
		while (i < fields.length) {
			if (!fields[i].equals("p") && !fields[i].equals("r") && !fields[i].equals("s") && !fields[i].equals("c")) {
				break;
			}
			if (!parseFloats(fields, i + 1, vals)) {
				return;
			}
			switch (fields[i].charAt(0)) {
				case 'p': obj.setPos(vals); break;
				case 'r': obj.setRot(vals); break;
				case 's': obj.setScale(vals); break;
				case 'c': obj.setColor(vals); break;
			}
			i += 4;
		}
		
		while (i < fields.length) {
			
			if (!parseFloats(fields, i, vals)) {
				return;
			}
			obj.addPoint(vals);
			i += 3;
		}
		
		objects.put(name, obj);
	}
	
	/*
	 The format is
	 d <name>
	*/
	void parseDel(String fields[]) {
		if (fields.length < 2) {
			System.err.println("invalid del command");
			return;
		}
		String name = fields[1];
		if (objects.containsKey(name)) {
			objects.remove(name);
			return;
		}
		if (texts.containsKey(name)) {
			texts.remove(name);
			return;
		}
		System.err.println(String.format("object %s does not exist", name));
	}
	
	/*
	 The format is
	 c <name> [p <px> <py> <pz>] [r <rx> <ry> <rz>] [s <sx> <sy> <sz>] [v <v1x> <v1y> <v1z> ...]
	*/
	void parseChange(String fields[]) {
		if (fields.length < 2) {
			System.err.println("invalid change command");
			return;
		}
		
		String name = fields[1];
		if (!objects.containsKey(name)) {
			System.err.println(String.format("object %s does not exist", name));
			return;
		}
		DispObj obj = objects.get(name);
		
		double vals[] = new double[3];
		int i = 2;
		while (i < fields.length) {
			if (fields[i].equals("p") && fields[i].equals("r") && fields[i].equals("s")) {
				break;
			}
			if (!parseFloats(fields, i + 1, vals)) {
				return;
			}
			switch (fields[i].charAt(0)) {
				case 'p': obj.setPos(vals); break;
				case 'r': obj.setRot(vals); break;
				case 's': obj.setScale(vals); break;
			}
			i += 4;
		}
		
		if (i == fields.length || fields[i] != "v") {
			return;
		}
		
		i++;
		obj.clearPoints();
		while (i < fields.length) {
			if (!parseFloats(fields, i, vals)) {
				return;
			}
			obj.addPoint(vals);
			i += 3;
		}
	}
	
	boolean parseFloats(String[] fields, int begin, double[] vals) {
		if (begin + vals.length > fields.length) {
			System.err.println(String.format("expecting at least %d fields, only got %d", begin + vals.length, fields.length));
			return false;
		}
		for (int i = 0; i < vals.length; i++) {
			try {
				vals[i] = Double.parseDouble(fields[begin + i]);
			} catch (NumberFormatException e) {
				System.err.println(String.format("expecting a number in field %d, got %s", begin + i, fields[begin + i]));
				return false;
			}
		}
		return true;
	}
	
	void parseText(String[] fields) {
		if (fields.length < 5) {
			System.err.println("invalid text command");
			return;
		}
		String name = fields[1], s = "";
		double[] p = new double[3];
		
		if (texts.containsKey(name)) {
			System.err.println(String.format("text %s already exists", name));
		}
		if (!parseFloats(fields, 2, p)) {
			return;
		}
		
		for (int i = 5; i < fields.length; i++) {
			s += fields[i] + " ";
		}
		
		texts.put(name, new DispText(s, p));
	}
	
	class RenderThread extends Thread {
		int FPS = 10;
		
		public void run() {
			while (true) {
				draw();
				TimeUtil.sleep(1000 / FPS);
			}
		}
		
		void draw() {
			synchronized (objects) {
				for (String name : objects.keySet()) {
					VisChain c;
					try {
						c = objects.get(name).getChain();
					} catch (IllegalArgumentException e) {
						System.err.println(e);
						System.err.println("deleting " + name);
						objects.remove(name);
						continue;
					}
					if (c != null) {
						buf.addBuffered(c);
					}
				}
				buf.switchBuffer();
				
				for (String name : texts.keySet()) {
					textbuf.addBuffered(texts.get(name).getText());
				}
				textbuf.switchBuffer();
			}
		}
	}
	
	
	static public void main(String args[]) {
		if (args.length == 0) {
			System.err.println("specify path to fifo");
			System.exit(1);
		}
		
		String line = "", fifo_path = args[0];
		BufferedReader brd = null;
		try {
			brd = new BufferedReader(new FileReader(fifo_path));
		} catch (FileNotFoundException e) {
			System.err.println("fifo not found");
			System.exit(1);
		}
		
		Display d = new Display();
		while (true) {
			try {
				line = brd.readLine();
			} catch (IOException e) {
				System.err.println(e);
				System.exit(1);
			}
			if (line == null) {
				try {
					brd = new BufferedReader(new FileReader(fifo_path));
				} catch (FileNotFoundException e) {
					System.err.println("fifo not found");
					System.exit(1);
				}
			} else {
				System.out.println(line);
				d.parseLine(line);
			}
		}
	}
}
