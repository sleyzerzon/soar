@REM Builds the debugger

javac.exe -classpath .;..\soar-library\swt.jar;..\soar-library\sml.jar debugger\Application.java
jar.exe cfm ..\soar-library\SoarJavaDebugger.jar JarManifest .
pause