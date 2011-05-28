import java.io.*;
import java.util.Formatter;
import april.lcmtypes.*;
import april.util.TimeUtil;
import april.jmat.LinAlg;
import lcm.lcm.*;

public class RobotClient implements LCMSubscriber {
	LCM lcm;
	differential_drive_command_t dc;
	pose_t pose;
	map_metadata_t map;
	boolean mapreceived, mapwritten, posechanged;
	double rotz;

	public static void perror(Exception err) {
		System.err.println(err);
		System.exit(1);
	}
	
	public static boolean close(double a, double b) {
		double tol = Math.abs(0.001 * a);
		return (a - tol <= b) && (b <= a + tol);
	}
	
	public static void main(String args[]) {
		RobotClient r = new RobotClient();
		r.run();
	}
	
	RobotClient() {
		pose = null;
		dc = new differential_drive_command_t();
		dc.left_enabled = true;
		dc.right_enabled = true;
		lcm = LCM.getSingleton();
		lcm.subscribe(".*", this);
		mapreceived = false;
		mapwritten = false;
		posechanged = false;
		rotz = Double.NaN;
	}
	
	public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
		synchronized (this) {
			try {
				if (channel.startsWith("POSE")) {
					pose = new pose_t(ins);
					posechanged = true;
				} else if (channel.startsWith("AREA_DESCRIPTION")) {
					map = new map_metadata_t(ins);
					mapreceived = true;
				}
			} catch (IOException err) {
				perror(err);
			}
		}
	}
	
	public void writeInitialPose() {
		System.out.println("a splinter world v -1 -1 0 1 -1 0 1 1 0 -1 1 0");
	}
	
	public void writePose(pose_t p) {
		Formatter fmt = new Formatter();
		double rot[] = LinAlg.quatToRollPitchYaw(p.orientation);
		
		if (Double.isNaN(rotz)) {
			rotz = rot[2];
		} else {
			rotz = rotz + p.rotation_rate[2];
		}
		assert close(rotz % (2 * Math.PI), rot[2]);
		
		fmt.format("c splinter p %g %g %g r 0.0 0.0 %g\n", p.pos[0], p.pos[1], p.pos[2], rotz);
		for (int i = 0; i < p.vel.length; ++i) {
			fmt.format("p vel_%d %g\n", i, p.vel[i]);
		}
		/*
		for (int i = 0; i < p.accel.length; ++i) {
			fmt.format("p accel_%d %g\n", i, p.accel[i]);
		}
		*/
		for (int i = 0; i < p.rotation_rate.length; ++i) {
			fmt.format("p rotation_rate_%d %g\n", i, p.rotation_rate[i]);
		}
		fmt.format("p left_rads_per_sec %g\n", p.rads_per_sec[0]);
		fmt.format("p right_rads_per_sec %g\n", p.rads_per_sec[1]);
		System.out.print(fmt.toString());
	}
	
	public void writeMap() {
		if (!mapreceived || mapwritten) {
			return;
		}
		mapwritten = true;
		
		Formatter fmt = new Formatter();
		april.lcmtypes.map_metadata_t m;
		
		if (map == null) {
			return;
		}
		synchronized (this) {
			m = map;
		}
		
		for (int i = 0; i < m.areas.length; ++i) {
			double x = m.areas[i][0], y = m.areas[i][1];
			double w = m.areas[i][2], h = m.areas[i][3];
			fmt.format("a area%d world v 0 0 0 0 %f 0 %f 0 0 %f %f 0 p %f %f 0\n", i, w, h, w, h, x, y);
		}
		System.out.print(fmt.toString());
	}
	
	public boolean readInput() {
		String line = "";
		String fields[];
		BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
		
		while (true) {
			try {
				line = stdin.readLine();
			} catch (IOException e) {
				perror(e);
			}
			
			if (line == null) {
				return false;
			} else if (line.equals("***")) {
				break;
			}
			fields = line.split(" +");
			if (fields.length != 2) {
				continue;
			}
			if (fields[0].equals("left")) {
				try {
					dc.left = Double.parseDouble(fields[1]);
				} catch (NumberFormatException err) {
					continue;
				}
			} else if (fields[0].equals("right")) {
				try {
					dc.right = Double.parseDouble(fields[1]);
				} catch (NumberFormatException err) {
					continue;
				}
			} else {
				continue;
			}
		}
		dc.utime = TimeUtil.utime();
		lcm.publish("DIFFERENTIAL_DRIVE_COMMAND_seek", dc);
		return true;
	}

	public void run() {
		long lasttime = -1;
		pose_t p;
		
		writeInitialPose();
		while (true) {
			if (posechanged && pose != null) {
				posechanged = false;
				
				synchronized (this) {
					p = pose.copy();
				}
				
				if (lasttime > 0) {
					System.out.println("t " + (p.utime - lasttime));
				}
				lasttime = p.utime;
				
				writeMap();
				writePose(p);
			}
			System.out.println("***");
			readInput();
		}
	}
	
}
