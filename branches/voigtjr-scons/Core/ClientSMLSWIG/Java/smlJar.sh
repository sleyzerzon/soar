#!/bin/sh

pushd Core/ClientSMLSWIG/Java
javac build/*.java
cp -pRP build ../../../SoarLibrary/bin/sml
jar cf ../../../SoarLibrary/bin/sml.jar -C ../../../SoarLibrary/bin sml
rm -rf ../../../SoarLibrary/bin/sml
popd
