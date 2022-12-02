/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/attributes.h>
#include <gnuradio/pulsed_power/statistics.h>
#include <boost/test/unit_test.hpp>

namespace gr {
namespace pulsed_power {

BOOST_AUTO_TEST_CASE(test_statistics_replace_with_specific_test_name)
{
    auto integration_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { 1.0, 1.0, 1.0 };
    const float* sample = sample_float;
    int num_samples = 3;
}

} /* namespace pulsed_power */
} /* namespace gr */
