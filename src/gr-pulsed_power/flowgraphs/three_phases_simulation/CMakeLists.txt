# GNU Radio C++ Flow Graph CMakeLists.txt
#
# Title: three_phases_simulation
# Author: buchner
# GNU Radio version: 3.10.4.0


cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

project(three_phases_simulation)

find_package(Gnuradio "3.10" COMPONENTS
    blocks
    fec
    fft
    filter
    analog
    digital
    dtv
    audio
    channels
    pdu
    iio
    qtgui
    trellis
    uhd
    uhd UHD 4.0 RFNoC
    video-sdl
    vocoder
    wavelet
    zeromq
    network
    soapy
)

find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC TRUE)



add_executable(three_phases_simulation three_phases_simulation.cpp)
target_link_libraries(three_phases_simulation
    gnuradio::gnuradio-blocks
    gnuradio::gnuradio-qtgui
    gnuradio::gnuradio-analog
    gnuradio::gnuradio-qtgui
    gnuradio::gnuradio-filter

)

