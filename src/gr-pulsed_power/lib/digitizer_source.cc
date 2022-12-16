/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/pulsed_power/digitizer_source.h>

namespace gr {
namespace pulsed_power {


/**********************************************************************
 * Error codes
 *********************************************************************/

struct digitizer_block_err_category : std::error_category {
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const char* digitizer_block_err_category::name() const noexcept
{
    return "digitizer_block";
}

std::string digitizer_block_err_category::message(int ev) const
{
    switch (static_cast<digitizer_block_errc>(ev)) {
    case digitizer_block_errc::Interrupted:
        return "Wit interrupted";

    default:
        return "(unrecognized error)";
    }
}

const digitizer_block_err_category __digitizer_block_category{};

std::error_code make_error_code(digitizer_block_errc e)
{
    return { static_cast<int>(e), __digitizer_block_category };
}


/**********************************************************************
 * Structors
 *********************************************************************/

// Relevant for debugging. If gnuradio calls this block not often enough, we will get
// "WARN: XX digitizer data buffers lost" The rate in which this block is called is given
// by the number of free slots on its output buffer. So we choose a big value via
// set_min_output_buffer
uint64_t last_call_utc = 0;

static const int AVERAGE_HISTORY_LENGTH = 100000;

digitizer_source::digitizer_source(int ai_channels, int di_ports, bool auto_arm)
    : d_samp_rate(10000),
      d_actual_samp_rate(d_samp_rate),
      d_time_per_sample_ns(1000000000. / d_samp_rate),
      d_pre_samples(1000),
      d_post_samples(9000),
      d_nr_captures(1),
      d_buffer_size(8192),
      d_nr_buffers(100),
      d_driver_buffer_size(100000),
      d_acquisition_mode(acquisition_mode_t::STREAMING),
      d_poll_rate(0.001),
      d_downsampling_mode(downsampling_mode_t::DOWNSAMPLING_MODE_NONE),
      d_downsampling_factor(1),
      d_ai_channels(ai_channels),
      d_ports(di_ports),
      d_channel_settings(),
      d_port_settings(),
      d_trigger_settings(),
      d_status(ai_channels),
      d_app_buffer(),
      d_was_last_callback_timestamp_taken(false),
      d_estimated_sample_rate(AVERAGE_HISTORY_LENGTH),
      d_initialized(false),
      d_closed(false),
      d_armed(false),
      d_auto_arm(auto_arm),
      d_trigger_once(false),
      d_was_triggered_once(false),
      d_timebase_published(false),
      ai_buffers(ai_channels),
      ai_error_buffers(ai_channels),
      port_buffers(di_ports),
      d_data_rdy(false),
      d_trigger_state(0),
      d_read_idx(0),
      d_buffer_samples(0),
      d_errors(128),
      d_poller_state(poller_state_t::IDLE)
{
    d_ai_buffers = std::vector<std::vector<float>>(d_ai_channels);
    d_ai_error_buffers = std::vector<std::vector<float>>(d_ai_channels);

    if (di_ports) {
        d_port_buffers = std::vector<std::vector<uint8_t>>(di_ports);
    }

    assert(d_ai_channels < MAX_SUPPORTED_AI_CHANNELS);
    assert(d_ports < MAX_SUPPORTED_PORTS);
}

digitizer_source::~digitizer_source() {}

/**********************************************************************
 * Helpers
 **********************************************************************/

uint32_t digitizer_source::get_pre_trigger_samples_with_downsampling() const
{
    if (d_downsampling_mode != downsampling_mode_t::DOWNSAMPLING_MODE_NONE)
        return d_pre_samples / d_downsampling_factor;
    return d_pre_samples;
}

uint32_t digitizer_source::get_post_trigger_samples_with_downsampling() const
{
    if (d_downsampling_mode != downsampling_mode_t::DOWNSAMPLING_MODE_NONE)
        return d_post_samples / d_downsampling_factor;
    return d_post_samples;
}

uint32_t digitizer_source::get_block_size() const
{
    return d_post_samples + d_pre_samples;
}

uint32_t digitizer_source::get_block_size_with_downsampling() const
{
    return get_pre_trigger_samples_with_downsampling() +
           get_post_trigger_samples_with_downsampling();
}

double digitizer_source::get_timebase_with_downsampling() const
{
    if (d_downsampling_mode == downsampling_mode_t::DOWNSAMPLING_MODE_NONE) {
        return 1.0 / d_actual_samp_rate;
    } else {
        return d_downsampling_factor / d_actual_samp_rate;
    }
}

void digitizer_source::add_error_code(std::error_code ec) { d_errors.push(ec); }

std::vector<int> digitizer_source::find_analog_triggers(float const* const samples,
                                                        int nsamples)
{
    std::vector<int> trigger_offsets; // relative offset of detected triggers

    assert(nsamples >= 0);

    if (!d_trigger_settings.is_enabled() || nsamples == 0) {
        return trigger_offsets;
    }

    assert(d_trigger_settings.is_analog());

    auto aichan = convert_to_aichan_idx(d_trigger_settings.source);

    if (d_trigger_settings.direction == TRIGGER_DIRECTION_RISING ||
        d_trigger_settings.direction == TRIGGER_DIRECTION_HIGH) {

        float band = d_channel_settings[aichan].range / 100.0;
        float lo = static_cast<float>(d_trigger_settings.threshold - band);

        for (auto i = 0; i < nsamples; i++) {
            if (!d_trigger_state && samples[i] >= d_trigger_settings.threshold) {
                d_trigger_state = 1;
                trigger_offsets.push_back(i);
            } else if (d_trigger_state && samples[i] <= lo) {
                d_trigger_state = 0;
            }
        }
    } else if (d_trigger_settings.direction == TRIGGER_DIRECTION_FALLING ||
               d_trigger_settings.direction == TRIGGER_DIRECTION_LOW) {

        float band = d_channel_settings[aichan].range / 100.0;
        float hi = static_cast<float>(d_trigger_settings.threshold + band);

        for (auto i = 0; i < nsamples; i++) {
            if (d_trigger_state && samples[i] <= d_trigger_settings.threshold) {
                d_trigger_state = 0;
                trigger_offsets.push_back(i);
            } else if (!d_trigger_state && samples[i] >= hi) {
                d_trigger_state = 1;
            }
        }
    }

    return trigger_offsets;
}

std::vector<int> digitizer_source::find_digital_triggers(uint8_t const* const samples,
                                                         int nsamples,
                                                         uint8_t mask)
{
    std::vector<int> trigger_offsets;

    if (d_trigger_settings.direction == TRIGGER_DIRECTION_RISING ||
        d_trigger_settings.direction == TRIGGER_DIRECTION_HIGH) {

        for (auto i = 0; i < nsamples; i++) {
            if (!d_trigger_state && (samples[i] & mask)) {
                d_trigger_state = 1;
                trigger_offsets.push_back(i);
            } else if (d_trigger_state && !(samples[i] & mask)) {
                d_trigger_state = 0;
            }
        }
    } else if (d_trigger_settings.direction == TRIGGER_DIRECTION_FALLING ||
               d_trigger_settings.direction == TRIGGER_DIRECTION_LOW) {

        for (auto i = 0; i < nsamples; i++) {
            if (d_trigger_state && !(samples[i] & mask)) {
                d_trigger_state = 0;
                trigger_offsets.push_back(i);
            } else if (!d_trigger_state && (samples[i] & mask)) {
                d_trigger_state = 1;
            }
        }
    }

    return trigger_offsets;
}

/**********************************************************************
 * Public API
 **********************************************************************/

acquisition_mode_t digitizer_source::get_acquisition_mode() { return d_acquisition_mode; }

void digitizer_source::set_samples(int pre_samples, int post_samples)
{
    if (post_samples < 1) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": post-trigger samples can't be less than one";
        throw std::invalid_argument(message.str());
    }

