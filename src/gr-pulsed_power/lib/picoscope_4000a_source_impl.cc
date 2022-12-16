/* -*- c++ -*- */
/*
 * Copyright 2021 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picoscope_4000a_source_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace pulsed_power {

picoscope_4000a_source::sptr picoscope_4000a_source::make(std::string serial_number,
                                                          bool auto_arm)
{
    return gnuradio::make_block_sptr<picoscope_4000a_source_impl>(serial_number,
                                                                  auto_arm);
}

/**********************************************************************
 * Converters - helper functions
 *********************************************************************/

static PS4000A_COUPLING convert_to_ps4000a_coupling(coupling_t coupling)
{
    if (coupling == AC_1M)
        return PS4000A_AC;
    else if (coupling == DC_1M)
        return PS4000A_DC;
    else {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": unsupported coupling mode:" << coupling;
        throw std::runtime_error(message.str());
    }
}

static PS4000A_RANGE convert_to_ps4000a_range(float range)
{
    if (range == 0.01)
        return PS4000A_10MV;
    else if (range == 0.02)
        return PS4000A_20MV;
    else if (range == 0.05)
        return PS4000A_50MV;
    else if (range == 0.1)
        return PS4000A_100MV;
    else if (range == 0.2)
        return PS4000A_200MV;
    else if (range == 0.5)
        return PS4000A_500MV;
    else if (range == 1.0)
        return PS4000A_1V;
    else if (range == 2.0)
        return PS4000A_2V;
    else if (range == 5.0)
        return PS4000A_5V;
    else if (range == 10.0)
        return PS4000A_10V;
    else if (range == 20.0)
        return PS4000A_20V;
    else if (range == 50.0)
        return PS4000A_50V;
    else if (range == 100.0)
        return PS4000A_100V;
    else if (range == 200.0)
        return PS4000A_200V;
    else {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": Range value not supported: " << range;
        throw std::runtime_error(message.str());
    }
}

void validate_desired_actual_frequency_ps4000(double desired_freq, double actual_freq)
{
    // In order to prevent exceptions/exit due to rounding errors, we dont directly
    // compare actual_freq to desired_freq, but instead allow a difference up to 0.001%
    double max_diff_percentage = 0.001;
    double diff_percent = (actual_freq - desired_freq) * 100 / desired_freq;
    if (abs(diff_percent) > max_diff_percentage) {
        std::ostringstream message;
        message << "Critical Error in " << __FILE__ << ":" << __LINE__
                << ": Desired and actual frequency do not match. desired: "
                << desired_freq << " actual: " << actual_freq << std::endl;
        // GR_LOG_ERROR(d_logger, message);
        throw std::runtime_error(message.str());
    }
}

/*!
 * Note this function has to be called after the call to the ps3000aSetChannel function,
 * that is just befor the arm!!!
 */
