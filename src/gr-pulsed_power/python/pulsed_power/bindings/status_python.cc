/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(status.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(5ee6d22ceb703c1cecf635559acaac82)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/status.h>
// pydoc.h is automatically generated in the build directory
#include <status_pydoc.h>

void bind_status(py::module& m)
{


    py::enum_<::gr::pulsed_power::channel_status_t>(m, "channel_status_t")
        .value("CHANNEL_STATUS_OVERFLOW",
               ::gr::pulsed_power::CHANNEL_STATUS_OVERFLOW) // 1
        .value("CHANNEL_STATUS_REALIGNMENT_ERROR",
               ::gr::pulsed_power::CHANNEL_STATUS_REALIGNMENT_ERROR) // 2
        .value("CHANNEL_STATUS_NOT_ALL_DATA_EXTRACTED",
               ::gr::pulsed_power::CHANNEL_STATUS_NOT_ALL_DATA_EXTRACTED) // 4
        .value("CHANNEL_STATUS_TIMEOUT_WAITING_WR_OR_REALIGNMENT_EVENT",
               ::gr::pulsed_power::
                   CHANNEL_STATUS_TIMEOUT_WAITING_WR_OR_REALIGNMENT_EVENT) // 8
        .export_values();

    py::implicitly_convertible<int, ::gr::pulsed_power::channel_status_t>();
    py::enum_<::gr::pulsed_power::algorithm_id_t>(m, "algorithm_id_t")
        .value("FIR_LP", ::gr::pulsed_power::FIR_LP)                 // 0
        .value("FIR_BP", ::gr::pulsed_power::FIR_BP)                 // 1
        .value("FIR_CUSTOM", ::gr::pulsed_power::FIR_CUSTOM)         // 2
        .value("FIR_CUSTOM_FFT", ::gr::pulsed_power::FIR_CUSTOM_FFT) // 3
        .value("IIR_LP", ::gr::pulsed_power::IIR_LP)                 // 4
        .value("IIR_HP", ::gr::pulsed_power::IIR_HP)                 // 5
        .value("IIR_CUSTOM", ::gr::pulsed_power::IIR_CUSTOM)         // 6
        .value("AVERAGE", ::gr::pulsed_power::AVERAGE)               // 7
        .export_values();

    py::implicitly_convertible<int, ::gr::pulsed_power::algorithm_id_t>();
}