    if (pre_samples < 0) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": pre-trigger samples can't be less than zero";
        throw std::invalid_argument(message.str());
    }

    d_post_samples = static_cast<uint32_t>(post_samples);
    d_pre_samples = static_cast<uint32_t>(pre_samples);
    d_buffer_size = d_post_samples + d_pre_samples;
}

void digitizer_source::set_samp_rate(double rate)
{
    if (rate <= 0.0) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": sample rate has to be greater than zero";
        throw std::invalid_argument(message.str());
    }
    d_samp_rate = rate;
    d_actual_samp_rate = rate;
    d_time_per_sample_ns = 1000000000. / d_actual_samp_rate;
}

double digitizer_source::get_samp_rate() { return d_actual_samp_rate; }

void digitizer_source::set_buffer_size(int buffer_size)
{
    if (buffer_size < 0) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": buffer size can't be negative:" << buffer_size;
        throw std::invalid_argument(message.str());
    }

    d_buffer_size = static_cast<uint32_t>(buffer_size);

    set_output_multiple(buffer_size);
}

void digitizer_source::set_nr_buffers(int nr_buffers)
{
    if (nr_buffers < 1) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": number of buffers can't be a negative number:" << nr_buffers;
        throw std::invalid_argument(message.str());
    }

    d_nr_buffers = static_cast<uint32_t>(nr_buffers);
}

