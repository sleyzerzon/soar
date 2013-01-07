#!/bin/bash

cwd="$(pwd)"

# if it's not an absolute path, insert the previous directory as prefix
config="$1"
if [ "${config:0:1}" != "/" ]; then
	config="$cwd/$config"
fi

cd $(dirname "$0")
unset DYLD_LIBRARY_PATH
java -jar SoarRobotServer.jar --config "$config"
