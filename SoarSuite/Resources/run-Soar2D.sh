#!/bin/sh
cd SoarSuite/Environments/Soar2D

platform=$(uname)
if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="../../SoarLibrary/lib"
	java -XstartOnFirstThread -jar Soar2D.jar
else
	export LD_LIBRARY_PATH="../../SoarLibrary/lib"
	java -jar Soar2D.jar
fi

