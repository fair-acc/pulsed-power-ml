/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_STATISTICS_IMPL_H
#define INCLUDED_PULSED_POWER_STATISTICS_IMPL_H

#include <gnuradio/pulsed_power/statistics.h>

namespace gr {
namespace pulsed_power {

class statistics_impl : public statistics
{
public:
    statistics_impl();
    ~statistics_impl();

    void calculate_statistics(float& mean,
                              float& min,
                              float& max,
                              float& std_deviation,
                              const float* in,
                              int ninput_items) override;

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_STATISTICS_IMPL_H */
