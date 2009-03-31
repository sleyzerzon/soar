@echo off

@rem Step 1: Set up build environment
call "%VS80COMNTOOLS%\vsvars32.bat"

@rem Step 2: Build C++
devenv /rebuild "Distribution SCU" SML.sln
if not errorlevel 0 goto fail
devenv /build Release Tools\TestCSharpSML\TestCSharpSML.sln
if not errorlevel 0 goto fail

@rem Step 3: Clean and Build Java
%ANT_HOME%\bin\ant clean
%ANT_HOME%\bin\ant
if not errorlevel 0 goto fail

exit 0

:fail
exit 1
