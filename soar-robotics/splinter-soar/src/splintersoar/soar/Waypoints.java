package splintersoar.soar;

import java.awt.Point;
import java.util.HashMap;
import java.util.Iterator;

import sml.*;

public class Waypoints {
	
	Agent agent;
	Identifier waypoints;
	double robotX;
	double robotY;
	double robotYaw;
	
	HashMap< String, Waypoint > waypointList = new HashMap< String, Waypoint >();
	
	class Waypoint
	{
		double waypointX;
		double waypointY;
		String name;
		
		Identifier waypoint;
		FloatElement absRelativeBearing;
		FloatElement distance;
		FloatElement relativeBearing;
		FloatElement yaw;
		
		public Waypoint( double waypointX, double waypointY, String name )
		{
			this.waypointX = waypointX;
			this.waypointY = waypointY;
			this.name = new String( name );
			
			createWmes();
			updateWmes();
		}
		
		String getName()
		{
			return name;
		}
		
		boolean equals( String other )
		{
			return other.equals( name );
		}
		
		void createWmes()
		{
			waypoint = agent.CreateIdWME( waypoints, "waypoint" );
			agent.CreateStringWME( waypoint, "id", name );
			agent.CreateFloatWME( waypoint, "x", waypointX );
			agent.CreateFloatWME( waypoint, "y", waypointY );
			
			distance = agent.CreateFloatWME( waypoint, "distance", 0 );
			yaw = agent.CreateFloatWME( waypoint, "yaw", 0 );
			relativeBearing = agent.CreateFloatWME( waypoint, "relative-bearing", 0 );
			absRelativeBearing = agent.CreateFloatWME( waypoint, "abs-relative-bearing", 0 );
		}
		
		void updateWmes()
		{
			System.out.println( "robotX, robotY, waypointX, waypointY, robotYaw, distanceValue, yawValue, relativeBearingValue" );
			System.out.print( robotX + ", " );
			System.out.print( robotY + ", " );
			System.out.print( waypointX + ", " );
			System.out.print( waypointY + ", " );
			System.out.print( robotYaw + ", " );

			double distanceValue = Point.distance( robotX, robotY, waypointX, waypointY );
			double yawValue = Math.atan2( waypointY - robotY, waypointX - robotX );
			while ( yawValue > Math.PI )
			{
				yawValue -= 360;
			}
			double relativeBearingValue = yawValue - robotYaw;
			
			System.out.print( distanceValue + ", " );
			System.out.print( yawValue + ", " );
			System.out.print( relativeBearingValue + "\n" );
			
			agent.Update( distance, distanceValue );
			agent.Update( yaw, Math.toDegrees( yawValue ) );
			agent.Update( relativeBearing, Math.toDegrees( relativeBearingValue ) );
			agent.Update( absRelativeBearing, Math.toDegrees( Math.abs( relativeBearingValue ) ) );
		}
		
		void enable()
		{
			if ( waypoint != null )
			{
				return;
			}
			
			createWmes();
			updateWmes();
		}
		
		void disable()
		{
			if ( waypoint == null )
			{
				return;
			}
			
			agent.DestroyWME( waypoint );
			
			waypoint = null;
			absRelativeBearing = null;
			distance = null;
			relativeBearing = null;
			yaw = null;
		}
	}

	Waypoints( Agent agent )
	{
		this.agent = agent;
	}
	
	void setRootIdentifier( Identifier waypoints )
	{
		this.waypoints = waypoints;
	}

	public void add( double waypointX, double waypointY, String name ) 
	{
		waypointList.remove( name );
		waypointList.put( name, new Waypoint( waypointX, waypointY, name ) );
	}

	public boolean remove( String name ) 
	{
		return waypointList.remove( name ) != null;
	}

	public boolean enable( String name ) 
	{
		Waypoint waypoint = waypointList.get( name );
		if ( name == null )
		{
			return false;
		}
		
		waypoint.enable();
		return true;
	}

	public boolean disable( String name ) 
	{
		Waypoint waypoint = waypointList.get( name );
		if ( name == null )
		{
			return false;
		}
		
		waypoint.disable();
		return true;
	}
	
	public void setNewRobotPose( double robotX, double robotY, double robotYaw )
	{
		this.robotX = robotX;
		this.robotY = robotY;
		this.robotYaw = robotYaw;
	}
	
	public void update()
	{
		Iterator< Waypoint > iter = waypointList.values().iterator();
		while ( iter.hasNext() )
		{
			Waypoint waypoint = iter.next();
			waypoint.updateWmes();
		}
	}
}
