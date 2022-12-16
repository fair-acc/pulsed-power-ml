#ifndef INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_IMPL_H
#define INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_IMPL_H

#include "gnuradio/pulsed_power/picoscope_4000a_source.h"

namespace {
boost::mutex g_init_mutex;
}

namespace gr {
namespace pulsed_power {

class picoscope_4000a_source_impl : public pulsed_power::picoscope_4000a_source
{
private:
    /**
     * @brief Structure used for streaming setup.
     *
     */
    struct ps4000a_unit_interval_t {
        PS4000A_TIME_UNITS unit;
        uint32_t interval;
    };

    int16_t d_handle;   // picoscope handle
    int16_t d_overflow; // status returned from getValues

public:
    picoscope_4000a_source_impl(std::string serial_number, bool auto_arm);
    ~picoscope_4000a_source_impl();

    // Picoscope error

    /**
     * @brief A struct holding category of the status code and its corresponding message.
     *
     */
    struct PicoStatus4000aErrc : std::error_category {
        /**********************************************************************
         * Picoscope - helper functions
         *********************************************************************/

        /**
         * @brief The name of the picoscope device.
         *
         * @return const char*, name
         */
        const char* name() const noexcept override { return "Ps4000a"; }

        /**
         * @brief The message transmitted with the error code.
         *
         * @param ev Error message
         * @return std::string, message
         */
        std::string message(int ev) const override
        {
            PICO_STATUS status = static_cast<PICO_STATUS>(ev);
            return ps4000a_get_error_message(status);
        }
    };

    const PicoStatus4000aErrc thePsErrCategory;

    /**
     * @brief The category of picoscope error code.
     * @note This method is needed because PICO_STATUS is not a distinct type (e.g. an
     * enum), therfore we cannot really hook this into the std error code properly.
     * @param e Picoscope status error
     * @return std::error_code, category
     */
    std::error_code make_pico_4000a_error_code(PICO_STATUS e)
    {
        return { static_cast<int>(e), thePsErrCategory };
    }

    uint32_t convert_frequency_to_ps4000a_timebase(double desired_freq,
                                                   double& actual_freq);

    struct ps4000a_unit_interval_t
    convert_frequency_to_ps4000a_time_units_and_interval(double desired_freq,
                                                         double& actual_freq);

    // Driver
    std::string get_driver_version() override;

    std::string get_hardware_version() override;

    std::error_code driver_initialize() override;

    std::error_code driver_configure() override;

    std::error_code driver_arm() override;

    std::error_code driver_disarm() override;

    std::error_code driver_close() override;

    std::error_code driver_prefetch_block(size_t length, size_t block_number) override;

    std::error_code driver_get_rapid_block_data(size_t offset,
                                                size_t length,
                                                size_t waveform,
                                                gr_vector_void_star& arrays,
                                                std::vector<uint32_t>& status) override;

    std::error_code driver_poll() override;

    void rapid_block_callback(int16_t handle, PICO_STATUS status);

    std::string get_unit_info_topic(PICO_INFO info);

    std::error_code set_buffers(size_t samples, uint32_t block_number);

    void set_trigger_once(bool trigger_once) override;

    void set_samp_rate(double rate) override;

    void set_downsampling(downsampling_mode_t mode, int downsample_factor = 0) override;

    void set_aichan(const std::string& id,
                    bool enabled,
                    double range,
                    coupling_t coupling,
                    double range_offset = 0) override;

    void set_aichan_trigger(const std::string& id,
                            trigger_direction_t direction,
                            double threshold) override;

    void set_samples(int pre_samples, int post_samples);


    void set_aichan_a(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_b(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_c(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_d(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_e(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_f(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_g(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);
    void set_aichan_h(bool enabled,
                      double range,
                      coupling_t coupling,
                      double range_offset = 0);

    void set_rapid_block(int nr_waveforms);

    void set_nr_buffers(int nr_buffers);

    void set_streaming(double poll_rate = 0.001);

    void set_driver_buffer_size(int driver_buffer_size);

    void set_buffer_size(int buffer_size);
    // uint32_t convert_frequency_to_ps4000a_timebase(double desired_freq, double
    // &actual_freq);

    // // Where all the action really happens
    // int work(
    //         int noutput_items,
    //         gr_vector_const_void_star &input_items,
    //         gr_vector_void_star &output_items
    // );
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_IMPL_H */
