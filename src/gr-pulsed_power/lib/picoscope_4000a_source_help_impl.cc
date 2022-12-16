#include "picoscope_4000a_source_impl.h"

/**********************************************************************
 * Converters - helper functions - PRIVATE
 *********************************************************************/

namespace gr {
namespace pulsed_power {

/**
 * @brief
 *
 * @param coupling
 * @return PS4000A_COUPLING
 */
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

/**
 * @brief
 *
 * @param range
 * @return PS4000A_RANGE
 */
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

/**
 * @brief
 *
 * @param desired_freq
 * @param actual_freq
 */
void picoscope_4000a_source_impl::validate_desired_actual_frequency_ps4000(
    double desired_freq, double actual_freq)
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
        GR_LOG_ERROR(d_logger, message.str());
        throw std::runtime_error(message.str());
    }
}

/**
 * @brief
 * @note Note this function has to be called after the call to the ps3000aSetChannel
 * function, that is just before the arm!!!
 * @param desired_freq
 * @param actual_freq
 * @return uint32_t
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

/**
 * @brief
 *
 * @param desired_freq
 * @param actual_freq
 * @return ps4000a_unit_interval_t
 */
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

/**
 * @brief
 *
 * @param mode
 * @return PS4000A_RATIO_MODE
 */
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

/**
 * @brief
 *
 * @param direction
 * @return PS4000A_THRESHOLD_DIRECTION
 */
PS4000A_THRESHOLD_DIRECTION
picoscope_4000a_source_impl::convert_to_ps4000a_threshold_direction(
    trigger_direction_t direction)
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

/**
 * @brief
 *
 * @param value
 * @return int16_t
 */
int16_t
picoscope_4000a_source_impl::convert_voltage_to_ps4000a_raw_logic_value(double value)
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

/**
 * @brief
 *
 * @param source
 * @return PS4000A_CHANNEL
 */
PS4000A_CHANNEL
picoscope_4000a_source_impl::convert_to_ps4000a_channel(const std::string& source)
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
} // namespace pulsed_power
} /* namespace gr */
