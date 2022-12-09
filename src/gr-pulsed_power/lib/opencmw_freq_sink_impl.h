#ifndef INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H
#define INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>

namespace gr {
namespace pulsed_power {

class opencmw_freq_sink_impl : public opencmw_freq_sink
{
private:
    std::vector<std::string> _signal_names;
    std::vector<std::string> _signal_units;
    float _sample_rate;
    float _bandwidth;
    size_t _vector_size;
    int64_t _timestamp;
    std::vector<cb_copy_data_t> _cb_copy_data;

public:
    opencmw_freq_sink_impl(std::vector<std::string> signal_names,
                           std::vector<std::string> signal_units,
                           float sample_rate,
                           float bandwidth,
                           size_t vector_size);
    ~opencmw_freq_sink_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

    void register_sink();

    void deregister_sink();

    void set_callback(cb_copy_data_t cb_copy_data) override;

    float get_sample_rate() override;

    float get_bandwidth() override;

    std::vector<std::string> get_signal_names() override;

    std::vector<std::string> get_signal_units() override;

    size_t get_vector_size() override;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_IMPL_H */
