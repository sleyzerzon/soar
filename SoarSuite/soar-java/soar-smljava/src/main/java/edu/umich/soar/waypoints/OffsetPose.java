package edu.umich.soar.waypoints;

import lcmtypes.pose_t;

public interface OffsetPose {
	pose_t getPose();
	void setOffset(double[] offset);
	double[] getOffset();
}
