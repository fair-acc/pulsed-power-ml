/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "opencmw_freq_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace pulsed_power {

std::mutex globalFrequencySinksRegistryMutex;
std::vector<opencmw_freq_sink*> globalFrequencySinksRegistry;

using input_type = float;
opencmw_freq_sink::sptr opencmw_freq_sink::make(std::string signal_name,
                                                std::string signal_unit,
                                                float sample_rate,
                                                float bandwidth,
                                                size_t vector_size)
{
    return gnuradio::make_block_sptr<opencmw_freq_sink_impl>(
        signal_name, signal_unit, sample_rate, bandwidth, vector_size);
}


/*
 * The private constructor
 */
opencmw_freq_sink_impl::opencmw_freq_sink_impl(std::string signal_name,
                                               std::string signal_unit,
                                               float sample_rate,
                                               float bandwidth,
                                               size_t vector_size)
    : gr::sync_block("opencmw_freq_sink",
                     gr::io_signature::make(1 /* min inputs */,
                                            1 /* max inputs */,
                                            sizeof(input_type) * vector_size),
                     gr::io_signature::make(0, 0, 0)),
      d_signal_name(signal_name),
      d_signal_unit(signal_unit),
      d_sample_rate(sample_rate),
      d_bandwidth(bandwidth),
      d_vector_size(vector_size)
{
    std::scoped_lock lock(globalFrequencySinksRegistryMutex);
    register_sink();
}

/*
 * Our virtual destructor.
 */
opencmw_freq_sink_impl::~opencmw_freq_sink_impl()
{
    std::scoped_lock lock(globalFrequencySinksRegistryMutex);
    deregister_sink();
}

int opencmw_freq_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{
    size_t nsignals = input_items.size();
    size_t block_size = input_signature()->sizeof_stream_item(0);
    auto in = static_cast<const input_type*>(input_items[0]);

    using namespace std::chrono;
    int64_t timestamp =
        duration_cast<nanoseconds>(high_resolution_clock().now().time_since_epoch())
            .count();

    for (auto callback : d_cb_copy_data) {
        std::invoke(callback,
                    in,
                    noutput_items,
                    d_vector_size,
                    d_signal_name,
                    d_sample_rate,
                    timestamp);
    }

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void opencmw_freq_sink_impl::register_sink()
{
    globalFrequencySinksRegistry.push_back(this);
}

void opencmw_freq_sink_impl::deregister_sink()
{
    auto result = std::find(
        globalFrequencySinksRegistry.begin(), globalFrequencySinksRegistry.end(), this);
    if (result != globalFrequencySinksRegistry.end()) {
        globalFrequencySinksRegistry.erase(result);
    }
}

void opencmw_freq_sink_impl::set_callback(cb_copy_data_t cb_copy_data)
{
    d_cb_copy_data.push_back(cb_copy_data);
}

float opencmw_freq_sink_impl::get_bandwidth() { return d_bandwidth; }

float opencmw_freq_sink_impl::get_sample_rate() { return d_sample_rate; }

std::string opencmw_freq_sink_impl::get_signal_name() { return d_signal_name; }

std::string opencmw_freq_sink_impl::get_signal_unit() { return d_signal_unit; }

size_t opencmw_freq_sink_impl::get_vector_size() { return d_vector_size; }

} /* namespace pulsed_power */
} /* namespace gr */
