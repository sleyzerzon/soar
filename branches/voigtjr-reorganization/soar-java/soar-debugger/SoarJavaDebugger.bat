@echo off
cd soar-java\lib
set PATH=..\..\soar-core\bin;%PATH%
start javaw -jar SoarJavaDebugger.jar

