/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/attributes.h>
#include <gnuradio/pulsed_power/integration.h>
#include <boost/test/unit_test.hpp>

namespace gr {
namespace pulsed_power {

BOOST_AUTO_TEST_CASE(constantPositiveSamplesReturnPositiveValue)
{
    // Create instance
    float sample_rate = 100.0;
    auto integration_block = gr::pulsed_power::integration::make(sample_rate);
    // Create pointer and samples
    float sample_float[2] = { 1.0, 1.0 };
    const float* sample = sample_float;
    int num_samples = 2;
    // Create pointer for output
    float out_float[1] = { 0.0 };
    float* out = out_float;

    // Call function
    integration_block->integrate(out, sample, num_samples);

    // Check result
    BOOST_TEST(out[0] == (1.0 * 1 / sample_rate), boost::test_tools::tolerance(0.001));
}

} /* namespace pulsed_power */
} /* namespace gr */