uint32_t
picoscope_4000a_source_impl::convert_frequency_to_ps4000a_timebase(double desired_freq,
                                                                   double& actual_freq)
{
    // It is assumed that the timebase is calculated like this:
    // (timebase–2) / 125,000,000
    // e.g. timeebase == 3 --> 8ns sample interval
    //
    // Note, for some devices, the above formula might be wrong! To overcome this
    // limitation we use the ps3000aGetTimebase2 function to find the closest possible
    // timebase. The below timebase estimate is therefore used as a fallback only.
    auto time_interval_ns = 1000000000.0 / desired_freq;
    uint32_t timebase_estimate = (static_cast<uint32_t>(time_interval_ns) / 8) + 2;

    // In order to cover for all possible 30000 series devices, we use ps3000aGetTimebase2
    // function to get step size in ns between timebase 3 and 4. Based on that the actual
    // timebase is calculated.
    int32_t dummy;
    std::array<float, 2> time_interval_ns_34;

    for (auto i = 0; i < 2; i++) {
        auto status = ps4000aGetTimebase2(
            d_handle, 3 + i, 1024, &time_interval_ns_34[i], &dummy, 0);
        if (status != PICO_OK) {
            GR_LOG_NOTICE(d_logger,
                          "timebase cannot be obtained: " +
                              ps4000a_get_error_message(status));
            GR_LOG_NOTICE(d_logger, "    estimated timebase will be used...");

            float time_interval_ns;
            status = ps4000aGetTimebase2(
                d_handle, timebase_estimate, 1024, &time_interval_ns, &dummy, 0);
            if (status != PICO_OK) {
                std::ostringstream message;
                message << "Exception in " << __FILE__ << ":" << __LINE__
                        << ": local time " << timebase_estimate
                        << " Error: " << ps4000a_get_error_message(status);
                throw std::runtime_error(message.str());
            }

            actual_freq = 1000000000.0 / time_interval_ns;
            validate_desired_actual_frequency_ps4000(desired_freq, actual_freq);
            return timebase_estimate;
        }
    }

    // Calculate steps between timebase 3 and 4 and correct start_timebase estimate based
    // on that
    auto step = time_interval_ns_34[1] - time_interval_ns_34[0];
    timebase_estimate =
        static_cast<uint32_t>((time_interval_ns - time_interval_ns_34[0]) / step) + 3;

    // The below code iterates trought the neighbouring timebases in order to find the
    // best match. In principle we could check only timebases on the left and right but
    // since first three timebases are in most cases special we make search space a bit
    // bigger.
    const int search_space = 8;
    std::array<float, search_space> timebases;
    std::array<float, search_space> error_estimates;

    uint32_t start_timebase = timebase_estimate > (search_space / 2)
                                  ? timebase_estimate - (search_space / 2)
                                  : 0;

    for (auto i = 0; i < search_space; i++) {

        float obtained_time_interval_ns;
        auto status = ps4000aGetTimebase2(
            d_handle, start_timebase + i, 1024, &obtained_time_interval_ns, &dummy, 0);
        if (status != PICO_OK) {
            // this timebase can't be used, lets set error estimate to something big
            timebases[i] = -1;
            error_estimates[i] = 10000000000.0;
        } else {
            timebases[i] = obtained_time_interval_ns;
            error_estimates[i] = fabs(time_interval_ns - obtained_time_interval_ns);
        }
    }

    auto it = std::min_element(&error_estimates[0],
                               &error_estimates[0] + error_estimates.size());
    auto distance = std::distance(&error_estimates[0], it);

    assert(distance < search_space);

    // update actual update rate and return timebase number
    actual_freq = 1000000000.0 / double(timebases[distance]);
    validate_desired_actual_frequency_ps4000(desired_freq, actual_freq);
    return start_timebase + distance;
}

struct picoscope_4000a_source_impl::ps4000a_unit_interval_t
picoscope_4000a_source_impl::convert_frequency_to_ps4000a_time_units_and_interval(
    double desired_freq, double& actual_freq)
{
    ps4000a_unit_interval_t unint;
    auto interval = 1.0 / desired_freq;

    if (interval < 0.000001) {
        unint.unit = PS4000A_PS;
        unint.interval = static_cast<uint32_t>(1000000000000.0 / desired_freq);
        actual_freq = 1000000000000.0 / static_cast<double>(unint.interval);
    } else if (interval < 0.001) {
        unint.unit = PS4000A_NS;
        unint.interval = static_cast<uint32_t>(1000000000.0 / desired_freq);
        actual_freq = 1000000000.0 / static_cast<double>(unint.interval);
    } else if (interval < 0.1) {
        unint.unit = PS4000A_US;
        unint.interval = static_cast<uint32_t>(1000000.0 / desired_freq);
        actual_freq = 1000000.0 / static_cast<double>(unint.interval);
    } else {
        unint.unit = PS4000A_MS;
        unint.interval = static_cast<uint32_t>(1000.0 / desired_freq);
        actual_freq = 1000.0 / static_cast<double>(unint.interval);
    }
    validate_desired_actual_frequency_ps4000(desired_freq, actual_freq);
    return unint;
}

static PS4000A_RATIO_MODE convert_to_ps4000a_ratio_mode(downsampling_mode_t mode)
{
    switch (mode) {
    case downsampling_mode_t::DOWNSAMPLING_MODE_MIN_MAX_AGG:
        return PS4000A_RATIO_MODE_AGGREGATE;
    case downsampling_mode_t::DOWNSAMPLING_MODE_DECIMATE:
        return PS4000A_RATIO_MODE_DECIMATE;
    case downsampling_mode_t::DOWNSAMPLING_MODE_AVERAGE:
        return PS4000A_RATIO_MODE_AVERAGE;
    case downsampling_mode_t::DOWNSAMPLING_MODE_NONE:
    default:
        return PS4000A_RATIO_MODE_NONE;
    }
}

