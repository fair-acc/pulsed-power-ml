/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/pulsed_power/picoscope_base.h>

namespace gr {
namespace pulsed_power {

/**********************************************************************
 * Structors
 *********************************************************************/

picoscope_base::picoscope_base(std::string serial_number,
                               int max_ai_channels,
                               int max_di_ports,
                               bool auto_arm,
                               int16_t max_raw_analog_value,
                               float vertical_precision)
    : digitizer_source(max_ai_channels, max_di_ports, auto_arm),
      d_serial_number(serial_number),
      d_max_value(max_raw_analog_value),
      d_vertical_precision(vertical_precision),
      d_ranges(),
      d_streaming_callback(
          boost::bind(&picoscope_base::streaming_callback, this, _1, _2, _3)),
      d_buffers(max_ai_channels),
      d_buffers_min(max_ai_channels),
      d_port_buffers(max_di_ports),
      d_tmp_buffer(nullptr),
      d_tmp_buffer_size(0),
      d_lost_count(0)
{
    for (auto i = 0; i < max_ai_channels; i++) {
        d_channel_ids.emplace_back("" + static_cast<char>('A' + i));
    }
}

picoscope_base::~picoscope_base() {}


/**********************************************************************
 * Driver implementation
 *********************************************************************/

std::vector<std::string> picoscope_base::get_aichan_ids() { return d_channel_ids; }

meta_range_t picoscope_base::get_aichan_ranges() { return d_ranges; }

void picoscope_base::streaming_callback(int32_t nr_samples,
                                        uint32_t start_index,
                                        int16_t overflow)
{
    // trigger timestamp
    uint64_t local_timestamp = get_timestamp_nano_utc();

    // According to well informed sources, the driver indicates the buffer overrun by
    // setting all the bits of the overflow argument to true.
    if (static_cast<uint16_t>(overflow) == 0xFFFF) {
        // TODO: FIX logging
        // GR_LOG_ERROR(d_logger, "Buffer overrun detected, continue...");
    }

    // monitor sampler rate (estimated used by the watchdog)
    auto timestamp_now = std::chrono::high_resolution_clock::now();

    if (d_was_last_callback_timestamp_taken) {
        std::chrono::duration<float> time_diff =
            timestamp_now - d_last_callback_timestamp;
        auto samp_rate = static_cast<float>(nr_samples) / time_diff.count();

        {
            // Mutex is not needed because this callback is called from the poll thread
            // boost::mutex::scoped_lock watchdog_guard(d_watchdog_mutex);
            d_estimated_sample_rate.add(samp_rate);
        }
    } else {
        d_was_last_callback_timestamp_taken = true;
    }

    d_last_callback_timestamp = timestamp_now;

    // Buffer size per channel in bytes
    const auto channel_buffer_size_bytes = d_buffer_size * sizeof(float);

    while (nr_samples > 0) {

        // Check if we need to retrieve new data chunk
        if (d_tmp_buffer_size == 0) {
            assert(d_tmp_buffer == nullptr);
            d_tmp_buffer = d_app_buffer.get_free_data_chunk();

            if (d_tmp_buffer == nullptr) {
                d_lost_count++;
                nr_samples -= d_buffer_size;
                continue;
            }
        }

        // Figure out how many samples need to be converted before the temporary data
        // buffer is full. Also calculate how many iterations will be required.
        unsigned samples_to_convert =
            std::min((unsigned)nr_samples, (unsigned)(d_buffer_size - d_tmp_buffer_size));
        nr_samples -= samples_to_convert;

        auto tmp_channel_idx = 0; // to iterate enabled channels only

        for (auto channel_idx = 0; channel_idx < d_ai_channels; channel_idx++) {

            if (!d_channel_settings[channel_idx].enabled) {
                continue;
            }

            const float voltage_multiplier =
                (float)d_channel_settings[channel_idx].range / (float)d_max_value;

            // Buffer organization:
            //   <chan 1 values> <chan 1 errors> <chan 2 values> <chan 2 errors> ...
            // Note here the address of the very first sample we are about to save is
            // calculated, meaning number of samples already in the buffer are accounted
            // for.
            float* tmp_buffer_values =
                reinterpret_cast<float*>(
                    &d_tmp_buffer->d_data[0] +
                    (tmp_channel_idx * channel_buffer_size_bytes * 2)) +
                d_tmp_buffer_size;
            float* tmp_buffer_errors =
                reinterpret_cast<float*>(
                    &d_tmp_buffer->d_data[0] +
                    (tmp_channel_idx * channel_buffer_size_bytes * 2) +
                    channel_buffer_size_bytes) +
                d_tmp_buffer_size;

            // Points to the first raw sample we are about to convert. NOTE, there is a
            // dedicated driver buffer available per channels therefore we need to use
            // variable channel_idx and not tmp_channel_idx!!!
            int16_t* driver_buffer = &d_buffers[channel_idx][start_index];

            if (d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_NONE ||
                d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_DECIMATE) {

                // void volk_16i_s32f_convert_32f(float* outputVector, const int16_t*
                // inputVector, const float scalar, unsigned int num_points);
                volk_16i_s32f_convert_32f(tmp_buffer_values,
                                          driver_buffer,
                                          1.0f / voltage_multiplier,
                                          samples_to_convert);

                // for (uint32_t i = 0; i < samples_to_convert; i++) {
                //   tmp_buffer_values[i] = (voltage_multiplier *
                //   (float)driver_buffer[i]);
                // }

                // According to specs
                const auto error_estimate =
                    d_channel_settings[channel_idx].range * d_vertical_precision;
                for (uint32_t i = 0; i < samples_to_convert; i++) {
                    tmp_buffer_errors[i] = error_estimate;
                }
            } else if (d_downsampling_mode ==
                       downsampling_mode_t::DOWNSAMPLING_MODE_MIN_MAX_AGG) {
                for (uint32_t i = 0; i < samples_to_convert; i++) {
                    int16_t* driver_buffer_min = &d_buffers_min[channel_idx][start_index];

                    auto max = (voltage_multiplier * (float)driver_buffer[i]);
                    auto min = (voltage_multiplier * (float)driver_buffer_min[i]);

                    tmp_buffer_values[i] = (max + min) / 2.0;
                    tmp_buffer_errors[i] = (max - min) / 4.0;
                }
            } else if (d_downsampling_mode ==
                       downsampling_mode_t::DOWNSAMPLING_MODE_AVERAGE) {

                volk_16i_s32f_convert_32f(tmp_buffer_values,
                                          driver_buffer,
                                          1.0f / voltage_multiplier,
                                          samples_to_convert);

                // for (uint32_t i = 0; i < samples_to_convert; i++) {
                //   tmp_buffer_values[i] = voltage_multiplier * (float)driver_buffer[i];
                // }

                // According to specs
                const auto error_estimate_single =
                    d_channel_settings[channel_idx].range * d_vertical_precision;
                const auto error_estimate =
                    error_estimate_single / std::sqrt((float)d_downsampling_factor);
                for (uint32_t i = 0; i < samples_to_convert; i++) {
                    tmp_buffer_errors[i] = error_estimate;
                }
            } else {
                assert(false);
            }

            // move to another channel slot
            tmp_channel_idx++;
        } // for each channel

        auto tmp_port_idx = 0;
        const auto port_buffer_size = d_buffer_size * sizeof(uint8_t);
        uint8_t* first_port_sample =
            &d_tmp_buffer->d_data[0] + (tmp_channel_idx * channel_buffer_size_bytes * 2);

        for (auto port_idx = 0; port_idx < d_ports; port_idx++) {

            if (!d_port_settings[port_idx].enabled) {
                continue;
            }

            uint8_t* port_values =
                first_port_sample + port_buffer_size * tmp_port_idx + d_tmp_buffer_size;
            const int16_t* driver_buffer = &d_port_buffers[port_idx][start_index];

            for (uint32_t i = 0; i < samples_to_convert; i++) {
                port_values[i] = static_cast<uint8_t>(0x00ff & driver_buffer[i]);
            }

            tmp_port_idx++;
        }

        d_tmp_buffer_size += samples_to_convert;
        assert(d_tmp_buffer_size <= d_buffer_size);

        // move
        start_index += samples_to_convert;

        // Temporary buffer is full, push data into application buffer
        if (d_tmp_buffer_size == d_buffer_size) {
            d_tmp_buffer->d_local_timestamp = local_timestamp;

            d_tmp_buffer->d_lost_count = d_lost_count;
            d_lost_count = 0;

            // convert status to the format expected by the digitizer base class
            d_tmp_buffer->d_status.resize(get_enabled_aichan_count());

            for (auto i = 0; i < get_enabled_aichan_count(); i++) {
                if (overflow & (1 << i)) {
                    d_tmp_buffer->d_status[i] = channel_status_t::CHANNEL_STATUS_OVERFLOW;
                } else {
                    d_tmp_buffer->d_status[i] = 0;
                }
            }

            d_app_buffer.add_full_data_chunk(d_tmp_buffer);

            d_tmp_buffer = nullptr;
            d_tmp_buffer_size = 0;
        }
    } // iteration
}

} // namespace pulsed_power
} /* namespace gr */
