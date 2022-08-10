#!/bin/bash

## script for building the implot_visualization app using emscripten toolchain with CMake

# execute with: bash cmake_build.sh

# Here the root directories for emscripten and emsdk should be specified according to your system

EMSCRIPTEN_ROOT_PATH=~/work/emscripten
EMSDK_ROOT_PATH=~/work/emsdk

# specification of the projects build folder

BUILD_DIR=build

# Set the emscripten environment correctly first
source "$EMSDK_ROOT_PATH"/emsdk_env.sh

# check if BUILD_DIR is already available or if we first have to create it

if [ -d "$BUILD_DIR" ]; then
    cd "$BUILD_DIR"
else
    mkdir "$BUILD_DIR"
    cd "$BUILD_DIR"
fi

# build the app using cmake and the emscripten toolchain

cmake -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT_PATH/cmake/Modules/Platform/Emscripten.cmake ..
cmake --build .

