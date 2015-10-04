Note: these notes are old (from 2007) and may contain stale information.

# General Notes and Links #

## Official ##

[MS Robotics Home](http://msdn2.microsoft.com/en-us/robotics/aa731520.aspx)

[C# Service Tutorials](http://msdn2.microsoft.com/en-us/library/bb896702.aspx)
  * (Tutorial 1) Note that when using a web browser to connect to services, it may ask for user/pass, it is expecting your Windows login.
  * (Tutorial 3) This does not appear to be true: ''The ServiceUri parameter specifies a location for the initial state document. This is expressed as a URI relative to the store directory. So in this case, the initial state document should be store/ServiceTutorial3.xml''
    * For me, it appeared in the base dir: '''C:\Microsoft Robotics Studio (1.5)\ServiceTutorial3.xml'''
  * (Tutorial 6) This does not appear to be correct: ''The above code uses a concise syntax for sending...''

[C# Simulation Tutorials](http://msdn2.microsoft.com/en-us/library/bb964568.aspx)
[Forums](http://forums.microsoft.com/MSDN/default.aspx?ForumGroupID=383&SiteID=1)

## Community ##

[Video Instruction](http://blogs.msdn.com/dawate/archive/2007/12/14/robotics-and-net-fundamentals-series.aspx)

## Misc ##

MS Robotics Studio simulation environment **DOES NOT WORK** on Windows XP 64

[Trevor Taylor MSRS Code Page](http://www.soft-tech.com.au/MSRS/)

# Soar MSRS Service #

## Maze Simulation Dependency ##

First, install Trevor Taylor's code:
  * http://www.soft-tech.com.au/MSRS/Downloads/QUTJul2007.zip
  * Unzip to C:\Microsoft Robotics Studio (1.5) so that it adds an Apps folder tree (and some other files in the MSRS root).

Run RebuildQUTApps.cmd to build Maze Simulation
  * Test simulation by running `RunIntro.bat` from within a robotics command prompt (so the environment is set-up)
  * Make sure this is running before proceeding.

## Soar service ##

Next, check out Soar project:
  * Check out to a new folder in Apps/QUT

Copy to bin folder from a running Soar installation or trunk:

```
CSharp_sml_ClientInterface.dll ElementXML.dll Java_SMLClientInterface.dll sml.jar smlCSharp.dll SoarJavaDebugger.jar SoarKernelSML.dll swt.jar
```

Open SoarMSRService solution and build

Run/Debug (F5) to start

Modify settings in `C:\Microsoft Robotics Studio (1.5)\Apps\QUT\Config\SoarMSR.Config.xml` after first run
  * That file is created on first run

## Configuration Settings ##
`C:\Microsoft Robotics Studio (1.5)\Apps\QUT\Config\SoarMSR.Config.xml`

  * AgentName
  * Productions
  * SpawnDebugger
  * HasRandomSeed
  * RandomSeed
  * DrivePower
    * Passed to input link
  * ReversePower
    * Passed to input link
  * StopTimeout
    * Passed to input link
  * BackUpTimeout
    * Passed to input link
  * TurnTimeout
    * Passed to input link
  * TimeoutVariance
    * Passed to input link
  * ObstacleAngleRange
    * Used with simple SickLRF obstacle detection
  * MinimumObstacleRange
    * Used with simple SickLRF obstacle detection
  * ObstacleRangeAverage
    * Used with simple SickLRF obstacle detection

## I/O documentation ##

input-link
  * sensors
    * bumper
      * front
        * was-pressed (true, false): true if the bumper changed state from not pressed to pressed since last update
        * pressed (true, false): true if the bumper is in the pressed state at the time of the update
      * rear
        * was-pressed (true, false): true if the bumper changed state from not pressed to pressed since last update
        * pressed (true, false): true if the bumper is in the pressed state at the time of the update
    * com
      * broadcast
        * next
        * value (string): not implemented communicated word or nil
          * next (identifier): not implemented points to structure same as self
    * sicklrf
      * obstacle (true, false): true if the area immediately in front of the robot is blocked
    * override
      * active (true, false): true if override should be used
      * drive-power
        * left (0.0 - 1.0): '''not implemented''' suggested power for left wheel
        * right (0.0 - 1.0): '''not implemented''' suggested power for right wheel
      * stop (true, false): '''not implemented''' stop all motion (not the simulation)
    * time (0.0 - ..): time in milliseconds since agent creation
    * random (0.0 - 1.0): random number different each update
  * config (see above)
    * power
      * drive (0.0 - 1.0): power to use for normal movement
      * reverse (0.0 - 1.0): power to use for reverse movement
    * delay
      * stop (0.0 - ..): base time to wait after a stop, milliseconds
      * reverse (0.0 - ..): base time to spend backing up, milliseconds
      * turn (0.0 - ..): base time to spend turning, milliseconds
      * variance (0.0 - ..): maximum amount to add to above delays, milliseconds
output-link
  * drive-power
    * left (0.0 - 1.0): power to left wheel (percentage of torque)
    * right (0.0 - 1.0): power to right wheel (percentage of torque)
    * stop (true): all stop (not quite a brake, but close)