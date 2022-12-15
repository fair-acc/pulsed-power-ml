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
/* BINDTOOL_HEADER_FILE(opencmw_freq_sink.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(5183e9cc3a3748f27aded421c8889596)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
// pydoc.h is automatically generated in the build directory
#include <opencmw_freq_sink_pydoc.h>

void bind_opencmw_freq_sink(py::module& m)
{

    using opencmw_freq_sink = ::gr::pulsed_power::opencmw_freq_sink;


    py::class_<opencmw_freq_sink,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<opencmw_freq_sink>>(
        m, "opencmw_freq_sink", D(opencmw_freq_sink))

        .def(py::init(&opencmw_freq_sink::make),
             py::arg("signal_names"),
             py::arg("signal_units"),
             py::arg("sample_rate"),
             py::arg("bandwidth"),
             py::arg("vector_size") = 1024,
             D(opencmw_freq_sink, make))


        .def("set_callback",
             &opencmw_freq_sink::set_callback,
             py::arg("cb_copy_data"),
             D(opencmw_freq_sink, set_callback))


        .def("get_bandwidth",
             &opencmw_freq_sink::get_bandwidth,
             D(opencmw_freq_sink, get_bandwidth))


        .def("get_sample_rate",
             &opencmw_freq_sink::get_sample_rate,
             D(opencmw_freq_sink, get_sample_rate))


        .def("get_signal_names",
             &opencmw_freq_sink::get_signal_names,
             D(opencmw_freq_sink, get_signal_names))


        .def("get_signal_units",
             &opencmw_freq_sink::get_signal_units,
             D(opencmw_freq_sink, get_signal_units))


        .def("get_vector_size",
             &opencmw_freq_sink::get_vector_size,
             D(opencmw_freq_sink, get_vector_size))

        ;
}
