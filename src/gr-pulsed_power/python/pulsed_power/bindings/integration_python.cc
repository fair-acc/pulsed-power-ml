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
/* BINDTOOL_HEADER_FILE(integration.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(589a0073905ed9a3dbf8a7857fa4864c)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/integration.h>
// pydoc.h is automatically generated in the build directory
#include <integration_pydoc.h>

void bind_integration(py::module& m)
{

    using integration = ::gr::pulsed_power::integration;


    py::class_<integration, gr::block, gr::basic_block, std::shared_ptr<integration>>(
        m, "integration", D(integration))

        .def(py::init(&integration::make), py::arg("sample_rate"), D(integration, make))


        .def("integrate",
             &integration::integrate,
             py::arg("out"),
             py::arg("sample"),
             py::arg("noutput_items"),
             py::arg("calculate_with_last_value"),
             D(integration, integrate))

        .def("add_new_steps",
             &integration::add_new_steps,
             py::arg("out"),
             py::arg("sample"),
             py::arg("noutput_items"),
             D(integration, add_new_steps))

        ;
}
