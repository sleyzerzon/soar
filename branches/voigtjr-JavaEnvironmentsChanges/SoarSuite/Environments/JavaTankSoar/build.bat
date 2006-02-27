mkdir bin
javac -d bin -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar;..\..\SoarLibrary\bin\JavaBaseEnvironment.jar -sourcepath source source\eaters\TankSoar.java
xcopy /y source\* bin

jar cfm ..\..\SoarLibrary\bin\JavaTankSoar.jar JarManifest -C bin .

IF NOT "%1"=="--nopause" pause