PS4000A_THRESHOLD_DIRECTION
convert_to_ps4000a_threshold_direction(trigger_direction_t direction)
{
    switch (direction) {
    case trigger_direction_t::TRIGGER_DIRECTION_RISING:
        return PS4000A_RISING;
    case trigger_direction_t::TRIGGER_DIRECTION_FALLING:
        return PS4000A_FALLING;
    case trigger_direction_t::TRIGGER_DIRECTION_LOW:
        return PS4000A_BELOW;
    case trigger_direction_t::TRIGGER_DIRECTION_HIGH:
        return PS4000A_ABOVE;
    default:
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": unsupported trigger direction:" << direction;
        throw std::runtime_error(message.str());
    }
};

int16_t convert_voltage_to_ps4000a_raw_logic_value(double value)
{
    double max_logical_voltage = 5.0;

    if (value > max_logical_voltage) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": max logical level is: " << max_logical_voltage;
        throw std::invalid_argument(message.str());
    }
    // Note max channel value not provided with PicoScope API, we use ext max value
    return (int16_t)((value / max_logical_voltage) * (double)PS4000A_EXT_MAX_VALUE);
}

PS4000A_CHANNEL
convert_to_ps4000a_channel(const std::string& source)
{
    if (source == "A") {
        return PS4000A_CHANNEL_A;
    } else if (source == "B") {
        return PS4000A_CHANNEL_B;
    } else if (source == "C") {
        return PS4000A_CHANNEL_C;
    } else if (source == "D") {
        return PS4000A_CHANNEL_D;
    } else if (source == "E") {
        return PS4000A_CHANNEL_E;
    } else if (source == "F") {
        return PS4000A_CHANNEL_F;
    } else if (source == "G") {
        return PS4000A_CHANNEL_G;
    } else if (source == "H") {
        return PS4000A_CHANNEL_H;
    } else if (source == "EXTERNAL") {
        return PS4000A_EXTERNAL;
    } else {
        // return invalid value
        return PS4000A_MAX_TRIGGER_SOURCES;
    }
}

/*
 * The private constructor
 */
picoscope_4000a_source_impl::picoscope_4000a_source_impl(std::string serial_number,
                                                         bool auto_arm)
    : gr::sync_block("picoscope_4000a_source",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(
                         16 /* min outputs */, 16 /*max outputs */, sizeof(float))),
      picoscope_base(serial_number, PS4000A_MAX_CHANNELS, 0, auto_arm, 255, 0.01),
      d_handle(-1),
      d_overflow(0)
{
    get_aichan_ranges().push_back(range_t(0.01));
    get_aichan_ranges().push_back(range_t(0.02));
    get_aichan_ranges().push_back(range_t(0.05));
    get_aichan_ranges().push_back(range_t(0.1));
    get_aichan_ranges().push_back(range_t(0.2));
    get_aichan_ranges().push_back(range_t(0.5));
    get_aichan_ranges().push_back(range_t(1));
    get_aichan_ranges().push_back(range_t(2));
    get_aichan_ranges().push_back(range_t(5));
    get_aichan_ranges().push_back(range_t(10));
    get_aichan_ranges().push_back(range_t(20));
    get_aichan_ranges().push_back(range_t(50));
    get_aichan_ranges().push_back(range_t(100));
    get_aichan_ranges().push_back(range_t(200));
}

/*
 * Our virtual destructor.
 */
picoscope_4000a_source_impl::~picoscope_4000a_source_impl() { driver_close(); }

/**********************************************************************
 * Driver implementation
 *********************************************************************/

std::string picoscope_4000a_source_impl::get_unit_info_topic(PICO_INFO info)
{
    char line[40];
    int16_t required_size;

    auto status = ps4000aGetUnitInfo(
        d_handle, reinterpret_cast<int8_t*>(line), sizeof(line), &required_size, info);
    if (status == PICO_OK) {
        return std::string(line, required_size);
    } else {
        return "NA";
    }
}

