#!/bin/sh
# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

SOARLIB=../../../soar-library

javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath . Application.java
jar cfm ${SOARLIB}/TestJavaSML.jar JarManifest .
