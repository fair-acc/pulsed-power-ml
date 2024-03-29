# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT pulsed_power_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT pulsed_power_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################
list(APPEND pulsed_power_python_files
    opencmw_time_sink_python.cc
    opencmw_freq_sink_python.cc
    integration_python.cc
    statistics_python.cc
    picoscope_4000a_source_python.cc
    power_calc_ff_python.cc
    mains_frequency_calc_python.cc
    power_calc_cc_python.cc 
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(pulsed_power
   ../../..
   gr::pulsed_power
   "${pulsed_power_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(TARGET pulsed_power_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pulsed_power_python>
    ${CMAKE_BINARY_DIR}/test_modules/gnuradio/pulsed_power/
)

install(TARGETS pulsed_power_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/pulsed_power COMPONENT pythonapi)
