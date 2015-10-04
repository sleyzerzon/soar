# PHP Example Project #
This project contains an example PHP project that interfaces with Soar. It includes a sample agent that is a slight modification of the water-jug-rl demo agent included with Soar.  The main difference is that the initialization application and the goal-detection elaboration rules condition upon server-side input-link structures to dynamically generate water-jug problem instances.

### Download Links ###
  * [PHP\_Interface\_Example.zip](http://web.eecs.umich.edu/~soar/downloads/ExampleDomainDevelopmentCode/PHP_Interface_Example.zip)

### Documentation ###
For the PHP bindings to build correctly, SWIG Version 1.3.40 is required.

For Soar to interact with PHP, there are a couple manual steps:

1. Open php.ini and set the enable\_dl = On
2. Copy (or, preferably, create a symbolic link) of libPHP\_sml\_ClientInterface.so (in lib) to the PHP extension\_dir (sans the lib prefix).
> This is visible via phpInfo() (search for extension\_dir) or `php-config --extension-dir`.


For Soar to work with Apache via PHP, there are another couple steps:

1. The module needs to be loaded by default.
> Open php.ini, add extension=PHP\_sml_ClientInterface.so (at the end of the list of extensions)._

2. The SML shared library (i.e. libSoarKernelSML) needs to be accessible to Apache.
> Easiest way: copy the library to system library path (like /usr/local/lib).

### Developer ###
  * Nate Derbinsky

### Soar Versions ###
  * Soar 8, 9

### Language ###
  * PHP





