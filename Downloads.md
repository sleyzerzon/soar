### Download Soar ###

The current stable version of Soar is **9.3.4**.

This release of Soar continues the 9.3 line which includes modules for reinforcement learning (RL), episodic memory (EpMem), and semantic memory (SMem), as well as everything from previous versions of Soar. It includes a new "forgetting" mechanism, several bug fixes, a new distribution structure, and a simplification of how libraries are built in Soar.  All learning mechanisms are disabled by default.

[Here are full release notes for the 9.3.4 release](ReleaseNotes#9.3.4.md).

**Note**: If you are new to Soar, you'll probably want to download the [Soar Tutorial package](SoarTutorial.md), which is a single self-contained package with all the things you'll need to go through the Soar Tutorial, namely Soar, the Java Debugger, VisualSoar, Eaters, TankSoar, sample agents, and Soar documentation.  It's also an excellent companion download for those reading [The Soar Cognitive Architecture](http://mitpress.mit.edu/catalog/item/default.asp?ttype=2&tid=12784).  The downloads below only contain Soar, VisualSoar and the debugger.

### Prerequisites ###

Downloading and installing Java is required since most users will want to use the Soar Java Debugger and other Soar components written in Java.

  * If you are running Ubuntu Linux or OSX, you probably already have Java installed.
  * Otherwise, you can download the [latest version of the Sun JRE or JDK here](http://www.oracle.com/technetwork/java/javase/downloads/index.html).
  * **Important**: the 64-bit version of Soar require the 64-bit version Java!  Make sure to download the correct one.

#### Windows ####
  1. Determine if you're running 32- or 64-bit Windows.[Click here if you need help figuring out which type of Windows you are running.](http://windows.microsoft.com/en-US/windows-vista/32-bit-and-64-bit-Windows-frequently-asked-questions)
  1. Download the appropriate version of Soar
    * [9.3.4 for Windows, 32-bit](http://web.eecs.umich.edu/~soar/downloads/SoarSuite/SoarSuite_9.3.4-Windows_32bit.zip)
    * [9.3.4 for Windows, 64-bit](http://web.eecs.umich.edu/~soar/downloads/SoarSuite/SoarSuite_9.3.4-Windows_64bit.zip)

#### OS X ####
  * [9.3.4 for Mac, 64-bit](http://web.eecs.umich.edu/~soar/downloads/SoarSuite/SoarSuite_9.3.4-OSX.zip)

#### Linux ####
  1. Determine if you're running 32- or 64-bit Linux. If the command `uname -m` yields `x86_64`, you have a 64-bit Linux OS.
  1. Download the appropriate version of Soar
    * [9.3.4 for Linux, 32-bit](http://web.eecs.umich.edu/~soar/downloads/SoarSuite/SoarSuite_9.3.4-Linux_32bit.zip)
    * [9.3.4 for Linux, 64-bit](http://web.eecs.umich.edu/~soar/downloads/SoarSuite/SoarSuite_9.3.4-Linux_64bit.zip)

#### Others ####
  * Follow the [instructions](Build.md) for building Soar from the platform-independent source code.

### Usage ###

  1. Download
  1. Extract to the folder of your choice
  1. Navigate to the folder you extracted to
  1. Double click batch/shell/command files such as `SoarJavaDebugger.bat`.