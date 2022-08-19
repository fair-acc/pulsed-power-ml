cmake_minimum_required(VERSION 3.17)

# set a variable in CMake
set(projectName "implot_visualization")

project(${projectName} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# necessary to tell CMake (with emscripten) to extend the standard .js and .wasm output with .html output (so that we can execute it with the webserver) 
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# locate the "executable" output in the build/web directory

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/web)

# CAUTION: The "SHELL:" before some compile/link options has to be used to explicity tell CMake to not sum up options 
# (e.g. -option A -option B => -option A B)(CMake default behaviour), but instead keep them separated (needed for
# proper operation of emscripten)

add_compile_options(
        -DIMGUI_DISABLE_FILE_FUNCTIONS
        -g
        -Wall -Wformat -Os
        "SHELL:-s USE_SDL=2"
        "SHELL:-s DISABLE_EXCEPTION_CATCHING=1"
)

add_link_options(
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s NO_EXIT_RUNTIME=0"
        "SHELL:-s ASSERTIONS=1"
        "SHELL:-s NO_FILESYSTEM=1"
        --shell-file ${PROJECT_SOURCE_DIR}/app/shell_minimal.html
        "SHELL:-s USE_SDL=2"
        "SHELL:-s DISABLE_EXCEPTION_CATCHING=1"
        -sFETCH
)

set(outputFileName "index")
add_executable(${outputFileName})

# dependencies
add_subdirectory(external)
add_subdirectory(app)