std::string picoscope_4000a_source_impl::get_driver_version()
{
    const std::string prefix = "PS4000A Linux Driver, ";
    auto version = get_unit_info_topic(PICO_DRIVER_VERSION);

    auto i = version.find(prefix);
    if (i != std::string::npos)
        version.erase(i, prefix.length());

    return version;
}

std::string picoscope_4000a_source_impl::get_hardware_version()
{
    if (!d_initialized)
        return "NA";
    return get_unit_info_topic(PICO_HARDWARE_VERSION);
}

std::error_code picoscope_4000a_source_impl::driver_initialize()
{
    PICO_STATUS status;

    // Required to force sequence execution of open unit calls...
    boost::mutex::scoped_lock init_guard(g_init_mutex);

    // take any if serial number is not provided (usefull for testing purposes)
    if (d_serial_number.empty()) {
        status = ps4000aOpenUnit(&(d_handle), NULL);
    } else {
        status = ps4000aOpenUnit(&(d_handle), (int8_t*)d_serial_number.c_str());
    }

    // ignore ext. power not connected error/warning
    if (status == PICO_POWER_SUPPLY_NOT_CONNECTED ||
        status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT) {
        status = ps4000aChangePowerSource(d_handle, status);
        if (status == PICO_POWER_SUPPLY_NOT_CONNECTED ||
            status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT) {
            status = ps4000aChangePowerSource(d_handle, status);
        }
    }

    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger, "open unit failed: " + ps4000a_get_error_message(status));
        return make_pico_4000a_error_code(status);
    }

    // maximum value is used for conversion to volts
    status = ps4000aMaximumValue(d_handle, &d_max_value);
    if (status != PICO_OK) {
        ps4000aCloseUnit(d_handle);
        GR_LOG_ERROR(d_logger,
                     "ps4000aMaximumValue: " + ps4000a_get_error_message(status));
        return make_pico_4000a_error_code(status);
    }

    return std::error_code{};
}

std::error_code picoscope_4000a_source_impl::driver_configure()
{
    assert(d_ai_channels <= PS4000A_MAX_CHANNELS);

    int32_t max_samples;
    PICO_STATUS status = ps4000aMemorySegments(d_handle, d_nr_captures, &max_samples);
    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger,
                     "ps4000aMemorySegments: " + ps4000a_get_error_message(status));
        return make_pico_4000a_error_code(status);
    }

    if (d_acquisition_mode == acquisition_mode_t::RAPID_BLOCK) {
        status = ps4000aSetNoOfCaptures(d_handle, d_nr_captures);
        if (status != PICO_OK) {
            GR_LOG_ERROR(d_logger,
                         "ps4000aSetNoOfCaptures: " + ps4000a_get_error_message(status));
            return make_pico_4000a_error_code(status);
        }
    }

    // configure analog channels
    for (auto i = 0; i < d_ai_channels; i++) {
        auto enabled = d_channel_settings[i].enabled;
        auto coupling = convert_to_ps4000a_coupling(d_channel_settings[i].coupling);
        auto range = convert_to_ps4000a_range(d_channel_settings[i].range);
        auto offset = d_channel_settings[i].offset;

        status = ps4000aSetChannel(d_handle,
                                   static_cast<PS4000A_CHANNEL>(i),
                                   enabled,
                                   coupling,
                                   static_cast<PICO_CONNECT_PROBE_RANGE>(range),
                                   offset);
        if (status != PICO_OK) {
            GR_LOG_ERROR(d_logger,
                         "ps3000aSetChannel (chan " + std::to_string(i) +
                             "): " + ps4000a_get_error_message(status));
            return make_pico_4000a_error_code(status);
        }
    }

    // apply trigger configuration
    if (d_trigger_settings.is_analog() &&
        d_acquisition_mode == acquisition_mode_t::RAPID_BLOCK) {
        status = ps4000aSetSimpleTrigger(
            d_handle,
            true, // enable
            convert_to_ps4000a_channel(d_trigger_settings.source),
            convert_voltage_to_ps4000a_raw_logic_value(d_trigger_settings.threshold),
            convert_to_ps4000a_threshold_direction(d_trigger_settings.direction),
            0,   // delay
            -1); // auto trigger
        if (status != PICO_OK) {
            GR_LOG_ERROR(d_logger,
                         "ps4000aSetSimpleTrigger: " + ps4000a_get_error_message(status));
            return make_pico_4000a_error_code(status);
        }
    } else {
        // disable triggers...
        for (int i = 0; i < PS4000A_MAX_CHANNELS; i++) {
            PS4000A_CONDITION cond;
            cond.source = static_cast<PS4000A_CHANNEL>(i);
            cond.condition = PS4000A_CONDITION_DONT_CARE;
            status =
                ps4000aSetTriggerChannelConditions(d_handle, &cond, 1, PS4000A_CLEAR);
            if (status != PICO_OK) {
                GR_LOG_ERROR(d_logger,
                             "ps4000aSetTriggerChannelConditionsV2: " +
                                 ps4000a_get_error_message(status));
                return make_pico_4000a_error_code(status);
            }
        }
    }

    // In order to validate desired frequency before startup
    double actual_freq;
    convert_frequency_to_ps4000a_timebase(d_samp_rate, actual_freq);

    return std::error_code{};
}

