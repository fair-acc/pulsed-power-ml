# OpenCMW example project

This is a minimal example project to show how the opencmw library can be used from any CMake project.
It uses CMake's FetchContent mechanism to obtain opencmw and (some of) its dependencies.
Some dependencies (openssl, zlib) are assumed to be installed system wide such that the opencmw project can access them with find_package.

### Building

``` bash
cmake -S . -B build
cmake --build build
build/src/MajordomoRest_example
```
### Building against local opencmw-cpp

Note: If you just want to implement a service, it will not be required and brings no benefit to checkout and build opencmw-cpp separatly.

There are environment variables which optionally can be used to point to your checkout of libfnt and/or opencmw-cpp.
(The same works for opencmw's dependencies as it internally uses fetchContent as well)

To do so, refer to the top folder of the other projects (which contains the toplevel `CMakeList.txt`) by doing:

``` bash
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_FMT=~/libfmt -DFETCHCONTENT_SOURCE_DIR_OPENCMW-CPP=~/opencmw-cpp
```

OpenCMW will autumatically detect if it is used as a subproject and disable certain parts of the build like examples or tests.
