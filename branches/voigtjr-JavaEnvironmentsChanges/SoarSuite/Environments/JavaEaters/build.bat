del /s /Q source\*.class

javac -classpath ..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath source source\eaters\Eaters.java

jar cfm ..\..\SoarLibrary\bin\JavaEaters.jar JarManifest -C source .

IF NOT "%1"=="--nopause" pause