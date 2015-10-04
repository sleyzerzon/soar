#summary Soar release notes archive
#



---

# 9.3.4 #
<pre>
Soar 9.3.4 Release Notes, June 2014<br>
===================================<br>
<br>
This release of Soar continues the 9.3 line. It includes the ability to<br>
include search control knowledge into chunks, a new form of integrated<br>
Tcl support, improved episodic and semantic memory and a large<br>
assortment of bug fixes and user interface enhancements.  All learning<br>
mechanisms are disabled by default.<br>
<br>
Soar can be downloaded by following the download link on the Soar home<br>
page at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Soar releases include source code, demo programs, and a number of<br>
applications that serve as examples of how to interface Soar to an<br>
external environment.  There is support for integrating Soar with C++,<br>
Java, Tcl and Python applications.  Many tools that aid in development<br>
of Soar programs are also available.  The newly expanded download<br>
section of the wiki allows you to browse and download all of the<br>
different distributions, tools and agents.<br>
<br>
[Help and Contact information]<br>
<br>
You can find many helpful resources on the Soar wiki at:<br>
<br>
https://code.google.com/p/soar/wiki/Home?tm=6<br>
<br>
To contact the Soar group, you may join and post to one of our mailing<br>
lists:<br>
<br>
For general Soar-related discussion and announcements:<br>
<br>
soar-group@lists.sourceforge.net<br>
<br>
For more technical developer discussion:<br>
<br>
soar-sml-list@lists.sourceforge.net<br>
<br>
Also, please do not hesitate to file bugs on our issue tracker:<br>
<br>
https://github.com/SoarGroup/Soar/issues<br>
<br>
To avoid redundant entries, please search for duplicate issues first.<br>
<br>
Important Changes for 9.3.4<br>
===========================<br>
<br>
--------<br>
Chunking<br>
--------<br>
<br>
(1) Soar can now include search control knowledge in chunks.<br>
<br>
This is done by keeping track of all *relevant* search control rules<br>
that produced preferences that influenced the selection of the operator<br>
that produced a result.  This set of preferences, called the<br>
context-dependent preference set (CDPS), will be backtraced through,<br>
possibly producing additional conditions in a chunk.  This setting is<br>
off by default but can be enabled by 'learn --desirability-prefs' or<br>
'learn -p'. This new feature can allow chunking to be used in situations<br>
that would previously produce over-general chunks. See the Soar Manual<br>
for more information.<br>
<br>
----------------------------<br>
Episodic and Semantic Memory<br>
----------------------------<br>
<br>
(1) Epmem search now uses less memory and computational resources.<br>
<br>
(2) Default epmem storage trigger is now each decision cycle instead of<br>
after output generation.<br>
<br>
(3) Soar now uses a new schema for both databases.  This means old<br>
databases can no longer be loaded.  A utility will soon be provided to<br>
convert from the old schema to the new one.<br>
<br>
(4) Users can now specify mathematical constraints to their smem<br>
retrieval cues.  See the manual for more information.<br>
<br>
(5) Both systems now have a new append setting that determines<br>
whether Soar will erase the database when starting or re-initializing<br>
via init-soar.<br>
<br>
- By default, APPEND IS ON FOR SEMANTIC MEMORY AND OFF FOR EPISODIC<br>
MEMORY.  The reasoning behind this is that the more common use case<br>
for episodic memory is to record the experiences within one agent<br>
run, while the more common use case for semantic memory is to build<br>
up or use a large pre-existing database of semantic knowledge.<br>
<br>
- With append-mode on, init-soar will have no effect on memories.<br>
<br>
- Due to certain technical reasons, episodic memory cannot append to an<br>
in-memory database, only semantic memory can.  If you want to record<br>
episodes across multiple agent runs, you must use an on-disk database.<br>
<br>
- Soar will now warn users that initializing Soar can clear their<br>
memories.<br>
<br>
- Soar will now notify users that there was no change to epmem/smem<br>
memories when reinitializing if append mode is on.<br>
<br>
(6) For both episodic and semantic memory, users may change<br>
path/database/append settings arbitrarily until they get what<br>
they'd like.  Soar will now no longer automatically switch to database mode the<br>
first time that you set the path.  Soar will not effect any changes until the<br>
database is used after an initialization. This happens either shortly after<br>
launch (on 1st use) or after a database initialization command is issued. To<br>
switch databases or storage settings after an agent has been running, set your<br>
new parameters and then perform an --init command.<br>
<br>
(7) SMem and epmem now have options for --on, --off, -e, -d, --enable,<br>
--disable parameters to epmem and smem to make interface consistent with<br>
other systems.<br>
<br>
(8) Soar now has an 'epmem --init' command.  This will close and<br>
re-initialize the database like 'smem --init'.<br>
<br>
--------------<br>
User Interface<br>
--------------<br>
<br>
(1) TclSoarLib<br>
<br>
- Soar now includes a dynamically loaded module that alters the Soar<br>
command line interface to allow you to use Tcl code on the command<br>
line and within your agent.  For example, users can use Tcl<br>
variables and functions directly within their soar code or make tcl<br>
function calls on the RHS of rules.<br>
<br>
- This mode is enabled via a new Soar command 'cli'.  For example,<br>
to load the new Tcl module, users can enter a 'cli tcl on' command,<br>
and Soar will look for a DLL in the same directory called<br>
TclSoarLib and load it if found.  Similarly, using an argument of Python<br>
will try to load PythonSoarLib, if it exists.  (which doesn't)<br>
<br>
- The full release notes for TclSoarLib are at the end of this document.<br>
<br>
(2) Soar will now give feedback on many commands and errors that it did<br>
not before.<br>
<br>
(3) Soar will now give more informative messages to various command line<br>
errors instead of just always saying "invalid attribute".<br>
<br>
(4) Commands now display their settings using a consistent display theme<br>
that should be much clearer.<br>
<br>
----------------------<br>
Reinforcement Learning<br>
----------------------<br>
<br>
(1) Reinforcement learning now has a new trace mechanism to help users<br>
understand why operators are selected.  When enabled, all proposed<br>
operators at all goal levels will be recorded in the trace, along with<br>
their names, other attribute-value pairs, and transition probabilities<br>
derived from their numeric preferences. The format in which the trace is<br>
printed is designed to be used by the program dot, which is part of the<br>
Graphviz suite.  See the help entry for rl for more information.<br>
<br>
(2) Reinforcement learning now has a new experimental delta-bar-delta<br>
mode based on the work in Richard S. Sutton's paper "Adapting Bias by<br>
Gradient Descent: An Incremental Version of Delta-Bar-Delta". Delta Bar<br>
Delta is implemented in Soar RL as a decay mode. It changes the way all<br>
the rules in the eligibility trace get their values updated. The key<br>
idea is that the agent uses meta parameters to keep track of how much a<br>
rule's RL value has been updated recently, and if a rule gets updates in<br>
the same direction multiple times in a row then subsequent updates in<br>
the same direction will have more effect. In some sense, DBD acts like<br>
momentum for the learning rate.<br>
<br>
(3) Users can now record a log file that contains every update to<br>
productions' reinforcement learning values with the 'update-log-path'<br>
setting.  For example, 'rl --set update-log-path rl_log.txt' would start<br>
recording a log to the file rl_log.txt<br>
<br>
(4) The Soar 'preferences' command now prints the actual probabilities<br>
for operators based on the current selection method. Soar previously<br>
always assumed softmax.<br>
<br>
---------<br>
Bug Fixes<br>
---------<br>
<br>
(1) Fixed issue 105 in which a rule with an empty RHS caused a crash in<br>
Soar.<br>
<br>
(2) Fixed issue 127 Soar no longer inaccurately classifies certain rule<br>
firings as operator proposals.  Instead of just looking for an attribute<br>
named operator in a rhs action, it checks to see if that id element<br>
points to a real state and hence is a real operator proposal.  If so, it<br>
gives i-support to all of the rhs make actions of that rule firing.<br>
<br>
(3) Fixed issue 130 in which Soar would exit with the error "Wanted to<br>
create a GDS for a WME level different from the instantiation<br>
level.....Big problems....exiting".<br>
<br>
(4) Fixed issue 132 in which memory corruption could occur when wma and<br>
epmem are both active.<br>
<br>
(5) Fixed issue 133 in which extremely low decay rates caused Soar to<br>
crash.<br>
<br>
(6) Fixed bug that resulted in epmem recording breaking after init-soar.<br>
Soar will now properly close and re-initialize epmem.<br>
<br>
(7) Fixed bug in epmem retrieval that could result in failed queries.<br>
<br>
(8) Fixed bug in smem that caused Soar to crash when attempting to print,<br>
visualize, get stats on the semantic or episodic memory databases after the<br>
settings pointing it to the correct database were configured but before it was<br>
first used.  It will now connect to the database before attempting any of<br>
those commands.<br>
<br>
(9) Fixed a segfault in smem, triggered when a previously unhashed value<br>
appeared in a neg-query.<br>
<br>
(10) Fixed bug in which LTIs with i-supported children were not properly<br>
forgotten.<br>
<br>
(11) Fixed bug in cli that caused crashes on certain commands like an<br>
empty echo command.<br>
<br>
(12) Fixed bug in cli identifier printing shortcut. (You can againg type<br>
"S1" to print S1 instead of print S1)<br>
<br>
(13) Fixed bug in which data was not being written to epmem/smem<br>
databases when using the cli.  It now shuts down and deletes kernel,<br>
allowing proper clean-up.<br>
<br>
(14) Fixed bug in which Python callbacks silently ignored exceptions.<br>
<br>
(15) Fixed memory leak in Eaters when a wme is replaced.<br>
<br>
(16) Fixed bug due to dubious logic in ParseXML that could cause working<br>
memory to grow unnecessarily large.<br>
<br>
(17) Fixed a bug in which very large messages could not be passed via an<br>
SML filter callback.<br>
<br>
-------------------------<br>
Building From Source Code<br>
-------------------------<br>
<br>
(1) The debug build is now the default.  Build script now uses --opt for<br>
an optimized build instead of --dbg.<br>
<br>
(2) Added option for static build (--static).<br>
<br>
(3) Build script for iOS updated for latest version.<br>
<br>
(4) Tcl SWIG interface updated and fully functional with latest version<br>
of Soar and the latest version of ActiveTcl (8.6).<br>
<br>
(5) Can now build C# SWIG wrappers.<br>
<br>
(6) Building soar without single compilation units (--no-scu) now works<br>
again.<br>
<br>
(7) On OSX, build process now uses clang/clang++.<br>
<br>
(8) Now has LSB support, which can facilitate compilation on certain<br>
*nix system.<br>
<br>
Other Changes in 9.3.4<br>
======================<br>
<br>
--------------<br>
User Interface<br>
--------------<br>
<br>
(1) Soar has a new C-based command line interface for Soar that supports<br>
multiple agents (called mcli).<br>
<br>
- Adds four new commands to manipulate agents: list, create, delete,<br>
switch<br>
<br>
- If more than one agent exists, the CLI will enter multi-agent<br>
mode, in which the CLI will print the output of all agents<br>
simulataneously, with each of their trace messages prefixed with the<br>
agent's color-coded name.<br>
<br>
- Caveats:  Currently color does not show up on windows machines. On<br>
linux and macs, colors are configured to look best on a dark<br>
background color.<br>
<br>
(2) Preferences command will now indicate whether a WME has different<br>
support at different levels, as well as whether the WME is linked to a<br>
state but has no support there.<br>
<br>
(3) When verbose mode is on, Soar will print the filename of each file<br>
being sourced as it sources them.<br>
<br>
(4) Soar now properly explains that a rule has bad RHS when it either it<br>
creates a variable that is not tested in a positive condition on the LHS<br>
(negative conditions don't count) or it passes an unbound variable to a<br>
rhs function.<br>
<br>
(5) The cli now prints the port used and what type of kernel it's<br>
creating.<br>
<br>
---------------<br>
Episodic Memory<br>
---------------<br>
<br>
(1) Soar now only prints out new king episode for an epmem search if a<br>
new episode has actually been crowned king (considered best).<br>
<br>
(2) Epmem search now uses most constraining values for ^before and<br>
^after (smallest for before, largest for after)<br>
<br>
(3) 'epmem --stats' now shows how many prev and retrieve commands have<br>
been made<br>
<br>
(4) Statistics for previous epmem queries once again available.<br>
<br>
--------<br>
Chunking<br>
--------<br>
<br>
(1) The settings 'enable-through-local-negations' and<br>
'disable-through-local-negations' have been renamed to be more succinct<br>
and consistent with other settings.  They are now 'local-negations' and<br>
'no-local-negations'.  The short version is still '-n' and '-N'.<br>
<br>
(2) After changing chunking settings, Soar will give feedback of all<br>
current settings.<br>
<br>
---------<br>
Bug Fixes<br>
---------<br>
<br>
(1) Fixed bug that occurred when epmem is queried with an LTI, which was<br>
promoted (stored into smem), in the middle of its most recent interval.<br>
Soar now sets the priority to its promotion time instead of the interval<br>
start time.<br>
<br>
(2) Fixed bug that resulted in epmem activation count not being properly<br>
tracked.<br>
<br>
(3) Fixed bug in epmem storage bug that would occur when it attempted to<br>
recurse through a Y-shaped linkage between wme's.<br>
<br>
(4) Fixed bug in smem where Soar would give users no feedback that<br>
certain smem -add commands were bad (for example smem -add {fdsfds})<br>
<br>
(5) Fixed working memory activation bug due to the fact that the test<br>
for the o-support set of a WME is not sufficiently tight. In particular,<br>
if one of the tested WMEs has i-support, but *that i-support is removed<br>
at the same time as a rule that depends on it fires*, the result of that<br>
rule will have that i-supported WME in it's o-set.<br>
<br>
(6) Fixed bug in which a newline would not be printed when sourcing a<br>
soar file in which a soar command is issued immediately after a<br>
production is loaded in that same file.<br>
<br>
(7) Fixed bug in default rules for A* search, when start == goal.  The<br>
change here is that the total-estimated-cost should be updated when<br>
estimated-cost != 0, and final-cost should be updated when<br>
estimated-cost == 0. This is changed from the original code where the<br>
total-estimated-cost is updated if path-cost != 0, which would fail if<br>
start == goal, where path-cost == estimated-cost == total-estimated-cost<br>
== final-cost == 0.  The change makes sense when you think about A* ()<br>
when the estimated cost is 0, that means you've reached the desired<br>
state, so the final cost is known.  Otherwise, the total cost is still<br>
an estimate, so the final cost is not stated and it's only a total<br>
*estimated* cost.<br>
<br>
(8) Fixed bug with calls to output handlers that occurred when command<br>
WMEs are deleted.<br>
<br>
(9) Fixed some bugs that resulted in Soar not adding a new line when it<br>
should have.  Handling of newlines is more consistent. (but still not<br>
perfect.)<br>
<br>
(10) Fixed bug in which Soar would not return an error message despite a<br>
command's parameters being invalid.<br>
<br>
(11) Fixed bug in Eaters and TankSoar when attempting to update an<br>
output's status twice.<br>
<br>
(12) Fixed bug caused by newline issues in jar manifest files. If a<br>
manifest file doesn't have a terminating newline, certain system setups<br>
will fail to read the last line, and the resulting jar file will be<br>
missing that line.<br>
<br>
(13) Fixed bug detecting correct python library on *nix platforms.<br>
<br>
(14) Fixed compiling bugs because of inconsistent mac/windows/linux<br>
linefeeds.<br>
<br>
(15) Fixed compile error in GDS debug code.<br>
<br>
-------------------------<br>
Building From Source Code<br>
-------------------------<br>
<br>
(1) Java SWIG source generation improved.<br>
<br>
(2) Scons no longer traverse hidden directories looking for targets to<br>
build<br>
<br>
(3) Fixed bug in which scons was not properly rebuilding sml_java when<br>
it should have.<br>
<br>
(4) Scons will no longer try to build java targets when swig is not<br>
installed<br>
<br>
(5) Fixed bug in which it would not properly copy files into directories<br>
that starts with a '.'<br>
<br>
(6) Project/Solution generation for MS Visual Studio now works better<br>
<br>
(7) Kernel now has doxygen setup files for automatic code help<br>
generation<br>
<br>
(8) Added --python option to scons for different python versions, which<br>
takes a path to the python executable. Scons will then use it to<br>
determine the appropriate python libraries for building the SML client.<br>
<br>
------------<br>
Code Changes<br>
------------<br>
(This section is really only really relevant to people who modify or<br>
interface<br>
to the source code.)<br>
<br>
(1) Significant refactoring and reorganization of epmem/smem code for<br>
clarity<br>
<br>
(2) Migrated to new episodic memory and semantic memory database schemas<br>
<br>
- Designed to be much easier to understand.  Database relations are<br>
now strict.<br>
<br>
- Eliminated the temporal hash table.  Epmem will now look up<br>
constants from four symbol lookup tables instead.<br>
<br>
- In general, epmem is now more similar to smem database, and both<br>
use same schema versioning representation.<br>
<br>
(3) Lots of new debugging facilities<br>
<br>
- SQL tracing<br>
<br>
- More debug messages in general<br>
<br>
- Many debug messages now print out things using the same function<br>
rather than an assortment of different ways (cout, printf,<br>
callbacks, etc.)<br>
<br>
(4) Added new CLI utility functions to support more consistent printing<br>
<br>
(5) Added new Soar Instance class that acts as a consistent hub across<br>
various SML configurations. It is a singleton object that corresponds to<br>
an instance of the soar dll, as opposed to an agent or kernel.  Added<br>
initially to support the TclSoarLib dll, but could also be used for<br>
other things as well.<br>
<br>
(6) An internal output manager to simplify and consolidate the various<br>
way that different portions of the Soar code produce output.  It also<br>
allows us to re-direct output dynamically to other listeners (or<br>
multiple ones), for example a database.<br>
<br>
(7) Removed code for various experimental modes created by previous<br>
graduate students.<br>
<br>
(8) Scons updated and hacked to support Visual Studio 2013 until<br>
official version does.<br>
<br>
(9) CppUnit, PCRE removed.  STLSoft replaced with simpler timer<br>
implementation.<br>
<br>
(10) All support of obsolete unary parallel and binary parallel<br>
preferences now removed<br>
<br>
(11) TestSoarPerformance now has some #defines to allow you to set a<br>
different test agent, change the number of trials and control the amount<br>
of information printed.<br>
<br>
</pre>


---

# 9.3.3 #
<pre>
Soar 9.3.3 Release Notes, June 2014<br>
===================================<br>
<br>
This release is identical to Soar 9.3.4, but had a few issues related to<br>
episodic and semantic memory database initialization that were resolved in<br>
9.3.4.<br>
<br>
</pre>


---

# 9.3.2 #
<pre>
Soar 9.3.2 Release Notes, April 2012<br>
===================================<br>
<br>
This release of Soar continues the 9.3 line which includes modules for<br>
reinforcement learning (RL), episodic memory (EpMem), and semantic<br>
memory (SMem), as well as everything from previous versions of Soar. It<br>
includes a new "forgetting" mechanism, several bug fixes, a new<br>
distribution<br>
structure, and a simplification of how libraries are built in Soar.  All<br>
learning mechanisms are disabled by default.<br>
<br>
Soar can be downloaded from the Soar home page at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Soar releases include source code, demo programs, and a number of<br>
applications that serve as examples of how to interface Soar to an<br>
external environment.  There is support for integrating Soar with C++,<br>
Java, and Python applications.  Many tools that aid in development of<br>
Soar programs are also available.  The newly expanded download section<br>
of the wiki allows you to browse and download all of the different<br>
distributions, tools and agents.<br>
<br>
[Help and Contact information]<br>
<br>
Please do not hesitate to contact the Soar group by sending mail to:<br>
<br>
For general Soar-related discussion and announcements:<br>
soar-group@lists.sourceforge.net<br>
<br>
For more technical developer discussion:<br>
soar-sml-list@lists.sourceforge.net<br>
<br>
Please do not hesitate to file bugs on our issue tracker (search for<br>
duplicates first):<br>
<br>
http://code.google.com/p/soar/issues/list<br>
<br>
[Important Changes for 9.3.2]<br>
<br>
Soar's build process has undergone significant modifications, and as a<br>
result<br>
several things have changed in the binary release:<br>
<br>
1. Instead of separate libraries for ClientSML, ConnectionSML,<br>
ElementXML,<br>
KernelSML, and the Soar Kernel, there is now only a single Soar library.<br>
On<br>
Windows this is Soar.dll, on Linux libSoar.so, and on OSX libSoar.dylib.<br>
This<br>
means that when you are building SML clients, you only need to link to<br>
that<br>
single library. The SML header files have not changed: you still need to<br>
include<br>
sml_Client.h.<br>
<br>
2. Due to the library merge, SML clients no longer have to locate the<br>
KernelSML<br>
library at run time. Therefore, all references to the location and name<br>
of that<br>
library, such as DefaultLibraryLocation, have been removed from the SML<br>
API.<br>
Specifically:<br>
- The set-library-location CLI command has been removed<br>
<br>
- The following functions have been removed:<br>
sml::Kernel::GetDefaultLibraryName<br>
sml::Kernel::GetLibraryLocation<br>
<br>
- The library location argument has been removed from the signatures<br>
of the<br>
following functions:<br>
sml::Agent::SpawnDebugger (see below)<br>
sml::Kernel::CreateKernelInCurrentThread<br>
<br>
sml::Kernel::CreateKernelInNewThread<br>
<br>
sml::Kernel::CreateEmbeddedConnection<br>
<br>
3. The sml::Agent::SpawnDebugger function no longer uses the KernelSML<br>
library<br>
location to find SoarJavaDebugger.jar. Instead,<br>
1. If the path of the jar (not of the containing directory) is<br>
provided as<br>
an explicit argument, it is used.<br>
2. Otherwise, if SoarJavaDebugger.jar is in the current working<br>
directory,<br>
then that jar is used.<br>
2. Otherwise, if the environment variable SOAR_HOME is defined and<br>
points<br>
to the directory that contains SoarJavaDebugger.jar, then that jar is<br>
used.<br>
3. Otherwise the search fails and the function returns false.<br>
<br>
4. Due to the reduction in the number of files, we have merged the lib<br>
directory<br>
into the bin directory. The Soar library and the SWIG wrapper libraries<br>
now<br>
reside in the same directory as the executables. This makes life easier<br>
for<br>
Windows users running programs such as TestCLI or the Java debugger,<br>
because<br>
Windows searches the current directory for library dependencies by<br>
default.<br>
<br>
5. Linux and OSX users no longer need to set LD_LIBRARY_PATH and<br>
DYLD_LIBRARY_PATH to point to the bin directory when running the native<br>
executables in the distribution, such as TestCLI or TestSoarPerformance.<br>
The<br>
relative location of the library has been hard-coded into those<br>
executables<br>
using the rpath and install_name mechanisms in Linux and OSX,<br>
respectively.<br>
However, those environment variables must still be set for running the<br>
java<br>
debugger, as required by the JVM.<br>
<br>
6. SoarSuite is now extremely streamlined.  It is intended for users<br>
who want to either (1) just run Soar or (2) plug-in the latest version<br>
of Soar into their current work.  It only contains the Soar kernel, the<br>
Soar Java Debugger, Visual Soar, the Soar Manual, the default rules,<br>
unit test programs and the Soar header files.<br>
<br>
7. The Soar Tutorial now has its own download.  It contains the Soar<br>
kernel,<br>
the Java Debugger, Eaters, TankSoar, the Soar Manual, the Soar Tutorial,<br>
VisualSoar, and all the introductory agents.<br>
<br>
8. Nearly everything we provide now has stand-alone versions, including<br>
TankSoar<br>
and Eaters.  All agents, tools and environments now have their own wiki<br>
pages<br>
with thorough descriptions and individual downloads.  We highly<br>
encourage<br>
people to make their own work available on the Soar wiki.<br>
<br>
9. The Soar SVN repository has went through major restructuring and<br>
clean up<br>
to support these changes.<br>
<br>
10. When a tie impasse occurs, the substate is augmented with two new<br>
WME<br>
classes, which document operators that do not have numeric-indifferent<br>
preferences associated with them. These augmentations are<br>
"non-numeric" and "non-numeric-count" and operate analogously to the<br>
existing "item" and "item-count" augmentations.<br>
<br>
11. We have added a new forgetting mechanism to Soar to automatically<br>
excise chunks via the base-level decay model. It is controlled via the<br>
"rl" command using the "apoptosis" parameters. This functionality<br>
helps develop learning agents that can persist for long periods of<br>
time, accumulating a large amount of procedural knowledge while<br>
bounding memory consumption. Version 9.3.1 of Soar included forgetting<br>
of WMEs via the "wma" command, which served to bound procedural<br>
matching and episodic reconstruction. WME forgetting worked well with<br>
automatic semantic encoding via the "mirroring" parameter of the<br>
"smem" command. Together, these forgetting policies facilitate the<br>
development of long-living, learning agents that remain reactive in<br>
real-time environments.<br>
<br>
A more comprehensive list of changes is below.<br>
<br>
DETAILED CHANGELIST<br>
===================<br>
<br>
The scons build process now works in Windows and is the preferred way to<br>
build<br>
Soar on all three supported platforms.<br>
<br>
The old Visual Studio project and solution files have been removed from<br>
the<br>
distribution. Scons can generate those files to make it easier for<br>
Windows users<br>
to debug the kernel. Scons is now also included in all source<br>
distributions to<br>
simplify building Soar.<br>
<br>
Ant is no longer needed to build the Java debugger.<br>
<br>
Soar-specific environment variables are no longer needed to compile Soar<br>
on<br>
Linux and OSX.<br>
<br>
Improved documentation and completely restructured Soar web page and<br>
wiki.<br>
A lot of outdated information has been updated or removed.<br>
<br>
Demo agents have been removed from the Java debugger and SoarSuite. They<br>
are now<br>
simply packaged in the Agents directory of the SoarTutorial.  They are<br>
also<br>
available for individual download.<br>
<br>
New chapters have been added to the tutorial for episodic and semantic<br>
memory.<br>
<br>
VisualSoar has been removed from Soar core but can still be downloaded<br>
from its<br>
own wiki page.<br>
<br>
SoarSuite now only includes the Soar kernel, the Soar Java Debugger,<br>
Visual<br>
Soar, the Soar Manual, the default rules, unit test programs and the<br>
Soar<br>
header files.<br>
<br>
Added a new download specifically for the SoarTutorial.  It includes the<br>
Soar kernel, the Java Debugger, Eaters, TankSoar, the Soar Manual, the<br>
Soar Tutorial documents, VisualSoar, and all the introductory agents.<br>
<br>
There are now stand-alone versions of TankSoar and Eaters.  They include<br>
self-contained copies of the Soar kernel and the debugger and can be run<br>
from<br>
any directory.<br>
<br>
A large variety of agents are now available for individual download.<br>
They<br>
each have their own wiki page with a complete description.<br>
<br>
Default rules updated for Soar 9.<br>
<br>
Soar now works with the OpenJava JDK, which is what is installed by<br>
default on<br>
Ubuntu linux.<br>
<br>
All releases for the various platforms now have identical naming and<br>
directory<br>
structures.  This should make things much more consistent and less<br>
brittle.<br>
<br>
Issue 78: The selection probabilities outputted by the "pref" command<br>
now<br>
make sense for all exploration strategies, not just softmax.<br>
<br>
Issue 92: Removed the "run forever" command. "run" without any arguments<br>
will<br>
achieve teh same thing.<br>
<br>
Issue 93: Alias command now adheres to the documentation.  First<br>
argument is<br>
once again the alias name.<br>
<br>
Issue 95: Certain types of comments on the RHS of productions no longer<br>
produces an error in the parser.<br>
<br>
Issue 98: Soar will no longer automatically change the<br>
indifference-selection<br>
policy to episilon-greedy when turning reinforcement learning on.  This<br>
may<br>
break the expected behavior of some agents, so users should make sure to<br>
set<br>
the policy the want explicitly.  All of the agents in our repository<br>
have been<br>
updated to explicitly set it to epsilon-greedy after they turn RL on.<br>
<br>
Issue 98: To increase performance, Soar will no longer discount updates<br>
over<br>
subgoals as though they were gaps, i.e. HRL-Discount is now off by<br>
default.<br>
<br>
Issue 98: By default, Soar will no longer create a chunk that only<br>
differs<br>
from another rule in the value of the numeric indifferent preference,<br>
i.e.<br>
Chunk-Stop is now on by default.<br>
<br>
Issue 102: CLI now strips out comments at the end of the line properly.<br>
<br>
Issue 103: Watch-wmes command now works again.  You can now set up<br>
arbitrary<br>
filters on which wmes are printed out.<br>
</pre>

---

# 9.3.1 #
<pre>Soar 9.3.1 Release Notes, June 2011<br>
===================================<br>
<br>
This release of Soar continues the 9.3 line which includes modules for<br>
reinforcement learning (RL), episodic memory (EpMem), and semantic<br>
memory (SMem), as well as everything from previous versions of Soar. It<br>
includes many bugfixes, a major reworking of working memory activation,<br>
and under-the-hood improvements to support very long agent runs. All<br>
learning mechanisms are disabled by default.<br>
<br>
Soar can be downloaded from the Soar home page at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Soar releases include source code, demo programs, and a number of<br>
applications that serve as examples of how to interface Soar to an<br>
external environment.  There is support for integrating Soar with C++,<br>
Java, and Python applications.  Many tools that aid in development of<br>
Soar programs are also included.<br>
<br>
[Help and Contact information]<br>
<br>
Please do not hesitate to contact the Soar group by sending mail to:<br>
<br>
For general Soar-related discussion and announcements:<br>
soar-group@lists.sourceforge.net<br>
<br>
For more technical developer discussion:<br>
soar-sml-list@lists.sourceforge.net<br>
<br>
Please do not hesitate to file bugs on our issue tracker (search for<br>
duplicates first):<br>
<br>
http://code.google.com/p/soar/issues/list<br>
<br>
[Important Changes for 9.3.1]<br>
<br>
Inequality comparisons on string constants and identifiers are now<br>
evaluated lexically rather than always returning false. Watch out--we<br>
found that this changed the behavior on some agents that relied on the<br>
old, broken behavior.<br>
<br>
Improvements in stability and performance for the learning modules that<br>
rely on databases (EpMem and SMem). Sqlite, the backing database, has<br>
been upgraded to 3.7.4. Database schemas have been changed breaking<br>
backwards compatibility.<br>
<br>
The Soar kernel and SML interface now uses 64-bit integers for things<br>
like WME values, timetags, stats, counters, and rete data structures.<br>
Motivation for these changes is current research involving long-running<br>
agents that were rolling over 32-bit integers in some instances. The<br>
client interface is mostly backwards compatible but some errors or<br>
warnings (because of loss of precision) may be encountered.<br>
<br>
Working memory activation based forgetting is now stable and<br>
efficient. However, parameters controlling its behavior are still<br>
under experimentation and will likely change in the future.<br>
<br>
The command line interface has been updated to behave much more like it<br>
behaved before 8.6.0. The parsing is simplified and follows most of the<br>
rules of the Tcl language and its behavior is now much easier to<br>
validate (and many tests have been added). This should be mostly<br>
transparent to the end user.<br>
<br>
The help command now displays command documentation text directly<br>
in the terminal again, similar to the pre-9.0 behavior. The help<br>
text can now be automatically pulled from the wiki pages, making<br>
it much easier to keep up to date. See Core/CLI/gen_cli_help.sh for<br>
more details.<br>
<br>
The Soar manual has been revised significantly. New sections<br>
describing reinforcement learning, episodic memory, and semantic<br>
memory have been added, and other sections have been updated to<br>
reflect recent kernel changes.<br>
<br>
A more comprehensive list of changes is below.<br>
<br>
Prerequisites<br>
=============<br>
<br>
[Platforms]<br>
<br>
Officially supported platforms are:<br>
<br>
* Windows XP (32 bit), Vista & 7 32/64 bit<br>
* Ubuntu Linux 10/11 32/64 bit<br>
* Mac OS X 10.6 64 bit<br>
<br>
Other platforms may work but have not specifically been tested, you can<br>
try binaries or compile from source:<br>
<br>
http://code.google.com/p/soar/wiki/Build<br>
<br>
Due to path length limits on some versions of Windows, you may need to<br>
extract Soar to C:\ -- watch for errors during the extraction process.<br>
<br>
[Java]<br>
<br>
Java is required to use the Java applications distributed with Soar,<br>
including the debugger. Java must be available in the path. Some<br>
operating systems already have Java installed, but be aware that we only<br>
develop and test our applications using the Sun Java runtime environment<br>
6 and other JVMs (such as GCJ) may not work:<br>
<br>
http://www.oracle.com/technetwork/java/javase/downloads/index.html<br>
<br>
IMPORTANT NOTE ABOUT 64-BIT BINARIES: 32-bit Java virtual machines<br>
cannot load 64-bit shared libraries. Please download appropriate<br>
binaries for your installed virtual machine. Attempting to open tools<br>
such as the Soar Java Debugger with the wrong JVM can cause the java<br>
process to hang on Windows (use task manager to terminate it).<br>
<br>
OS X USERS CAN CHECK/SELECT WHAT JVM THEY ARE USING WITH A UTILITY: Use<br>
spotlight or look for the utility application "Java Preferences". More<br>
information:<br>
<br>
http://developer.apple.com/java/faq/<br>
<br>
[Python]<br>
<br>
The included Python libraries support Python versions 2.6.  Your<br>
installed Python architecture (32- or 64-bit) must be the same as the<br>
binaries you download.<br>
<br>
DETAILED CHANGELIST<br>
===================<br>
<br>
Bugfixes<br>
--------<br>
<br>
Issue 65: chunks being built despite justifications in backtrace when<br>
creating result on super-superstate.<br>
<br>
Issue 70: Weird printing for strings that look like identifiers<br>
<br>
Issue 75: Incorrect id printed in GDS removal messages<br>
<br>
Issue 77: Lots of duplicate code in sml_KernelHelpers.cpp<br>
<br>
Issue 79: Print command now works as documented.<br>
<br>
GDS trace output now reports state number instead of internal level.<br>
<br>
Connecting to a remote kernel via process number overflows a short.<br>
<br>
Major performance fix in semantic memory when validating long-term<br>
identifiers on rules with lots of actions.<br>
<br>
vsnprintf_with_symbols was not null-terminating strings converted from<br>
ids causing overruns.<br>
<br>
Client SML identifier symbol bug fixed that was causing crashes<br>
(r12087).<br>
<br>
Crash caused by disabling output link change tracking.<br>
<br>
Memory leak caused during chunking (r12151).<br>
<br>
GDS issue where multiple references to removed states weren't all<br>
getting cleared.<br>
<br>
Segfault when storing a wme in epmem whose value was an identifier and<br>
id a long-term identifier that had never been stored in epmem.<br>
<br>
Segfault that would occur when a wme is added and removed in the same<br>
phase<br>
<br>
Major bugfix in interaction between epmem and smem.  (r12411)<br>
<br>
WMA bug wherein preferences in an i-supported wme's cached o-set were<br>
getting deallocated.<br>
<br>
Code cleanup and maintenance fixes throughout the code.<br>
<br>
<br>
Episodic & Semantic Memory Changes<br>
----------------------------------<br>
<br>
Semantic and episodic memory retrievals can now produce chunks as<br>
opposed to only justifications.<br>
<br>
Various experimental activation behaviors added to semantic memory.<br>
Activation is now represented as a real number.  As a result, the<br>
database schema was changed, breaking backwards compatibility.  A<br>
frequency-based activation mode is introduced.<br>
<br>
An experimental merge parameter added to semantic and episodic<br>
memories allowing modification of long-term identifiers in working<br>
memory.<br>
<br>
An experimental parameter added to episodic memory that controls how<br>
cue wmes are ordered during graph matching.<br>
<br>
<br>
Command Line Interface Changes<br>
------------------------------<br>
<br>
-g/--gds flag added to the watch command for watching only GDS<br>
messages.<br>
<br>
Printing productions with print command now displays name of file<br>
production was sourced from.<br>
<br>
It is now possible to disable per-cycle stat tracking.<br>
<br>
-d flag added to stat which makes it print only the current decision<br>
count.<br>
<br>
The TestCLI program has been rewritten, is much cleaner and a good<br>
example of a lightweight debugger.<br>
<br>
Added check to prevent crash when disconnecting Java debugger from<br>
remote Soar when not connected.<br>
<br>
Added new max-dc-time command to interrupt kernel execution after a<br>
long decision cycle.<br>
<br>
<br>
New SML Applications<br>
--------------------<br>
<br>
RLCLI: a simple debugger for RL experiments.<br>
<br>
SoarQNA: facilitates agent access to external knowledge stores via the<br>
IO system.<br>
<br>
Liar's Dice probability calculator.<br>
<br>
<br>
Miscellaneous Changes<br>
---------------------<br>
<br>
Visual Studio 2010 migration started, not officially supported yet.<br>
<br>
New features and fixes in SMLJava library.<br>
<br>
Experimental JMX API interface added for debugger for Soar IDE<br>
integration.  Not officially supported.<br>
<br>
Support for swig 2 added.<br>
<br>
Build procedure cleanup, some stuff wasn't getting built or cleaned<br>
correctly.  Some issues still exist but can be worked around by make<br>
clean && make.<br>
<br>
Jars all target Java 1.5 for better compatibility.<br>
<br>
Output-link change tracking is now disabled by default until the<br>
output-link identifier is requested unless the user explicitly enables<br>
it.<br>
<br>
GDS stats added to stats xml output.<br>
<br>
Lots of changes to stats reporting especially with respect to timers<br>
and time per decision cycle.  Precision increased in many places.<br>
Configurable at runtime using timers command.<br>
<br>
Experimental support to discard learned chunks that are duplicates of<br>
existing RL rules modulo numeric preference value.<br>
</pre>

---

# 9.3.0 #
<pre>Soar 9.3.0 Release Notes, March, 2010<br>
=====================================<br>
<br>
This release of Soar includes modules for reinforcement learning<br>
(introduced in 9.0), episodic memory (introduced in 9.1), and semantic<br>
memory (introduced in 9.2), along with many new features and stability<br>
updates. All learning mechanisms are now disabled by default.<br>
<br>
Soar can be downloaded from the Soar home page at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Soar releases include source code, demo programs, and a number of<br>
applications that serve as examples of how to interface Soar to an<br>
external<br>
environment.  There is support for integrating Soar with C++, Java, and<br>
Python applications.  Many tools that aid in development of Soar<br>
programs<br>
are also included.<br>
<br>
[Reinforcement Learning (RL)]<br>
<br>
RL is the architectural integration of reinforcement learning with Soar.<br>
The RL mechanism will automatically tune the values of<br>
numeric-indifferent<br>
preference rules based on reward generated while a Soar agent executes.<br>
These values represent the expected discounted sum of future rewards the<br>
agent will receive if it selects that operator for states matched by the<br>
condition of the rule. See the RL manual and tutorial in Documentation.<br>
<br>
To see RL in action right away, try the Water Jug RL demo (it can be<br>
loaded<br>
from the Demos->Water Jug menu in the Java Debugger). To see the effects<br>
of<br>
RL, run it to completion, then init-soar and run it again.  Repeat 4-5<br>
times to see it reach optimal behavior. (Note: the agent may<br>
occasionally<br>
perform non-optimal behavior even after it has converged because of its<br>
exploration policy. See the RL manual and tutorial for details).<br>
<br>
[Episodic Memory (EpMem)]<br>
<br>
EpMem is a task-independent, architectural integration of an artificial<br>
episodic memory with Soar. The EpMem mechanism will automatically record<br>
episodes as a Soar agent executes. These episodes can later be queried<br>
and<br>
retrieved in order to improve performance on future tasks. See the EpMem<br>
manual for details.<br>
<br>
[Semantic Memory (SMem)]<br>
<br>
SMem is a task-independent, architectural integration of an artificial<br>
semantic memory with Soar. The SMem mechanism facilitates deliberate<br>
recording and querying of semantic chunks as a Soar agent executes.<br>
<br>
[Help and Contact information]<br>
<br>
Please do not hesitate to contact the Soar group by sending mail to:<br>
<br>
For general Soar-related discussion and announcements:<br>
soar-group@lists.sourceforge.net<br>
<br>
For more technical developer discussion:<br>
soar-sml-list@lists.sourceforge.net<br>
<br>
Please do not hesitate to file bugs on our issue tracker (search for<br>
duplicates first):<br>
<br>
http://code.google.com/p/soar/issues/list<br>
<br>
[Important Changes]<br>
<br>
There have been performance, correctness, and stability improvements<br>
across<br>
the board, especially with the learning mechanisms, which are all<br>
disabled<br>
by default.<br>
<br>
Long-term identifiers are now integrated in all symbolic memory systems.<br>
More information on long-term identifiers can be found in section 4.2 of<br>
the Soar-SMem manual in the documentation folder.<br>
<br>
The allocate command has been added to Soar to allow agents to set aside<br>
memory before starting a run so that the cost of allocation is not<br>
incurred<br>
during the run.<br>
<br>
The probability of selection for proposed operators is now included in<br>
the<br>
preferences command output, helpful for debugging agents using<br>
indifferent<br>
selection or RL, or agents that have complex operator preference<br>
semantics.<br>
<br>
SML tracks changes on the Soar output-link in order for a number of<br>
useful<br>
functions to perform correctly. This change tracking comes at a cost and<br>
may now be disabled for a significant increase in performance -- even<br>
for<br>
agents that do not use the output-link. See the new output link guide<br>
for<br>
more information ("Examine in Detail" is the only option when disabling<br>
change tracking):<br>
<br>
http://code.google.com/p/soar/wiki/SMLOutputLinkGuide<br>
<br>
SML often opens ports when a Kernel is created. This behavior has been<br>
extended so that it can bind to any available port, and to use local<br>
sockets or local pipes with names based on the process id, fixing a lot<br>
of<br>
issues people were having. Complete documentation is included in the<br>
ClientKernel header file.<br>
<br>
A more comprehensive list of changes is below.<br>
<br>
Prerequisites<br>
=============<br>
<br>
[Platforms]<br>
<br>
Officially supported platforms include 32- and 64-bit versions of these<br>
systems:<br>
<br>
* Windows XP, Vista, 7<br>
* Ubuntu Linux 9.10<br>
* Mac OS X 10.5, 10.6<br>
<br>
Other platforms may work but have not specifically been tested, you can<br>
try<br>
binaries or attempt to compile from source:<br>
<br>
http://code.google.com/p/soar/wiki/Build<br>
<br>
Due to path length limits on some versions of Windows, you may need to<br>
extract Soar to C:\ -- watch for errors during the extraction process.<br>
<br>
[Java]<br>
<br>
Java is required to use the Java applications distributed with Soar,<br>
including the debugger. Java must be available in the PATH. Some<br>
operating<br>
systems already have Java installed, but be aware that we only develop<br>
and<br>
test our applications using the Sun Java runtime environment 6 and other<br>
JVMs (such as GCJ) may not work:<br>
<br>
http://developers.sun.com/downloads/top.jsp<br>
<br>
IMPORTANT NOTE ABOUT 64-BIT BINARIES: 32-bit Java virtual machines<br>
cannot<br>
load 64-bit shared libraries. Please download appropriate binaries for<br>
your<br>
installed virtual machine. Attempting to open tools such as the Soar<br>
Java<br>
Debugger with the wrong JVM can cause the java process to hang on<br>
Windows<br>
(use task manager to terminate it).<br>
<br>
OS X USERS CAN CHECK/SELECT WHAT JVM THEY ARE USING WITH A UTILITY: Use<br>
spotlight or look for the utility application "Java Preferences". More<br>
information:<br>
<br>
http://developer.apple.com/java/faq/<br>
<br>
[Python]<br>
<br>
The included Python libraries support Python 2.6. Your installed Python<br>
architecture (32- or 64-bit) must be the same as the binaries you<br>
download.<br>
<br>
Changes for 9.3.0<br>
=================<br>
<br>
We have moved some of our development hosting over to Google Code,<br>
including the wiki and issue tracker. Please check it out at<br>
<br>
http://soar.googlecode.com<br>
<br>
Much more documentation and example code is on the Google Code wiki.<br>
<br>
A new "smem --init" command has been introduced for reinitialization of<br>
all<br>
symbolic memory systems.<br>
<br>
RL: Greatly improved template performance.<br>
<br>
RL: Added ability to disable hierarchical discounting.<br>
<br>
Working Memory Activation (WMA): Rewritten from scratch for performance<br>
(no<br>
more ring) and correctness (including no more need for top-level ref<br>
counts), many fewer parameters.<br>
<br>
WMA: Printing working memory with the "internal" flag shows activation<br>
value.<br>
<br>
EpMem: Added ability to output visualization of episodes in Graphviz.<br>
<br>
Epmem: Removal of lots of legacy code (including "tree" mode).<br>
<br>
Epmem: Status WME (success/failure) refers to the initiating command.<br>
<br>
SMem: Added ability to output visualization of semantic store in<br>
Graphviz.<br>
<br>
SMem: Storage occurs at the end of every phase.<br>
<br>
SMem: Greatly improved retrieval performance.<br>
<br>
SMem: Status WME (success/failure) refers to the initiating command.<br>
<br>
The build procedure has changed dramatically and is detailed on the<br>
Google<br>
Code wiki.<br>
<br>
Issue 31: Kernel timers for performance tuning have been updated to use<br>
code from stlsoft.org, addressing many timer issues.<br>
<br>
Issue 16: rand command and rand rhs functions implemented. This was<br>
erroneously included in the previous release's change log.<br>
<br>
Issue 60, 42: Invalid select command crash, other select command fixes.<br>
<br>
Issue 61: Non-single compiliation units work now.<br>
<br>
Issue 57: Many changes related to output-link change tracking including<br>
a<br>
new method to disable output-link change tracking for a significant<br>
performance increase if not using specific SML methods. See Wiki:<br>
SMLOutputLinkGuide and method documentation for ClientAgent for much<br>
more<br>
information.<br>
<br>
Issue 62: KernelSML is no longer a singleton per process. Multiple<br>
Kernels<br>
can now be created (and deleted) which have distinct sets of agents.<br>
<br>
Issue 59: Added an RL parameter to turn off discounting during gaps.<br>
<br>
Issue 53: Capture input string quoting problem fixed.<br>
<br>
Issue 27: Listener port semantics extended to allow random listener<br>
ports<br>
and process-specific local sockets/pipes.<br>
<br>
Issue 40: Java Debugger layout file issues.<br>
<br>
Issue 51: Run command extended with a new option to interrupt when a<br>
goal<br>
retracts.<br>
<br>
Issue 39: Memory leaks fixed.<br>
<br>
Issue 36: Increased callback performance in SML (affects agents not<br>
using<br>
any callbacks).<br>
<br>
Issue 7: Memory pool preallocation command added.<br>
<br>
Issue 18: preferences command extended to include probability that<br>
current<br>
operators may be selected.<br>
</pre>

---

# 9.2.0 #
<pre>Soar 9.0.1, 9.1.1, 9.2.0 Release Notes<br>
======================================<br>
<br>
This release of Soar continues the 9.x series of releases during which<br>
we<br>
expect to introduce several new mechanisms.<br>
<br>
Soar can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
These Soar releases include all source code, many demo programs, and a<br>
number<br>
of applications that serve as examples of how to interface Soar to an<br>
external<br>
environment.  There is support for integrating Soar with C++, Java, Tcl,<br>
Python<br>
and C# applications.  Many tools that aid in development of Soar<br>
programs are<br>
also included.<br>
<br>
Soar 9.0.1 (Soar-RL)<br>
====================<br>
<br>
Soar 9.0 includes reinforcement learning code (Soar-RL), which was first<br>
released alongside the 8.6.4 beta. Soar 9.0.1 is primarily a maintenance<br>
release, although many bugs have been fixed and minor features added.<br>
<br>
Soar-RL is the architectural integration of reinforcement learning (RL)<br>
with<br>
Soar. The RL mechanism will automatically tune the values of<br>
numeric-indifferent preference rules based on reward generated while a<br>
Soar<br>
agent executes.  These values represent the expected discounted sum of<br>
future<br>
rewards the agent will receive if it selects that operator for states<br>
matched<br>
by the condition of the rule.  See the RL manual and tutorial in<br>
Documentation.<br>
<br>
To see Soar-RL in action right away, try the Water Jug RL demo (it can<br>
be<br>
loaded from the Demos->Water Jug menu in the Java Debugger). To see the<br>
effects<br>
of RL, run it to completion, then init-soar and run it again. Repeat 4-5<br>
times<br>
to see it reach optimal behavior. (Note: the agent may occasionally<br>
perform<br>
non-optimal behavior even after it has converged because of its<br>
exploration<br>
policy. See the RL manual and tutorial for details).<br>
<br>
Soar 9.1.1 (Episodic Memory)<br>
============================<br>
<br>
Soar 9.1 includes an Episodic Memory module, which was first released as<br>
Soar<br>
9.1.0.  Soar 9.1.1 is primarily a maintenance release with bugs fixed<br>
and minor<br>
features added. 9.1 also includes all of the features from 9.0<br>
(Soar-RL).<br>
<br>
Soar-EpMem is a task-independent, architectural integration of an<br>
artificial<br>
episodic memory (EpMem) with Soar.  The EpMem mechanism will<br>
automatically<br>
record episodes as a Soar agent executes.  These episodes can later be<br>
queried<br>
and retrieved in order to improve performance on future tasks. See the<br>
EpMem<br>
manual for details.<br>
<br>
Soar 9.2.0 (Semantic Memory)<br>
============================<br>
<br>
Soar 9.2 includes a Semantic Memory module, and this release of Soar<br>
9.2.0 is<br>
the first official release of Soar with semantic memory capabilities.<br>
9.2 also<br>
includes all features from 9.1 (EpMem) and 9.0 (Soar-RL).<br>
<br>
Soar-SMem is a task-independent, architectural integration of an<br>
artificial<br>
semantic memory (SMem) with Soar.  The SMem mechanism facilitates<br>
deliberate<br>
recording and querying of semantic chunks as a Soar agent executes.<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
These releases are generated for Windows, Linux, and OSX systems.<br>
Windows XP<br>
and Vista are supported, as well as 32- and 64-bit architectures.<br>
<br>
* IMPORTANT NOTE ABOUT 64-BIT BINARIES: To use the 64-bit Soar tools<br>
with Java<br>
(such as the Java debugger) you must be using the 64-bit JVM.<br>
Attempting to<br>
start 64-bit Soar programs with the 32-bit JVM on Windows will cause a<br>
javaw.exe process to hang open (terminate it using task manager). Also<br>
note:<br>
attempting to start 32-bit Java tools, such as those included in<br>
previous<br>
distributions of Soar, with a 64-bit JVM will also result in the<br>
hanging<br>
javaw.exe process.<br>
<br>
* Sun Java 1.5 or newer is required to use the Java applications,<br>
including the<br>
debugger (Java must be available in the path). Get it at<br>
http://java.sun.com/. Most operating systems probably already have<br>
Java<br>
installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* The Soar Suite needs under 100 MB for regular use, but closer to 1 GB<br>
of disk<br>
space for object and debugging files if rebuilding all projects.<br>
<br>
* Due to path length limits on some versions of Windows, it is<br>
recommended<br>
to extract Soar to the C:\ root or similar. Some parts of the Soar<br>
Suite<br>
directory structure are deep, so users wishing to put Soar in a deep<br>
location<br>
on their machines (such as<br>
C:\Documents and Settings\username\My Documents\Visual Studio<br>
Projects) may<br>
run into issues.<br>
<br>
* Tcl version 8.4 is required for the Tcl SML interface. Threaded Tcl<br>
builds<br>
are not supported. Windows 64-bit does not include the Tcl SML<br>
interface.<br>
Your installed Tcl architecture must be the same (32 or 64 bit) as the<br>
Soar<br>
binaries for the Tcl SML interface to load correctly.<br>
<br>
* The included Python libraries support Python 2.5.  Python 2.6 and 3<br>
support<br>
requires a rebuild of the ClientSMLPython project. Your installed<br>
Python<br>
architecture must be the same (32 or 64 bit) as the Soar binaries for<br>
the<br>
Python SML interface to load correctly.<br>
<br>
Build Requirements<br>
==================<br>
<br>
Binaries are released for Windows/Linux/OS X, so building is not<br>
required.  For<br>
those on other platforms or developers:<br>
<br>
* Compilers: Windows: Visual Studio 2005 SP1 or 2008, Linux/OS X: GCC<br>
3.3<br>
or later<br>
<br>
* SWIG 1.3.31 or later is required to build with cross-language support<br>
(e.g.<br>
to use the Java debugger).<br>
<br>
* Apache ant is required to build Java projects.<br>
<br>
* Only the Sun JDK will work, 1.5 or newer.<br>
<br>
* On Linux/OS X: Scons, Python 2.5 or newer, Tcl 8.4<br>
<br>
* Please see detailed build instructions available on http://msoar.org<br>
<br>
New in 9.0.1, 9.1.1, 9.2.0<br>
==========================<br>
<br>
* Native 64-bit platform support for Windows, Linux, OSX (bugs 1139,<br>
1064, 1036, 531) resulting in different, usually better performance<br>
on these platforms, and definite ease of compilation and integration<br>
with tools such as Java and Python.<br>
<br>
* Visual Studio 2008 is now supported and its use is recommended.<br>
Visual Studio 2005 support will likely be dropped with the next<br>
release.<br>
<br>
* Soar 7 mode has been completely removed.<br>
<br>
* Preference semantics regarding the generation of operator conflict<br>
impasses have changed, see appendix D in the Soar Manual for details.<br>
<br>
* Apache ant is now used to build java projects. This results in much<br>
faster and reliable Java builds.<br>
<br>
Detailed fixes:<br>
<br>
* Bug 873: Match-time interrupt with :interrupt production tag re-<br>
enabled.<br>
<br>
* Bug 234: Preference semantics changed regarding operator conflicts.<br>
<br>
* Bug 1145: dont-learn/force-learn RHS actions fixed.<br>
<br>
* Bug 1144: Crash bug related to chunking caveat mentioned in manual<br>
section<br>
4.6.5 fixed, helpful error message displayed instead.<br>
<br>
* Bug 1011: Impasses that existed in a superstate were not getting<br>
regenerated<br>
if the sub-state was removed due to the GDS, this has been fixed.<br>
<br>
* Bug 878: With verbose enabled, the general reason states are getting<br>
removed<br>
is printed when the state is removed.<br>
<br>
* Bug 510: Chunking bug fixed when trying to chunk over negated,<br>
conjuntive<br>
conditions.<br>
<br>
* Bug 1121: RL template production (:template) bug fixed that was<br>
failing to<br>
correctly generate productions if variables starting with "o" were<br>
used.<br>
<br>
* Bug 882: Changing trace filters in the Soar Java Debugger no longer<br>
collapses<br>
the whole trace.<br>
<br>
* Bug 1143: Direct I/O functions for WMEs in SML were acting in the<br>
wrong phase<br>
(usually the output phase instead of the input phase), changed these<br>
functions to instead buffer their actions for later execution in the<br>
input<br>
phase.<br>
<br>
* Bug 866: Optimization settings with Visual Studio tweaked.<br>
<br>
* Bug 517: Fixed a bug where an unbound variable included in a negated<br>
condition where a bound variable was required would crash Soar.<br>
<br>
* Bug 1138: Fixed FindByAttribute bug not handling piped strings<br>
correctly and,<br>
as a result, made it impossible to find some WMEs created on the<br>
kernel side.<br>
<br>
* Bug 867: Extended SML Identifier interface adding a number of methods<br>
that<br>
don't have the redundant Agent pointer in their argument list.<br>
<br>
* Bug 1049: Fixed io_header_output memory leak.<br>
<br>
* Bug 1060: CreateSharedIdWME now explicitly fails if an existing WME is<br>
specified, violating the working memory set restriction.<br>
<br>
* Bug 987, 1013: Echo command now ignores braces, pipes and other Soar<br>
characters so that better ascii art is possible.<br>
<br>
* Bug 1084, 1124, 1141: gp command extended with better error messages,<br>
maximum<br>
rule threshold, VisualSoar support, newline indifference.<br>
<br>
* Bug 1057: Many buffers replaced with growable, safer containers such<br>
as<br>
std::string and std::stringstream in performance-indifferent code.<br>
<br>
* Bug 776: New random command implemented and random right hand side<br>
functions<br>
implemented for random numbers on command.<br>
<br>
* Bug 1106: New print --exact flag added to avoid printing unnecessary<br>
copies<br>
of objects with multi-valued attributes.<br>
<br>
* Bug 927: Command line interface made more reentrant to avoid weird<br>
"true"<br>
results output with multiple clients connected to the agent.<br>
<br>
* Bug 952: New agents can now be created on the fly, during a run.<br>
<br>
* Bug 1104: Fixed TestCLI hang if exited during run.<br>
<br>
* Bug 1033: CreateKernelInNewThread error message fixed.<br>
<br>
* Bug 794: edit-production now reports failure if visual soar is not<br>
connected.<br>
<br>
* Bug 1043: Added SML Agent methods to launch the debugger.<br>
<br>
* Bug 1140: RHS halt does not cancel multiple Soar2D runs.<br>
<br>
* Bug 1099: Find function in Soar Java Debugger scrolls window to result<br>
now.<br>
<br>
* Bug 638: Tutorials updated.<br>
<br>
* Bug 931: Multiple productions can be pasted in to debugger at same<br>
time now.<br>
<br>
* Bug 1012: Unterminated strings in productions fail at parse-time now.<br>
<br>
* Bug 1025: GetOutputLink() no longer returns null.<br>
<br>
* Bug 1042: Debugger command line options fixed.<br>
<br>
* Bug 1045: Status complete was getting added twice on output link<br>
commands.<br>
<br>
* Bug 1056: Kernel changed to use const char more.<br>
<br>
* Bug 627: Help command lists folder instead of using command-names<br>
file,<br>
allowing arbitrary help topics to be easily added.<br>
<br>
* Bug 1105: Capture/replay input implemented.<br>
<br>
* Large Soar production file parsing speed improvements.<br>
<br>
* Soar2d rewritten, faster, easier to configure.<br>
<br>
* Various memory leaks fixed, some major.<br>
<br>
* Console logging fixed (clog command).<br>
<br>
* Build procedure streamlined on all platforms.<br>
<br>
* Removed all build warnings.<br>
<br>
* Added many regression tests.<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

# 9.1.1 #
<pre>Soar 9.0.1, 9.1.1, 9.2.0 Release Notes<br>
======================================<br>
<br>
This release of Soar continues the 9.x series of releases during which<br>
we<br>
expect to introduce several new mechanisms.<br>
<br>
Soar can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
These Soar releases include all source code, many demo programs, and a<br>
number<br>
of applications that serve as examples of how to interface Soar to an<br>
external<br>
environment.  There is support for integrating Soar with C++, Java, Tcl,<br>
Python<br>
and C# applications.  Many tools that aid in development of Soar<br>
programs are<br>
also included.<br>
<br>
Soar 9.0.1 (Soar-RL)<br>
====================<br>
<br>
Soar 9.0 includes reinforcement learning code (Soar-RL), which was first<br>
released alongside the 8.6.4 beta. Soar 9.0.1 is primarily a maintenance<br>
release, although many bugs have been fixed and minor features added.<br>
<br>
Soar-RL is the architectural integration of reinforcement learning (RL)<br>
with<br>
Soar. The RL mechanism will automatically tune the values of<br>
numeric-indifferent preference rules based on reward generated while a<br>
Soar<br>
agent executes.  These values represent the expected discounted sum of<br>
future<br>
rewards the agent will receive if it selects that operator for states<br>
matched<br>
by the condition of the rule.  See the RL manual and tutorial in<br>
Documentation.<br>
<br>
To see Soar-RL in action right away, try the Water Jug RL demo (it can<br>
be<br>
loaded from the Demos->Water Jug menu in the Java Debugger). To see the<br>
effects<br>
of RL, run it to completion, then init-soar and run it again. Repeat 4-5<br>
times<br>
to see it reach optimal behavior. (Note: the agent may occasionally<br>
perform<br>
non-optimal behavior even after it has converged because of its<br>
exploration<br>
policy. See the RL manual and tutorial for details).<br>
<br>
Soar 9.1.1 (Episodic Memory)<br>
============================<br>
<br>
Soar 9.1 includes an Episodic Memory module, which was first released as<br>
Soar<br>
9.1.0.  Soar 9.1.1 is primarily a maintenance release with bugs fixed<br>
and minor<br>
features added. 9.1 also includes all of the features from 9.0<br>
(Soar-RL).<br>
<br>
Soar-EpMem is a task-independent, architectural integration of an<br>
artificial<br>
episodic memory (EpMem) with Soar.  The EpMem mechanism will<br>
automatically<br>
record episodes as a Soar agent executes.  These episodes can later be<br>
queried<br>
and retrieved in order to improve performance on future tasks. See the<br>
EpMem<br>
manual for details.<br>
<br>
Soar 9.2.0 (Semantic Memory)<br>
============================<br>
<br>
Soar 9.2 includes a Semantic Memory module, and this release of Soar<br>
9.2.0 is<br>
the first official release of Soar with semantic memory capabilities.<br>
9.2 also<br>
includes all features from 9.1 (EpMem) and 9.0 (Soar-RL).<br>
<br>
Soar-SMem is a task-independent, architectural integration of an<br>
artificial<br>
semantic memory (SMem) with Soar.  The SMem mechanism facilitates<br>
deliberate<br>
recording and querying of semantic chunks as a Soar agent executes.<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
These releases are generated for Windows, Linux, and OSX systems.<br>
Windows XP<br>
and Vista are supported, as well as 32- and 64-bit architectures.<br>
<br>
* IMPORTANT NOTE ABOUT 64-BIT BINARIES: To use the 64-bit Soar tools<br>
with Java<br>
(such as the Java debugger) you must be using the 64-bit JVM.<br>
Attempting to<br>
start 64-bit Soar programs with the 32-bit JVM on Windows will cause a<br>
javaw.exe process to hang open (terminate it using task manager). Also<br>
note:<br>
attempting to start 32-bit Java tools, such as those included in<br>
previous<br>
distributions of Soar, with a 64-bit JVM will also result in the<br>
hanging<br>
javaw.exe process.<br>
<br>
* Sun Java 1.5 or newer is required to use the Java applications,<br>
including the<br>
debugger (Java must be available in the path). Get it at<br>
http://java.sun.com/. Most operating systems probably already have<br>
Java<br>
installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* The Soar Suite needs under 100 MB for regular use, but closer to 1 GB<br>
of disk<br>
space for object and debugging files if rebuilding all projects.<br>
<br>
* Due to path length limits on some versions of Windows, it is<br>
recommended<br>
to extract Soar to the C:\ root or similar. Some parts of the Soar<br>
Suite<br>
directory structure are deep, so users wishing to put Soar in a deep<br>
location<br>
on their machines (such as<br>
C:\Documents and Settings\username\My Documents\Visual Studio<br>
Projects) may<br>
run into issues.<br>
<br>
* Tcl version 8.4 is required for the Tcl SML interface. Threaded Tcl<br>
builds<br>
are not supported. Windows 64-bit does not include the Tcl SML<br>
interface.<br>
Your installed Tcl architecture must be the same (32 or 64 bit) as the<br>
Soar<br>
binaries for the Tcl SML interface to load correctly.<br>
<br>
* The included Python libraries support Python 2.5.  Python 2.6 and 3<br>
support<br>
requires a rebuild of the ClientSMLPython project. Your installed<br>
Python<br>
architecture must be the same (32 or 64 bit) as the Soar binaries for<br>
the<br>
Python SML interface to load correctly.<br>
<br>
Build Requirements<br>
==================<br>
<br>
Binaries are released for Windows/Linux/OS X, so building is not<br>
required.  For<br>
those on other platforms or developers:<br>
<br>
* Compilers: Windows: Visual Studio 2005 SP1 or 2008, Linux/OS X: GCC<br>
3.3<br>
or later<br>
<br>
* SWIG 1.3.31 or later is required to build with cross-language support<br>
(e.g.<br>
to use the Java debugger).<br>
<br>
* Apache ant is required to build Java projects.<br>
<br>
* Only the Sun JDK will work, 1.5 or newer.<br>
<br>
* On Linux/OS X: Scons, Python 2.5 or newer, Tcl 8.4<br>
<br>
* Please see detailed build instructions available on http://msoar.org<br>
<br>
New in 9.0.1, 9.1.1, 9.2.0<br>
==========================<br>
<br>
* Native 64-bit platform support for Windows, Linux, OSX (bugs 1139,<br>
1064, 1036, 531) resulting in different, usually better performance<br>
on these platforms, and definite ease of compilation and integration<br>
with tools such as Java and Python.<br>
<br>
* Visual Studio 2008 is now supported and its use is recommended.<br>
Visual Studio 2005 support will likely be dropped with the next<br>
release.<br>
<br>
* Soar 7 mode has been completely removed.<br>
<br>
* Preference semantics regarding the generation of operator conflict<br>
impasses have changed, see appendix D in the Soar Manual for details.<br>
<br>
* Apache ant is now used to build java projects. This results in much<br>
faster and reliable Java builds.<br>
<br>
Detailed fixes:<br>
<br>
* Bug 873: Match-time interrupt with :interrupt production tag re-<br>
enabled.<br>
<br>
* Bug 234: Preference semantics changed regarding operator conflicts.<br>
<br>
* Bug 1145: dont-learn/force-learn RHS actions fixed.<br>
<br>
* Bug 1144: Crash bug related to chunking caveat mentioned in manual<br>
section<br>
4.6.5 fixed, helpful error message displayed instead.<br>
<br>
* Bug 1011: Impasses that existed in a superstate were not getting<br>
regenerated<br>
if the sub-state was removed due to the GDS, this has been fixed.<br>
<br>
* Bug 878: With verbose enabled, the general reason states are getting<br>
removed<br>
is printed when the state is removed.<br>
<br>
* Bug 510: Chunking bug fixed when trying to chunk over negated,<br>
conjuntive<br>
conditions.<br>
<br>
* Bug 1121: RL template production (:template) bug fixed that was<br>
failing to<br>
correctly generate productions if variables starting with "o" were<br>
used.<br>
<br>
* Bug 882: Changing trace filters in the Soar Java Debugger no longer<br>
collapses<br>
the whole trace.<br>
<br>
* Bug 1143: Direct I/O functions for WMEs in SML were acting in the<br>
wrong phase<br>
(usually the output phase instead of the input phase), changed these<br>
functions to instead buffer their actions for later execution in the<br>
input<br>
phase.<br>
<br>
* Bug 866: Optimization settings with Visual Studio tweaked.<br>
<br>
* Bug 517: Fixed a bug where an unbound variable included in a negated<br>
condition where a bound variable was required would crash Soar.<br>
<br>
* Bug 1138: Fixed FindByAttribute bug not handling piped strings<br>
correctly and,<br>
as a result, made it impossible to find some WMEs created on the<br>
kernel side.<br>
<br>
* Bug 867: Extended SML Identifier interface adding a number of methods<br>
that<br>
don't have the redundant Agent pointer in their argument list.<br>
<br>
* Bug 1049: Fixed io_header_output memory leak.<br>
<br>
* Bug 1060: CreateSharedIdWME now explicitly fails if an existing WME is<br>
specified, violating the working memory set restriction.<br>
<br>
* Bug 987, 1013: Echo command now ignores braces, pipes and other Soar<br>
characters so that better ascii art is possible.<br>
<br>
* Bug 1084, 1124, 1141: gp command extended with better error messages,<br>
maximum<br>
rule threshold, VisualSoar support, newline indifference.<br>
<br>
* Bug 1057: Many buffers replaced with growable, safer containers such<br>
as<br>
std::string and std::stringstream in performance-indifferent code.<br>
<br>
* Bug 776: New random command implemented and random right hand side<br>
functions<br>
implemented for random numbers on command.<br>
<br>
* Bug 1106: New print --exact flag added to avoid printing unnecessary<br>
copies<br>
of objects with multi-valued attributes.<br>
<br>
* Bug 927: Command line interface made more reentrant to avoid weird<br>
"true"<br>
results output with multiple clients connected to the agent.<br>
<br>
* Bug 952: New agents can now be created on the fly, during a run.<br>
<br>
* Bug 1104: Fixed TestCLI hang if exited during run.<br>
<br>
* Bug 1033: CreateKernelInNewThread error message fixed.<br>
<br>
* Bug 794: edit-production now reports failure if visual soar is not<br>
connected.<br>
<br>
* Bug 1043: Added SML Agent methods to launch the debugger.<br>
<br>
* Bug 1140: RHS halt does not cancel multiple Soar2D runs.<br>
<br>
* Bug 1099: Find function in Soar Java Debugger scrolls window to result<br>
now.<br>
<br>
* Bug 638: Tutorials updated.<br>
<br>
* Bug 931: Multiple productions can be pasted in to debugger at same<br>
time now.<br>
<br>
* Bug 1012: Unterminated strings in productions fail at parse-time now.<br>
<br>
* Bug 1025: GetOutputLink() no longer returns null.<br>
<br>
* Bug 1042: Debugger command line options fixed.<br>
<br>
* Bug 1045: Status complete was getting added twice on output link<br>
commands.<br>
<br>
* Bug 1056: Kernel changed to use const char more.<br>
<br>
* Bug 627: Help command lists folder instead of using command-names<br>
file,<br>
allowing arbitrary help topics to be easily added.<br>
<br>
* Bug 1105: Capture/replay input implemented.<br>
<br>
* Large Soar production file parsing speed improvements.<br>
<br>
* Soar2d rewritten, faster, easier to configure.<br>
<br>
* Various memory leaks fixed, some major.<br>
<br>
* Console logging fixed (clog command).<br>
<br>
* Build procedure streamlined on all platforms.<br>
<br>
* Removed all build warnings.<br>
<br>
* Added many regression tests.<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 9.1.0 #
<pre>Soar 9.1.0-beta Release Notes<br>
=============================<br>
<br>
This release of Soar continues the 9.x series of releases during which<br>
we<br>
expect to introduce several new mechanisms. 9.0.0 included reinforcement<br>
learning code (Soar-RL), which was first released alongside the 8.6.4<br>
beta. 9.0.0 was also the first release to include the SML interface<br>
without gSKI.  9.1.0-beta includes episodic memory (Soar-EpMem) and<br>
Working<br>
Memory Activation (WMA).<br>
<br>
Soar-EpMem is a task-independent, architectural integration of an<br>
artificial<br>
episodic memory (EpMem) with Soar.  The EpMem mechanism will<br>
automatically<br>
record episodes as a Soar agent executes.  These episodes can later be<br>
queried<br>
and retrieved in order to improve performance on future tasks.  See the<br>
EpMem<br>
manual in Documentation.<br>
<br>
This release of Soar-EpMem does not include a tutorial.  To see examples<br>
of using Soar-EpMem with a Soar agent, see the "kb" agent in the Demos<br>
directory.  This agent will run a series of EpMem "unit tests,"<br>
exploring<br>
all forms of the Soar-EpMem agent API.<br>
<br>
Soar 9.1.0-beta includes all source code, many demo programs, and a<br>
number of applications that serve as examples of how to interface<br>
Soar to an external Environment.  There is support for integrating<br>
Soar with C++, Java, Tcl, Python and C# applications.  Many Tools<br>
that aid in development of Soar programs are also included.<br>
<br>
These Release Notes describe the primary changes in this<br>
latest release.  More detailed information can be obtained at<br>
http://winter.eecs.umich.edu/soarwiki<br>
<br>
This release was generated for Windows, Linux, and OSX systems.<br>
Windows XP and Vista are supported.<br>
<br>
Soar 9.1.0-beta can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
* On Windows, the 19MB download file expands to ~45MB when extracted.<br>
Linux and OSX disk requirements are similar. Note that Windows has<br>
a 255 character path length limit; some parts of the directory<br>
structure are deep, so users wishing to put Soar in a deep location<br>
on their machines may run into issues.<br>
<br>
* Java 1.5.0 or newer is required to use the Java applications,<br>
including the debugger (Java must be available in the path). Get it<br>
at http://java.sun.com/. Most operating systems probably already have<br>
Java installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* If you want to run Tcl applications on Linux, Tcl 8.4.x or later is<br>
required (note that the tutorial applications are now available in<br>
Java).  Also note that we do not currently support threaded builds<br>
of Tcl (e.g. ActiveTcl 8.4.11.2 or later).<br>
<br>
* The included Python libraries support Python 2.4.  Python 2.5 support<br>
requires a rebuild of the ClientSMLPython project.<br>
<br>
Build Requirements<br>
==================<br>
Binaries are released for Windows/Linux/OS X, so building is not<br>
required.<br>
For those on other platforms or developers:<br>
* On all platforms, SWIG 1.3.31 or later is required to build with<br>
cross-language support (e.g. to use the Java debugger).<br>
* On Windows, Visual Studio 2005 SP1 is required.<br>
* On Linux/Unix/OS X, Python, SCons, gcc 3.3 or later required.<br>
<br>
Soar-EpMem & SQLite<br>
===================<br>
Soar-EpMem requires both statically and dynamically linked SQLite<br>
components.<br>
The former is provided in source. The latter, at this time, is a manual<br>
process<br>
(SQLite uses make scripts for their compile). EpMem includes pre-built<br>
binaries<br>
for Windows, Linux (tested on Ubuntu) and OS X Leopard. If your install<br>
works with<br>
these components, you can skip this section. Otherwise, the following<br>
process<br>
documents how to build the required components.<br>
<br>
First, download the source tarball from the SQLite homepage<br>
(http://www.sqlite.org/download.html).<br>
To be clear, you want the non-amalgamated source code tarball<br>
(probably named sqlite-A.B.C.tar.gz, for version A.B.C).  Next, extract<br>
the contents,<br>
change to the directory, and run ./configure followed by make at the<br>
command line.<br>
These commands will create the required libraries for your system.  The<br>
final step<br>
is to copy the required components from the SQLite directory to your<br>
EpMem<br>
directory. The only file of interest is named libsqliteA.B.C.D.ext<br>
(ext is dylib on Mac, so for Linux; A.B.C.D will vary on the version)<br>
and is<br>
located in the .libs folder of your SQLite directory. You will want to<br>
copy this<br>
file out of .libs and into the SoarLibrary/lib folder of your EpMem<br>
install.<br>
Furthermore, SQLite requires that this file exist under two additional<br>
names:<br>
libqsliteA.ext and libsqliteA.B.ext. To accommodate, you can either<br>
create a symbolic<br>
link under these two names or duplicate the file (it makes no<br>
difference, except<br>
for added/saved disk space).  Once you have these components in the<br>
appropriate directory,<br>
your EpMem build (and subsequent runs) should work fine.<br>
<br>
Components of Soar 9.1.0-beta<br>
=============================<br>
* Pre-built libraries and executables (SoarLibrary/bin)<br>
* all source code, config files, and OS tools for automated Soar builds<br>
* SoarJavaDebugger<br>
* VisualSoar<br>
* JavaEaters, JavaTankSoar, TclEaters<br>
* Test and Demo programs<br>
* Documentation<br>
<br>
New in 9.1.0-beta<br>
=================<br>
<br>
* Components and Capabilities added<br>
-- EpMem commands and documentation added (see EpMem manual in<br>
Documentation directory).<br>
-- WMA commands and documentation added (see EpMem manual in<br>
Documentation directory).<br>
-- Other minor enhancements.<br>
<br>
* Bug fixes<br>
-- Several other minor fixes.<br>
<br>
Known issues:<br>
=============<br>
<br>
* No EpMem tutorial<br>
* EpMem "provenance" parameter is unimplemented<br>
* status complete appears on the output-link immediately instead<br>
of during the next input phase.<br>
* The Soar 7 mode implementation currently relies on an ugly hack. If<br>
you experience problems in this mode, please let us know.<br>
* Many other minor issues (see bugzilla for details).<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 9.0.1 #
<pre>Soar 9.0.1, 9.1.1, 9.2.0 Release Notes<br>
======================================<br>
<br>
This release of Soar continues the 9.x series of releases during which<br>
we<br>
expect to introduce several new mechanisms.<br>
<br>
Soar can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
These Soar releases include all source code, many demo programs, and a<br>
number<br>
of applications that serve as examples of how to interface Soar to an<br>
external<br>
environment.  There is support for integrating Soar with C++, Java, Tcl,<br>
Python<br>
and C# applications.  Many tools that aid in development of Soar<br>
programs are<br>
also included.<br>
<br>
Soar 9.0.1 (Soar-RL)<br>
====================<br>
<br>
Soar 9.0 includes reinforcement learning code (Soar-RL), which was first<br>
released alongside the 8.6.4 beta. Soar 9.0.1 is primarily a maintenance<br>
release, although many bugs have been fixed and minor features added.<br>
<br>
Soar-RL is the architectural integration of reinforcement learning (RL)<br>
with<br>
Soar. The RL mechanism will automatically tune the values of<br>
numeric-indifferent preference rules based on reward generated while a<br>
Soar<br>
agent executes.  These values represent the expected discounted sum of<br>
future<br>
rewards the agent will receive if it selects that operator for states<br>
matched<br>
by the condition of the rule.  See the RL manual and tutorial in<br>
Documentation.<br>
<br>
To see Soar-RL in action right away, try the Water Jug RL demo (it can<br>
be<br>
loaded from the Demos->Water Jug menu in the Java Debugger). To see the<br>
effects<br>
of RL, run it to completion, then init-soar and run it again. Repeat 4-5<br>
times<br>
to see it reach optimal behavior. (Note: the agent may occasionally<br>
perform<br>
non-optimal behavior even after it has converged because of its<br>
exploration<br>
policy. See the RL manual and tutorial for details).<br>
<br>
Soar 9.1.1 (Episodic Memory)<br>
============================<br>
<br>
Soar 9.1 includes an Episodic Memory module, which was first released as<br>
Soar<br>
9.1.0.  Soar 9.1.1 is primarily a maintenance release with bugs fixed<br>
and minor<br>
features added. 9.1 also includes all of the features from 9.0<br>
(Soar-RL).<br>
<br>
Soar-EpMem is a task-independent, architectural integration of an<br>
artificial<br>
episodic memory (EpMem) with Soar.  The EpMem mechanism will<br>
automatically<br>
record episodes as a Soar agent executes.  These episodes can later be<br>
queried<br>
and retrieved in order to improve performance on future tasks. See the<br>
EpMem<br>
manual for details.<br>
<br>
Soar 9.2.0 (Semantic Memory)<br>
============================<br>
<br>
Soar 9.2 includes a Semantic Memory module, and this release of Soar<br>
9.2.0 is<br>
the first official release of Soar with semantic memory capabilities.<br>
9.2 also<br>
includes all features from 9.1 (EpMem) and 9.0 (Soar-RL).<br>
<br>
Soar-SMem is a task-independent, architectural integration of an<br>
artificial<br>
semantic memory (SMem) with Soar.  The SMem mechanism facilitates<br>
deliberate<br>
recording and querying of semantic chunks as a Soar agent executes.<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
These releases are generated for Windows, Linux, and OSX systems.<br>
Windows XP<br>
and Vista are supported, as well as 32- and 64-bit architectures.<br>
<br>
* IMPORTANT NOTE ABOUT 64-BIT BINARIES: To use the 64-bit Soar tools<br>
with Java<br>
(such as the Java debugger) you must be using the 64-bit JVM.<br>
Attempting to<br>
start 64-bit Soar programs with the 32-bit JVM on Windows will cause a<br>
javaw.exe process to hang open (terminate it using task manager). Also<br>
note:<br>
attempting to start 32-bit Java tools, such as those included in<br>
previous<br>
distributions of Soar, with a 64-bit JVM will also result in the<br>
hanging<br>
javaw.exe process.<br>
<br>
* Sun Java 1.5 or newer is required to use the Java applications,<br>
including the<br>
debugger (Java must be available in the path). Get it at<br>
http://java.sun.com/. Most operating systems probably already have<br>
Java<br>
installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* The Soar Suite needs under 100 MB for regular use, but closer to 1 GB<br>
of disk<br>
space for object and debugging files if rebuilding all projects.<br>
<br>
* Due to path length limits on some versions of Windows, it is<br>
recommended<br>
to extract Soar to the C:\ root or similar. Some parts of the Soar<br>
Suite<br>
directory structure are deep, so users wishing to put Soar in a deep<br>
location<br>
on their machines (such as<br>
C:\Documents and Settings\username\My Documents\Visual Studio<br>
Projects) may<br>
run into issues.<br>
<br>
* Tcl version 8.4 is required for the Tcl SML interface. Threaded Tcl<br>
builds<br>
are not supported. Windows 64-bit does not include the Tcl SML<br>
interface.<br>
Your installed Tcl architecture must be the same (32 or 64 bit) as the<br>
Soar<br>
binaries for the Tcl SML interface to load correctly.<br>
<br>
* The included Python libraries support Python 2.5.  Python 2.6 and 3<br>
support<br>
requires a rebuild of the ClientSMLPython project. Your installed<br>
Python<br>
architecture must be the same (32 or 64 bit) as the Soar binaries for<br>
the<br>
Python SML interface to load correctly.<br>
<br>
Build Requirements<br>
==================<br>
<br>
Binaries are released for Windows/Linux/OS X, so building is not<br>
required.  For<br>
those on other platforms or developers:<br>
<br>
* Compilers: Windows: Visual Studio 2005 SP1 or 2008, Linux/OS X: GCC<br>
3.3<br>
or later<br>
<br>
* SWIG 1.3.31 or later is required to build with cross-language support<br>
(e.g.<br>
to use the Java debugger).<br>
<br>
* Apache ant is required to build Java projects.<br>
<br>
* Only the Sun JDK will work, 1.5 or newer.<br>
<br>
* On Linux/OS X: Scons, Python 2.5 or newer, Tcl 8.4<br>
<br>
* Please see detailed build instructions available on http://msoar.org<br>
<br>
New in 9.0.1, 9.1.1, 9.2.0<br>
==========================<br>
<br>
* Native 64-bit platform support for Windows, Linux, OSX (bugs 1139,<br>
1064, 1036, 531) resulting in different, usually better performance<br>
on these platforms, and definite ease of compilation and integration<br>
with tools such as Java and Python.<br>
<br>
* Visual Studio 2008 is now supported and its use is recommended.<br>
Visual Studio 2005 support will likely be dropped with the next<br>
release.<br>
<br>
* Soar 7 mode has been completely removed.<br>
<br>
* Preference semantics regarding the generation of operator conflict<br>
impasses have changed, see appendix D in the Soar Manual for details.<br>
<br>
* Apache ant is now used to build java projects. This results in much<br>
faster and reliable Java builds.<br>
<br>
Detailed fixes:<br>
<br>
* Bug 873: Match-time interrupt with :interrupt production tag re-<br>
enabled.<br>
<br>
* Bug 234: Preference semantics changed regarding operator conflicts.<br>
<br>
* Bug 1145: dont-learn/force-learn RHS actions fixed.<br>
<br>
* Bug 1144: Crash bug related to chunking caveat mentioned in manual<br>
section<br>
4.6.5 fixed, helpful error message displayed instead.<br>
<br>
* Bug 1011: Impasses that existed in a superstate were not getting<br>
regenerated<br>
if the sub-state was removed due to the GDS, this has been fixed.<br>
<br>
* Bug 878: With verbose enabled, the general reason states are getting<br>
removed<br>
is printed when the state is removed.<br>
<br>
* Bug 510: Chunking bug fixed when trying to chunk over negated,<br>
conjuntive<br>
conditions.<br>
<br>
* Bug 1121: RL template production (:template) bug fixed that was<br>
failing to<br>
correctly generate productions if variables starting with "o" were<br>
used.<br>
<br>
* Bug 882: Changing trace filters in the Soar Java Debugger no longer<br>
collapses<br>
the whole trace.<br>
<br>
* Bug 1143: Direct I/O functions for WMEs in SML were acting in the<br>
wrong phase<br>
(usually the output phase instead of the input phase), changed these<br>
functions to instead buffer their actions for later execution in the<br>
input<br>
phase.<br>
<br>
* Bug 866: Optimization settings with Visual Studio tweaked.<br>
<br>
* Bug 517: Fixed a bug where an unbound variable included in a negated<br>
condition where a bound variable was required would crash Soar.<br>
<br>
* Bug 1138: Fixed FindByAttribute bug not handling piped strings<br>
correctly and,<br>
as a result, made it impossible to find some WMEs created on the<br>
kernel side.<br>
<br>
* Bug 867: Extended SML Identifier interface adding a number of methods<br>
that<br>
don't have the redundant Agent pointer in their argument list.<br>
<br>
* Bug 1049: Fixed io_header_output memory leak.<br>
<br>
* Bug 1060: CreateSharedIdWME now explicitly fails if an existing WME is<br>
specified, violating the working memory set restriction.<br>
<br>
* Bug 987, 1013: Echo command now ignores braces, pipes and other Soar<br>
characters so that better ascii art is possible.<br>
<br>
* Bug 1084, 1124, 1141: gp command extended with better error messages,<br>
maximum<br>
rule threshold, VisualSoar support, newline indifference.<br>
<br>
* Bug 1057: Many buffers replaced with growable, safer containers such<br>
as<br>
std::string and std::stringstream in performance-indifferent code.<br>
<br>
* Bug 776: New random command implemented and random right hand side<br>
functions<br>
implemented for random numbers on command.<br>
<br>
* Bug 1106: New print --exact flag added to avoid printing unnecessary<br>
copies<br>
of objects with multi-valued attributes.<br>
<br>
* Bug 927: Command line interface made more reentrant to avoid weird<br>
"true"<br>
results output with multiple clients connected to the agent.<br>
<br>
* Bug 952: New agents can now be created on the fly, during a run.<br>
<br>
* Bug 1104: Fixed TestCLI hang if exited during run.<br>
<br>
* Bug 1033: CreateKernelInNewThread error message fixed.<br>
<br>
* Bug 794: edit-production now reports failure if visual soar is not<br>
connected.<br>
<br>
* Bug 1043: Added SML Agent methods to launch the debugger.<br>
<br>
* Bug 1140: RHS halt does not cancel multiple Soar2D runs.<br>
<br>
* Bug 1099: Find function in Soar Java Debugger scrolls window to result<br>
now.<br>
<br>
* Bug 638: Tutorials updated.<br>
<br>
* Bug 931: Multiple productions can be pasted in to debugger at same<br>
time now.<br>
<br>
* Bug 1012: Unterminated strings in productions fail at parse-time now.<br>
<br>
* Bug 1025: GetOutputLink() no longer returns null.<br>
<br>
* Bug 1042: Debugger command line options fixed.<br>
<br>
* Bug 1045: Status complete was getting added twice on output link<br>
commands.<br>
<br>
* Bug 1056: Kernel changed to use const char more.<br>
<br>
* Bug 627: Help command lists folder instead of using command-names<br>
file,<br>
allowing arbitrary help topics to be easily added.<br>
<br>
* Bug 1105: Capture/replay input implemented.<br>
<br>
* Large Soar production file parsing speed improvements.<br>
<br>
* Soar2d rewritten, faster, easier to configure.<br>
<br>
* Various memory leaks fixed, some major.<br>
<br>
* Console logging fixed (clog command).<br>
<br>
* Build procedure streamlined on all platforms.<br>
<br>
* Removed all build warnings.<br>
<br>
* Added many regression tests.<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 9.0.0 #
<pre>Soar 9.0.0 Release Notes<br>
========================<br>
<br>
This release of Soar begins the 9.x series of releases during which we<br>
expect to introduce several new mechanisms. 9.0.0 includes reinforcement<br>
learning code (Soar-RL), which was first released alongside the 8.6.4<br>
beta. 9.0.0 is also the first release to include the SML interface<br>
without gSKI.<br>
<br>
Soar-RL is the architectural integration of reinforcement learning (RL)<br>
with<br>
Soar.  The RL mechanism will automatically tune the values of<br>
numeric-indifferent preference rules based on reward generated while a<br>
Soar<br>
agent executes.  These values represent the expected discounted sum of<br>
future rewards the agent will receive if it selects that operator for<br>
states<br>
matched by the condition of the rule.  See the RL manual and tutorial in<br>
Documentation.<br>
<br>
To see Soar-RL in action right away, try the Water Jug RL demo (it can<br>
be<br>
loaded from the Demos->Water Jug menu in the Java Debugger). To see the<br>
effects<br>
of RL, run it to completion, then init-soar and run it again. Repeat 4-5<br>
times<br>
to see it reach optimal behavior. (Note: the agent may occasionally<br>
perform<br>
non-optimal behavior even after it has converged because of its<br>
exploration<br>
policy. See the RL manual and tutorial for details).<br>
<br>
Soar-9.0.0 includes all source code, many demo programs, and a<br>
number of applications that serve as examples of how to interface<br>
Soar to an external Environment.  There is support for integrating<br>
Soar with C++, Java, Tcl, Python and C# applications.  Many Tools<br>
that aid in development of Soar programs are also included.<br>
<br>
These Release Notes describe the primary changes in this<br>
latest release.  More detailed information can be obtained at<br>
http://winter.eecs.umich.edu/soarwiki<br>
<br>
This release was generated for Windows, Linux, and OSX systems.<br>
Windows XP and Vista are supported.<br>
<br>
Soar 9.0.0 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
* On Windows, the 19MB download file expands to ~45MB when extracted.<br>
Linux and OSX disk requirements are similar. Note that Windows has<br>
a 255 character path length limit; some parts of the directory<br>
structure are deep, so users wishing to put Soar in a deep location<br>
on their machines may run into issues.<br>
<br>
* Java 1.5.0 or newer is required to use the Java applications,<br>
including the debugger (Java must be available in the path). Get it<br>
at http://java.sun.com/. Most operating systems probably already have<br>
Java installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* If you want to run Tcl applications on Linux, Tcl 8.4.x or later is<br>
required (note that the tutorial applications are now available in<br>
Java).  Also note that we do not currently support threaded builds<br>
of Tcl (e.g. ActiveTcl 8.4.11.2 or later).<br>
<br>
* The included Python libraries support Python 2.4.  Python 2.5 support<br>
requires a rebuild of the ClientSMLPython project.<br>
<br>
Build Requirements<br>
==================<br>
Binaries are released for Windows/Linux/OS X, so building is not<br>
required.<br>
For those on other platforms or developers:<br>
* On all platforms, SWIG 1.3.31 or later is required to build with<br>
cross-language support (e.g. to use the Java debugger).<br>
* On Windows, Visual Studio 2005 SP1 is required.<br>
* On Linux/Unix/OS X, Python, SCons, gcc 3.3 or later required.<br>
<br>
Components of Soar 9.0.0<br>
========================<br>
* Pre-built libraries and executables for Windows (SoarLibrary/bin)<br>
* all source code, config files, and OS tools for automated Soar builds<br>
* SoarJavaDebugger<br>
* VisualSoar<br>
* JavaEaters, JavaTankSoar, TclEaters<br>
* Test and Demo programs<br>
* Documentation<br>
<br>
New in 9.0.0<br>
============<br>
<br>
* Components and Capabilities added<br>
-- RL commands and documentation added (see RL manual and tutorial<br>
in Documentation directory).<br>
-- indifferent-selection command updated to support options primarily<br>
intended for (but not limited to) RL.<br>
-- select and predict commands added to control operator selection<br>
(essentially replaces defunct indifferent-selection --ask<br>
functionality).<br>
-- New "gp" command added. Generates rules based on simple patterns<br>
(intended for use with RL).<br>
-- Added iterative deepening support to selection space. To see this<br>
in action, load the Water Jug Look-Ahead demo, then source<br>
selection-iterative-deepening.soar (in SoarLibrary/Demos/default)<br>
and run. Init-soar and run again. Iterative deepening is guaranteed<br>
to find the path with the fewest steps.<br>
-- Added options to learn command to turn chunking through local<br>
negations<br>
on or off (on by default for backwards compatibility). If off, a<br>
message<br>
will be generated if "watch --learning print" is enabled that says<br>
a chunk<br>
was not created because of local negations, and what those<br>
negations were.<br>
-- Updated selection space to remove local negations (for<br>
compatibility with<br>
new --disable-through-local-negations chunking mode).<br>
-- In a tie impasse, an item-count WME containing the number of tied<br>
operators<br>
is now architecturally generated.<br>
-- Depth at which productions are firing is reported at watch 5.<br>
-- Multiple rules can be loaded via the Java debugger's Edit<br>
Production<br>
widget, scratch pad, and by pasting into the trace window. This is<br>
the<br>
result of an underlying change to SML in which arguments to<br>
ExecuteCommandLine are passed through logic associated with the<br>
source<br>
command, and thus multiple commands can be given at once.<br>
-- Command boxes in the Java debugger have been enhanced so that<br>
history<br>
browsing with the up/down arrows works on all platforms.<br>
-- New "single compilation unit" build configurations (greatly speeds<br>
build time).<br>
-- Unit test suite added.<br>
-- Other minor enhancements.<br>
<br>
* Bug fixes<br>
-- Output link supports circular structures.<br>
-- Duplicate productions no longer cause sourcing to stop.<br>
-- Various memory leaks fixed.<br>
-- Several other minor fixes.<br>
<br>
* Other updates<br>
-- Removed gSKI (10-30% speed increase in simple benchmarking).<br>
-- The Soar Java Debugger now uses its default layout the first time<br>
it is run with a new version of Soar. Old layouts are preserved<br>
and can be loaded if desired (which makes them the default).<br>
<br>
<br>
Known issues:<br>
=============<br>
<br>
* status complete appears on the output-link immediately instead<br>
of during the next input phase.<br>
* The Soar 7 mode implementation currently relies on an ugly hack. If<br>
you experience problems in this mode, please let us know.<br>
* Many other minor issues (see bugzilla for details).<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 8.6.4 #
<pre>Soar 8.6.4 Release Notes<br>
========================<br>
<br>
This release of Soar includes reinforcement learning code (Soar-RL).<br>
<br>
Soar-RL is the architectural integration of reinforcement learning<br>
(RL) with Soar.  The RL mechanism will automatically learn value<br>
functions as a Soar agent executes.  These value functions represent,<br>
for a given working memory state and proposed operator, the expected<br>
sum of future rewards the agent will receive if it selects that<br>
operator.<br>
<br>
Note: in the final version of Soar-RL, all Soar impasses will be<br>
treated identically.  In the case of an impasse other than<br>
operator no-change, the time period at the superstate during<br>
which no operator is selected will be treated as a gap in rule<br>
coverage, and perceived reward will be discounted with respect to<br>
decision cycles passed.<br>
<br>
This release also includes an important bug fix regarding shared ids,<br>
see bugzilla bug number 1027 for details.<br>
<br>
The 8.6.4 release notes follow.<br>
<br>
Soar 8.6.4 Release Notes<br>
========================<br>
<br>
Soar-8.6.4 includes all source code, many demo programs, and a<br>
number of applications that serve as examples of how to interface<br>
Soar to an external Environment.  There is support for integrating<br>
Soar with C++, Java, Tcl, Python and C# applications.  Many Tools<br>
that aid in development of Soar programs are also included.<br>
<br>
These Release Notes describe the primary changes in this<br>
latest release.  More detailed information can be obtained at<br>
http://winter.eecs.umich.edu/soarwiki<br>
<br>
This release was generated for Windows, Linux, and OSX systems.<br>
Windows XP and Vista are supported.<br>
<br>
Soar 8.6.4 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
* On Windows, the 19MB download file expands to ~45MB when extracted.<br>
Linux and OSX disk requirements are similar.<br>
<br>
* Java 1.5.0 or newer is required to use the Java applications,<br>
including the debugger (Java must be available in the path). Get it<br>
at http://java.sun.com/. Most operating systems probably already have<br>
Java installed. NOTE: Java 1.4.2 is no longer supported.<br>
<br>
* If you want to run Tcl applications on Linux, Tcl 8.4.x or later is<br>
required (note that the tutorial applications are now available in<br>
Java).  Also note that we do not currently support threaded builds<br>
of Tcl (e.g. ActiveTcl 8.4.11.2 or later).<br>
<br>
* The included Python libraries support Python 2.4.  Python 2.5 support<br>
requires a rebuild of the ClientSMLPython project.<br>
<br>
Build Requirements<br>
==================<br>
<br>
* On all platforms, SWIG 1.3.31 is required for cross-language support<br>
(e.g. to use the Java debugger).<br>
* On Windows, Visual Studio 2005 SP1 is required.<br>
* On Linux/Unix/OS X, Python, SCons, gcc 3.3 or later required.<br>
<br>
Components of Soar 8.6.4<br>
========================<br>
<br>
* Pre-built libraries and executables for Windows (SoarLibrary/bin)<br>
* all source code, config files, and OS tools for automated Soar builds<br>
* SoarJavaDebugger<br>
* VisualSoar<br>
* JavaEaters, JavaTankSoar, TclEaters<br>
* Test and Demo programs<br>
* Documentation<br>
<br>
New in 8.6.4<br>
============<br>
<br>
* Components and Capabilities added<br>
-- Eaters and TankSoar have been rewritten and are now part of the<br>
Soar2D framework. This includes many enhancements.<br>
-- New command: load-library. See manual for details.<br>
-- Extended preferences command to print wme support (see the Soar<br>
wiki for details).<br>
-- Extended print command to support printing tree structures (see<br>
manual for details).<br>
-- Added IsRemoteConnection() to client Kernel interface.<br>
-- Modified StringEventCallback to return a string.<br>
-- Now generate proper Java enums for SML Java interface.<br>
-- Major refactoring of portability support (e.g. portability.h)<br>
-- Other minor enhancements.<br>
<br>
* Bug fixes<br>
-- Lots of TankSoar and Eaters bugfixes<br>
-- Fixed command-to-file command<br>
-- Fixed more memory leaks.<br>
-- Fixed stats bug.<br>
-- Fixed print depth/indentation bug.<br>
-- Several other minor fixes<br>
<br>
* Other updates<br>
-- Updated to SWT 3.3<br>
<br>
<br>
Known issues:<br>
=============<br>
<br>
* Remote connections on Linux can be slow.<br>
* status complete appears on the output-link immediately instead<br>
of during the next input phase.<br>
* The Soar 7 mode implementation currently relies on an ugly hack. If<br>
you experience problems in this mode, please let us know.<br>
* In TankSoar, the obscure-bot sometimes generates a scary-looking<br>
kernel error message.  It appears to be safe to ignore.<br>
* Many other minor issues (see bugzilla for details).<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.netv<br>
</pre>

---

# 8.6.3 #
<pre>Soar 8.6.3 Release Notes<br>
========================<br>
<br>
Soar-8.6.3 includes all source code, many demo programs, and a<br>
number of applications that serve as examples of how to interface<br>
Soar to an external Environment.  There is support for integrating<br>
Soar with C++, Java, Tcl, Python and C# applications.  Many Tools<br>
that aid in development of Soar programs are also included.<br>
<br>
These Release Notes describe the primary changes in this<br>
latest release.  More detailed information can be obtained at<br>
http://winter.eecs.umich.edu/soarwiki<br>
<br>
This release was generated for Windows and Linux systems.<br>
OS X support is pending a volunteer willing to work on it.<br>
<br>
Soar 8.6.3 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
* On Windows, the 19MB download file expands to ~46MB when installed.<br>
Linux disk requirements are similar.<br>
<br>
* Java 1.4.2 or newer is required to use the SoarJavaDebugger, which is<br>
required for most of the demos and applications.  (java must be<br>
available in the path). Get it at http://java.sun.com/. Most operating<br>
systems probably already have Java installed.<br>
<br>
* If you want to run Tcl applications on Linux, Tcl 8.4.x or later is<br>
required (note that the tutorial applications are now available in<br>
Java).  Also note that we do not currently support threaded builds<br>
of Tcl (e.g. ActiveTcl 8.4.11.2 or later).<br>
<br>
* The included Python libraries support Python 2.4.  Python 2.5 support<br>
requires a rebuild of the ClientSMLPython project.<br>
<br>
<br>
Components of Soar 8.6.3<br>
========================<br>
* pre-built libraries and executables for Windows (SoarLibrary/bin)<br>
* all source code, config files, and OS tools for automated Soar builds<br>
* SoarJavaDebugger<br>
* VisualSoar<br>
* JavaEaters, JavaTankSoar, TclEaters<br>
* Test and Demo programs<br>
* Documentation<br>
<br>
<br>
New in 8.6.3<br>
============<br>
<br>
* Components and Capabilities added<br>
-- Added support for multiple clients modifying different parts of the<br>
input link<br>
-- Added Python support<br>
-- Requires use of --enable-python configure flag on Linux<br>
-- Added CheckForIncomingEvents to help single-threaded applications<br>
-- Matches command can now generate XML output<br>
-- Added max-memory-usage command (not yet functional)<br>
-- Watching wmes (level 4 and higher) now reports GDS changes<br>
-- Added "after halted" event<br>
-- Improved output of the "preferences" command for Soar8.  New arg<br>
"--objects" added.  Invoke "help preferences" for usage<br>
-- New alias "support" added for "preferences --object"<br>
-- Right-clicking on identifiers and attributes in debugger allows<br>
"preferences" and "preferences --object" to be selected<br>
-- added TestSMLPerformance project<br>
-- Added ^status error support to TankSoar<br>
<br>
-- TankSoar can now be configured to stop after a specified number of<br>
updates<br>
-- Reset in TankSoar/Eaters now returns agents to starting locations<br>
if specified<br>
<br>
* Bug fixes<br>
-- Lots of TankSoar and Eaters bugfixes<br>
-- Fixed command-to-file command<br>
-- Fixed alias command for adding flags to aliases; previously the<br>
flags were sometimes interpreted as flags for the alias command<br>
itself rather than the command for which an alias was being<br>
created.  "alias" no longer takes any flags.  use "unalias" to<br>
remove defined aliases.<br>
-- An empty string can now be returned by a client message handler<br>
-- Fixed handling of comments by sp<br>
-- Fixed lots of memory leaks (including some very old ones in the<br>
kernel)<br>
-- Debugger bug fixes<br>
-- properly handle right-clicks in horizontally-scrolled window<br>
-- recognize identifiers in preferences command output for<br>
right-click purposes<br>
-- fixed up logging example apps (LoggerCWin, LoggerJava)<br>
-- Several other minor fixes<br>
<br>
* Other updates<br>
-- Minor performance improvements (~5%)<br>
-- Windows builds now done with Visual Studio 2005<br>
-- Now use Microsoft C library dlls<br>
-- VS2003 solution/project files still available<br>
-- Cleaned up Java build batch files on Windows<br>
-- Cleaned up Linux builds somewhat<br>
-- Updated to SWT 3.2.0<br>
<br>
<br>
Known issues:<br>
=============<br>
<br>
* TankSoar and Eaters have some redraw issues (forcing a redraw, e.g. by<br>
moving the window, fixes this).<br>
* Remote connections on Linux can be slow.<br>
* status complete appears on the output-link immediately instead<br>
of during the next input phase.<br>
* The Soar 7 mode implementation currently relies on an ugly hack. If<br>
you experience problems in this mode, please let us know.<br>
* In TankSoar, the obscure-bot sometimes generates a scary-looking<br>
kernel error message.  It appears to be safe to ignore.<br>
* Many other minor issues (see bugzilla for details).<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@umich.edu (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 8.6.2 #
<pre>Soar 8.6.2 Release Notes<br>
========================<br>
<br>
Soar-8.6.2 includes all source code, many demo programs, and a<br>
number of applications that serve as examples of how to interface<br>
Soar to an external Environment.  There is support for integrating<br>
Soar with C++, Java, Tcl and C# applications.  Many Tools that aid<br>
in development of Soar programs are also included.<br>
<br>
These Release Notes describe the primary changes in this<br>
latest release.  More detailed information can be obtained at<br>
http://winter.eecs.umich.edu/soarwiki<br>
<br>
This release was generated for Windows and Linux systems (OS X<br>
forthcoming).<br>
<br>
Soar 8.6.2 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
<br>
Installation Requirements<br>
=========================<br>
<br>
* On Windows, the 19MB download file expands to ~46MB when installed.<br>
Linux disk requirements are similar.<br>
<br>
* Java 1.4.2 or newer is required to use the SoarJavaDebugger, which is<br>
required for most of the demos and applications.  (java must be<br>
available in the path). Get it at http://java.sun.com/. Most operating<br>
systems probably already have Java installed.<br>
<br>
* If you want to run Tcl applications on Linux, Tcl 8.4.x or later is<br>
required (note that the tutorial applications are now available in<br>
Java).  Also note that we do not currently support threaded builds<br>
of Tcl (e.g. ActiveTcl 8.4.11.2 or later).<br>
<br>
<br>
Components of Soar 8.6.2<br>
========================<br>
* pre-built libraries and executables for Windows (SoarLibrary/bin)<br>
* all source code, config files, and OS tools for automated Soar builds<br>
* SoarJavaDebugger<br>
* VisualSoar<br>
* JavaEaters, JavaTankSoar, TclEaters<br>
* Test and Demo programs<br>
* Documentation<br>
<br>
<br>
New in 8.6.2<br>
============<br>
<br>
* Components and Capabilities added:<br>
-- C# added to list of supported languages (now C++, Java, Tcl, C#).<br>
-- Java versions of Eaters and TankSoar<br>
-- Support for arbitrary communication from one client to<br>
another (not just to/from the kernel)<br>
-- Tcl commands can now be used from the SoarJavaDebugger by running<br>
a separate Tcl filter alongside the debugger<br>
(this is a preliminary version in this release).<br>
-- Support for filtering of command line input in any supported<br>
language<br>
Filtering can be chained.<br>
-- Support for logging output in SoarJavaDebugger on a<br>
window-by-window basis<br>
-- Support for arbitrary logging of Soar execution through custom<br>
logging<br>
applications.  Sample programs included in Tools directory.<br>
-- Added new SML tutorial for developers (in Environments/JavaTOH)<br>
-- Ability to unregister an event during its callback execution<br>
-- Support for setting a "stop-point" before any phase in Soar's<br>
execution cycle. Upon interrupt, or when stepping, agents will<br>
stop at this location.  By default Soar will stop after the<br>
decision phase so users can examine the match set.<br>
-- "run 0" (zero) will move agents to the stop-point location,<br>
unless already there.  (mechanism for synching agents easily)<br>
-- Applications can interleave the execution of multiple agents by<br>
any increment supported in SoarKernel: elaboration, phase, decision<br>
or output-generation.<br>
-- Phase-specific event generation  (e.g. BEFORE_PROPOSE_PHAE &<br>
AFTER_PROPOSE_PHASE etc)<br>
-- Input wmes are now automatically committed by default removing the<br>
need<br>
to manually call commit().<br>
<br>
* Performance improvements:<br>
-- Windows performance improved (> 30% faster in Towers of Hanoi SML).<br>
-- Vastly improved Linux performance (~20x in some cases).<br>
-- Java versions of Eaters and TankSoar are many times faster than<br>
their Tcl equivalents.<br>
<br>
* Updates and code fixes<br>
-- New algorithm in SoarKernel supports more efficient garbage<br>
collection, won't blow callstack when lots of work done in subgoal,<br>
<br>
and  supports undefining DO_TOP_LEVEL_REF_CTS which inherently<br>
leaks memory at the top state<br>
-- Fixed logic for checking quiescence in Propose phase.  For some<br>
applications using I/O, could result in fewer SNCs in substates<br>
-- Reorganized directory structure to be more intuitive.<br>
-- Static libraries required for building SML applications are now<br>
distributed in SoarLibrary/lib.<br>
-- Many memory leaks fixed in SoarKernel, gSKI and SML.<br>
-- RunEvents generated directly in SoarKernel rather than by gSKI.<br>
-- SML RunScheduler rewritten.  Order of RunEvents guaranteed for all<br>
agents.<br>
-- init-soar support works much better for all clients.<br>
-- Updated Java apps to SWT 3.1.2.<br>
-- Java 5.0 supported.<br>
-- Visual Studio 2005 supported.<br>
-- Java Debugger now uses GTK interface by default in Linux (fixes<br>
many motif-specific bugs).<br>
-- Lots of SML changes to improve correctness and functionality.<br>
-- Registering for callbacks in Java now uses interfaces.<br>
-- Several Soar7-mode commands added back in.<br>
-- Soar kernel now uses a new random number generator.<br>
-- Improved portability and removed legacy files from SoarKernel.<br>
-- Lots of documentation updates.<br>
-- Many, many other changes (see bugzilla for details).<br>
-- more details posted at<br>
http://winter.eecs.umich.edu/soarwiki/Items_for_Consortium_Review<br>
<br>
<br>
Known issues:<br>
=============<br>
<br>
* TankSoar and Eaters have some redraw issues (forcing a redraw, e.g. by<br>
moving the window, fixes this).<br>
* Remote connections on Linux can be slow.<br>
* status complete appears on the output-link immediately instead<br>
of during the next input phase.<br>
* The Soar 7 mode implementation currently relies on an ugly hack. If<br>
you<br>
experience problems in this mode, please let us know.<br>
* In TankSoar, the obscure-bot sometimes generates a scary-looking<br>
kernel<br>
error message.  It appears to be safe to ignore.<br>
* Many other minor issues (see bugzilla for details).<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net (preferred)<br>
-or-<br>
soar-group@lists.sourceforge.net<br>
</pre>

---

# 8.6.1 #
<pre>Announcing Soar 8.6.1!<br>
======================<br>
<br>
This announcement describes the primary changes in this latest release.<br>
It also<br>
includes the 8.6.0 announcement for reference (read that first if you<br>
didn't<br>
download 8.6.0).<br>
<br>
Soar 8.6.1 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Requirements<br>
============<br>
<br>
* Java 1.4.2 or later required to use the Java debugger (java should be<br>
available in the path). Get it at http://java.sun.com/. Most machines<br>
probably already have this installed.<br>
<br>
* Tcl 8.4.x or later required to run Eaters on linux and OS X 10.3 (wish<br>
should be available in the path). Get it at http://www.tcl.tk/. OS X<br>
10.4<br>
comes with this pre-installed, and the Windows installer includes it.<br>
<br>
New in this release<br>
===================<br>
<br>
* Capabilities added:<br>
-- Linux and Mac support<br>
-- Soar 7 mode now works.  Some Soar7 demos are included as well.<br>
-- Tcl Eaters is included.  Tcl Eaters uses the familiar Eaters<br>
Control Panel,<br>
but the default Agent window is the new SoarJavaDebugger.  The TSI<br>
is only<br>
partially converted to the new SML interface to Soar, and the<br>
SoarJavaDebugger<br>
is more capable and better integrated.<br>
-- Java Missionaries and Cannibals is included.<br>
-- Added ability to filter trace output in debugger after a trace has<br>
been created<br>
-- Added ability to stop scrolling trace window in debugger<br>
-- Added partial command matching for command line.<br>
-- Introduced new event-driven simulation support, removing the need<br>
for<br>
"start-system" command (see "SML Quick Start Guide" for details).<br>
-- Clients can now determine when other clients are connected and<br>
ready<br>
-- sml.jar now included in soar-library for Java developers.<br>
-- TestSMLEvents application added: provides a rudimentary command<br>
line interface<br>
so the users can test the events and view examples of each.<br>
<br>
* Performance further improved:<br>
-- Improved and extended kernel XML generation resulting in<br>
performance boost<br>
for those applications which use it (i.e. the Java debugger)<br>
-- Better use of threads in Java debugger resulting in massive<br>
performance boost<br>
-- Replaced tree trace window in debugger with faster version<br>
-- New threading model in SML for better performace with multiple<br>
applications<br>
<br>
* Updates and fixes<br>
-- Running by phases and elaborations now works properly: Propose and<br>
Apply run<br>
to full quiescence when running by Phase; when running by<br>
Elaboration, Soar<br>
will always run one round of production firings, which might<br>
require that<br>
Input, Decide or Output phases be executed as well.<br>
-- Manual updated<br>
-- Tutorials for new debugger updated<br>
-- Added lots of aliases that existed in older versions of Soar<br>
-- Rewrote command line parsing code to avoid potential licensing<br>
issues<br>
-- Numerous other smaller bug fixes.<br>
<br>
Clarification regarding the "tcl" RHS function:<br>
===============================================<br>
<br>
While we are no longer providing a built-in "tcl" RHS function, there is nothing<br>
to prevent someone from registering the old "tcl" RHS function which does the<br>
same thing. Those needing to support legacy code should do this.  Those<br>
applications will then require that Tcl is always part of the build.<br>
For new applications,  "exec" provides the same functionality in a<br>
language-independent way.<br>
<br>
Known issues:<br>
=============<br>
<br>
* VisualSoar seems to have intermittent issues on some OS X machines.<br>
This appears to be a bug in Java on that platform.<br>
* Log command only works properly from Java debugger if in text mode (as<br>
opposed to tree mode).<br>
* Tanksoar is not yet available.<br>
* Soar seems to "stutter" under Mac OS 10.4 (but it does work properly).<br>
* Sometimes the trace output can fall behind actual execution, making<br>
the debugger appear unresponsive (it's really just trying to catch up).<br>
* Some rare warning messages may not appear fully in the Java debugger's<br>
tree view (but they appear fine in the text view). If you find specific<br>
cases of this, please let us know.<br>
* Pausing trace window in debugger causes the text to flicker on Windows<br>
and Linux and doesn't work well on OS X (these are os and SWT issues; we can't<br>
fix them).<br>
* VisualSoar can only connect to kernel using default values for<br>
port/ip.<br>
* When running Eaters on Linux with a debugger connected, the debugger<br>
seems to "steal" the focus while Soar is running.<br>
* For Soar7 mode, the commands attribute-preferences-mode and<br>
input-period  are not yet implemented.<br>
* Also in Soar 7 mode, a potential crash from tentative-retractions of<br>
justifications, that has existed since 8.5.2 and possibly earlier,<br>
is avoided with an ugly hack;  please contact us if you suspect you are<br>
running into problems related to this issue.<br>
<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-group@lists.sourceforge.net<br>
</pre>


---

# 8.6.0 #
<pre>Announcing Soar 8.6.0!<br>
======================<br>
<br>
This announcement describes the primary changes in this latest release.<br>
Among<br>
them are:<br>
<br>
* Limitations in this release<br>
* SML: A new XML-based API for interfacing with Soar<br>
* A new Soar debugger written in Java<br>
* Command line changes<br>
<br>
Soar 8.6.0 can be downloaded from the Soar homepage at:<br>
<br>
http://sitemaker.umich.edu/soar<br>
<br>
Limitations in this release<br>
===========================<br>
<br>
* 8.6.0 is a Windows-only release.<br>
* The manual needs more work (but it is included).<br>
* Soar7 mode (i.e. soar8 --off) does not work yet.<br>
* The tutorial needs to be updated so it is not included.<br>
* Eaters and TankSoar are not included.<br>
* The Java components have only been tested on Java 1.4.2. There may be<br>
problems<br>
if used with Java 1.5.<br>
* The "Soar Runtime" menu in VisualSoar is not functional.<br>
<br>
We anticipate an 8.6.1 release before the Soar workshop which should<br>
include<br>
Linux and Mac releases, support for Soar7 mode, and will possibly<br>
address some<br>
of these other issues as well.<br>
<br>
If any other issues or bugs are found, please send mail to:<br>
<br>
soar-sml-list@lists.sourceforge.net<br>
<br>
SML: A new XML-based API for interfacing with Soar<br>
==================================================<br>
<br>
Interfacing to Soar has always been something of a challenge.  Initially<br>
we were<br>
limited to building interfaces in the Soar implementation language (LISP<br>
then<br>
C/C++).  We extended this to allow the use of Tcl but the extension<br>
ended up<br>
requiring Tcl for all Soar systems.  SGIO opened up support for having a<br>
simulation running either with Soar embedded or as a separate process<br>
but it was<br>
limited to sending just I/O information which prevented debugging of<br>
embedded<br>
processes.  gSKI provided a clean interface into the kernel but support<br>
for<br>
other languages, debugging or remote I/O has yet to be implemented.<br>
<br>
With this 8.6 release we are providing an alternative approach for<br>
interfacing<br>
into Soar called SML (Soar Markup Language). This interface:<br>
<br>
* supports multiple languages (Java, C++ and Tcl currently) while<br>
removing any<br>
requirement on Tcl<br>
<br>
* supports a uniform method for building I/O interfaces (for<br>
simulations) as<br>
well as command interfaces (for debugging)<br>
<br>
* supports embedding Soar kernels within a simulation or debugger and<br>
communicating remotely between the two<br>
<br>
* supports multiple clients (debuggers, simulations, other tools)<br>
connecting<br>
to a single kernel instance<br>
<br>
* supports dynamic connection and disconnection of tools (esp debuggers)<br>
from a running kernel<br>
<br>
* provides a uniform, high level and data-driven model for the interface<br>
while<br>
retaining high performance<br>
<br>
* moves command line support out of the kernel, while providing<br>
universal<br>
access to it from any client<br>
<br>
* includes a new, cleaned up command line syntax<br>
<br>
* in many cases the interface is faster (sometimes an order of magnitude<br>
faster) than our 8.5.2 interfaces<br>
<br>
* includes gSKI in the main Soar Consortium kernel for the first time<br>
<br>
* follows the SGIO interface model in many ways, making converting<br>
existing<br>
simulations easier<br>
<br>
Through these changes, the kernel itself has been largely unchanged so<br>
sets of<br>
productions from 8.5.2 should run identically in 8.6 although some<br>
command line<br>
arguments (e.g. "learning -off") have changed slightly so they will<br>
require<br>
updating.  Details on that below.<br>
<br>
The Soar Java debugger<br>
======================<br>
A preliminary new debugger in Java is provided which interfaces with<br>
Soar via SML.<br>
This new debugger:<br>
<br>
* has much higher performance than the TSI for detailed traces<br>
* already offers a structured view of high watch traces<br>
* can be dynamically attached to and detached from remote Soar instances<br>
* is designed to be highly customizable by each user<br>
* employs a plug-in architecture to allow for modular extensions in the<br>
future<br>
<br>
At this point the debugger should be considered a beta version.<br>
<br>
Command Line Changes<br>
====================<br>
One of the major changes in Soar 8.6 is syntax changes for the commands.<br>
The<br>
syntax was changed to increase consistency across commands and greatly<br>
increase ease of parsing of commands (making adding new commands much<br>
easier).<br>
Unfortunately, this means that some common commands used in Soar source<br>
files need<br>
to be changed before those files will load in Soar 8.6.<br>
<br>
Options now require two dashes before the unabbreviated name (e.g. run<br>
--decision 3)<br>
and a single dash before the abbreviated name (e.g. run -d 3).<br>
<br>
For complete documentation on the current syntax of the commands, use<br>
the "help"<br>
command, read the manual, or read the documentation online at:<br>
http://winter.eecs.umich.edu/soarwiki/Soar_Command_Line_Interface.<br>
<br>
For your convenience, here are some of the most common changes that are<br>
required<br>
to allow existing source files to load:<br>
<br>
* learn -on => learn --on<br>
* set warnings off => warnings --off<br>
<br>
RHS function changes:<br>
<br>
* "tcl" is no longer supported.  It has been replaced by "exec" (for<br>
user-provided<br>
functions) and "cmd" to execute standard command-line commands.<br>
User-provided<br>
functions must be registered with the Soar kernel via a call to<br>
Kernel::AddRhsFunction.<br>
<br>
* Built-in RHS functions like "write" and the math functionality are<br>
unchanged.<br>
<br>
* See sections 3.3.6.6 to 3.3.6.12 (especially 3.3.6.11) for more<br>
information on RHS<br>
functions.<br>
<br>
These common commands have NOT changed:<br>
<br>
* multi-attributes<br>
* pushd<br>
* popd<br>
* sp<br>
<br>
To learn more about SML, the debugger or how to build a simulation using<br>
the new<br>
interface please see the documentation included with the release.<br>
</pre>


---

# 8.5.2 #
<pre>We are pleased to announce that Soar Suite 8.5.2 is ready for the<br>
Windows and<br>
Linux platforms! Download it here:<br>
<br>
--> http://sitemaker.umich.edu/soar/soar_software_downloads<br>
<br>
We expect this to be the last 8.5.x release. Soar Suite 8.6.0, expected<br>
fall<br>
2004, will be the next release and will include gSKI/TgD.<br>
<br>
*** Components included in this release: Soar Kernel 8.5.2, VisualSoar<br>
4.5.3,<br>
Eaters 3.0.8, TankSoar 3.0.8, DMGen 1.0.0, SoarDoc 0.9.3, SGIO 1.1.2,<br>
Documentation (including Soar manual and tutorial).<br>
<br>
*** Almost every component has been updated for this release, some<br>
significantly. Please check out the release notes (available with the<br>
installer<br>
and on the web page).<br>
<br>
*** All necessary Tcl/Tk files are included with the Windows version.<br>
Linux<br>
users will have to install Tcl/Tk, see installation notes for details.<br>
<br>
*** Mac OSX is currently not available but will be soon.<br>
<br>
*** Visit the Soar home page for news, documentation, and updates:<br>
--> http://sitemaker.umich.edu/soar/<br>
<br>
*** Report all Soar suite bugs using the Soar Bugzilla bug tracking<br>
system.<br>
Please do not use the trackers on SourceForge.net.<br>
--> http://winter.eecs.umich.edu/soar-bugzilla/<br>
<br>
*** Thank you to all who helped make this release possible!<br>
<br>
Summary of changes between SoarSuite 8.5.1 and SoarSuite 8.5.2<br>
July, 2004<br>
<br>
<br>
A summary of changes in SoarSuite 8.5.2 is listed immediately below. For<br>
more<br>
details, including changes not mentioned in the summary and a list of<br>
known<br>
issues see the Details section after the summary.<br>
<br>
======================<br>
New Windows Installer:<br>
======================<br>
<br>
The Windows installer is now built with a different technology which<br>
is much easier for us and results in a much smaller package.<br>
<br>
=========================<br>
Several kernel bug fixes:<br>
=========================<br>
<br>
Almost all known bugs fixed, including all known crash bugs. See the<br>
details section below for more information.<br>
<br>
==================<br>
New TankSoar Bots:<br>
==================<br>
<br>
TankSoar now comes with two new bots.  Mapping-bot demonstrates how a<br>
tank might build up an internal map of the level.  Obscure-bot is a<br>
challenge bot<br>
for people to test their own bots against.  The code is provided in an<br>
obfuscated<br>
binary.  See obscure-bot.txt in the TankSoar agents directory for<br>
details.<br>
<br>
==========================<br>
SoarDoc and DMGen updated:<br>
==========================<br>
<br>
These changes were actually in the the 8.5.1 release, but we failed<br>
to note them.<br>
SoarDoc:<br>
- Fixed special character handling when UseExistingComments is used.<br>
- Added ShowProdSourceInline which shows production source code<br>
inline in the production documentation<br>
- Ability disable datamap node coloring<br>
- Fixed case-sensitivity issues for production names and list sorting.<br>
DMGen:<br>
- Fixed handling of mixed-case production names.<br>
<br>
======================<br>
Documentation Updated:<br>
======================<br>
<br>
The tutorials have had some typo fixes and screenshot updates, and<br>
also<br>
explain substates better.<br>
<br>
==================<br>
Tcl Soar Debugger:<br>
==================<br>
<br>
The TSDebugger has several bug fixes which may have affected some<br>
users.<br>
<br>
===================<br>
VisualSoar Updated:<br>
===================<br>
<br>
VisualSoar has several small bug fixes.<br>
<br>
======================<br>
Critical SGIO bug fix:<br>
======================<br>
<br>
Our fix for bug 254 in the last release actually caused serious<br>
problems. This has been fixed for real now.<br>
<br>
====================================================================<br>
Details<br>
====================================================================<br>
<br>
Below is a fairly complete list of enhancements, changes, bug fixes, and<br>
known<br>
issues, with bug numbers for reference. For more information and up to<br>
date list<br>
of what's been fixed please visit the Soar bugzilla server at:<br>
<br>
https://winter.eecs.umich.edu/soar-bugzilla/.<br>
<br>
New bug submissions and enhancement requests are welcome!<br>
<br>
<br>
Soar-8.5.2<br>
==========<br>
<br>
Bug Fixes<br>
---------<br>
353  stats command keeps recording between runs<br>
389  missing error handling for set_watch_prod_group_setting<br>
<br>
Enhancements<br>
------------<br>
358  change soar8 command error message<br>
<br>
Known Issues<br>
------------<br>
312  agent structure isn't cleaned up properly<br>
356  All matches get same support as last processed match<br>
<br>
TSDebugger 4.0.1<br>
================<br>
<br>
Bug Fixes<br>
---------<br>
365  rules that contain a capital letter break productions tab<br>
376  stop button doesn't work with Eaters, TankSoar<br>
396  error when closing group from task bar<br>
398  extra 0 gets printed when soar connected to environments<br>
<br>
Known Issues<br>
------------<br>
336  negated conditions don't print properly in the console window<br>
397  grouping bar in production tab not always right<br>
<br>
SGIO 1.1.2<br>
==========<br>
<br>
Bug Fixes<br>
---------<br>
254  Some special characters in StringWME's do not transfer to Soar<br>
<br>
Eaters 3.0.8<br>
============<br>
<br>
Bug Fixes<br>
---------<br>
391  garbage printed to debugger output when Stop After Decision Phase<br>
is on<br>
<br>
TankSoar 3.0.8<br>
==============<br>
<br>
Bug Fixes<br>
---------<br>
map loading menu now defaults to maps directory<br>
188  Selecting Map->Random Map causes error message when agent present<br>
on<br>
current map<br>
392  garbage printed to debugger output when Stop After Decision Phase<br>
is on<br>
393  command 'run 1' generates 'invalid command step' error<br>
<br>
Enhancements<br>
------------<br>
new challenge bot (obscure-bot) and mapping bot provided<br>
<br>
Known Issues<br>
------------<br>
399  can't use reset tank command in tank control panel tanks menu<br>
<br>
Documentation<br>
=============<br>
<br>
Bug Fixes<br>
---------<br>
349  tutorial images need to be updated<br>
375  typo in tutorial part 1: move-north<br>
386  typos in tutorial part 1 (search control)<br>
<br>
Enhancements<br>
------------<br>
Added information to tutorial about substates.<br>
<br>
Known Issues<br>
------------<br>
380  typo in section 5.6.3<br>
381  o-support-mode command missing newest support modes<br>
<br>
VisualSoar 4.5.3<br>
================<br>
<br>
Bug Fixes<br>
---------<br>
361  adding a dir that doesn't exist to agent path on file->new project<br>
fails with no warning<br>
362  initial rules in a project are not justified correctly<br>
363  if project loaded then create new project, title bar does not<br>
change<br>
364  file with same name as datamap have same window names<br>
366  Initialize-<task> Operator not in datamap<br>
368  dragging node onto itself makes node disappear<br>
372  fails to detect extra whitespace after :<br>
<br>
Known Issues<br>
------------<br>
Too many to list. See bugzilla for details.<br>
</pre>


---

# 8.5.1 #
<pre>Summary of changes between SoarSuite 8.5.0 and SoarSuite 8.5.1<br>
June, 2004<br>
<br>
<br>
A summary of changes in SoarSuite 8.5.1 is listed immediately below. For<br>
more<br>
details, including changes not mentioned in the summary and a list of<br>
known<br>
issues see the Details section after the summary.<br>
<br>
==================<br>
Several bug fixes:<br>
==================<br>
<br>
Almost all known bugs fixed, including all known crash bugs. See the<br>
details<br>
section below for more information.<br>
<br>
======================<br>
Documentation Updated:<br>
======================<br>
<br>
The manual has been updated to include appendices on the BNF grammar<br>
for Soar's syntax, the calculation of O Support, the resolution of<br>
preferences<br>
and the Goal Dependency Set, as well as many other small changes. The<br>
manual<br>
is also now searchable, and the table of contents has links to the<br>
associated<br>
sections. Finally, the tutorial has been reorganized.<br>
<br>
==================================<br>
New Tcl Soar Debugger (TSI 4.0.0):<br>
==================================<br>
<br>
The Tcl debugger, TSI, has undergone a major revision, improving its<br>
intuitiveness and presentation of information dramatically. For more<br>
details,<br>
see the TSI 4 Tour documentation.<br>
<br>
=======================<br>
Updated Version of Tcl:<br>
=======================<br>
<br>
The system has been tested with the latest version of Tcl, 8.4.6, and<br>
the Windows distribution ships with this version included.<br>
<br>
========================<br>
Learning Off by Default:<br>
========================<br>
<br>
In previous versions of Soar, learning was turned on by default. This<br>
could be a source of confusion and frustration since most users do not<br>
intend to<br>
have learning on. Thus, starting with Soar 8.5.1, learning is turned<br>
off.<br>
<br>
====================================<br>
O Support Mode 4 is now the Default:<br>
====================================<br>
<br>
O Support Mode 4 was introduced in Soar 8.5.0, and starting with Soar<br>
8.5.1 it is now the default mode. Those requiring other modes (i.e. for<br>
compatibility) can switch to them using the o-support-mode command.<br>
O Support Mode 3 (the default mode in Soar 8.3 - 8.5.0) is meant to<br>
give I-Support to elaborations of the operator.  At certain times,<br>
these elaborations were not detected as such, and received O-Support.<br>
Soar 8.5's O Support Mode 4 can overcome these problems by detecting<br>
more operator elaborations.  However, it also makes the following change<br>
from O-Support Mode 3:<br>
mixed productions (i.e. operator applications + operator elaborations)<br>
now receive i-support instead of o-support.  Running with 'warnings -on'<br>
will print messages when any such productions fire.  You should try<br>
to be careful not to build any such mixed productions when using<br>
o-support mode 4<br>
While O Support Mode 3 remains the default for this release, this is<br>
for backwards compatibility only.  It is recommended that O Support Mode<br>
4 is<br>
used from now on; in a future release, O Support Mode 4 will be the<br>
default.<br>
<br>
======================<br>
New Interrupt Command:<br>
======================<br>
<br>
Soar 8.5.1 now allows matchtime interrupts to be turned on and off at<br>
runtime. Matchtime interrupts were introduced in Soar 8.5.0 (see<br>
immediately<br>
below for details). The syntax for the interrupt command is:<br>
<br>
interrupt [-on|-off] [production name]<br>
<br>
If the -on or -off switch is used without a production name, then the<br>
list of all productions with interrupts turned on or off is given. If<br>
the<br>
production name is used without the -on or -off switch, then the<br>
interrupt<br>
status of the production is given.<br>
<br>
Soar 8.5.0 introducted support for interrupting execution at<br>
production<br>
match-time in addition to interrupts when a production matches.  This<br>
new<br>
capability makes it possible to examine Soar's state before the<br>
production<br>
fires, and even makes it possible to excise the production that matches<br>
before<br>
its effects are realized.  To use the matchtime interrupt feature,<br>
productions<br>
must add the ':interrupt' directive as in the following example<br>
production:<br>
<br>
sp { interrupt*on*application<br>
:interrupt<br>
(state <s> ^operator.name stop)<br>
--><br>
(write (crlf) |I've already Halted|)}<br>
<br>
Note that supporting this feature adds a small amount of computational<br>
overhead<br>
to the match phase.  If you desire a maximally streamlined version of<br>
Soar,<br>
recompile /without/ the MATCHTIME_INTERRUPT flag in soarBuildOptions.h<br>
<br>
===================<br>
VisualSoar Updated:<br>
===================<br>
<br>
VisualSoar has undergone several usability improvements for this<br>
development<br>
iteration.  Key among these are:<br>
- highlighting code for the user that has been identified<br>
as the cause of a syntax error, datamap error or text search.<br>
- improvements to window focus, size and positioning.  In the past,<br>
VisualSoar has been a little too "bullheaded" about enforcing the<br>
default display type of tile mode.  Users who maximize windows or<br>
cascade them will find VS easier to manage.<br>
- some additional search capabilities.  In particular, it's easy to<br>
find code corresponding to entries in the datamap and also easy to<br>
find the datamap that goes with particular code.  You can also<br>
limit your text searches to specific operator subtrees.<br>
- Finally, VisualSoar has been made compatible with MacOS.<br>
<br>
<br>
====================================================================<br>
Details<br>
====================================================================<br>
<br>
Below is a fairly complete list of enhancements, changes, bug fixes, and<br>
known<br>
issues, with bug numbers for reference. For more information and up to<br>
date list<br>
of what's been fixed please visit the Soar bugzilla server at:<br>
<br>
https://winter.eecs.umich.edu/soar-bugzilla/.<br>
<br>
New bug submissions and enhancement requests are welcome!<br>
<br>
<br>
Soar-8.5.1<br>
==========<br>
<br>
Enhancements<br>
------------<br>
186  New interrupt command<br>
<br>
Changes<br>
-------<br>
338  turn off learning by default<br>
339  default o-support-mode changed from 3 to 4<br>
351  some demos need to be updated<br>
<br>
Bug Fixes<br>
---------<br>
139  Soar crashes when it spends some time in a state no-change substate<br>
204  Soar crashes when multiple agents are running and one of them halts<br>
or<br>
reaches an interrupt<br>
213  GDS crashes when backtracing through result WMEs<br>
214  fixed behavior of "monitor -delete" command<br>
274  operator treated as keyword even when not attached to a state<br>
154  Print message explaining that can't save RETE when justifications<br>
are<br>
present.<br>
311  extra newline in Soar output<br>
278  o-support mode 3 & 4 do not correctly identify mixed<br>
elaboration-application productions<br>
194  warnings when running demos in O Support Mode 4<br>
357  o-support determination fails to recognize operator elaboration<br>
355  New GDS bug<br>
<br>
Known Issues<br>
------------<br>
312  agent structure isn't cleaned up properly<br>
353  stats command keeps recording between runs<br>
356  All matches get same support as last processed match<br>
<br>
TSI 4.0.0<br>
=========<br>
<br>
Enhancements<br>
------------<br>
Too many to list -- really rewritten to a large extent.<br>
<br>
Bug Fixes<br>
---------<br>
215  Learn settings printed in log when stepping<br>
206, 211, 212  The output of some commands wasn't redirectable, causing<br>
problems<br>
in the TSI<br>
192  wish.exe process doesn't go away sometimes<br>
<br>
Known Issues<br>
------------<br>
327  bottom of phase graphic is cut off in non-minimal views<br>
336  negated conditions don't print properly in the console window<br>
<br>
SGIO 1.1.1<br>
==========<br>
<br>
Bug Fixes<br>
---------<br>
254  Spaces and $'s in StringWME's do not transfer to Soar<br>
<br>
Eaters 3.0.7<br>
============<br>
<br>
Bug Fixes<br>
---------<br>
197  Eaters sets TSI version number<br>
<br>
TankSoar 3.0.7<br>
==============<br>
<br>
Enhancements<br>
------------<br>
352 updates to default agent<br>
<br>
Bug Fixes<br>
---------<br>
196  TankSoar sets TSI version number<br>
<br>
Known Issues<br>
------------<br>
188  Selecting Map->Random Map causes error message when agent present<br>
on<br>
current map<br>
151  Can't use run <n> command in TankSoar<br>
<br>
Documentation<br>
=============<br>
<br>
Enhancements<br>
------------<br>
343  Make manual searchable<br>
344  Make table of contents into links<br>
350  reorganize tutorials<br>
341  Preference resolution appendix needs updating<br>
345  section 1.3: "contacting the Soar group" is outdated<br>
348  need GDS description<br>
340  BNF grammar appendix needs to be updated<br>
342  o-support appendix needs to be updated<br>
<br>
Known Issues<br>
------------<br>
349  tutorial images need to be updated<br>
<br>
VisualSoar 4.5.2<br>
================<br>
<br>
Enhancements<br>
------------<br>
3    Expand datamap as it is being generated.<br>
37   Document all shortcut and key bindings.<br>
48   Search / replace should highlight the line found<br>
53   VS doesn't display new windows in size/position that is intuitive<br>
91   Parse error should highlight offending line.<br>
92   Remove parse error pop up dialog and place all errors in feedback<br>
pane<br>
93   Allow separate syntax and datamap checks.<br>
<br>
100  Change template default from "01" to Op1" to reduce confusion<br>
159  The save operation should always save the entire project<br>
171  agent name in title bar<br>
222  Better tab-formatting for multiple values on separate lines<br>
225  Allow searching within a specific operator subtree<br>
255  IO Error message fails to specify filename that caused the error<br>
263  Syntax error should not popup dialog when checking productions<br>
265  Find productions that use a particular datamap WME<br>
333  Should be able to generate datamap for just one file<br>
<br>
Bug Fixes<br>
---------<br>
90   Undo should move the cursor (and display) to what is being undone<br>
106  Better tab-formatting after a comment and after a condition<br>
122  If you close an editor window another window does not become active<br>
128  The neon green color used in data map verification is very annoying<br>
131  Opening a new project should close old<br>
133  Close project menu options don't invoke same dialog as close button<br>
153  VisualSoar broken on Mac OS X<br>
177  Fix spelling and capitalization errors in VS menu<br>
199  The KnownIssues.txt file should be removed from the CVS archive<br>
200  VS contains deprecated function calls<br>
205  Runtime menu should be deactivated when the library is absent<br>
220  New file windows should receive focus<br>
228  Syntax coloring is incorrect for ".{"<br>
250  Syntax coloring is incorrect for { ... } { ... }<br>
262  Operator with a non-enumeration named "name" causes crash<br>
<br>
Known Issues<br>
------------<br>
Too many to list. See bugzilla for details.<br>
</pre>


---

# 8.5.0 #
Soar 8.5.0 is now available for Windows platforms and Linux! Mac OSX
support is forthcoming. Download the installer (Win32) or tarball
(Linux) and get installation instructions here:
http://sitemaker.umich.edu/soar/soar_software_downloads

## Release notes for Soar Suite 8.5 ##
  * For the Win32 platform, all Soar components are hosted on
SourceForge.Net in one installer. A Linux tarball is also available at
the same URL. http://sourceforge.net/projects/soar/
  * The Soar web site has been redesigned and moved to a new location.
Links to the old site should redirect to the new site.
http://sitemaker.umich.edu/soar/
  * VisualSoar has been updated for this release with many bug fixes and
is available for the Win32 platform (VisualSoar support for Linux and
Mac OSX is forthcoming).
  * The Soar manual has been updated and is included in the installer.
  * Per a renewed commitment to bug fixing in Soar, the Soar project now
uses Bugzilla for bug tracking and feature requests. All bug tracking
and feature requests should be posted using the new Soar Bugzilla and
not with SourceForge.Net's trackers.
http://winter.eecs.umich.edu/soar-bugzilla
  * New SoarDoc and DataMap Generator tools from Soar Technology are
included with this Soar 8.5.0 release
http://www.eecs.umich.edu/~soar/sitemaker/projects/soardoc/soardoc.html
  * Major changes for Soar 8.5.0:
    * All known crashes fixed.
    * Numeric indifference preferences added.
    * 8.3.5 and 8.4.5 have been merged, only one version of Soar is
needed now.
    * All Soar components have been combined into one installer.
    * Updated for latest version of Tcl/Tk (8.4.4).
    * Various bug fixes, see CHANGES for details
  * This release was made possible by: Mazin Assanie, Karen Coulter,
John Laird, Bob Mariner, Shelley Nason, Andrew Nuxoll, David Ray,
Jonathan Voigt, Robert Wray and Soar Technology.

Summary of changes between Soar 8.4.5 and Soar 8.5. Scott Wallace
August, 2003

The change from Soar8.4.5 to Soar8.5 introduces some new features, and
bug fixes. Below is a brief summary of the salient changes (all
modifications have been verified with the soar-8.5-test-suite which is
released independently from this source code).

## Operator No Changes ##

In Soar 8.4.5, Operator No Changes were identified agressively by the
architecture. This meant that they could be created before the end of
the decision cycle. In some situations, this led to odd behavior,
notably within some of the tutorial lessions. As a result, this behavior
has now become a compile-time option and is no longer included in the
deafult-builds. If you desire Soar with aggressive no changes, simply
add the AGRESSIVE\_ONC flag to soarBuildOptions.h and recompile.

## MatchTime Interrupt ##

Soar 8.5 now supports interrupting execution at production match-time in
addition to interrupts when a production matches. This new capability
makes it possible to examine Soar's state before the production fires,
and even makes it possible to excise the production that matches before
its effects are realized. To use the matchtime interrupt feature,
productions must add the `:interrupt` directive as in the following
example production:
<pre>sp { interrupt*on*application<br>
:interrupt<br>
(state <s> ^operator.name stop)<br>
--><br>
(write (crlf) |I've already Halted|)<br>
}<br>
</pre>

Note that supporting this feature adds a small amount of computational
overhead to the match phase. If you desire a maximally streamlined
version of Soar, recompile /without/ the MATCHTIME\_INTERRUPT flag in
soarBuildOptions.h

## Reject Preferences ##

Earlier versions of Soar processed all reject preferences after
acceptable preferences. One of the unintuitive side effects of this is
that when a user wants to change the value of a wme, they typically need
to test that its current value is not equal to the new value. As a first
attempt to alleviate this artifact, by default Soar 8.5 changes the
order in which preferences are evaluated by examining O-Supported Reject
preferences first thus (^cheese-type cheddar - chedder +) will result in
the wme (... ^cheese-type cheddar) remaining in working-memory. Note
that this may have some unanticipated consequences, and can be turned
off be undefining the O\_REJECTS\_FIRST flag in soarBuildOptions.h and
recompiling.

## Cmdline Interface Parser ##

Now appropriately parses strings containing the '#' character.

## Run Till Output ##

In Soar 8.3 - 8.4.5, if an output command was issued in the first
decision-cycle, it would not be appropriately detected. As a result, if
Soar was instructed to run until output, and output occured on the 1st
decision cycle, Soar would continue running for 15 decision cycles
before returning control to the user. This bug was most often noticed
using simple agents in TankSoar, and is now fixed in Soar 8.5.

## O Support Mode 4 ##
O Support Mode 3 (the default mode in Soar 8.3 - 8.5.0) is meant to give
I-Support to elaborations of the operator. At certain times, these
elaborations were not detected as such, and received O-Support. Soar
8.5's O Support Mode 4 can overcome these problems by detecting more
operator elaborations. However, it also makes the following change from
O-Support Mode 3:

mixed productions (i.e. operator applications + operator elaborations)
now receive i-support instead of o-support. Running with 'warnings -on'
will print messages when any such productions fire. You should try to be
careful not to build any such mixed productions when using o-support
mode 4

While O Support Mode 3 remains the default for this release, this is for
backwards compatibility only. It is recommended that O Support Mode 4 is
used from now on; in a future release, O Support Mode 4 will be the
default.

## Numeric Indifference Preferences ##

Soar 8.5 has better capabilities to support behavioral diversity than
previous versions of Soar. The difference stems from Soar 8.5 numeric
indifferent preferences that allow operators to be assigned weights
along with the '=' preference. In the event of a tie, the indifference
selection mechanism is invoked. In previous versions of Soar, the
probability of selecting any of the tied operators was distributed
evenly among the candidates (assuming the indifferent-selection mode was
'random'). Soar 8.5's numeric indifferent preferences allow the
programmer to skew this distribution such that certain operators are
chosen more or less frequently. Preferences are specified simply by
adding an integer or float value following an operator's '=' preference.
As in the example below:

<pre>sp { example<br>
(state <s> superstate nil -initialized true)<br>
--><br>
(<s> ^operator <o> + = 30)<br>
(&lt;o&gt; ^name initialize-a)<br>
}<br>
</pre>

Two algorithms are available via a run-time switch:
<pre>numeric-indifferent-mode -avg<br>
</pre>

Here, we calculate the 'average' preference value associated with a
particular operator. Thus, if preference A for operator X has value 30
while preference B for operator X has value 60, the average will be 45.
Once this value has been calculated for all competeing operators, we sum
their average preferences and select a random number between 0 and this
sum. Each operator is associated with a range on this scal equal to its
average numeric preference. Thus, the randomly selected number can
simply be mapped back to an operator that then becomes for 'selected'
operator for the current decison. In average mode, indifferent
preferences without a numeric value are given the default value of 50
for selection purposes. For programmers, we expect that this mode of
numeric-indifferent selection will be most intuitive. Note however, that
this scheme doesn't make a lot of sense for preferences are less than
zero. If an operator's average value is ever below zero, Soar will print
a warning indicating this situation. As an initial recommendation, we
suggest using values between 0.0 and 100.0 for your operators.

<pre>numeric-indifferent-mode -sum<br>
</pre>

Here, the preferences for each operator are first summed. Thus, if
preference A for operator X has value 30 while preference B for operator
X has value 70.5 the sum will be 100.5. This sum is calculated for all
competing operators. Next, we calculate: e ^ (preference\_sum /
agent\_temperature) where agent\_temperature is a compile time constant
currently set at 25.0 We'll call this value the operator's final numeric
preference. Finally, we sum all the competing operator's final numeric
preferences and select a random number between 0 and this sum. Each
operator is associated with a range on this scale equal to the size of
its final numeric preference. Thus, the randomly selected number can
simply be mapped back to an operator that then becomes the 'selected'
operator for the current decision. In the sum mode, indifferent
preferences without a numeric value are given the default value 0.0 for
the selection purposes. Note it does make sense to use this methodology
even if the numeric preferences are negative, however, because the
numeric preference are used in exponientation, their values should be
somewhat limited so floating-point overflow does not become a problem.
We expect this sum mode to be more useful for future
reinforcement-learning methods, and have included it here mainly for
experimental purposes.

## Timer Warnings ##

Timer warnings are not displayed unless the following are true:

  * Soar was compiled with the WARN\_IF\_TIMERS\_REPORT\_ZERO flag
  * at runtime, the 'warnings' sysparam is set to TRUE (as with the
'warnings -on' command

Other changes for Soar 8.5 made after the above document was written:
  * Removal of many depricated Tcl/Tk functions, upgrading them to
Tcl/Tk 8.4.4.
  * Various stability enhancements, including:
    * Kernel now uses snprintf() instead of sprintf() whenever possible.
    * Kernel now uses strncpy() instead of strcpy() whenever possible.
    * Kernel now uses strncat() instead of strcat() whenever possible.
  * 'verbose' command bug fix, it was not working as documented when
multiple agents were active.
  * 'init-soar' kernel crash fixed in both Eaters and TankSoar.
  * MSVC and GCC warning levels moved to maximum, all warnings silenced.
  * Logging bug fixed, did not work properly with multiple active
agents.
  * 'indifferent-selection -ask' command fixed.
  * Decision cycle bug fix.
  * All demos are in VisualSoar format now.
  * Fixed a kernel crash related to justifications.