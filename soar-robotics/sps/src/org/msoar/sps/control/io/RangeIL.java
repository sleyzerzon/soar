package org.msoar.sps.control.io;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

class RangeIL {
	private Identifier rangewme;
	private Agent agent;

	private FloatElement startwmef;
	private FloatElement endwmef;

	private IntElement startwmei;
	private IntElement endwmei;

	private FloatElement distancewme;

	RangeIL(Agent agent, Identifier rangeswme, int id) {
		this.agent = agent;
		rangewme = agent.CreateIdWME(rangeswme, "range");
		agent.CreateIntWME(rangewme, "id", id);
	}

	void update(double start, double end, double distance, boolean useFloatYawWmes) {
		start = Math.toDegrees(start);
		end = Math.toDegrees(end);
		
		if (useFloatYawWmes) {
			if (startwmei != null) {
				agent.DestroyWME(startwmei);
				startwmei = null;
			}
			if (startwmef == null) {
				startwmef = agent.CreateFloatWME(rangewme, "start", start);
			} else {
				agent.Update(startwmef, start);
			}
			
			if (endwmei != null) {
				agent.DestroyWME(endwmei);
				endwmei = null;
			}
			if (endwmef == null) {
				endwmef = agent.CreateFloatWME(rangewme, "end", end);
			} else {
				agent.Update(endwmef, end);
			}
		} else {
			if (startwmef != null) {
				agent.DestroyWME(startwmef);
				startwmef = null;
			}
			if (startwmei == null) {
				startwmei = agent.CreateIntWME(rangewme, "start", (int)Math.round(start));
			} else {
				agent.Update(startwmei, (int)Math.round(start));
			}
			
			if (endwmef != null) {
				agent.DestroyWME(endwmef);
				endwmef = null;
			}
			if (endwmei == null) {
				endwmei = agent.CreateIntWME(rangewme, "end", (int)Math.round(end));
			} else {
				agent.Update(endwmei, (int)Math.round(end));
			}
		}
		
		if (distancewme == null) {
			distancewme = agent.CreateFloatWME(rangewme, "distance", distance);
		} else {
			agent.Update(distancewme, distance);
		}
	}
}
