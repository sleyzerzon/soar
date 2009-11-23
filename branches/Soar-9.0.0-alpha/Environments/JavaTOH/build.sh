#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
        mkdir bin
else
        for file in `find bin -iname *.class`
        do
                rm $file
        done
fi

if ! javac -source 1.4 -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/toh/TowersOfHanoi.java; then
        echo "Build failed."
        exit 1;
fi

jar cfm toh.jar JarManifest -C bin .

#if [[ `uname -s` == "Darwin" ]]
#then
#    echo "on Mac OS X, building application package for JavaTOH..."
#
#    APP_PATH=$SOARLIB/TowersOfHanoi.app/Contents
#    mkdir -p $APP_PATH/MacOS
#    mkdir -p $APP_PATH/Resources/Java
#
#    cp TowersOfHanoi.plist $APP_PATH/Info.plist
#    cp $SOARLIB/icons/toh.icns $APP_PATH/Resources
#    cp $SOARLIB/toh.jar $APP_PATH/Resources/Java
#    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
#    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
#fi