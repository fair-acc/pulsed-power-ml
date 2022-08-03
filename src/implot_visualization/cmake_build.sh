#!/bin/bash

## script for building the implot_visualization app using emscripten toolchain with CMake

# Set the emscripten environment correctly first

source ~/FAIR/emsdk/emsdk_env.sh

# build the app

cd build 
cmake -DCMAKE_TOOLCHAIN_FILE=/home/stefan/FAIR/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
#cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Emscripten.cmake ..
cmake --build .