void rapid_block_callback_redirector_4000a(int16_t handle, PICO_STATUS status, void* vobj)
{
    static_cast<picoscope_4000a_source_impl*>(vobj)->rapid_block_callback(handle, status);
}

void picoscope_4000a_source_impl::rapid_block_callback(int16_t handle, PICO_STATUS status)
{
    auto errc = make_pico_4000a_error_code(status);
    notify_data_ready(errc);
}

std::error_code picoscope_4000a_source_impl::driver_arm()
{

    if (d_acquisition_mode == acquisition_mode_t::RAPID_BLOCK) {
        uint32_t timebase =
            convert_frequency_to_ps4000a_timebase(d_samp_rate, d_actual_samp_rate);

        auto status =
            ps4000aRunBlock(d_handle,
                            d_pre_samples,  // pre-triggersamples
                            d_post_samples, // post-trigger samples
                            timebase,       // timebase
                            NULL,           // time indispossed
                            0,              // segment index
                            (ps4000aBlockReady)rapid_block_callback_redirector_4000a,
                            this);

        if (status != PICO_OK) {

            GR_LOG_ERROR(d_logger,
                         "ps4000aRunBlock: " + ps4000a_get_error_message(status));
            return make_pico_4000a_error_code(status);
        }
    } else {
        set_buffers(d_driver_buffer_size, 0);

        ps4000a_unit_interval_t unit_int =
            convert_frequency_to_ps4000a_time_units_and_interval(d_samp_rate,
                                                                 d_actual_samp_rate);

        auto status =
            ps4000aRunStreaming(d_handle,
                                &(unit_int.interval), // sample interval
                                unit_int.unit,        // time unit of sample interval
                                0,                    // pre-triggersamples (unused)
                                d_driver_buffer_size,
                                false,
                                d_downsampling_factor,
                                convert_to_ps4000a_ratio_mode(d_downsampling_mode),
                                d_driver_buffer_size);


        // if(status != PICO_OK) {
        //   GR_LOG_ERROR(d_logger, "ps4000aRunStreaming: " +
        //   ps4000a_get_error_message(status)); return
        //   make_pico_4000a_error_code(status);
        // }
    }

    return std::error_code{};
}

std::error_code picoscope_4000a_source_impl::driver_disarm()
{
    auto status = ps4000aStop(d_handle);
    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger, "ps4000aStop: " + ps4000a_get_error_message(status));
    }

    return make_pico_4000a_error_code(status);
}

std::error_code picoscope_4000a_source_impl::driver_close()
{
    if (d_handle == -1) {
        return std::error_code{};
    }

    auto status = ps4000aCloseUnit(d_handle);
    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger, "ps4000aCloseUnit: " + ps4000a_get_error_message(status));
    }

    d_handle = -1;
    return make_pico_4000a_error_code(status);
}

