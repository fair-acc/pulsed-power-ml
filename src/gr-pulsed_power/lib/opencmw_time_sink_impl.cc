#include "opencmw_time_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace pulsed_power {

std::mutex globalTimeSinksRegistryMutex;
std::vector<opencmw_time_sink*> globalTimeSinksRegistry;

using input_type = float;
opencmw_time_sink::sptr opencmw_time_sink::make(float sample_rate,
                                                std::vector<std::string> signal_names,
                                                std::vector<std::string> signal_units)
{
    return gnuradio::make_block_sptr<opencmw_time_sink_impl>(
        sample_rate, signal_names, signal_units);
}


opencmw_time_sink_impl::opencmw_time_sink_impl(float sample_rate,
                                               std::vector<std::string> signal_names,
                                               std::vector<std::string> signal_units)
    : gr::sync_block("opencmw_time_sink",
                     gr::io_signature::make(
                         1 /* min inputs */, 10 /* max inputs */, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      _sample_rate(sample_rate),
      _signal_names(signal_names),
      _signal_units(signal_units),
      _timestamp(0)
{
    std::scoped_lock lock(globalTimeSinksRegistryMutex);
    register_sink();
}

opencmw_time_sink_impl::~opencmw_time_sink_impl()
{
    std::scoped_lock lock(globalTimeSinksRegistryMutex);
    deregister_sink();
}

int opencmw_time_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{

    using namespace std::chrono;
    if (_timestamp == 0) {
        _timestamp =
            duration_cast<nanoseconds>(high_resolution_clock().now().time_since_epoch())
                .count();
    } else {
        _timestamp =
            duration_cast<nanoseconds>(high_resolution_clock().now().time_since_epoch())
                .count();
        // _timestamp +=
        //     static_cast<double>(noutput_items) * (1 /
        //     static_cast<double>(_sample_rate));
    }
    int nitems_to_process = noutput_items;

    for (auto callback : _cb_copy_data) {
        std::invoke(callback,
                    input_items,
                    noutput_items,
                    _signal_names,
                    _sample_rate,
                    _timestamp);
    }

    if (noutput_items == 0) {
        std::cout << "noutput_items " << nitems_to_process << " -> processed 0 items\n";
    }
    return noutput_items;
}
void opencmw_time_sink_impl::register_sink() { globalTimeSinksRegistry.push_back(this); }

void opencmw_time_sink_impl::deregister_sink()
{
    auto result =
        std::find(globalTimeSinksRegistry.begin(), globalTimeSinksRegistry.end(), this);
    if (result != globalTimeSinksRegistry.end()) {
        globalTimeSinksRegistry.erase(result);
    }
}

void opencmw_time_sink_impl::set_callback(cb_copy_data_t cb_copy_data)
{
    _cb_copy_data.push_back(cb_copy_data);
}

float opencmw_time_sink_impl::get_sample_rate() { return _sample_rate; }

std::vector<std::string> opencmw_time_sink_impl::get_signal_names()
{
    return _signal_names;
}

std::vector<std::string> opencmw_time_sink_impl::get_signal_units()
{
    return _signal_units;
}

} /* namespace pulsed_power */
} /* namespace gr */
