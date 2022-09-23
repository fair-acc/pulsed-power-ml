# Pulsed Power Monitoring

![image](src/assets/visualize_multiple_gr_signals_dark.png)

![image](src/assets/visualize_multiple_gr_signals_light.png)

## Getting Started

1. Install Emscripten from https://emscripten.org/docs/getting_started/downloads.html

2. Set the environment variables as described in https://emscripten.org/docs/getting_started/downloads.html#installation-instructions

3. Make sure to configure the EMSDK_ROOT_PATH variable in `cmake_build.sh` according to the path of emsdk on your local machine. This makes sure that the environement variables are set correctly and the emscripten toolchain is available for building the project.

## Usage

In order to visualize data passed by OpenCMW you need to build and run the OpenCMW worker first and then build and run the ImPlot Visualization.The following explains how to build and run OpenCMW and ImPlot Visualization.

### OpenCMW Worker

How to Build

```bash
cmake -S . -B build
cmake --build build
```

How to Run

- Run `./opencmw_worker/build/src/PulsedPowerService`

### ImPlot Visualization

How to Build

```bash
emcmake cmake -S . -B build && (cd build && emmake make -j 20)
```

NOTE: emcmake breaks `cmake --build` so we have to call make directly

How to Run

- run `cmake --build build --target serve`
- This will use Python3 to spawn a local webserver
- Browse http://localhost:8000 to access your build.
