#!/bin/sh
SOARLIB="../soar-library"

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/mac.jar macJarManifest mac -C src .

if ! test -d ${SOARLIB}/mac; then
  mkdir ${SOARLIB}/mac;
fi

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
