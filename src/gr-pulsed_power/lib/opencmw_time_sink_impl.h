#ifndef INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H
#define INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

namespace gr {
namespace pulsed_power {

class opencmw_time_sink_impl : public opencmw_time_sink
{
private:
    std::vector<std::string> _signal_names;
    std::vector<std::string> _signal_units;
    float _sample_rate;
    std::vector<cb_copy_data_t> _cb_copy_data;
    int64_t _timestamp;

public:
    opencmw_time_sink_impl(const std::vector<std::string>& signal_names,
                           const std::vector<std::string>& signal_units,
                           float sample_rate);
    ~opencmw_time_sink_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
    void register_sink();

    void deregister_sink();

    void set_callback(cb_copy_data_t cb_copy_data) override;

    float get_sample_rate() override;

    std::vector<std::string> get_signal_names() override;

    std::vector<std::string> get_signal_units() override;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_IMPL_H */
