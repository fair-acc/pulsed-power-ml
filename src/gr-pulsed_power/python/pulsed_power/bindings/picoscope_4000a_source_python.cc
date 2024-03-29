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
/* BINDTOOL_HEADER_FILE(picoscope_4000a_source.h) */
/* BINDTOOL_HEADER_FILE_HASH(491ccb22d7be9fa3f257367e25b373b2)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/picoscope_4000a_source.h>
// pydoc.h is automatically generated in the build directory
#include <picoscope_4000a_source_pydoc.h>

void bind_picoscope_4000a_source(py::module& m)
{

    using picoscope_4000a_source = ::gr::pulsed_power::picoscope_4000a_source;


    py::class_<picoscope_4000a_source,
               gr::pulsed_power::picoscope_base,
               std::shared_ptr<picoscope_4000a_source>>(
        m, "picoscope_4000a_source", D(picoscope_4000a_source))

        .def(py::init(&picoscope_4000a_source::make),
             py::arg("serial_number"),
             py::arg("auto_arm"),
             D(picoscope_4000a_source, make))


        .def("set_trigger_once",
             &picoscope_4000a_source::set_trigger_once,
             py::arg("auto_arm"),
             D(picoscope_4000a_source, set_trigger_once))


        .def("set_samp_rate",
             &picoscope_4000a_source::set_samp_rate,
             py::arg("rate"),
             D(picoscope_4000a_source, set_samp_rate))


        .def("set_downsampling",
             &picoscope_4000a_source::set_downsampling,
             py::arg("mode"),
             py::arg("downsample_factor") = 0,
             D(picoscope_4000a_source, set_downsampling))


        .def("set_aichan",
             &picoscope_4000a_source::set_aichan,
             py::arg("id"),
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan))


        .def("set_aichan_a",
             &picoscope_4000a_source::set_aichan_a,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_a))


        .def("set_aichan_b",
             &picoscope_4000a_source::set_aichan_b,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_b))


        .def("set_aichan_c",
             &picoscope_4000a_source::set_aichan_c,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_c))


        .def("set_aichan_d",
             &picoscope_4000a_source::set_aichan_d,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_d))


        .def("set_aichan_e",
             &picoscope_4000a_source::set_aichan_e,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_e))


        .def("set_aichan_f",
             &picoscope_4000a_source::set_aichan_f,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_f))


        .def("set_aichan_g",
             &picoscope_4000a_source::set_aichan_g,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_g))


        .def("set_aichan_h",
             &picoscope_4000a_source::set_aichan_h,
             py::arg("enabled"),
             py::arg("range"),
             py::arg("coupling"),
             py::arg("range_offset") = 0,
             D(picoscope_4000a_source, set_aichan_h))


        .def("set_aichan_trigger",
             &picoscope_4000a_source::set_aichan_trigger,
             py::arg("id"),
             py::arg("direction"),
             py::arg("threshold"),
             D(picoscope_4000a_source, set_aichan_trigger))


        .def("set_samples",
             &picoscope_4000a_source::set_samples,
             py::arg("pre_samples"),
             py::arg("post_samples"),
             D(picoscope_4000a_source, set_samples))


        .def("set_rapid_block",
             &picoscope_4000a_source::set_rapid_block,
             py::arg("nr_waveforms"),
             D(picoscope_4000a_source, set_rapid_block))


        .def("set_nr_buffers",
             &picoscope_4000a_source::set_nr_buffers,
             py::arg("nr_buffers"),
             D(picoscope_4000a_source, set_nr_buffers))


        .def("set_streaming",
             &picoscope_4000a_source::set_streaming,
             py::arg("poll_rate") = 0.001,
             D(picoscope_4000a_source, set_streaming))


        .def("set_driver_buffer_size",
             &picoscope_4000a_source::set_driver_buffer_size,
             py::arg("driver_buffer_size"),
             D(picoscope_4000a_source, set_driver_buffer_size))


        .def("set_buffer_size",
             &picoscope_4000a_source::set_buffer_size,
             py::arg("buffer_size"),
             D(picoscope_4000a_source, set_buffer_size))

        ;
}
