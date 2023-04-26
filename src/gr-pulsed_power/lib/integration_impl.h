/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H
#define INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H

#include <gnuradio/pulsed_power/integration.h>
#include <chrono>

namespace gr {
namespace pulsed_power {
using namespace std::chrono;

class integration_impl : public integration
{
private:
    double d_step_size;
    int d_decimation;
    float d_last_value;
    time_point<system_clock> d_last_save;
    time_point<system_clock> d_last_reset;
    float d_sum;
    int d_duration;
    std::string d_filename;

    int get_values_from_file(time_point<system_clock>& last_reset,
                             time_point<system_clock>& last_save,
                             float& sum);
    int write_to_file(time_point<system_clock> last_reset,
                      time_point<system_clock> last_save,
                      float sum);

public:
    integration_impl(int decimation,
                     int sample_rate,
                     INTEGRATION_DURATION duration,
                     std::string savefilename);
    ~integration_impl() override;

    void add_new_steps(float* out, const float* sample, int noutput_items) override;

    void integrate(float& out,
                   const float* sample,
                   int n_samples,
                   bool calculate_with_last_value) override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};


} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_INTEGRATION_IMPL_H */
