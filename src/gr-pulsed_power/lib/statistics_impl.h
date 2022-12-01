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
private:
    float current_mean;
    float current_min;
    float current_max;
    float sum;

public:
    statistics_impl();
    ~statistics_impl();

    void calculate_mean(float* out, float* current_mean, int nitems);
    void determine_min(float* out, float* current_min);
    void determine_max(float* out, float* current_max);
    void calculate_std_deviation(float* out);

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
