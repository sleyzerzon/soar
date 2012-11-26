#!/bin/bash

soardir="$( cd ~/SoarSuite/ && pwd)"
basedir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

clean() {
	pushd april/java && ant clean && popd
	pushd SoarRobotLibrary && ant clean && popd
	pushd libgrrc-java && ant clean && popd
	pushd SoarRobotServer && ant clean && popd
}

april() {
	# build april
	pushd april/java
	ant
	mv april.jar "$basedir/SoarRobotServer/lib/"
	popd
}

soar-robot-library() {
	# build SoarRobotLibrary 
	pushd SoarRobotLibrary
	ant
	mv build/jar/SoarRobotLibrary.jar "$basedir/SoarRobotServer/lib/"
	popd
}

grrc() {
	# build libgrrc-java
	pushd libgrrc-java
	ant
	mv build/jar/libgrrc-java.jar "$basedir/SoarRobotServer/lib/"
	popd
}

copy-soar-libraries() {
	# copy soar libraries
	uname="$(uname)"
	case "$uname" in
		"Linux")
			export arch="linux_32"
			cp -f "$soardir/out/libJava_sml_ClientInterface.so" "native/$arch/"
			cp -f "$soardir/out/libSoar.so" "native/$arch/";;
		"Darwin")
			export arch="osx"
			cp -f "$soardir/out/libJava_sml_ClientInterface.jnilib" "native/$arch/"
			cp -f "$soardir/out/libSoar.dylib" "native/$arch/";;
		*)
			echo "unknown architecture: $uname"
			exit 1;;
	esac
	# where is TestExternalLibrary?;;
	cp -f "$soardir/out/java/sml.jar" "native/$arch/"
	cp -f "$soardir/out/java/soar-debugger-api.jar" "native/$arch/"
	cp -f "$soardir/out/java/soar-debugger.jar" "native/$arch/"
	cp -f "$soardir/out/java/soar-smljava.jar" "native/$arch/"
	cp -f "$soardir/out/java/swt.jar" "native/$arch/"
}

soar-robot-server() {
	# build SoarRobotServer
	pushd SoarRobotServer
	ant
	popd
}

if [ ! $soardir ]; then
	echo "Please modify script to use the correct path to Soar"
	exit 1
fi

# navigate to RoomsWorld base
cd "$basedir"

if [ $# -gt 1 ]; then
	while [ $# -gt 0 ]; do
		case "$1" in
			"clean")
				clean
				exit;;
			"april")
				april;;
			"copy")
				copy-soar-libraries;;
			"soar-robot-library")
				soar-robot-library;;
			"soar-robot-server")
				soar-robot-server;;
			"grrc")
				grrc;;
			"all")
				april
				copy-soar-libraries
				soar-robot-library
				grrc
				soar-robot-server;;
			*)
				echo "target '$1' unrecognized"
				exit 1
		esac
		shift
	done
else
	soar-robot-server
fi
