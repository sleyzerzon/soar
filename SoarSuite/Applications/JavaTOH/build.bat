del /S /Q *.class
javac -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\toh\TowersOfHanoi.java
jar cfm ..\..\SoarLibrary\bin\toh.jar JarManifest -C src .
IF NOT "%1"=="--nopause" pause