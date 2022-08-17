#!/bin/bash

## script for building the implot_visualization app using emscripten toolchain with CMake

# execute with: bash cmake_build.sh

# Here the root directory for emsdk should be specified according to your system

EMSDK_ROOT_PATH=~/work/emsdk

# specification of the projects build folder

BUILD_DIR=build

# Set the emscripten environment correctly first
source "$EMSDK_ROOT_PATH"/emsdk_env.sh

# deviate emscripten root path from emsdk root path
EMSCRIPTEN_ROOT_PATH=$EMSDK_ROOT_PATH/upstream/emscripten

# build the app using cmake and the emscripten toolchain
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT_PATH/cmake/Modules/Platform/Emscripten.cmake
cmake --build build

