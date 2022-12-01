/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H
#define INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H

#include <gnuradio/pulsed_power/integration.h>

namespace gr {
namespace pulsed_power {

class integration_impl : public integration
{
private:
    float d_step_size;

public:
    integration_impl(float step_size);
    ~integration_impl();

    void integrate(float* out, const float* sample, int noutput_items);

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H */
