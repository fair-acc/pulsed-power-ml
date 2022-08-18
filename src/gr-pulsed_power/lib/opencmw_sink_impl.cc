/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "opencmw_sink_impl.h"
#include <gnuradio/io_signature.h>

#include <iostream>

namespace gr {
namespace pulsed_power {

std::mutex globalSinksRegistryMutex;
std::vector<opencmw_sink*> globalSinksRegistry;

using input_type = float;
opencmw_sink::sptr opencmw_sink::make()
{
    return gnuradio::make_block_sptr<opencmw_sink_impl>();
}


/*
 * The private constructor
 */
opencmw_sink_impl::opencmw_sink_impl()
    : gr::sync_block("opencmw_sink",
                     gr::io_signature::make(
                         1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0))
{
    std::scoped_lock lock(globalSinksRegistryMutex);
    registerSink();
}

/*
 * Our virtual destructor.
 */
opencmw_sink_impl::~opencmw_sink_impl() 
{
    std::scoped_lock lock(globalSinksRegistryMutex);
    deregisterSink();
}

int opencmw_sink_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);

    // Do <+signal processing+>
    for (auto callBack: _callbackFct) {
        std::invoke(callBack, in, noutput_items);
    }

    std::cout << "noutput_items: " << noutput_items << std::endl;

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void opencmw_sink_impl::registerSink()
{
    globalSinksRegistry.push_back(this);
}

void opencmw_sink_impl::deregisterSink()
{
    auto result = std::find(globalSinksRegistry.begin(), globalSinksRegistry.end(), this);
    if (result != globalSinksRegistry.end()) {
        globalSinksRegistry.erase(result);
    }
}

void opencmw_sink_impl::set_callback(std::function<void(const float*, int)> cb_copy_data) {
    _callbackFct.push_back(cb_copy_data);
}

} /* namespace pulsed_power */
} /* namespace gr */
