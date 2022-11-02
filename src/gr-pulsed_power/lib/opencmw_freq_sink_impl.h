/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H
#define INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>

namespace gr {
namespace pulsed_power {

class opencmw_freq_sink_impl : public opencmw_freq_sink
{
private:
    std::string d_signal_name;
    std::string d_signal_unit;
    float d_sample_rate;
    float d_bandwidth;
    size_t d_vector_size;
    std::vector<cb_copy_data_t> d_cb_copy_data;

public:
    opencmw_freq_sink_impl(std::string signal_name,
                           std::string signal_unit,
                           float sample_rate,
                           float bandwidth,
                           size_t vector_size);
    ~opencmw_freq_sink_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

    void register_sink();

    void deregister_sink();

    void set_callback(cb_copy_data_t cb_copy_data) override;

    float get_sample_rate() override;

    float get_bandwidth() override;

    std::string get_signal_name() override;

    std::string get_signal_unit() override;

    size_t get_vector_size() override;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H */
