package edu.umich.robot.superdroid;

import java.io.IOException;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.lcmtypes.pose_t;
import edu.umich.robot.util.PoseProvider;
import edu.umich.robot.util.Poses;

public class SuperdroidPose extends PoseProvider
{
    private static final Log logger = LogFactory.getLog(Poses.class);

    public static final String POSE_CHANNEL_BASE = "POSE_";

    private pose_t pose;

    private pose_t elaboratedPose = new pose_t();

    SuperdroidPose(String id)
    {
        super();
        String channel = POSE_CHANNEL_BASE + id;
        LCM lcm = LCM.getSingleton();
        lcm.subscribe(channel, new LCMSubscriber()
        {
            public void messageReceived(LCM lcm, String channel,
                    LCMDataInputStream ins)
            {
                try
                {
                    pose = new pose_t(ins);
                    //System.out.println("Pose: " + pose.pos[0] + ", " + pose.pos[1] + ", " + pose.pos[2]);
                    poseChanged();
                }
                catch (IOException e)
                {
                    logger.error("Error decoding pose_t message: "
                            + e.getMessage());
                }
            }
        });
    }
    
    public pose_t getPose()
    {
        pose_t newPose = pose; // grab a reference once

        if (newPose != null && newPose.utime > elaboratedPose.utime)
            elaboratedPose = Poses.elaborate(elaboratedPose, newPose);
        
        return elaboratedPose.copy();
    }

}
