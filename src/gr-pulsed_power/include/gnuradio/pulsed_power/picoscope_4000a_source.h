/* -*- c++ -*- */
/*
 * Copyright 2021 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_H
#define INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_H

// Digitizer
#include "api.h"

// GNU Radio
#include <gnuradio/logger.h>
#include <gnuradio/sync_block.h>

// Picoscope - fair-GSI
#include "picoscope_base.h"
#include "ps_4000a_defs.h"
#include "status.h"

// Picoscope - picotech
#include </opt/picoscope/include/libps4000a/PicoStatus.h>
#include </opt/picoscope/include/libps4000a/ps4000aApi.h> //TODO: remove ugly workaround

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API picoscope_4000a_source : virtual public picoscope_base
{
public:
    typedef std::shared_ptr<picoscope_4000a_source> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of
     * pulsed_power::picoscope_4000a_source.
     *
     * To avoid accidental use of raw pointers, pulsed_power::picoscope_4000a_source's
     * constructor is in a private implementation
     * class. pulsed_power::picoscope_4000a_source::make is the public interface for
     * creating new instances.
     */
    static sptr make(std::string serial_number, bool auto_arm);

    virtual void set_trigger_once(bool auto_arm) = 0;
    virtual void set_samp_rate(double rate) = 0;
    virtual void set_downsampling(downsampling_mode_t mode,
                                  int downsample_factor = 0) = 0;

    virtual void set_aichan(const std::string& id,
                            bool enabled,
                            double range,
                            coupling_t coupling,
                            double range_offset = 0) = 0;
    virtual void set_aichan_a(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_b(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_c(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_d(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_e(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_f(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_g(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;
    virtual void set_aichan_h(bool enabled,
                              double range,
                              coupling_t coupling,
                              double range_offset = 0) = 0;


    virtual void set_aichan_trigger(const std::string& id,
                                    trigger_direction_t direction,
                                    double threshold) = 0;
    virtual void set_samples(int pre_samples, int post_samples) = 0;
    virtual void set_rapid_block(int nr_waveforms) = 0;

    virtual void set_nr_buffers(int nr_buffers) = 0;
    virtual void set_streaming(double poll_rate = 0.001) = 0;
    virtual void set_driver_buffer_size(int driver_buffer_size) = 0;
    virtual void set_buffer_size(int buffer_size) = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_PICOSCOPE_4000A_SOURCE_H */