void digitizer_source::set_driver_buffer_size(int driver_buffer_size)
{
    if (driver_buffer_size < 1) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": driver buffer size can't be a negative number:"
                << driver_buffer_size;
        throw std::invalid_argument(message.str());
    }

    d_driver_buffer_size = static_cast<uint32_t>(driver_buffer_size);
}

void digitizer_source::set_auto_arm(bool auto_arm) { d_auto_arm = auto_arm; }

void digitizer_source::set_trigger_once(bool once) { d_trigger_once = once; }

// Poll rate is in seconds
void digitizer_source::set_streaming(double poll_rate)
{
    if (poll_rate < 0.0) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": poll rate can't be negative:" << poll_rate;
        throw std::invalid_argument(message.str());
    }

    d_acquisition_mode = acquisition_mode_t::STREAMING;
    d_poll_rate = poll_rate;

    // just in case
    d_nr_captures = 1;
}

void digitizer_source::set_rapid_block(int nr_captures)
{
    if (nr_captures < 1) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": nr waveforms should be at least one" << nr_captures;
        throw std::invalid_argument(message.str());
    }

    d_acquisition_mode = acquisition_mode_t::RAPID_BLOCK;
    d_nr_captures = static_cast<uint32_t>(nr_captures);
}

void digitizer_source::set_downsampling(downsampling_mode_t mode, int downsample_factor)
{
    if (mode == downsampling_mode_t::DOWNSAMPLING_MODE_NONE) {
        downsample_factor = 1;
    } else if (downsample_factor < 2) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": downsampling factor should be at least 2: " << downsample_factor;
        throw std::invalid_argument(message.str());
    }

    d_downsampling_mode = mode;
    d_downsampling_factor = static_cast<uint32_t>(downsample_factor);
}

int digitizer_source::convert_to_aichan_idx(const std::string& id) const
{
    if (id.length() != 1) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": aichan id should be a single character: " << id;
        throw std::invalid_argument(message.str());
    }

    int idx = std::toupper(id[0]) - 'A';
    if (idx < 0 || idx > MAX_SUPPORTED_AI_CHANNELS) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid aichan id: " << id;
        throw std::invalid_argument(message.str());
    }

    return idx;
}

void digitizer_source::set_aichan(const std::string& id,
                                  bool enabled,
                                  double range,
                                  coupling_t coupling,
                                  double range_offset)
{
    auto idx = convert_to_aichan_idx(id);
    d_channel_settings[idx].range = range;
    d_channel_settings[idx].offset = range_offset;
    d_channel_settings[idx].enabled = enabled;
    d_channel_settings[idx].coupling = coupling;
}

int digitizer_source::get_enabled_aichan_count() const
{
    auto count = 0;
    for (const auto& c : d_channel_settings) {
        count += c.enabled;
    }
    return count;
}

void digitizer_source::set_aichan_range(const std::string& id,
                                        double range,
                                        double range_offset)
{
    auto idx = convert_to_aichan_idx(id);
    d_channel_settings[idx].range = range;
    d_channel_settings[idx].offset = range_offset;
}

void digitizer_source::set_aichan_trigger(const std::string& id,
                                          trigger_direction_t direction,
                                          double threshold)
{
    // Some scopes have an dedicated AUX Trigger-Input. Skip id verification for them
    if (id != "AUX")
        convert_to_aichan_idx(id); // Just to verify id

    d_trigger_settings.source = id;
    d_trigger_settings.threshold = threshold;
    d_trigger_settings.direction = direction;
    d_trigger_settings.pin_number = 0; // not used
}

int digitizer_source::convert_to_port_idx(const std::string& id) const
{
    if (id.length() != 5) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid port id: " << id
                << ", should be of the following format 'port<d>'";
        throw std::invalid_argument(message.str());
    }

    int idx = boost::lexical_cast<int>(id[4]);
    if (idx < 0 || idx > MAX_SUPPORTED_PORTS) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid port number: " << id;
        throw std::invalid_argument(message.str());
    }

    return idx;
}

