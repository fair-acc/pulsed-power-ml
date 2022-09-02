/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H
#define INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

namespace gr {
namespace pulsed_power {

class opencmw_time_sink_impl : public opencmw_time_sink
{
private:
    float d_sample_rate; 
    std::string d_signal_name; 
    std::string d_signal_unit;
    std::vector<cb_copy_data_t> d_cb_copy_data;

public:
    opencmw_time_sink_impl(float sample_rate,
                           std::string signal_name,
                           std::string signal_unit);
    ~opencmw_time_sink_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

    void register_sink();
    
    void deregister_sink();

    void set_callback(cb_copy_data_t cb_copy_data) override;

    float get_sample_rate() override;

    std::string get_signal_name() override;

    std::string get_signal_unit() override;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H */
