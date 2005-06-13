#!/bin/sh
# Builds the debugger
SOARLIB="../soar-library"

for file in `find . -name "*.class"`
do
  rm -f $file
done

if ! javac -classpath .:${SOARLIB}/sml.jar -sourcepath Source Source/edu/umich/visualsoar/VisualSoar.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/VisualSoar.jar Source/meta-inf/manifest.mf -C Source .

if [ `uname -s`="Darwin" ]
then
    echo "on Mac OS X, building application package for VisualSoar..."

    APP_PATH=$SOARLIB/VisualSoar.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp VisualSoar.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/vs.icns $APP_PATH/Resources
    cp $SOARLIB/visualsoar.jar $APP_PATH/Resources/Java
    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
fi