void digitizer_source::set_diport(const std::string& id,
                                  bool enabled,
                                  double thresh_voltage)
{
    auto port_number = convert_to_port_idx(id);

    d_port_settings[port_number].logic_level = thresh_voltage;
    d_port_settings[port_number].enabled = enabled;
}

int digitizer_source::get_enabled_diport_count() const
{
    auto count = 0;
    for (const auto& p : d_port_settings) {
        count += p.enabled;
    }
    return count;
}

void digitizer_source::set_di_trigger(uint32_t pin, trigger_direction_t direction)
{
    d_trigger_settings.source = TRIGGER_DIGITAL_SOURCE;
    d_trigger_settings.threshold = 0.0; // not used
    d_trigger_settings.direction = direction;
    d_trigger_settings.pin_number = pin;
}

void digitizer_source::disable_triggers()
{
    d_trigger_settings.source = TRIGGER_NONE_SOURCE;
}

void digitizer_source::initialize()
{
    if (d_initialized) {
        return;
    }

    auto ec = driver_initialize();
    if (ec) {
        add_error_code(ec);
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": initialize failed. ErrorCode: " << ec;
        throw std::runtime_error(message.str());
    }

    d_initialized = true;
}

void digitizer_source::configure()
{
    if (!d_initialized) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": initialize first!";
        throw std::runtime_error(message.str());
    }

    if (d_armed) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__ << ": disarm first!";
        throw std::runtime_error(message.str());
    }

    auto ec = driver_configure();
    if (ec) {
        add_error_code(ec);
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": configure failed. ErrorCode: " << ec;
        throw std::runtime_error(message.str());
    }
    // initialize application buffer
    d_app_buffer.initialize(get_enabled_aichan_count(),
                            get_enabled_diport_count(),
                            d_buffer_size,
                            d_nr_buffers);
}

void digitizer_source::arm()
{
    if (d_armed) {
        return;
    }

    // set estimated sample rate to expected
    float expected = static_cast<float>(get_samp_rate());
    for (auto i = 0; i < AVERAGE_HISTORY_LENGTH; i++) {
        d_estimated_sample_rate.add(expected);
    }

    // arm the driver
    auto ec = driver_arm();
    if (ec) {
        add_error_code(ec);
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": arm failed. ErrorCode: " << ec;
        throw std::runtime_error(message.str());
    }

    d_armed = true;
    d_timebase_published = false;
    d_was_last_callback_timestamp_taken = false;

    // clear error condition in the application buffer
    d_app_buffer.notify_data_ready(std::error_code{});

    // notify poll thread to start with the poll request
    if (d_acquisition_mode == acquisition_mode_t::STREAMING) {
        transit_poll_thread_to_running();
    }

    // allocate buffer pointer vectors.
    int num_enabled_ai_channels = 0;
    int num_enabled_di_ports = 0;
    for (auto i = 0; i < d_ai_channels; i++) {
        if (d_channel_settings[i].enabled) {
            num_enabled_ai_channels++;
        }
    }
    for (auto i = 0; i < d_ports; i++) {
        if (d_port_settings[i].enabled) {
            num_enabled_di_ports++;
        }
    }
    ai_buffers.resize(num_enabled_ai_channels);
    ai_error_buffers.resize(num_enabled_ai_channels);
    port_buffers.resize(num_enabled_di_ports);
}

bool digitizer_source::is_armed() { return d_armed; }

void digitizer_source::disarm()
{
    if (!d_armed) {
        return;
    }

    if (d_acquisition_mode == acquisition_mode_t::STREAMING) {
        transit_poll_thread_to_idle();
    }

    auto ec = driver_disarm();
    if (ec) {
        add_error_code(ec);
        GR_LOG_WARN(d_logger, "disarm failed: " + to_string(ec));
    }

    d_armed = false;
}

void digitizer_source::close()
{
    auto ec = driver_close();
    if (ec) {
        add_error_code(ec);
        GR_LOG_WARN(d_logger, "close failed: " + to_string(ec));
    }
    d_closed = true;
    d_initialized = false;
}

std::vector<error_info_t> digitizer_source::get_errors() { return d_errors.get(); }

std::string digitizer_source::getConfigureExceptionMessage()
{
    return d_configure_exception_message;
}

bool digitizer_source::start()
{
    try {
        initialize();
        configure();

        // Needed in case start/run is called multiple times without destructing the
        // flowgraph
        d_was_triggered_once = false;
        d_data_rdy_errc = std::error_code{};
        d_data_rdy = false;

        if (d_acquisition_mode == acquisition_mode_t::STREAMING) {
            start_poll_thread();
        }

        if (d_auto_arm && d_acquisition_mode == acquisition_mode_t::STREAMING) {
            arm();
        }
    } catch (const std::exception& ex) {
        d_configure_exception_message = ex.what();
        return false;
    } catch (...) {
        d_configure_exception_message =
            "Unknown Exception received in digitizer_source::start";
        return false;
    }

    return true;
}

