/*
 * Copyright 2023 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(power_calc_cc.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(88a20dc25f3f9ffefeda0eab23deb931)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pulsed_power/power_calc_cc.h>
// pydoc.h is automatically generated in the build directory
#include <power_calc_cc_pydoc.h>

void bind_power_calc_cc(py::module& m)
{

    using power_calc_cc = ::gr::pulsed_power::power_calc_cc;


    py::class_<power_calc_cc,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<power_calc_cc>>(m, "power_calc_cc", D(power_calc_cc))

        .def(py::init(&power_calc_cc::make),
             py::arg("alpha") = 9.9999999999999995E-8,
             D(power_calc_cc, make))


        .def("set_alpha",
             &power_calc_cc::set_alpha,
             py::arg("alpha"),
             D(power_calc_cc, set_alpha))


        .def("calc_active_power",
             &power_calc_cc::calc_active_power,
             py::arg("out"),
             py::arg("voltage"),
             py::arg("current"),
             py::arg("phi_out"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_active_power))


        .def("calc_reactive_power",
             &power_calc_cc::calc_reactive_power,
             py::arg("out"),
             py::arg("voltage"),
             py::arg("current"),
             py::arg("phi_out"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_reactive_power))


        .def("calc_apparent_power",
             &power_calc_cc::calc_apparent_power,
             py::arg("out"),
             py::arg("voltage"),
             py::arg("current"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_apparent_power))


        .def("calc_phi",
             &power_calc_cc::calc_phi,
             py::arg("phi_out"),
             py::arg("u_in"),
             py::arg("i_in"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_phi))


        .def("calc_rms_u",
             &power_calc_cc::calc_rms_u,
             py::arg("output"),
             py::arg("input"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_rms_u))


        .def("calc_rms_i",
             &power_calc_cc::calc_rms_i,
             py::arg("output"),
             py::arg("input"),
             py::arg("noutput_items"),
             D(power_calc_cc, calc_rms_i))

        ;
}
