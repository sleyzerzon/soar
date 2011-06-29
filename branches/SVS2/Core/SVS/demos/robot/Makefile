SVS=../../src
INC=-I$(SVS) -I$(SVS)/models

all: splintersim splintersim2

splintersim: splintersim.cpp $(SVS)/linalg.cpp $(SVS)/linalg.h
	g++ $(INC) -o splintersim splintersim.cpp $(SVS)/linalg.cpp

splintersim2: splintersim2.cpp $(SVS)/linalg.cpp $(SVS)/linalg.h
	g++ $(INC) -o splintersim2 splintersim2.cpp $(SVS)/linalg.cpp

