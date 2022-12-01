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
    auto out0 = static_cast<output_type*>(output_items[0]);
    auto out1 = static_cast<output_type*>(output_items[1]);
    auto out2 = static_cast<output_type*>(output_items[2]);
    auto out3 = static_cast<output_type*>(output_items[3]);

    float mean = 0.0;
    float min = in[0];
    float max = in[0];
    float variance = 0.0;
    float std_deviation = 0.0;
    float sum = 0.0;
    float sum_of_squares = 0.0;
    float current = 0.0;
    for (int i = 0; i < noutput_items; i++) {
        current = in[i];
        if (current < min) {
            min = current;
        }
        if (current > max) {
            max = current;
        }
        sum += current;
        sum_of_squares += current * current;
    }
    mean = sum / noutput_items;
    variance = sum_of_squares - (sum * sum);
    std_deviation = sqrt(variance);

    out0 = &mean;
    out1 = &min;
    out2 = &max;
    out3 = &std_deviation;

    // Tell runtime system how many input items we consumed on
    // each input stream.
    consume_each(noutput_items);

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace pulsed_power */
} /* namespace gr */
