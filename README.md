jsonmtz - An MTZ to JSON converter
==================================

Introduction
------------

This program interconverts the binary reflection file
format MTZ used in macromolecular X-ray crystallography and the
data exchange format JSON. The program contains two executables.
mtz2json converts MTZ files to JSON format, and json2mtz does the reverse.

Example usage
-------------
```shell
$ mtz2json in.mtz out.json  
$ json2mtz in.json out.mtz
```

Building from source
--------------------
Use [CMake](https://cmake.org/) to build from source.

### UNIX-like operating systems
```shell
$ cd jsonmtz  
$ mkdir build  
$ cd build  
$ cmake -G "Unix Makefiles" ..  
$ make
```

### Windows
Building from source has been tested with the
[MinGW](http://mingw.org/) compiler, which should be on the path in addition
to [CMake](https://cmake.org/). In PowerShell:

```shell
$ cd jsonmtz  
$ mkdir build  
$ cd build  
$ cmake -G "MinGW Makefiles" ..  
$ mingw32-make
```

Dependencies
------------

__jsonmtz__ depends on the [Jansson](http://www.digip.org/jansson/) and
[CCP4io](http://www.ccp4.ac.uk/) libraries. They are included in the source
tree.

Use as a library
-------------------------

__jsonmtz__ can be used as a C library. Include jsonmtz.h in your source code. 
The functions _mtz2json_ and _json2mtz_ are the API.

Source code documentation
-------------------------

Source code documentation can be extracted with [Doxygen](http://www.doxygen.org/):  
```shell
$ doxygen Doxyfile
```