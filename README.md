libdirtrav
==========
Cross-platform C library for recursively traversing directory contents.

Description
-----------
The libdirtrav library provides a way to recursively read folder contents using callback functions.
This is done using opendir() and readdir() functions, except for Windows where native FindFirstFile()/FindFirstFileEx() and FindNextFile() functions are used.
On Windows a wide version is available which uses wchar_t* as string type for Unicode UTF-16 support.

Goal
----
The library was written with the following goals in mind:
- written in standard C, but allows being used by C++
- speed
- small footprint
- portable across different platforms (Windows, *nix)
- no dependancies

Libraries
---------

The following libraries are provided:
- `-ldirtrav` - requires `#include <dirtrav.h>`
- `-ldirtravw` - only supported on Windows, requires `#include <dirtravw.h>`

Command line utilities
----------------------
Some command line utilities are included:
- `tree` - lists folder tree structure
- `rdir` - recursively lists folder contents

Dependancies
------------
This project has no depencancies.

Building from source
--------------------
Requirements:
- a C compiler like gcc or clang, on Windows MinGW and MinGW-w64 are supported
- a shell environment, on Windows MSYS is supported
- the make command
- CMake version 2.6 or higher (optional, but preferred)

Building with CMake
- configure by running `cmake -G"Unix Makefiles"` (or `cmake -G"MSYS Makefiles"` on Windows) optionally followed by:
  + `-DCMAKE_INSTALL_PREFIX:PATH=<path>` Base path were files will be installed
  + `-DBUILD_STATIC:BOOL=OFF` - Don't build static libraries
  + `-DBUILD_SHARED:BOOL=OFF` - Don't build shared libraries
  + `-DBUILD_TOOLS:BOOL=OFF` - Don't build tools (only libraries)
  + `-DWITH_WIDE:BOOL=ON` - Only on Windows: also build UTF-16 library (libdirtravw)
  + `-DFORCE_OPENDIR:BOOL=ON` - Only on Windows: use opendir() and readdir() instead of native Windows functions
- build and install by running `make install` (or `make install/strip` to strip symbols)

For Windows prebuilt binaries are also available for download (both 32-bit and 64-bit)

License
-------
libdirtrav is released under the terms of the MIT License (MIT), see LICENSE.txt.

This means you are free to use libdirtrav in any of your projects, from open source to commercial.
