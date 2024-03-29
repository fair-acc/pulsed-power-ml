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
/* BINDTOOL_HEADER_FILE(tags.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(df58922bd156cb5605ae11f48a4c03f9)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/tags.h>
// pydoc.h is automatically generated in the build directory
#include <tags_pydoc.h>

void bind_tags(py::module& m)
{

    using acq_info_t = ::gr::pulsed_power::acq_info_t;
    using trigger_t = ::gr::pulsed_power::trigger_t;
    using wr_event_t = ::gr::pulsed_power::wr_event_t;


    py::class_<acq_info_t, std::shared_ptr<acq_info_t>>(m, "acq_info_t", D(acq_info_t))

        .def(py::init<>(), D(acq_info_t, acq_info_t, 0))
        .def(py::init<gr::pulsed_power::acq_info_t const&>(),
             py::arg("arg0"),
             D(acq_info_t, acq_info_t, 1))

        ;


    py::class_<trigger_t, std::shared_ptr<trigger_t>>(m, "trigger_t", D(trigger_t))

        .def(py::init<>(), D(trigger_t, trigger_t, 0))
        .def(py::init<gr::pulsed_power::trigger_t const&>(),
             py::arg("arg0"),
             D(trigger_t, trigger_t, 1))

        ;


    py::class_<wr_event_t, std::shared_ptr<wr_event_t>>(m, "wr_event_t", D(wr_event_t))

        .def(py::init<gr::pulsed_power::wr_event_t const&>(),
             py::arg("arg0"),
             D(wr_event_t, wr_event_t, 0))
        .def(py::init<>(), D(wr_event_t, wr_event_t, 1))

        ;


    m.def("make_acq_info_tag",
          &::gr::pulsed_power::make_acq_info_tag,
          py::arg("acq_info"),
          py::arg("offset"),
          D(make_acq_info_tag));


    m.def("decode_acq_info_tag",
          &::gr::pulsed_power::decode_acq_info_tag,
          py::arg("tag"),
          D(decode_acq_info_tag));


    m.def("make_trigger_tag",
          (gr::tag_t(::*)(gr::pulsed_power::trigger_t&, uint64_t)) &
              ::gr::pulsed_power::make_trigger_tag,
          py::arg("trigger_tag_data"),
          py::arg("offset"),
          D(make_trigger_tag, 0));


    m.def("make_trigger_tag",
          (gr::tag_t(::*)(uint32_t, int64_t, uint64_t, uint32_t)) &
              ::gr::pulsed_power::make_trigger_tag,
          py::arg("downsampling_factor"),
          py::arg("timestamp"),
          py::arg("offset"),
          py::arg("status"),
          D(make_trigger_tag, 1));


    m.def("make_trigger_tag",
          (gr::tag_t(::*)(uint64_t)) & ::gr::pulsed_power::make_trigger_tag,
          py::arg("offset"),
          D(make_trigger_tag, 2));


    m.def("decode_trigger_tag",
          &::gr::pulsed_power::decode_trigger_tag,
          py::arg("tag"),
          D(decode_trigger_tag));


    m.def("make_timebase_info_tag",
          &::gr::pulsed_power::make_timebase_info_tag,
          py::arg("timebase"),
          D(make_timebase_info_tag));


    m.def("decode_timebase_info_tag",
          &::gr::pulsed_power::decode_timebase_info_tag,
          py::arg("tag"),
          D(decode_timebase_info_tag));


    m.def("make_wr_event_tag",
          &::gr::pulsed_power::make_wr_event_tag,
          py::arg("event"),
          py::arg("offset"),
          D(make_wr_event_tag));


    m.def("decode_wr_event_tag",
          &::gr::pulsed_power::decode_wr_event_tag,
          py::arg("tag"),
          D(decode_wr_event_tag));
}
