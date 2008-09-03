@rem this is an example of how to build an sml application
@rem see the other build.bat files for more advanced examples

del /S /Q *.class
javac -classpath .;..\..\..\SoarLibrary\bin\swt.jar;..\..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
jar cfm ..\..\..\SoarLibrary\bin\toh.jar ..\JarManifest -C src .
xcopy ..\towers-of-hanoi-SML.soar ..\..\..\SoarLibrary\bin\* /Y
IF NOT "%1"=="--nopause" pause