std::error_code picoscope_4000a_source_impl::set_buffers(size_t samples,
                                                         uint32_t block_number)
{
    PICO_STATUS status;

    for (auto aichan = 0; aichan < d_ai_channels; aichan++) {
        if (!d_channel_settings[aichan].enabled)
            continue;

        if (d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_MIN_MAX_AGG) {
            d_buffers[aichan].reserve(samples);
            d_buffers_min[aichan].reserve(samples);

            status =
                ps4000aSetDataBuffers(d_handle,
                                      static_cast<PS4000A_CHANNEL>(aichan),
                                      &d_buffers[aichan][0],
                                      &d_buffers_min[aichan][0],
                                      samples,
                                      block_number,
                                      convert_to_ps4000a_ratio_mode(d_downsampling_mode));
        } else {
            d_buffers[aichan].reserve(samples);

            status =
                ps4000aSetDataBuffer(d_handle,
                                     static_cast<PS4000A_CHANNEL>(aichan),
                                     &d_buffers[aichan][0],
                                     samples,
                                     block_number,
                                     convert_to_ps4000a_ratio_mode(d_downsampling_mode));
        }

        if (status != PICO_OK) {
            GR_LOG_ERROR(d_logger,
                         "ps4000aSetDataBuffer (chan " + std::to_string(aichan) +
                             "): " + ps4000a_get_error_message(status));
            return make_pico_4000a_error_code(status);
        }
    }

    return std::error_code{};
}

std::error_code picoscope_4000a_source_impl::driver_prefetch_block(size_t samples,
                                                                   size_t block_number)
{
    auto erc = set_buffers(samples, block_number);
    if (erc) {
        return erc;
    }

    uint32_t nr_samples = samples;
    auto status = ps4000aGetValues(d_handle,
                                   0, // offset
                                   &nr_samples,
                                   d_downsampling_factor,
                                   convert_to_ps4000a_ratio_mode(d_downsampling_mode),
                                   block_number,
                                   &d_overflow);
    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger, "ps4000aGetValues: " + ps4000a_get_error_message(status));
    }

    return make_pico_4000a_error_code(status);
}


std::error_code
picoscope_4000a_source_impl::driver_get_rapid_block_data(size_t offset,
                                                         size_t length,
                                                         size_t waveform,
                                                         gr_vector_void_star& arrays,
                                                         std::vector<uint32_t>& status)
{
    int vec_index = 0;

    for (auto chan_idx = 0; chan_idx < d_ai_channels; chan_idx++, vec_index += 2) {
        if (!d_channel_settings[chan_idx].enabled) {
            continue;
        }

        if (d_overflow & (1 << chan_idx)) {
            status[chan_idx] = channel_status_t::CHANNEL_STATUS_OVERFLOW;
        } else {
            status[chan_idx] = 0;
        }

        float voltage_multiplier =
            d_channel_settings[chan_idx].range / (float)d_max_value;

        float* out = (float*)arrays.at(vec_index);
        float* err_out = (float*)arrays.at(vec_index + 1);
        int16_t* in = &d_buffers[chan_idx][0] + offset;

        if (d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_NONE ||
            d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_DECIMATE) {
            for (size_t i = 0; i < length; i++) {
                out[i] = (voltage_multiplier * (float)in[i]);
            }
            // According to specs
            auto error_estimate = d_channel_settings[chan_idx].range * 0.01;
            for (size_t i = 0; i < length; i++) {
                err_out[i] = error_estimate;
            }
        } else if (d_downsampling_mode ==
                   downsampling_mode_t::DOWNSAMPLING_MODE_MIN_MAX_AGG) {
            // this mode is different because samples are in two distinct buffers
            int16_t* in_min = &d_buffers_min[chan_idx][0] + offset;

            for (size_t i = 0; i < length; i++) {
                auto max = (voltage_multiplier * (float)in[i]);
                auto min = (voltage_multiplier * (float)in_min[i]);

                out[i] = (max + min) / 2.0;
                err_out[i] = (max - min) / 4.0;
            }
        } else if (d_downsampling_mode ==
                   downsampling_mode_t::DOWNSAMPLING_MODE_AVERAGE) {
            for (size_t i = 0; i < length; i++) {
                out[i] = (voltage_multiplier * (float)in[i]);
            }
            // According to specs
            float error_estimate_single = d_channel_settings[chan_idx].range * 0.01;
            float error_estimate =
                error_estimate_single / std::sqrt((float)d_downsampling_factor);
            for (size_t i = 0; i < length; i++) {
                err_out[i] = error_estimate;
            }
        } else {
            assert(false);
        }
    }

    return std::error_code{};
}

