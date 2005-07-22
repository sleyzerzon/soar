#!/bin/sh
SOARLIB="../soar-library"

if [[ `uname -s` == "Darwin" ]]
  if ! javac -classpath .:${SOARLIB}/swt-osx.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cfm ${SOARLIB}/mac.jar JarManifest-osx mac -C src .
else
fi
  if ! javac -classpath .:${SOARLIB}/swt-motif.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cfm ${SOARLIB}/mac.jar JarManifest-motif mac -C src .
fi

if ! test -d ${SOARLIB}/mac; then
  mkdir ${SOARLIB}/mac;


cp mac/mac.soar ${SOARLIB}/mac;

if [[ `uname -s` == "Darwin" ]]
then
    echo "on Mac OS X, building application package for JavaMissionaries..."

    APP_PATH=$SOARLIB/MissionariesAndCannibals.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp MissionariesAndCannibals.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/mac.icns $APP_PATH/Resources
    cp $SOARLIB/mac.jar $APP_PATH/Resources/Java
    cp $SOARLIB/java_swt $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/java_swt
fi
