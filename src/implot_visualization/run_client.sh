#!/bin/bash

## script for launching the implot_visualization app using a python webserver on localhost:8000

BUILD_DIR=./build

if [ -d "$BUILD_DIR/web" ]; then
    echo "Launching webserver"
    python3 -m http.server -d $BUILD_DIR/web
else
    echo "Error: Please build the application first"
fi