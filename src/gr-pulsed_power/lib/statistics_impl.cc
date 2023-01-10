/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "statistics_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace pulsed_power {

using input_type = float;
using output_type = float;
statistics::sptr statistics::make(int decimation)
{
    return gnuradio::make_block_sptr<statistics_impl>(decimation);
}


/*
 * The private constructor
 */
statistics_impl::statistics_impl(int decimation)
    : gr::sync_decimator("statistics",
                         gr::io_signature::make(
                             1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                         gr::io_signature::make(4 /* min outputs */,
                                                4 /*max outputs */,
                                                sizeof(output_type)),
                         decimation)
{
    d_decimation = decimation;
}

/*
 * Our virtual destructor.
 */
statistics_impl::~statistics_impl() {}

int statistics_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out_mean = static_cast<output_type*>(output_items[0]);
    auto out_min = static_cast<output_type*>(output_items[1]);
    auto out_max = static_cast<output_type*>(output_items[2]);
    auto out_std_deviation = static_cast<output_type*>(output_items[3]);

    for (int i = 0; i < noutput_items; i++) {
        calculate_statistics(out_mean[i],
                             out_min[i],
                             out_max[i],
                             out_std_deviation[i],
                             &in[i * d_decimation],
                             d_decimation);
    }

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void statistics_impl::calculate_statistics(float& mean,
                                           float& min,
                                           float& max,
                                           float& std_deviation,
                                           const float* in,
                                           int ninput_items)
{
    float sum = 0.0;
    float sum_of_squares = 0.0;
    float current, j;
    min = in[0];
    max = in[0];
    for (int i = 0; i < ninput_items; i++) {
        current = in[i];
        if (current < min) {
            min = current;
        }
        if (current > max) {
            max = current;
        }
        sum += current;
        j = (float)(i + 1);
        // calculate the sum of squares with the formular of Youngs and Cramer
        if (j > 1) {
            sum_of_squares +=
                (1 / (j * (j - 1.0))) * ((j * current) - sum) * ((j * current) - sum);
        }
    }
    mean = sum / ninput_items;
    std_deviation = sqrt(sum_of_squares / ninput_items);
}

} /* namespace pulsed_power */
} /* namespace gr */
