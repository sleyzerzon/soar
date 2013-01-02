#!/bin/bash

cd "$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ../../

cd "$HOME/roomsworld/release/soar_robot_server_osx"
find config/experiments -name "multiple.runs" | while read line; do
	./RoomsWorld.command "$line"
done