bool digitizer_source::stop()
{
    if (!d_initialized) {
        return true;
    }

    if (d_armed) {
        // Interrupt waiting function (workaround). From the scheduler point of view this
        // is not needed because it makes sure that the worker thread gets interrupted
        // before the stop method is called. But we have this in place in order to allow
        // for manual intervention.
        notify_data_ready(digitizer_block_errc::Stopped);

        disarm();
    }

    if (d_acquisition_mode == acquisition_mode_t::STREAMING) {
        stop_poll_thread();
    }

    d_configure_exception_message = "";

    return true;
}

/**********************************************************************
 * Driver interface
 **********************************************************************/

void digitizer_source::notify_data_ready(std::error_code ec)
{
    if (ec) {
        add_error_code(ec);
    }

    {
        boost::mutex::scoped_lock lock(d_mutex);
        d_data_rdy = true;
        d_data_rdy_errc = ec;
    }

    d_data_rdy_cv.notify_one();
}

std::error_code digitizer_source::wait_data_ready()
{
    boost::mutex::scoped_lock lock(d_mutex);

    d_data_rdy_cv.wait(lock, [this] { return d_data_rdy; });
    return d_data_rdy_errc;
}

void digitizer_source::clear_data_ready()
{
    boost::mutex::scoped_lock lock(d_mutex);

    d_data_rdy = false;
    d_data_rdy_errc = std::error_code{};
}


/**********************************************************************
 * GR worker functions
 **********************************************************************/

int digitizer_source::work_rapid_block(int noutput_items,
                                       gr_vector_void_star& output_items)
{
    if (d_bstate.state == rapid_block_state_t::WAITING) {

        if (d_trigger_once && d_was_triggered_once) {
            return -1;
        }

        if (d_auto_arm) {
            disarm();
            while (true) {
                try {
                    arm();
                    break;
                } catch (...) {
                    return -1;
                }
            }
        }

        // Wait conditional variable, when waken clear it
        auto ec = wait_data_ready();
        clear_data_ready();

        // Stop requested
        if (ec == digitizer_block_errc::Stopped) {
            GR_LOG_INFO(d_logger, "stop requested");
            return -1;
        } else if (ec) {
            GR_LOG_ERROR(d_logger,
                         "error occurred while waiting for data: " + to_string(ec));
            return 0;
        }

        // we assume all the blocks are ready
        d_bstate.initialize(d_nr_captures);
    }

    if (d_bstate.state == rapid_block_state_t::READING_PART1) {

        // If d_trigger_once is true we will signal all done in the next iteration
        // with the block state set to WAITING
        d_was_triggered_once = true;

        auto samples_to_fetch = get_block_size();
        auto downsampled_samples = get_block_size_with_downsampling();

        // Instruct the driver to prefetch samples. Drivers might choose to ignore this
        // call
        auto ec = driver_prefetch_block(samples_to_fetch, d_bstate.waveform_idx);
        if (ec) {
            add_error_code(ec);
            return -1;
        }

        // Initiate state machine for the current waveform. Note state machine track
        // and adjust the waveform index.
        d_bstate.set_waveform_params(0, downsampled_samples);

        timespec start_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        uint64_t timestamp_now_ns_utc =
            (start_time.tv_sec * 1000000000) + (start_time.tv_nsec);
        // We are good to read first batch of samples
        noutput_items = std::min(noutput_items, d_bstate.samples_left);

        ec = driver_get_rapid_block_data(d_bstate.offset,
                                         noutput_items,
                                         d_bstate.waveform_idx,
                                         output_items,
                                         d_status);
        if (ec) {
            add_error_code(ec);
            return -1;
        }

        // Attach trigger info to value outputs and to all ports
        auto vec_idx = 0;
        uint32_t pre_trigger_samples_with_downsampling =
            get_pre_trigger_samples_with_downsampling();
        double time_per_sample_with_downsampling_ns =
            d_time_per_sample_ns * d_downsampling_factor;

        for (auto i = 0; i < d_ai_channels && vec_idx < (int)output_items.size();
             i++, vec_idx += 2) {
            if (!d_channel_settings[i].enabled) {
                continue;
            }

            auto trigger_tag = make_trigger_tag(
                d_downsampling_factor,
                timestamp_now_ns_utc + (pre_trigger_samples_with_downsampling *
                                        time_per_sample_with_downsampling_ns),
                nitems_written(0) + pre_trigger_samples_with_downsampling,
                d_status[i]);

            add_item_tag(vec_idx, trigger_tag);
        }

        auto trigger_tag = make_trigger_tag(
            d_downsampling_factor,
            timestamp_now_ns_utc + (pre_trigger_samples_with_downsampling *
                                    time_per_sample_with_downsampling_ns),
            nitems_written(0) + pre_trigger_samples_with_downsampling,
            0); // status

        // Add tags to digital port
        for (auto i = 0; i < d_ports && vec_idx < (int)output_items.size();
             i++, vec_idx++) {
            if (d_port_settings[i].enabled)
                add_item_tag(vec_idx, trigger_tag);
        }

        // update state
        d_bstate.update_state(noutput_items);

        return noutput_items;
    } else if (d_bstate.state == rapid_block_state_t::READING_THE_REST) {

        noutput_items = std::min(noutput_items, d_bstate.samples_left);

        auto ec = driver_get_rapid_block_data(d_bstate.offset,
                                              noutput_items,
                                              d_bstate.waveform_idx,
                                              output_items,
                                              d_status);
        if (ec) {
            add_error_code(ec);
            return -1;
        }

        // update state
        d_bstate.update_state(noutput_items);

        return noutput_items;
    }

    return -1;
}

