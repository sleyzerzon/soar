Home page: http://playerstage.sourceforge.net/

Installation: http://playerstage.sourceforge.net/wiki/Download

Testing installation (see http://playerstage.sourceforge.net/doc/Player-cvs/player/start.html):

Process 1: player /prefix/share/stage/worlds/simple.cfg

Process 2: playerv -h localhost --position2d --laser

Player

Abstracts robotics hardware so there is a unified interface.

Uses stage to simulate both hardware and its interaction with the environment.

Communication is done over TCP using client-side libraries. C++ libraries available.

Stage

Maps can be defined by simple image files.