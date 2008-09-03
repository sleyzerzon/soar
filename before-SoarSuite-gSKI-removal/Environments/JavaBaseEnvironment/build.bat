@echo off
rem %2 is an optional command line argument specifying the java bin directory to use to build everything
rem NOTE: if this directory is specified, the trailing slash must be included!
rem Also, if there are spaces in the path, it must be wrapped in quotes

echo.
echo ************* Building JavaBaseEnvironment ****************
echo.

set SOARBIN=..\..\SoarLibrary\bin

IF NOT EXIST %SOARBIN%\swt.jar GOTO no_swt
IF NOT EXIST %SOARBIN%\sml.jar GOTO no_sml

echo ----------=====Setting up tmp dir====----------
IF EXIST tmp rmdir /S /Q tmp
mkdir tmp

@echo ----------=========Compiling=========----------
%2javac -source 1.4 -d tmp -classpath %SOARBIN%\swt.jar;%SOARBIN%\sml.jar -sourcepath source source\utilities\*.java source\simulation\*.java source\simulation\visuals\*.java

@echo ----------==========Jarring==========----------
%2jar cf %SOARBIN%\JavaBaseEnvironment.jar -C tmp .
@echo off

GOTO success

:no_swt
echo ERROR:  ..\..\SoarLibrary\bin\swt.jar is missing.  
echo         Did you remember to download it from: http://winter.eecs.umich.edu/jars/
echo Build Stopped
goto end

:no_sml
echo ERROR:  ..\..\SoarLibrary\bin\sml.jar is missing.  
echo Build Stopped
goto end

:success
echo Build Complete

:end
IF NOT "%1" == "--nopause" pause

