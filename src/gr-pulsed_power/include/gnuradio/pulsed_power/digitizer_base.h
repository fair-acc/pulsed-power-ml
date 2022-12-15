/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */


#ifndef INCLUDED_PULSED_POWER_DIGITIZER_BASE_H
#define INCLUDED_PULSED_POWER_DIGITIZER_BASE_H

// Digitizer
#include "api.h"
#include "range.h"

// GNU Radio
#include <gnuradio/logger.h>
#include <gnuradio/sync_block.h>


// Build-in
#include <system_error>
#include <string>

namespace gr {
namespace pulsed_power {

/*!
 * \brief An enum representing acquisition mode
 * \ingroup pulsed_power
 */
enum PULSED_POWER_API acquisition_mode_t { RAPID_BLOCK, STREAMING };

/*!
 * \brief An enum representing coupling mode
 * \ingroup pulsed_power
 */
enum PULSED_POWER_API coupling_t {
    DC_1M = 0,  /* DC, 1 MOhm */
    AC_1M = 1,  /* AC, 1 MOhm */
    DC_50R = 2, /* DC, 50 Ohm */
};

/*!
 * \brief Specifies a trigger mechanism
 * \ingroup pulsed_power
 */
enum PULSED_POWER_API trigger_direction_t {
    TRIGGER_DIRECTION_RISING,
    TRIGGER_DIRECTION_FALLING,
    TRIGGER_DIRECTION_LOW,
    TRIGGER_DIRECTION_HIGH
};

/*!
 * \brief Downsampling mode
 * \ingroup pulsed_power
 */
enum PULSED_POWER_API downsampling_mode_t {
    DOWNSAMPLING_MODE_NONE = 0,
    DOWNSAMPLING_MODE_MIN_MAX_AGG = 1,
    DOWNSAMPLING_MODE_DECIMATE = 2,
    DOWNSAMPLING_MODE_AVERAGE = 3,
};

/*!
 * \brief Error information.
 * \ingroup pulsed_power
 */
struct PULSED_POWER_API error_info_t {
    uint64_t timestamp;
    std::error_code code;
};

/*!
 * \brief Base class for digitizer blocks
 *
 * Note, both the value and the error estimate output needs to be connected for enabled
 * channels. If error estimate is not required it can be connected to a null sink.
 *
 * \ingroup pulsed_power
 */
class PULSED_POWER_API digitizer_base
{

public:
    /*!
     * \brief Version number of device driver.
     *
     * \return Version number
     */
    virtual std::string get_driver_version() = 0;

    /*!
     * \brief Hardware or firmware version number.
     *
     * \return Version number
     */
    virtual std::string get_hardware_version() = 0;

    /*!
     * \brief Gets acquisition mode.
     */
    virtual acquisition_mode_t get_acquisition_mode() = 0;

    /*!
     * \brief Configure number of pre- and post-trigger samples.
     *
     * Note in streaming mode pre- and post-trigger samples are used only when
     * a trigger is enabled, that a triggered_data tag is added allowing other
     * blocks to extracts trigger data if desired so.
     *
     * \param samples the number of samples to acquire before the trigger event
     * \param pre_samples the number of samples to acquire
     */
    virtual void set_samples(int pre_samples, int post_samples) = 0;

    /*!
     * \brief Sets maximum buffer size in samples per channel.
     *
     * Note, application buffer consists of N buffers.
     *
     * This function will also call GR's set_output_multiple making sure that the number
     * of output items will be always a multiple of buffer size.
     *
     * Make sure to configure the max output buffer size with the value greater or equal
     * to the buffer size.
     *
     * Applicable in streaming mode only.
     * \param buffer_size the buffer size in samples
     */
    virtual void set_buffer_size(int buffer_size) = 0;

    /*!
     * \brief Sets number of application buffers.
     *
     * Background: Data transfer between the drivers and GR digitizer block is implemented
     * using the circular-buffer-like structure holding N buffers.
     *
     * Applicable in streaming mode only.
     * \param nr_buffers the number of application buffers
     */
    virtual void set_nr_buffers(int nr_buffers) = 0;

    /*!
     * \brief Sets driver buffer size in samples per channel.
     *
     * \param driver_buffer_size the buffer size in samples
     */
    virtual void set_driver_buffer_size(int driver_buffer_size) = 0;

    /*!
     * \brief If auto arm is set then this block will automatically arm or rearm
     * the device, that is initially on start and afterwards whenever a desired
     * number of blocks is collected.
     *
     * \param auto_arm
     */
    virtual void set_auto_arm(bool auto_arm) = 0;

    /*!
     * \brief Arm device only once.
     *
     * Useful in combination with rapid-block mode only.
     *
     * \param auto_arm
     */
    virtual void set_trigger_once(bool auto_arm) = 0;

