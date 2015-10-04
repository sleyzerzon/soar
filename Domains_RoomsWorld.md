# RoomsWorld #

RoomsWorld is a simulated Soar Robot environment.  It is composed of a set of connected rooms, some of which contain blocks. One of the rooms is designated the storage room. The agent's task is to collect the blocks and move them to the storage room. The  agent can turn and move forward, and pick up and put down a block. The agent can only carry one block at a time. The agent's movement is continuous and takes time (it turns and moves at a fixed rate). The agent's perception is limited by a vision cone.

### Environment Properties ###
  * Deterministic, dynamic with respect to the agent's actions, continuous

### Download Links ###
  * [Linux 32-bit](http://web.eecs.umich.edu/~soar/downloads/Domains/RoomsWorld_9.3.4-Linux_32bit.zip)
  * [Linux 64-bit](http://web.eecs.umich.edu/~soar/downloads/Domains/RoomsWorld_9.3.4-Linux_64bit.zip)
  * [Windows 32-bit](http://web.eecs.umich.edu/~soar/downloads/Domains/RoomsWorld_9.3.4-Windows_32bit.zip)
  * [Windows 64-bit](http://web.eecs.umich.edu/~soar/downloads/Domains/RoomsWorld_9.3.4-Windows_64bit.zip)
  * [MacOS X](http://web.eecs.umich.edu/~soar/downloads/Domains/RoomsWorld_9.3.4-OSX.zip) (tested with Mountain Lion only)

### Associated Agents ###
  * The environment download contains agents.  Which agent loads depends on the configuration file you choose.

### Documentation ###
#### Component Overview ####

  1. April
    * [http://april.eecs.umich.edu/]
    * A major part of the project is the april libraries in the april.jar file. These contain all sorts of component and utility functions, including most of the simulator and 3D viewer code (Vis), and math routines and such.
    * Note that this is a fork of the official april code with heavy modifications. Acquiring the april code from the april folks will not work.
  1. libgrrc-java
    * This is a pure Java implementation of the libgrrc library used to communicate with a few of the robots such as Superdroid and the PackBot (Michigan GRRC interface). This allows the simulator code to interface directly with these robots.
  1. Soar
    * Soar SML 9.3.4 is used to interface the Simulator and Soar.
  1. soarrobot
    * The soarrobot project contains three Eclipse projects: SoarRobotServer for the main simulator components, SoarRobotTablet for a component that runs on the Android Galaxy tablet, and SoarRobotLibrary for shared code between the two.

#### Entry Point ####

The Application class (edu.umich.robot.Application) in the SoarRobotServer project serves as an entry point for the application. This program serves as a manager/container for most of the other components.

When the app is first run, the first thing it does is look at the configuration file supplied on the command line and determine if it is supposed to be in batch, or headless mode. More often it is not so it starts up an interactive GUI.

The headless/batch mode grabs configurations from a file (or multiple files) and runs them back to back to gather data over multiple runs.

The GUI uses a configuration file or prompts the user to find one. Defaults exist in config basic. 3x3.txt is a good one to start with.

#### Configuration Files ####

Configuration files are based on a configuration file utility in the april libraries. See april.config.Config and friends. ConfigUtil is used to load configuration files initially. Config files are key-value pairs with syntactic sugar to make things easy to read.
```
    splinters = seek;
    seek 
    {
        position = [2, -2];
        productions = "agents/robot3/clean-house-new.soar";
    }
```
This snippet has exactly three key value pairs and can be rewritten to emphasize this:
```
    splinters = [seek];
    seek.position = [2.0,-2.0];
    seek.productions = "agents/robot3/clean-house-new.soar";
```
The pound sign comments out the rest of a line. Brackets form blocks that prepend the block name to each key in the block. Nested blocks are allowed, along with keys that share the names as block names:
```
    alpha = foo;
    alpha
    {
        bravo
        {
            bravo = bar;
        }
        bravo = charlie;
    }
```
Yields:
```
    alpha = foo;
    alpha.bravo = charlie;
    alpha.bravo.bravo = bar;
```
All entries should end on a semicolon. All whitespace is ignored unless "it is in quotes like this." All values are actually stored as arrays and writing entries with only one value without array notation is optional. To encode multiple values, just add values in square brackets with commas separating things.
```
   encoding_a_single_value = foo;
   another_way = [foo];
   yet_another = [bar,];
   careful with spaces = becomes; # carefulwithspaces = becomes;
   values_too = "spaces must be in quotes";
   many = ["one", "two", "three"];
```
Everything is stored as a string. It is converted to a type when you read it. If you know that robot.pose is an array of floats, you'll call getDoubles(). If any fail to convert, you'll get an exception.

#### Configuration Content ####

Configuration files contain the following stuff:

  * `splinters`: an array of names of splinters
  * `SPLINTER_NAME`: a block for each splinter named in `splinters` with information about the splinter
    * `position`: location of the bot
    * `productions`: Soar productions to load
    * `OTHER`: There are a bunch more properties here controlled by guis
  * `image_path`: path to image file used for walls
  * `image_origin`: pixel marking the origin. The lower-left corner is used, I think
  * `meters_per_pixel`: pixel to meter unit conversion
  * `metadata`: block with information describing the metadata presented to Soar depending on where on the map it is in
    * `INTEGER`: starting at 0 and counting up, each room description in x, y, w, h pixels (relative to image).
    * `doors`: array indicating which rooms are doors
    * `closed`: array indicating which doors are closed
    * `locked`: array indicating which closed doors are locked, and with what key [door-id, key, door-id, key, ...etc...]
    * `objects`: block with object class/prototype descriptions with initial state
      * `OBJECT_NAME`: block with an object description
        * `size`: size of the object in meters: [width, height]
        * `color`: color from the set defined in java.awt.Color
        * `ANY`: any property with any value, shows up on input-link
      * `placed`: array with triples: object name, x, y where object name is a prototype defined in this block, and x, y where it is supposed to go. Example: [green\_cube, -3.2, 4.1]

Configuration files are usually started by hand but then tweaked and written out by the app. There are a few GUIs that mess around with various configuration files stuff in the app, and then write it out in a flat format. Nothing here is finished, it is all tailored to some research done in December.

When the app writes out the config file, it does so in the flat format where everything is explitly key/value pairs with array values of strings.

#### GUI Applicaiton ####

Most of the time, the simulator is started in the GUI/interactive mode as opposed to the data-collecting, headless, batch mode.

A simple swing application is started with a 3D view in to the environment and a list of robots and associated controllers.

Right click the controller's list to associate a Soar agent.

#### Robot Controllers ####

Robot controllers are things that, well, control the robots. The gamepad is a simple robot controller. Another one could be the web interface that was never ported over from the old sproom code. The main robot controller is Soar.

Robots are initially created with no controller. If their configuration includes productions, a Soar agent and debugger is created. If the gamepad is connected and the 2 button pressed, it takes over for whatever controller is on. Pressing 2 again pops it off returning control. Great for overriding behavior.

The only gamepad currently supported is the Logitech Wireless.

#### Feedback ####

Below the 3D scene is a feedback window with output sometimes useful for
debugging agents.

#### Controller ####

The GUI creates the main program controller. Many of the GUI actions interact with this controller. The controller manages Soar and Soar agents, robots and their controllers, the simulator and other components.

#### Event System ####

Many of the messages passed from controller to controller are though events. There are three types of event managers and events:

  * The main program event manager that serves events extending AbstractProgramEvent. Things like simulation start, stop, robots getting added or removed, controller changes, and time scale change are all handled here. There is only one of these event managers and it is in the Controller.
  * Robot Controller event managers, one in each robot controller. Each Soar agent and the gamepad have one of these to dish out events extending AbstractControlEvent. These events include drive commands and environment manipulation.
  * Robot Feedback event managers, one in each robot interface implemenation. These events extend AbstractFeedbackEvent and include the status of drive and effector commands.

The event types are switched using classes. Registering for a parent class registers you for all derived class event types. Events fire with a new instance of the event class. Event parameters are passed in to the event object. Events are (should be) immutable (actually I think some arent. Most are).

#### Simulator ####

A member of the Controller, the simulator is implemented in the april code and takes an initial configuration file to bootstrap.

The simulator takes the image, image origin, and meters per pixel and uses this data to generate simulated laser data and motion collision for the robots.

### IO link Specification ###
#### Input Link ####

At the top level there are the following identifiers:

  * _time_: World clock
  * _self_: Information about the robot
  * _configuration_: Simulator and robot settings and configuration
  * _waypoints_: Navigation point data as set by the robot
  * _received-messages_: Messages received by the robot
  * _area-description_: Information about the map based on the robot's current location
  * _objects_: Information about objects detected by the simulated object sensor
  * _lidar_: Laser range finder data

#### Time ####

  * seconds (_int_) (positive seconds): Use to measure elapsed time only.
  * microseconds (_int_) (microseconds 0 to 999999): Fraction of second, use to measure elapsed time only.

Use time.seconds and time.microseconds to measure elapsed time
only--they are not related to any other notion of system or wall-clock
time. Real current time is not implemented.

#### Configuration ####

  * Reports all current settings of output-link.configure.
  * _limits.velocity_
    * _linear_
      * maximum Maximum linear velocity in configured units.
      * minimum Minimum linear velocity in configured units.
    * _angular_
      * maximum Maximum linear velocity in configured units.
      * minimum Minimum linear velocity in configured units.
  * _geometry_
    * length
    * width
    * height
    * wheelbase
  * range-count (_int_): Number of coarse LIDAR ranges.
  * field-of-view (_angle-resolution type_): Object sensor field of view.
  * manipulation-distance-min (_float_): Minimum allowable distance for get-object (and commands like it) to work, also distance that objects are dropped.
  * manipulation-distance-max (_float_): Maximum allowable distance for get-object (and commands like it) to work.

Current interface configuration, configurable using the output link configure command. WME formats are identical to the command parameters except that there is no status reported here.

Additionally, some non-configurable information is presented on the configuration input link.

#### Self ####

  * name (_string_): Agent name.
  * area (_int_): Unique identifying number for area/room.
  * headlight (_string_) (on, off): Headlight state.
  * battery (_float_) (100.0-0.0): Percent battery life.
  * _pose_
    * x (_float_)
    * y (_float_)
    * z (_float_)
    * yaw (_angle-resolution type_) (-180 to 180, or -pi to pi)
    * x-velocity (_float_)
    * y-velocity (_float_)
    * z-velocity (_float_)
    * yaw-velocity (_float_) (positive for left turn, negative for right turn)

Self contains data about the robot itself.

self.pose gives information about the robot's position and velocity. This information is an aggregate of mapping and navigational subsystems (such as odometry or external localizers) between the interface and the
simulation or hardware. The origin can be configured using the configure command.

#### Waypoints ####

  * _waypoint_: One for each registered and enabled waypoint.
    * id (_any type_): User-defined unique identifier in user-defined type.
    * x (_float_)
    * y (_float_)
    * z (_float_)
    * distance (_float_) (positive): Distance to point.
    * yaw (_angle-resolution type_)  (-180 to 180, or -pi to pi): What absolute yaw to turn to to face the point.
    * relative-bearing (_angle-resolution type_) (-180 to 180, or -pi to pi): Which way to turn to face the point, positive means turn left. 0 means the point is straight ahead.
    * abs-relative-bearing (_angle-resolution type_) (0 to 180 or 0 to pi): Magnitude of relative bearing, represents how far to turn.

The Soar interface allows agents to define arbitrary points in space as named waypoints. Waypoints that are defined and enabled (they are enabled by default when defined) show up under the waypoints identifier. Distance to and useful angles are calculated to the waypoint.

#### Received Messages ####

  * _message_: One for each received message.
    * id (_int_): Unique identifying number for message.
    * from (_string_): Source of message (such as agent name).
    * _first_: Starts linked list.
      * word (_string_): The word data.
      * _next_ (_identifier_ or _string_ "nil" to end): Under this identifier are more word and next WMEs, terminated when next is "nil".

The messaging system places messages to and from the agent under the received-messages identifier. Each message has a unique id number (used with the messaging commands) and a from field and a linked list of space-delimited words ended when next is set to a string instead of being an identifier pointing to the next word.

For example, matching on "go to waypoint alpha" from alice:
```
    (<s> ^io.input-link.received-messages.message <m>)
    (<m> ^id 1 ^from alice ^first <w1>)
    (<w1> ^word go ^next <w2>)
    (<w2> ^word to ^next <w3>)
    (<w3> ^word waypoint ^next <w4>)
    (<w4> ^word alpha ^next nil)
```
#### Lidar ####

  * _range_: One per range to be measured (default is 5).
    * id (_int_): `-nranges / 2 to nranges / 2`, 0 is straight ahead (so, with 5, -2, -1, 0, 1, 2).
    * distance (_float_): Measured distance.
    * relative-bearing (_angle-resolution type_): Relative bearing to measured distance.

#### Area Description ####

  * id (_int_): Unique identifying number for area/room.
  * type (_string_) (door, room): Type of room.
  * light (_string_) (true, false): If the room is lit.
  * _gateway_ (multi-valued): One for each exit from the room.
    * id (_int_): Unique identifying number for gateway.
    * direction (_string_) (north, east, south, west): Which wall the gateway is on.
    * to (_int_): Area id on this side of gateway.
    * to (_int_): Area id on other side of gateway.
    * _door_: Details about the door
      * id (_int_): Door id number.
      * state (_string_) (open, closed, locked): State of door.
    * x (_float_): To center of wall
    * y (_float_): To center of wall
    * z (_float_): To center of wall
    * distance (_float_): To center of wall
  * _wall_
    * direction (_string_) (north, east, south, west): Which wall it is.
    * open (_string_) (true, false): If the entire length of wall is virtual (part of a larger room).
    * to (_int_) (potentially multi-valued): One area id for each area connected on the other side of this wall, usually only one area. Does not include this area.

Area descriptions currently assume rectangular rooms with walls and gateways aligned with the x/y axes. The rooms may be comprised of multiple areas connected with virtual or "open" walls so that more complex (non-rectangular) rooms may be represented as compositions of rectangular rooms.

#### Objects ####

  * _object_: One for each detected object
    * type (_string_) (block, brick, ball, player): Type of object.
    * id (_int_): Unique identifying number for objects.
    * name (_string_): Unique agent name if other player.
    * x (_float_): To center of object
    * y (_float_): To center of object
    * z (_float_): To center of object
    * distance (_float_): To center of object
    * visible (_string_) (true, false): Object stays on input link briefly after it is not visible.
    * Optional properties (see below).

Objects in the world are sensed with some of these application-specific properties.

  * color (_string_)
  * height (_string_)
  * smell (_string_)
  * shape (_string_)
  * weight (_float_)
  * diffusible (_string_) (true, false): Object can be diffused.
  * diffuse-wire (_string_) (red, green, blue): Which wire diffuses it.
  * diffused (_string_) (true, false): Object (diffusible only) is diffused.

#### Output Link ####

A note about status error: most commands that return with status error will have an error message attached for debugging. Example:

  * Parsing failure for set-velocity
```
    (<s> ^io.output-link.set-velocity <sv>)
    (<sv> ^status error ^message |set-velocity: Unable to parse linear-velocity| )
```
#### Drive Commands ####

Only _one_ of the drive commands can be issued at a time. If more than one command is issued, the system arbitrarily picks one unless one of them is estop, which overrides all other drive commands.

All parameters for the command must be on the same identifier of the command.

For example, all parameters for the command must be on the same identifier:
```
    (<s>  ^io.output-link <ol>)
    (<ol> ^set-velocity <sv>)
    (<sv> ^linear-velocity 1. ^angular velocity 1.) # Good
    
    (<ol>  ^set-velocity <sv1> ^set-velocity <sv2>)  # BAD: two separate commands
    (<sv1> ^linear-velocity 1.)    # BAD: missing angular velocity argument
    (<sv2> ^angular velocity 1.)   # BAD: missing linear velocity argument
```
For all drive commands:

  * throttle (_float_) A number from -1.0 to 1.0 representing percent throttle (or effort), negative is reverse.
  * linear velocity (_float_) Velocity, positive for forward or negative for reverse, in units defined by configuration. Subject to limitations defined in configuration.
  * angular velocity (_float_) Rotation, degrees or radians (as defined in configuration) per second, subject to limitations defined in configuration. Positive is left turn.
  * yaw (_float_ or _int_) Absolute yaw. Ranges outside of normal limits (such as 720 degrees or -3\*pi) are OK (and will be reduced internally to the usual ranges).
  * _motor_
    * left (_float_): Throttle, described above.
    * right (_float_): Throttle, described above.
    * status (_string_) (accepted, executing, complete, error)
  * _set-velocity_
    * linear-velocity
    * angular-velocity
    * status (_string_) (accepted, executing, complete, error)
  * _set-linear-velocity_
    * linear-velocity
    * status (_string_) (accepted, executing, complete, error)
  * _set-angular-velocity_
    * angular-velocity
    * status (_string_) (accepted, executing, complete, error)
  * _set-heading_
    * yaw (_any number type_)
    * status (_string_) (accepted, executing, interrupted, complete, error)
  * _set-heading-linear_
    * yaw (_any number type_)
    * linear-velocity (_float_)
    * status (_string_) (accepted, executing, interrupted, complete, error)
  * _stop_: Graceful stop.
    * status (_string_) (accepted, executing, interrupted, complete, error)
  * _estop_: Emergency/all stop.
    * status (_string_) (accepted, complete)

#### Effector and Robot State Commands ####

  * _get-object_: Pick up an object.
    * id (_int_): Target object id.
    * status (_string_) (accepted, complete, error)
  * _drop-object_: Drop a carried object.
    * id (_int_): Target object id.
    * status (_string_) (accepted, complete, error)
  * _diffuse-object_: Engage in a domain-specific diffuse task.
    * id (_int_): Target object id.
    * status (_string_) (accepted, complete, error)
  * _diffuse-object-by-wire_: Engage in a domain-specific diffuse task.
    * id (_int_): Target object id.
    * color (_string_) (red, green, blue): What wire to cut
    * status (_string_) (accepted, complete, error)
  * _set-headlight_: Turn headlight on/off.
    * setting (_string_) (on, off): Desired setting.
    * status (_string_) (accepted, complete, error)
  * _set-room-light_: Turn room light on/off.
    * setting (_string_) (on, off): Desired setting.
    * status (_string_) (accepted, complete, error)
  * _open-door_: Open a closed, unlocked door.
    * id (_int_): Target door id.
    * status (_string_) (accepted, complete, error)
  * _close-door_: Close an open door.
    * id (_int_): Target door id.
    * status (_string_) (accepted, complete, error)
  * _lock-door_: Lock a closed, unlocked door.
    * id (_int_): Target door id.
    * code (_int_): Passcode.
    * status (_string_) (accepted, complete, error)
  * _unlock-door_: Unlock a locked door.
    * id (_int_): Target door id.
    * code (_int_): Passcode.
    * status (_string_) (accepted, complete, error)

#### Waypoint Commands ####

  * _add-waypoint_: Enter a waypoint into the system, enabled.
    * id (_any type_): User-defined identifier string, type will be replicated on input-link.
    * x (_float_): Optional, default is 0.
    * y (_float_): Optional, default is 0.
    * z (_float_): Optional, default is 0.
    * status (_string_): (accepted, complete, error)
  * _remove-waypoint_: Remove the waypoint from the system.
    * id (_any type_)
    * status (_string_) (accepted, complete, error)
  * _enable-waypoint_: Enable a previously disabled waypoint.
    * id (_any type_)
    * status (_string_) (accepted, complete, error)
  * _disable-waypoint_: Disable an enabled waypoint.
    * id (_any type_)
    * status (_string_) (accepted, complete, error)

#### Communication Commands ####

Messages sent will appear on the input link after a one-decision-cycle delay.

The destination parameter for send-message is optional. If
omitted, message will be broadcast to all listeners, including the
sending agent (local echo).

The say destination is not currently implemented on windows
platforms.

The remove-message command uses the message id number, available at
io.input-link.self.received-messages.message.id

  * _send-message_
    * destination (_string_): Target of message or omit to broadcast to all available listeners.
    * _first_: See format and example under Self above.
    * status (_string_) (accepted, complete, error)
  * _remove-message_: Remove a specific message from the received-messages list.
    * id (_int_)
    * status (_string_) (accepted, complete, error)
  * _clear-messages_: Remove all messages from received-messages list.
    * status (_string_) (accepted, complete)

#### Configure Commands ####

  * length-units (_string_) (meters, feet): Units to use for length in I/O (globally).
  * angle-units (_string_) (radians, degrees): Units to use for angle measurement in I/O (globally).
  * angle-resolution (_string_) (int, float): Use integers or floating-point types for angle measurements in I/O (globally). Note that radians are always measured in floating-point.
  * linear-speed-units (_string_) (meters\_per\_sec, feet\_per\_sec, miles\_per\_hour): Units to use for linear speed measurement (globally).
  * angular-speed-units (_string_) (radians\_per\_sec, degrees\_per\_sec): Units to use for angular speed measurement (globally).
  * _pose-translation_: Translation to relocate origin of self.pose.
    * x (_float_)
    * y (_float_)
    * z (_float_)
  * _pid-gains_: Tune PID gains. Any parameters not given stay the same.
    * _heading_
      * p (_float_)
      * i (_float_)
      * d (_float_)
    * _angular_
      * p (_float_)
      * i (_float_)
      * d (_float_)
    * _linear_
      * p (_float_)
      * i (_float_)
      * d (_float_)
  * visible-time (_float_): Seconds objects stay on input-link after invisible.
  * status (_string_) (accepted, complete, error)

Configures the Soar interface. See the input link configuration identifier. Since there is only one status result, it is best to issue configure commands separately so that any errors will be easily traced.

### Associated Publications ###
  * [The Soar Cognitive Architecture](http://mitpress.mit.edu/catalog/item/default.asp?ttype=2&tid=12784)

### Developer ###
  * Jon Voigt
  * Miller Tinkerhess
  * Various others (project previously known as Soar Robot World and Sproom)

### Soar Versions ###
  * Soar 8, 9

### Language ###
  * C++