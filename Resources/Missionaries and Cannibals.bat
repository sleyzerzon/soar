@echo off
cd SoarSuite\Environments\JavaMissionaries

set SOARLIB=../../SoarLibrary/bin
set PATH=%SOARLIB%;%PATH%

start javaw -jar JavaMissionaries.jar
