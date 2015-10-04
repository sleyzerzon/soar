As of version 9.3.2, Soar is built by a single SCons script for all operating systems. SCons tries to be smart and uses the appropriate compilers, linkers, and other commands for your OS. A version of SCons is distributed in the Soar source tree, so you don't need to install it separately. To run the builder, start a shell (Visual Studio Command Prompt in Windows, see below), cd into the SoarSuite directory and type
```
  python scons/scons.py <options>
```
or if you want to use the SCons version you've installed separately,
```
  scons <options>
```
`<options>` is a place holder for options we'll pass into the builder, and should not be typed literally.

For Windows users, we have provided a batch file called build.bat which will perform several functions before launching the SCons script. This file will search for the locations of the python, java, and swig installs, and set the PATH environment variable so that the various compilers can run. Windows users can simply type:
```
  build <options>
```

The build script must be invoked from the Visual Studio Command Prompt so that scons can locate the VC++ compiler. A shortcut to launch this prompt can be found in the start menu under the Visual Studio program group.

For the rest of this document we'll be showing example build commands using the `"scons <options>"` form. Please substitute the appropriate command for "scons".

### Build Targets ###

The builder can be directed to compile any number of targets, i.e. components of Soar. Any number of targets can be passed to the builder together. For example:
```
  scons kernel debugger sml_python headers
```

The "all" target tells scons to build everything. To see a list of available targets, type
```
  scons list
```

This will give you the following full list of targets:
```
  all
  cli
  debugger
  debugger_api
  headers
  kernel
  sml_java
  sml_python
  sml_tcl
  tclsoarlib
  tests
```


If `build` is issued, i.e. no targets are specified, the default targets will be built which are `kernel, cli, sml_java, debugger, headers`.

### Output Directories ###

By default, the built libraries and executables will be placed into the "SoarSuite/out" subdirectory, while intermediates such as .o files will be placed in the "SoarSuite/build" subdirectory. You can change these using the `--out=<dir>` and `--build=<dir>` options.

```
  scons all --out=/home/soar --build=build_linux
```

These directories can be located or moved anywhere in your file system. However, we recommend you not change the file structure inside the "out" directory, as the executables expect the libraries to be in certain places relative to them.

To clean up all the files you just built, type
```
  scons -c all
```

### Build Flags ###

You can modify building behavior by passing in a number of flags in the form `'--<flagname>'`. To see the list of available flags, type
```
  scons --help
```
and look under the "Local Options" section (the other flags are for scons itself). Some flags of interest:
  * `--opt` will turn on compiler optimizations and strip debugging symbols from targets, resulting in slightly faster Soar agents and significantly smaller file sizes. By default optimizations are disabled for faster build times.
  * `--verbose` makes scons print out the full commands it uses to build each file. This is useful if the build doesn't succeed and you want to figure out what is happening under the hood.
  * `--static` will build a static version of the Soar library. The resulting library is normally named `libSoar.a` in Linux/OSX, and `Soar.lib` in Windows (there won't be a Soar.dll). This is necessary for running Soar in environments that don't support dynamic linking, such as iOS.
  * `--no-scu` will build without using single compilation units.  This will make compile time longer for full builds, but shorter when only a small subset of files have been altered.

Other options:
```
  --cc=COMPILER               Use argument as the C compiler.
  --cxx=COMPILER              Use argument as the C++ compiler.
  --cflags=CFLAGS             Compiler flags
  --lnflags=LNFLAGS           Linker flags
  --no-default-flags          Do not pass any default flags to the compiler or linker
  --out=DIR                   Directory to install binaries. Defaults to "out".
  --build=DIR                 Directory to store intermediate (object) files. Defaults to "build".
  --python=PYTHON             Python executable
  --install-sandbox=SANDBOX   A directory under which all installed files will be placed.
```
### Visual Studio Solution Generation ###

SCons can generate Visual Studio project and solution files that allow users to more easily modify and debug the kernel source code. You can generate these files by typing
```
  scons msvs
```
Unfortunately, the generated projects are not stand-alone, they still call SCons under the hood to build targets. This is not yet a well-supported feature, so the resulting project files will probably require some hand-tweaking.