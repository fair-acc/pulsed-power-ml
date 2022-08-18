/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_OPENCMW_SINK_IMPL_H
#define INCLUDED_PULSED_POWER_OPENCMW_SINK_IMPL_H

#include <gnuradio/pulsed_power/opencmw_sink.h>

namespace gr {
namespace pulsed_power {

class opencmw_sink_impl : public opencmw_sink
{
private:
    std::vector<std::function<void(const float*, int)>> _callbackFct;

public:
    opencmw_sink_impl();
    ~opencmw_sink_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

    void registerSink();
    void deregisterSink();

    void set_callback(std::function<void(const float*, int)> cb_copy_data) override;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_SINK_IMPL_H */
