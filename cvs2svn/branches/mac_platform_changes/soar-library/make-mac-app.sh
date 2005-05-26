#!/bin/sh

# $1 is the name of the app
# $2 is the jar name

APP_PATH="$1.app/Contents"

mkdir -p "$APP_PATH/MacOS"
mkdir -p "$APP_PATH/Resources/Java/classes"

cp "$1.plist" "$APP_PATH/Info.plist"
cp "$2" "$APP_PATH/Resources/Java"
cp "sml.jar" "$APP_PATH/Resources/Java"
cp "java_swt" "$APP_PATH/MacOS"
chmod a+x "$APP_PATH/MacOS/java_swt"
