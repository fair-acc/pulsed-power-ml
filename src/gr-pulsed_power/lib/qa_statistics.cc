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

BOOST_AUTO_TEST_CASE(constantPositiveSamples)
{
    auto statistics_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { 1.0, 3.0, 2.0 };
    int num_samples = 3;

    float min;
    float max;
    float mean;
    float std_deviation;

    statistics_block->calculate_statistics(
        mean, min, max, std_deviation, sample_float, num_samples);

    BOOST_TEST(mean == 2.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(min == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(max == 3.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(std_deviation == 0.8165, boost::test_tools::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(constantNegativeSamples)
{
    auto statistics_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { -1.0, -3.0, -2.0 };
    int num_samples = 3;

    float min;
    float max;
    float mean;
    float std_deviation;

    statistics_block->calculate_statistics(
        mean, min, max, std_deviation, sample_float, num_samples);

    BOOST_TEST(mean == -2.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(min == -3.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(max == -1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(std_deviation == 0.8165, boost::test_tools::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(constantNegativeAndPositiveSamples)
{
    auto statistics_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { 1.0, 0.0, -1.0 };
    int num_samples = 3;

    float min;
    float max;
    float mean;
    float std_deviation;

    statistics_block->calculate_statistics(
        mean, min, max, std_deviation, sample_float, num_samples);

    BOOST_TEST(mean == 0.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(min == -1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(max == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(std_deviation == 0.8165, boost::test_tools::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(constantSingleSamples)
{
    auto statistics_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { 1.0 };
    int num_samples = 1;

    float min;
    float max;
    float mean;
    float std_deviation;

    statistics_block->calculate_statistics(
        mean, min, max, std_deviation, sample_float, num_samples);

    BOOST_TEST(mean == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(min == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(max == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(std_deviation == 0.0, boost::test_tools::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(constantEqualSamples)
{
    auto statistics_block = gr::pulsed_power::statistics::make();
    float sample_float[3] = { 1.0, 1.0 };
    int num_samples = 2;

    float min;
    float max;
    float mean;
    float std_deviation;

    statistics_block->calculate_statistics(
        mean, min, max, std_deviation, sample_float, num_samples);

    BOOST_TEST(mean == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(min == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(max == 1.0, boost::test_tools::tolerance(0.001));
    BOOST_TEST(std_deviation == 0.0, boost::test_tools::tolerance(0.001));
}
} /* namespace pulsed_power */
} /* namespace gr */
