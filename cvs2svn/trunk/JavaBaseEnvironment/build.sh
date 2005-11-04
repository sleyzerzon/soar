#!/bin/sh
SOARLIB="../soar-library"

if [[ `uname -s` == "Darwin" ]]
then
  if ! javac -classpath .:${SOARLIB}/swt-osx.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/JavaBaseEnvironment/*.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cfm ${SOARLIB}/javabaseenvironment.jar JarManifest-osx -C src .
else
  if ! javac -classpath .:${SOARLIB}/swt-motif.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/JavaBaseEnvironment/*.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cfm ${SOARLIB}/javabaseenvironment.jar JarManifest-motif -C src .
fi

