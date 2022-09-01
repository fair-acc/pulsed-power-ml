# OpenCMW worker for n (Sinus-)signals

This is an implementation of the OpenCMW time domain worker that fetches data from a GNURadio OpenCMW Time Sink block.

It uses CMake's FetchContent mechanism to obtain opencmw and (some of) its dependencies.
Some dependencies (openssl, zlib) are assumed to be installed system wide such that the opencmw project can access them with find_package.

### Building

```bash
cmake -S . -B build
cmake --build build
build/src/PulsedPowerService
```