std::error_code picoscope_4000a_source_impl::driver_poll()
{
    auto status =
        ps4000aGetStreamingLatestValues(d_handle,
                                        (ps4000aStreamingReady)invoke_streaming_callback,
                                        &d_streaming_callback);
    if (status == PICO_BUSY || status == PICO_DRIVER_FUNCTION) {
        return std::error_code{};
    }
    if (status != PICO_OK) {
        GR_LOG_ERROR(d_logger,
                     "ps4000aRunStreaming: " + ps4000a_get_error_message(status));
        return make_pico_4000a_error_code(status);
    }
    return make_pico_4000a_error_code(status);
}


// quick hack to make these functions visible for gnuradio/pybind11
void picoscope_4000a_source_impl::set_trigger_once(bool trigger_once)
{
    digitizer_source::set_trigger_once(trigger_once);
}

void picoscope_4000a_source_impl::set_downsampling(downsampling_mode_t mode,
                                                   int downsample_factor)
{
    digitizer_source::set_downsampling(mode, downsample_factor);
}


void picoscope_4000a_source_impl::set_samp_rate(double rate)
{
    digitizer_source::set_samp_rate(rate);
}

void picoscope_4000a_source_impl::set_aichan(const std::string& id,
                                             bool enabled,
                                             double range,
                                             coupling_t coupling,
                                             double range_offset)
{
    digitizer_source::set_aichan(id, enabled, range, coupling, range_offset);
}

void picoscope_4000a_source_impl::set_aichan_trigger(const std::string& id,
                                                     trigger_direction_t direction,
                                                     double threshold)
{
    digitizer_source::set_aichan_trigger(id, direction, threshold);
}
void picoscope_4000a_source_impl::set_samples(int pre_samples, int post_samples)
{
    digitizer_source::set_samples(pre_samples, post_samples);
}
void picoscope_4000a_source_impl::set_rapid_block(int nr_waveforms)
{
    digitizer_source::set_rapid_block(nr_waveforms);
}

void picoscope_4000a_source_impl::set_nr_buffers(int nr_buffers)
{
    digitizer_source::set_nr_buffers(nr_buffers);
}

void picoscope_4000a_source_impl::set_streaming(double poll_rate)
{
    digitizer_source::set_streaming(poll_rate);
}


void picoscope_4000a_source_impl::set_driver_buffer_size(int driver_buffer_size)
{
    digitizer_source::set_driver_buffer_size(driver_buffer_size);
}

void picoscope_4000a_source_impl::set_buffer_size(int buffer_size)
{
    digitizer_source::set_buffer_size(buffer_size);
}
// TODO: verify
// ugly workaround to avoid gnuradio's confusion
void picoscope_4000a_source_impl::set_aichan_a(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("A", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_b(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("B", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_c(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("C", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_d(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("D", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_e(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("E", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_f(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("F", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_g(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("G", enabled, range, coupling, range_offset);
}
void picoscope_4000a_source_impl::set_aichan_h(bool enabled,
                                               double range,
                                               coupling_t coupling,
                                               double range_offset)
{
    digitizer_source::set_aichan("H", enabled, range, coupling, range_offset);
}
// int
// digitizer_source::work(int noutput_items,
//     gr_vector_const_void_star &input_items,
//     gr_vector_void_star &output_items)
// {
//   int retval = -1;

//   if(d_acquisition_mode == acquisition_mode_t::STREAMING) {
//     retval = work_stream(noutput_items, output_items);
//   }
//   else if(d_acquisition_mode == acquisition_mode_t::RAPID_BLOCK) {
//     retval = work_rapid_block(noutput_items, output_items);
//   }

//   if ((retval > 0) && !d_timebase_published) {
//     auto timebase_tag = make_timebase_info_tag(get_timebase_with_downsampling());
//     timebase_tag.offset = nitems_written(0);

//     for (gr_vector_void_star::size_type i = 0; i < output_items.size(); i++) {
//       add_item_tag(i, timebase_tag);
//     }

//     d_timebase_published = true;
//   }

//   return retval;
// }

} // namespace pulsed_power
} /* namespace gr */
