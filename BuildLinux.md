# Building Soar on Linux #

These instructions are for [Ubuntu](http://www.ubuntu.com/), which is the only officially supported Linux distribution right now. However, we've used Soar on a variety of distributions without any problems.

## Install the Prerequisites ##
  * You can install all of the prerequisites using this command:
```
sudo apt-get install build-essential swig openjdk-7-jdk python-all-dev
```

### What the components do ###

  * **build-essential**:  Required.
    * Gives you the GNU compiler tools.
  * **SWIG**: Required.
    * Acts as a bridge between the Soar C++ libraries and the various other languages.  Since nearly everyone uses the Soar Debugger, which is written in Java, you probably need this.  It is also needed if you plan to interface with Soar using Tcl, C# or Python.
  * **openjdk-7-jdk**: Required.
    * Needed for the Soar Debugger and the Java interface. The exact version doesn't matter. You can also use Sun's JDK.
  * **Python**: Required, but should be installed by default in Ubuntu.
    * **Note**: Please make sure the installed version is 2.x (where x >= 5), as Python 3 doesn't work. To check your version, run `python --version` in a terminal.
  * **python-all-dev**: Optional.
    * Needed to build the dll for the python SWIG interface. This package installs the development headers that are not included in the default `python` package.
    * **Note**:  Python 2 is required to build Soar Please make sure the installed version is 2.x (where x >= 5), as Python 3 doesn't work. To check your version, type `python --version`.
  * **ActiveTcl 8.6+**: Optional.
    * Needed if you want to build the SWIG Tcl interface and TclSoarLib (a module that adds the Tcl language to the Soar command line.)
    * Found [here](http://www.activestate.com/activetcl/downloads).

**Note**: For all of these packages, make sure to download the 64-bit version if you are on a 64-bit machine.

## Build Soar ##

Go into the SoarSuite folder and run:
```
python scons/scons.py all
```

All of your resulting binaries will be in the /out folder.  You can build specific components by specifying a different target than 'all'.  There are also several settings to control how Soar is built.  For example, the '--opt' setting will create a faster, optimized build instead of the default debug build.  For more detailed information about the Scons script, see the [SCons Script Documentation page](BuildSconsScript.md). See the [FAQ](FAQ.md) for common solutions to errors.

**Note**: If you are on 32-bit Ubuntu and get an error about libxss, try the command:
```
sudo apt-get install install libxss1:i386
```

## Troubleshooting Path Issues ##

(The following assumes that you're using the Bash shell)

Sometimes when building and running a program that uses Soar, your system may have trouble determining the locations of the Soar libraries and header files. If this should occur, specify the library location by setting the `LD_LIBRARY_PATH` environment variable:

```
export LD_LIBRARY_PATH=$HOME/SoarSuite/out:$LD_LIBRARY_PATH
```

The exact path depends on where you put the Soar distribution. It should contain the file `libSoar.so`. Specify the headers location by setting the `CPATH` environment variable:

```
export CPATH=$HOME/SoarSuite/out/include:$CPATH
```

Please make sure that the `include` directory actually exists. If it does not, you need to build the `headers` target with SCons.

Certain functions in the kernel may also use the SOAR\_HOME environment variable to find binaries.  For example, SpawnDebugger uses it to find SoarDebugger.jar. Set it like this:

```
export SOAR_HOME=$HOME/SoarSuite/out
```

You can add these commands to the `.bashrc` file in your home directory to automatically set these variables each time you start a Bash shell.

For more detailed information about library search paths, see the wiki page on [how library search paths work](BuildLibrarySearchPaths.md).