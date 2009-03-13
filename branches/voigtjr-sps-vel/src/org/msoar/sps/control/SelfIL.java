package org.msoar.sps.control;

import java.util.List;


import jmat.LinAlg;
import jmat.MathUtil;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

final class SelfIL implements InputLinkInterface {
	private final Agent agent;
	private final WaypointsIL waypointsIL;
	private final FloatElement xwme;
	private final FloatElement ywme;
	private final FloatElement zwme;
	private final Identifier posewme;
	private final long utimeLast = 0;
	private final ReceivedMessagesIL receivedMessagesIL;
	private final SplinterModel splinter;
	
	private IntElement yawwmei;
	private FloatElement yawwmef;
	
	SelfIL(Agent agent, Identifier self, SplinterModel splinter) {
		this.agent = agent;
		this.splinter = splinter;
		
		agent.CreateStringWME(self, "name", agent.GetAgentName());

		posewme = agent.CreateIdWME(self, "pose");
		xwme = agent.CreateFloatWME(posewme, "x", 0);
		ywme = agent.CreateFloatWME(posewme, "y", 0);
		zwme = agent.CreateFloatWME(posewme, "z", 0);
		yawwmef = agent.CreateFloatWME(posewme, "yaw", 0);
		
		Identifier waypoints = agent.CreateIdWME(self, "waypoints");
		waypointsIL = new WaypointsIL(agent, waypoints);
		
		Identifier receivedwme = agent.CreateIdWME(self, "received-messages");
		receivedMessagesIL = new ReceivedMessagesIL(agent, receivedwme);
	}
	
	private void updateYawWme(boolean useFloatYawWmes, double yawRadians) {
		double yawDegrees = Math.toDegrees(yawRadians);
		
		if (useFloatYawWmes) {
			if (yawwmei != null) {
				agent.DestroyWME(yawwmei);
				yawwmei = null;
				yawwmef = agent.CreateFloatWME(posewme, "yaw", 0);
			}
			agent.Update(yawwmef, yawDegrees);
		} else {
			if (yawwmef != null) {
				agent.DestroyWME(yawwmef);
				yawwmef = null;
				yawwmei = agent.CreateIntWME(posewme, "yaw", 0);
			}
			agent.Update(yawwmei, (int)Math.round(yawDegrees));
		}
	}
	
	void update(List<String> tokens, boolean useFloatYawWmes) {
		if (splinter.getSplinterPose() == null) {
			return; // no info
		}
		
		if (utimeLast == splinter.getSplinterPose().utime) {
			return; // same info
		}

		agent.Update(xwme, splinter.getSplinterPose().pos[0]);
		agent.Update(ywme, splinter.getSplinterPose().pos[1]);
		agent.Update(zwme, splinter.getSplinterPose().pos[2]);
		double yawRadians = LinAlg.quatToRollPitchYaw(splinter.getSplinterPose().orientation)[2];
		yawRadians = MathUtil.mod2pi(yawRadians);
		updateYawWme(useFloatYawWmes, yawRadians);
		
		waypointsIL.update(splinter);
		
		// TODO support multiple sources
		if (tokens != null) {
			// TODO tokens len 0 triggering clear is overloaded and messy
			if (tokens.size() == 0) {
				receivedMessagesIL.clear();
			} else {
				receivedMessagesIL.update(tokens);
			}
		}
	}
	
	WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}

	ReceivedMessagesIL getMessagesIL() {
		return receivedMessagesIL;
	}

	public void addWaypoint(double[] pos, String id, boolean useFloatYawWmes) {
		waypointsIL.add(pos, id, useFloatYawWmes);
	}

	public void clearMessages() {
		receivedMessagesIL.clear();
	}

	public boolean disableWaypoint(String id) {
		return waypointsIL.disable(id);
	}

	public boolean enableWaypoint(String id, SplinterModel splinter) {
		return waypointsIL.enable(id, splinter);
	}

	public boolean removeMessage(int id) {
		return receivedMessagesIL.remove(id);
	}

	public boolean removeWaypoint(String id) {
		return waypointsIL.remove(id);
	}
}

