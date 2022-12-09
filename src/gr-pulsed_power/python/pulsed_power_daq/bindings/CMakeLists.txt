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
    picoscope_4000a_source_python.cc
    power_calc_ff_python.cc
    mains_frequency_calc_python.cc
    power_calc_cc_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(pulsed_power
   ../..
   gr::pulsed_power
   "${pulsed_power_python_files}")

set(picoscope-ps4000a "/opt/picoscope/lib/libps4000.so")

install(TARGETS pulsed_power_python EXPORT picoscope-ps4000a DESTINATION ${GR_PYTHON_DIR}/pulsed_power COMPONENT pythonapi)
install(EXPORT picoscope-ps4000a DESTINATION ${GR_PYTHON_DIR}/pulsed_power)
