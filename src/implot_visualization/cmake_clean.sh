#!/bin/bash

## script for cleaning the build folder procuded by cmake

BUILD_DIR=./build

if [ -d "$BUILD_DIR" ]; then
    echo "cleaning ${BUILD_DIR} directory"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR" 
fi