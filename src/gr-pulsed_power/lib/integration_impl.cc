/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "integration_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace pulsed_power {

using input_type = float;
using output_type = float;
integration::sptr integration::make(float step_size)
{
    return gnuradio::make_block_sptr<integration_impl>(step_size);
}


/*
 * The private constructor
 */
integration_impl::integration_impl(float step_size)
    : gr::block("integration",
                gr::io_signature::make(
                    1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                gr::io_signature::make(
                    1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
      d_step_size(step_size)
{
}

/*
 * Our virtual destructor.
 */
integration_impl::~integration_impl() {}

void integration_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
#pragma message( \
    "implement a forecast that fills in how many items on each input you need to produce noutput_items and remove this warning")
    /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

int integration_impl::general_work(int noutput_items,
                                   gr_vector_int& ninput_items,
                                   gr_vector_const_void_star& input_items,
                                   gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    integrate(out, in, noutput_items);

    // Tell runtime system how many input items we consumed on
    // each input stream.
    consume_each(noutput_items);

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void integration_impl::integrate(float* out, const float* sample, int noutput_items)
{
    double value = 0;
    for (int i = 1; i < noutput_items; i++) {
        value += d_step_size * ((sample[i - 1] + sample[i]) / 2);
    }
    out[0] = value;
}

} /* namespace pulsed_power */
} /* namespace gr */
