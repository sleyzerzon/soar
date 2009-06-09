#!/bin/sh
platform=$(uname)

cd SoarSuite/Environments/Soar2D

if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="../../SoarLibrary/lib"
	java -XstartOnFirstThread -jar Soar2D.jar config/eaters.cnf
else
	export LD_LIBRARY_PATH="../../SoarLibrary/lib"
	java -jar Soar2D.jar config/eaters.cnf
fi

