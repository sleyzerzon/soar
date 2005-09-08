del /s /Q *.class

javac -classpath ..\soar-library\swt-windows.jar;..\soar-library\sml.jar;..\soar-library\JavaBaseEnvironment.jar -sourcepath src src\edu\umich\tanksoar\TanksoarJ.java

jar cfm ..\soar-library\tanksoar.jar JarManifest images -C src .

rem copy the images over to soar-library, because the code doesn't currently load them from the jar (even though they are in there)
xcopy /I /Q /E /Y images ..\soar-library\images

IF NOT "%1"=="--nopause" pause