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

    Statistic statistic;
    statistic.min = in[0];
    statistic.max = in[0];
    calculate_statistics(statistic, in, noutput_items);

    out_mean = &statistic.mean;
    out_min = &statistic.min;
    out_max = &statistic.max;
    out_std_deviation = &statistic.std_deviation;

    // Tell runtime system how many input items we consumed on
    // each input stream.
    consume_each(noutput_items);

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void calculate_statistics(Statistic& statistic, const float* in, int num_samples)
{
    float variance = 0.0;
    float sum = 0.0;
    float sum_of_squares = 0.0;
    float current = 0.0;
    for (int i = 0; i < num_samples; i++) {
        current = in[i];
        if (current < statistic.min) {
            statistic.min = current;
        }
        if (current > statistic.max) {
            statistic.max = current;
        }
        sum += current;
        sum_of_squares += current * current;
    }
    statistic.mean = sum / num_samples;
    variance = sum_of_squares - (sum * sum);
    statistic.std_deviation = sqrt(variance);
}

void calculate_mean(float* out, float* current_mean, int nitems) {}
void determine_min(float* out, float* current_min);
void determine_max(float* out, float* current_max);
void calculate_std_deviation(float* out);

} /* namespace pulsed_power */
} /* namespace gr */
