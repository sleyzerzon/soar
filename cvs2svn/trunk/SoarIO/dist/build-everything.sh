#!/bin/sh
./configure --prefix=/home/voigtjr/sandbox --enable-tcl && make && make install && cd SoarIO/examples/TestJavaSML && ./buildJava.sh && cd ../../../SoarJavaDebugger && ./buildDebugger.sh && cd ../JavaMissionaries && ./buildmac.sh && cd ../JavaTOH && ./buildtoh.sh && cd ..
