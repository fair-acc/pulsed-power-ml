# ImPlot Visualization

The ImPlot Visualization visualizes signals pulled from OpenCMW.

## Getting Started

1. Install Emscripten from https://emscripten.org/docs/getting_started/downloads.html

2. Set the environment variables as described in https://emscripten.org/docs/getting_started/downloads.html#installation-instructions

3. Additionally clone https://github.com/emscripten-core/emscripten repository to your local machine. This is necessary to integrate Emscripten as CMake toolchain.

4. Make sure to configure the EMSCRIPTEN_ROOT_PATH and EMSDK_ROOT_PATH variables in `cmake_build.sh` according to the paths of emsdk and emscripten on your local machine. This makes sure that the environement variables are set correctly and the emscripten toolchain is available for building the project.

## Usage

In order to visualize data passed by OpenCMW you need to build and run the OpenCMW worker first and then build and run the ImPlot Visualization.The following explains how to build and run OpenCMW and ImPlot Visualization.

### OpenCMW Counter Worker

How to Build

```bash
cmake -S . -B build
cmake --build build
build/src/opencmw_counter
```

How to Run

- Run `./opencmw_counter/build/src/opencmw_counter`

### ImPlot Visualization

How to Build

- Run `bash cmake_build.sh` while in the `implot_visualization` directory.
- To delete your build you may use `bash cmake_clean.sh`

How to Run

- Run `bash run_client.sh`. This will use Python3 to spawn a local webserver
- Browse http://localhost:8000 to access your build.
