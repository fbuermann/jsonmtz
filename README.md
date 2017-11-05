jsonmtz - An MTZ to JSON converter
==================================

Introduction
------------

This program can be used to interconvert between the binary reflection file
format MTZ, commonly used in X-ray crystallography, and the widespread
human-readable data exchange format JSON. The program contains two executables.
mtz2json converts MTZ files to JSON format, and json2mtz does the reverse.
__jsonmtz__ can easily be used as a C library.

Example usage
-------------
```shell
$ mtz2json in.mtz out.json  
$ json2mtz in.json out.mtz
```

Building from Source
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

Use as a (static) library
-------------------------

__jsonmtz__ can be used as a C library. Build __jsonmtz__ from source. Include
the _jsonmtz.h_ header in your code and make sure that all _ccp4io_ headers
and the _jansson.h_ / _jansson\_config.h_ headers are on the include path.
Link against _libjsonmtz.a_, _libcmtz.a_ and _libjansson.a_. On linux
platforms, also link against _libm.a_.

The functions _mtz2json()_ and _json2mtz()_ are the main API.

Source code documentation
-------------------------

Source code documentation can be extracted with [Doxygen](www.doxygen.org):  
```shell
$ doxygen Doxyfile
```