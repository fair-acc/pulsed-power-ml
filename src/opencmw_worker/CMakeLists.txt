cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)

include(cmake/Dependencies.cmake)

project(opencmw-worker CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(GNUInstallDirs)

find_package(Gnuradio "3.10" COMPONENTS
    blocks
    analog
    filter
    fft
)

find_package(gnuradio-pulsed_power)

# Check for supported compiler versions
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.1.0)
        message(FATAL_ERROR "GCC>=12.1.0 required, but gcc ${CMAKE_CXX_COMPILER_VERSION} detected.")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0.0)
        message(FATAL_ERROR "Clang>=13.0.0 required, but clang ${CMAKE_CXX_COMPILER_VERSION} detected.")
    endif ()
else ()
    message(WARN "No version check for your compiler (${CMAKE_CXX_COMPILER_ID}) implemented, "
            "in case of build problems consider updating your compiler or check if you can switch to gcc or clang")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if (ENABLE_BUILD_WITH_TIME_TRACE)
        target_compile_options(project_options INTERFACE -ftime-trace)
    endif ()
endif ()

# enable cache system
include(cmake/Cache.cmake)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(ENABLE_COVERAGE ON)
else ()
    set(ENABLE_COVERAGE OFF)
endif ()

if (ENABLE_COVERAGE)
    if (UNIX AND NOT APPLE) # Linux
        message("Coverage reporting enabled")
        include(cmake/CodeCoverage.cmake) # https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
        # (License: BSL-1.0)
        append_coverage_compiler_flags()
        setup_target_for_coverage_gcovr_xml(
                NAME coverage
                EXECUTABLE ctest
                EXCLUDE "$CMAKE_BUILD_DIR/*" "test/.*")
        setup_target_for_coverage_gcovr_html(
                NAME coverage_html
                EXECUTABLE ctest
                EXCLUDE "$CMAKE_BUILD_DIR/*" "test/.*")
    else ()
        message(WARNING "Coverage is only supported on linux")
    endif ()
endif ()

add_subdirectory(src)


include(CTest)
add_subdirectory(test)
