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
statistics::sptr statistics::make()
{
    return gnuradio::make_block_sptr<statistics_impl>();
}


/*
 * The private constructor
 */
statistics_impl::statistics_impl()
    : gr::block("statistics",
                gr::io_signature::make(
                    1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                gr::io_signature::make(
                    4 /* min outputs */, 4 /*max outputs */, sizeof(output_type)))
{
}

/*
 * Our virtual destructor.
 */
statistics_impl::~statistics_impl() {}

void statistics_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
#pragma message( \
    "implement a forecast that fills in how many items on each input you need to produce noutput_items and remove this warning")
    /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

int statistics_impl::general_work(int noutput_items,
                                  gr_vector_int& ninput_items,
                                  gr_vector_const_void_star& input_items,
                                  gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out_mean = static_cast<output_type*>(output_items[0]);
    auto out_min = static_cast<output_type*>(output_items[1]);
    auto out_max = static_cast<output_type*>(output_items[2]);
    auto out_std_deviation = static_cast<output_type*>(output_items[3]);

    calculate_statistics(
        out_mean[0], out_min[0], out_max[0], out_std_deviation[0], in, noutput_items);

    // Tell runtime system how many input items we consumed on
    // each input stream.
    consume_each(noutput_items);

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
