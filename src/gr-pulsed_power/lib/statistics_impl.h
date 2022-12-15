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
    statistics_impl(int decimation);
    ~statistics_impl();

    void calculate_statistics(float& mean, float& min, float& max, float& std_deviation, const float* in, int ninput_items) override;

    int work(int noutput_items,
	     gr_vector_const_void_star& input_items,
	     gr_vector_void_star& output_items) override;

private:
  int d_decimation;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_STATISTICS_IMPL_H */
