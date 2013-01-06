#!/bin/bash

uname="$(uname)"
case "$uname" in
	"Linux")
		release_dir="$HOME/roomsworld/release/soar_robot_server_linux_32"
		cmd="RoomsWorld.sh";;
	"Darwin")
		release_dir="$HOME/roomsworld/release/soar_robot_server_osx"
		cmd="RoomsWorld.command";;
	*)
		echo "kernel $uname not recognized; quitting"
		exit 1;;
esac

cd "$release_dir"
find ../../configs/experiments -name "multiple.runs" | while read line; do
	./$cmd "$line"
done
