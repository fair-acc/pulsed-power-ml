#include "opencmw_freq_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace pulsed_power {

std::mutex globalFrequencySinksRegistryMutex;
std::vector<opencmw_freq_sink*> globalFrequencySinksRegistry;

using input_type = float;
opencmw_freq_sink::sptr opencmw_freq_sink::make(std::vector<std::string> signal_names,
                                                std::vector<std::string> signal_units,
                                                float sample_rate,
                                                float bandwidth,
                                                size_t vector_size)
{
    return gnuradio::make_block_sptr<opencmw_freq_sink_impl>(
        signal_names, signal_units, sample_rate, bandwidth, vector_size);
}


/*
 * The private constructor
 */
opencmw_freq_sink_impl::opencmw_freq_sink_impl(std::vector<std::string> signal_names,
                                               std::vector<std::string> signal_units,
                                               float sample_rate,
                                               float bandwidth,
                                               size_t vector_size)
    : gr::sync_block("opencmw_freq_sink",
                     gr::io_signature::make(1 /* min inputs */,
                                            10 /* max inputs */,
                                            sizeof(input_type) * vector_size),
                     gr::io_signature::make(0, 0, 0)),
      _signal_names(signal_names),
      _signal_units(signal_units),
      _sample_rate(sample_rate),
      _bandwidth(bandwidth),
      _vector_size(vector_size)
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

    using namespace std::chrono;
    int64_t timestamp =
        duration_cast<nanoseconds>(high_resolution_clock().now().time_since_epoch())
            .count();
    for (size_t i = 0; i < _signal_names.size(); i++) {
        auto in = static_cast<const input_type*>(input_items[i]);
        for (auto callback : _cb_copy_data) {
            std::invoke(callback,
                        in,
                        noutput_items,
                        _vector_size,
                        _signal_names[i],
                        _sample_rate,
                        timestamp);
        }
    }

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
    _cb_copy_data.push_back(cb_copy_data);
}

float opencmw_freq_sink_impl::get_bandwidth() { return _bandwidth; }

float opencmw_freq_sink_impl::get_sample_rate() { return _sample_rate; }

std::vector<std::string> opencmw_freq_sink_impl::get_signal_names()
{
    return _signal_names;
}

std::vector<std::string> opencmw_freq_sink_impl::get_signal_units()
{
    return _signal_units;
}

size_t opencmw_freq_sink_impl::get_vector_size() { return _vector_size; }

} /* namespace pulsed_power */
} /* namespace gr */