    /*!
     * \brief Set rapid block mode.
     *
     * \param nr_waveforms
     */
    virtual void set_rapid_block(int nr_waveforms) = 0;

    /*!
     * \brief Set streaming mode.
     *
     * \param poll_rate
     */
    virtual void set_streaming(double poll_rate = 0.001) = 0;

    /*!
     * \brief Set downsampling mode and downsampling factor.
     *
     * If downsampling equals 0, i.e. no downsampling, the factor is disregarded.
     *
     * \param mode The type of downsampling the user wishes for the data acquisition.
     * \param downsample_factor The number of samples to be squished into a downsampled
     * sample.
     */
    virtual void set_downsampling(downsampling_mode_t mode,
                                  int downsample_factor = 0) = 0;

    /*!
     * \brief Set the sample rate.
     * \param rate a new rate in Sps
     */
    virtual void set_samp_rate(double rate) = 0;

    /*!
     * \brief Get the sample rate for this device.
     * This is the actual sample rate and may differ from the rate set.
     * \return the actual rate in Sps
     */
    virtual double get_samp_rate() = 0;

    /*!
     * \brief Get AI channel names.
     * \return a vector of channel names, e.g. "A", "B", ...
     */
    virtual std::vector<std::string> get_aichan_ids() = 0;

    /*!
     * \brief Get available AI ranges.
     * \return available ranges
     */
    virtual meta_range_t get_aichan_ranges() = 0;

    /*!
     * \brief Configure an AI channel
     *
     * Note, the underlying code is not thread safe. Disarm the device before changing
     * configuration.
     *
     * \param id Channel name e.g. "A", "B", "C", "D", ...
     * \param enabled Set desired state. Enabled or disabled channel
     * \param range desired voltage range in Volts
     * \param dc_coupling the coupling type (AC 1M / DC 1M / DC 50R)
     * \param range_offset desired voltage offset in Volts
     */
    virtual void set_aichan(const std::string& id,
                            bool enabled,
                            double range,
                            coupling_t coupling,
                            double range_offset = 0) = 0;

    /*!
     * \brief Configure an AI channel with user defined range and offset.
     *
     * \param id Channel name e.g. "A", "B", "C", "D", ...
     * \param range desired voltage range in Volts
     * \param range_offset desired voltage offset in Volts
     */
    virtual void
    set_aichan_range(const std::string& id, double range, double range_offset = 0) = 0;

    /*!
     * \brief Configure an AI channel trigger
     * \param id Channel name e.g. "A", "B", "C", "D", ...
     * \param direction Trigger direction
     * \param threshold Triggering voltage. The channel has to be over it for the device
     * to trigger.
     */
    virtual void set_aichan_trigger(const std::string& id,
                                    trigger_direction_t direction,
                                    double threshold) = 0;

    /**
     * \brief Set up a digital port, with user defined threshold and triggering mask.
     *
     * \param id port id
     * \param enabled Set desired state. Enabled or disabled port
     * \param thresh_voltage desired threshold voltage of the logic value switch(0<==>1)
     */
    virtual void
    set_diport(const std::string& id, bool enabled, double thresh_voltage) = 0;

    /*!
     * \brief Set up a digital input trigger.
     *
     * Note this interface assumes that digital inputs are numbered from
     * 0..MAX_DI_CHANNELS and it does not distinguish between ports. For example port 0
     * might contain 8 DI channels and port 1 another 8 channels. It is assumed that the
     * first pin of port 0 has a label 0 and last pin on second port label 15.
     *
     */
    virtual void set_di_trigger(uint32_t pin, trigger_direction_t direction) = 0;

    /*!
     * \brief Disable triggers if enabled.
     *
     * This function is forseen to be used mostly for testing purposes where it is
     * desidred to have an potion for changing configuration run-time.
     */
    virtual void disable_triggers() = 0;

    /*!
     * \brief explicitly initialize connection to the device
     */
    virtual void initialize() = 0;

    /*!
     * \brief Applies configuration
     */
    virtual void configure() = 0;

    /*!
     * \brief Arms the device with the given settings
     */
    virtual void arm() = 0;

    /*!
     * \brief returns the arm state of the device
     */
    virtual bool is_armed() = 0;

    /*!
     * \brief Disarms the device.
     */
    virtual void disarm() = 0;

    /*!
     * \brief Closes the device. Disarms if necessary
     */
    virtual void close() = 0;

    /*!
     * \brief Gets the list of the last 128 errors or warnings that
     * have occurred organized by time, with their corresponding timestamp.
     */
    virtual std::vector<error_info_t> get_errors() = 0;

    /*!
     * \brief Returns exception message of exception which occured during start/configure,
     * if any
     */
    virtual std::string getConfigureExceptionMessage() = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_DIGITIZER_BASE_H */
