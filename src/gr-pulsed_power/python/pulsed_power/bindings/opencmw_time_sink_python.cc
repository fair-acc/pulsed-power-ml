/*
 * Copyright 2022 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(opencmw_time_sink.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(b8cf3b76ba68b2ee2e679ea7bc0fb36f)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/opencmw_time_sink.h>
// pydoc.h is automatically generated in the build directory
#include <opencmw_time_sink_pydoc.h>

void bind_opencmw_time_sink(py::module& m)
{

    using opencmw_time_sink = ::gr::pulsed_power::opencmw_time_sink;


    py::class_<opencmw_time_sink,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<opencmw_time_sink>>(
        m, "opencmw_time_sink", D(opencmw_time_sink))

        .def(py::init(&opencmw_time_sink::make),
             py::arg("sample_rate"),
             py::arg("signal_name") = "Signal 1",
             py::arg("signal_unit") = "",
             D(opencmw_time_sink, make))


        .def("set_callback",
             &opencmw_time_sink::set_callback,
             py::arg("cb_copy_data"),
             D(opencmw_time_sink, set_callback))


        .def("get_sample_rate",
             &opencmw_time_sink::get_sample_rate,
             D(opencmw_time_sink, get_sample_rate))


        .def("get_signal_name",
             &opencmw_time_sink::get_signal_name,
             D(opencmw_time_sink, get_signal_name))


        .def("get_signal_unit",
             &opencmw_time_sink::get_signal_unit,
             D(opencmw_time_sink, get_signal_unit))

        ;
}