void digitizer_source::poll_work_function()
{
    std::unique_lock<std::mutex> lock(d_poller_mutex, std::defer_lock);
    auto poll_rate = std::chrono::microseconds((long)(d_poll_rate * 1000000));

    gr::thread::set_thread_name(pthread_self(), "poller");

    // relax cpu with less lock calls.
    unsigned int check_every_n_times = 10;
    unsigned int poller_state_check_counter = check_every_n_times;
    poller_state_t state = poller_state_t::IDLE;

    while (true) {

        poller_state_check_counter++;
        if (poller_state_check_counter >= check_every_n_times) {
            lock.lock();
            state = d_poller_state;
            lock.unlock();
            poller_state_check_counter = 0;
        }


        if (state == poller_state_t::RUNNING) {
            // Start watchdog a new
            auto poll_start = std::chrono::high_resolution_clock::now();
            auto ec = driver_poll();
            if (ec) {
                // Only print out an error message
                GR_LOG_ERROR(d_logger, "poll failed with: " + to_string(ec));
                // Notify work method about the error... Work method will re-arm the
                // driver if required.
                d_app_buffer.notify_data_ready(ec);

                // Prevent error-flood on close
                if (d_closed)
                    return;
            }

            // Watchdog is "turned on" only some time after the acquisition start for two
            // reasons:
            // - to avoid false positives
            // - to avoid fast rearm attempts
            float estimated_samp_rate = 0.0;
            {
                // Note, mutex is not needed in case of PicoScope implementations but in
                // order to make the base class relatively generic we use mutex (streaming
                // callback is called from this
                //  thread).
                boost::mutex::scoped_lock watchdog_guard(d_watchdog_mutex);
                estimated_samp_rate = d_estimated_sample_rate.get_avg_value();
            }

            if (estimated_samp_rate <
                (get_samp_rate() * WATCHDOG_SAMPLE_RATE_THRESHOLD)) {
                // This will wake up the worker thread (see do_work method), and that
                // thread will then rearm the device...
                GR_LOG_ERROR(d_logger,
                             "Watchdog: estimated sample rate " +
                                 std::to_string(estimated_samp_rate) + "Hz, expected: " +
                                 std::to_string(get_samp_rate()) + "Hz");
                d_app_buffer.notify_data_ready(digitizer_block_errc::Watchdog);
            }
            std::chrono::duration<float> poll_duration =
                std::chrono::high_resolution_clock::now() - poll_start;

            std::this_thread::sleep_for(poll_rate - poll_duration);
        } else {
            if (state == poller_state_t::PEND_IDLE) {
                lock.lock();
                d_poller_state = state = poller_state_t::IDLE;
                lock.unlock();

                d_poller_cv.notify_all();
            } else if (state == poller_state_t::PEND_EXIT) {
                lock.lock();
                d_poller_state = state = poller_state_t::EXIT;
                lock.unlock();

                d_poller_cv.notify_all();
                return;
            }

            // Relax CPU
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void digitizer_source::start_poll_thread()
{
    if (!d_poller.joinable()) {
        std::scoped_lock guard(d_poller_mutex);
        d_poller_state = poller_state_t::IDLE;
        d_poller = boost::thread(&digitizer_source::poll_work_function, this);
    }
}

void digitizer_source::stop_poll_thread()
{
    if (!d_poller.joinable()) {
        return;
    }

    std::unique_lock<std::mutex> lock(d_poller_mutex);
    d_poller_state = poller_state_t::PEND_EXIT;
    d_poller_cv.wait_for(lock, std::chrono::seconds(5), [this] {
        return d_poller_state == poller_state_t::EXIT;
    });
    lock.unlock();

    d_poller.join();
}

void digitizer_source::transit_poll_thread_to_idle()
{
    std::unique_lock<std::mutex> lock(d_poller_mutex);

    if (d_poller_state == poller_state_t::EXIT) {
        return; // nothing to do
    }

    d_poller_state = poller_state_t::PEND_IDLE;
    d_poller_cv.wait(lock, [this] { return d_poller_state == poller_state_t::IDLE; });
}

void digitizer_source::transit_poll_thread_to_running()
{
    std::scoped_lock guard(d_poller_mutex);
    d_poller_state = poller_state_t::RUNNING;
}

int digitizer_source::work_stream(int noutput_items, gr_vector_void_star& output_items)
{
    // used for debugging in order to see how often gr calls this block
    //     uint64_t now = get_timestamp_milli_utc();
    //     if( now - last_call_utc > 20)
    //       std::cout << "now - last_call_utc [ms]: " << now - last_call_utc <<
    //       std::endl;
    //     last_call_utc = get_timestamp_milli_utc();

    assert(noutput_items >= static_cast<int>(d_buffer_size));

    // process only one buffer per iteration
    noutput_items = d_buffer_size;

    // wait data on application buffer
    auto ec = d_app_buffer.wait_data_ready();

    if (ec) {
        add_error_code(ec);
    }

    if (ec == digitizer_block_errc::Stopped) {
        GR_LOG_INFO(d_logger, "stop requested");
        return -1; // stop
    } else if (ec == digitizer_block_errc::Watchdog) {
        GR_LOG_ERROR(d_logger, "Watchdog triggered, rearming device...");
        // Rearm device
        disarm();
        arm();
        return 0; // work will be called again
    }
    if (ec) {
        GR_LOG_ERROR(d_logger, "Error reading stream data: " + to_string(ec));
        return -1; // stop
    }

    int output_items_idx = 0;
    int buff_idx = 0;
    int port_idx = 0;

    for (auto i = 0; i < d_ai_channels; i++) {
        if (d_channel_settings[i].enabled) {
            ai_buffers[buff_idx] = static_cast<float*>(output_items[output_items_idx]);
            output_items_idx++;
            ai_error_buffers[buff_idx] =
                static_cast<float*>(output_items[output_items_idx]);
            output_items_idx++;
            buff_idx++;
        } else {
            output_items_idx += 2; // Skip disabled channels
        }
    }

    for (auto i = 0; i < d_ports; i++) {
        if (d_port_settings[i].enabled) {
            port_buffers[port_idx] =
                static_cast<uint8_t*>(output_items[output_items_idx]);
            output_items_idx++;
            port_idx++;
        } else {
            output_items_idx++;
        }
    }

    // This will write samples directly into GR output buffers
    std::vector<uint32_t> channel_status;

    int64_t timestamp_now_ns_utc;
    auto lost_count = d_app_buffer.get_data_chunk(
        ai_buffers, ai_error_buffers, port_buffers, channel_status, timestamp_now_ns_utc);
    // std::cout << "timestamp_now_ns_utc: " << int64_t(timestamp_now_ns_utc) <<
    // std::endl;

    if (lost_count) {
        GR_LOG_ERROR(d_logger,
                     std::to_string(lost_count) +
                         " digitizer data buffers lost. Usually the cause of this error "
                         "is, that the work method of the Digitizer block is called with "
                         "low frequency because of a 'traffic jam' in the flowgraph. "
                         "(One of the next blocks cannot process incoming data in time)");
    }

    // Compile acquisition info tag
    acq_info_t tag_info{};

    tag_info.timestamp = timestamp_now_ns_utc;
    tag_info.timebase = get_timebase_with_downsampling();
    tag_info.user_delay = 0.0;
    tag_info.actual_delay = 0.0;

    // Attach tags to the channel values...
    int output_idx = 0;

    for (auto i = 0; i < d_ai_channels; i++) {
        if (d_channel_settings[i].enabled) {
            // add channel specific status
            tag_info.status = channel_status.at(i);

            auto tag = make_acq_info_tag(tag_info, nitems_written(0));
            add_item_tag(output_idx, tag);

            output_idx += 2;
        }
    }

    // ...and to all digital ports
    tag_info.status = 0;
    auto tag = make_acq_info_tag(tag_info, nitems_written(0));

    for (auto i = 0; i < d_ports; i++) {
        if (d_port_settings[i].enabled) {
            add_item_tag(output_idx, tag);
            output_idx++;
        }
    }

    // Software-based trigger detection
    std::vector<int> trigger_offsets;

    if (d_trigger_settings.is_analog()) {

        // TODO: improve, check selected trigger on arm
        const auto aichan = convert_to_aichan_idx(d_trigger_settings.source);
        auto output_idx = 0;

        for (int i = 0; i < aichan; i++) {
            if (d_channel_settings[i].enabled) {
                output_idx += 2;
            }
        }

        auto buffer = static_cast<float const* const>(output_items[output_idx]);
        trigger_offsets = find_analog_triggers(buffer, d_buffer_size);
    } else if (d_trigger_settings.is_digital()) {
        auto port = d_trigger_settings.pin_number / 8;
        auto pin = d_trigger_settings.pin_number % 8;
        auto mask = 1 << pin;

        auto buffer = static_cast<uint8_t const* const>(
            output_items[output_items.size() - d_ports + port]);
        trigger_offsets = find_digital_triggers(buffer, d_buffer_size, mask);
    }

    double time_per_sample_with_downsampling_ns =
        d_time_per_sample_ns * d_downsampling_factor;

    // Attach trigger tags
    for (auto trigger_offset : trigger_offsets) {
        //       std::cout << "trigger_offset       : " << trigger_offset<<std::endl;
        //       std::cout << "noutput_items        : " << noutput_items <<std::endl;
        //       std::cout << "d_time_per_sample_ns : " <<
        //       time_per_sample_with_downsampling_ns <<std::endl; std::cout <<
        //       "local_timstamp        : " << local_timstamp << std::endl; std::cout <<
        //       "another now           :" << now << std::endl; std::cout <<
        //       "timestamp_now_ns_utc  : " << timestamp_now_ns_utc<<std::endl; std::cout
        //       << "diff[ns]             : " << uint64_t((noutput_items - trigger_offset
        //       ) * time_per_sample_with_downsampling_ns )<<std::endl; std::cout <<
        //       "stamp added           : " << uint64_t(timestamp_now_ns_utc - ((
        //       noutput_items - trigger_offset ) * time_per_sample_with_downsampling_ns
        //       )) <<std::endl; std::cout << "tag offset: " << nitems_written(0) +
        //       trigger_offset <<std::endl;
        auto trigger_tag = make_trigger_tag(
            d_downsampling_factor,
            timestamp_now_ns_utc - uint64_t((noutput_items - trigger_offset) *
                                            time_per_sample_with_downsampling_ns),
            nitems_written(0) + trigger_offset,
            0); // status

        int output_idx = 0;

        for (auto i = 0; i < d_ai_channels; i++) {
            if (d_channel_settings[i].enabled) {
                add_item_tag(output_idx, trigger_tag);
                output_idx += 2;
            }
        }

        for (auto i = 0; i < d_ports; i++) {
            if (d_port_settings[i].enabled) {
                add_item_tag(output_idx, trigger_tag);
                output_idx++;
            }
        }
    }

    return noutput_items;
}

int digitizer_source::work(int noutput_items,
                           gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items)
{
    int retval = -1;

    if (d_acquisition_mode == acquisition_mode_t::STREAMING) {
        retval = work_stream(noutput_items, output_items);
    } else if (d_acquisition_mode == acquisition_mode_t::RAPID_BLOCK) {
        retval = work_rapid_block(noutput_items, output_items);
    }

    if ((retval > 0) && !d_timebase_published) {
        auto timebase_tag = make_timebase_info_tag(get_timebase_with_downsampling());
        timebase_tag.offset = nitems_written(0);

        for (gr_vector_void_star::size_type i = 0; i < output_items.size(); i++) {
            add_item_tag(i, timebase_tag);
        }

        d_timebase_published = true;
    }

    return retval;
}

} // namespace pulsed_power
} /* namespace gr */
