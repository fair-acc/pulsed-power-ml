/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */

#ifndef INCLUDED_PULSED_POWER_PICOSCOPE_BASE_H
#define INCLUDED_PULSED_POWER_PICOSCOPE_BASE_H

// Digitizer
#include "digitizer_source.h"
#include "status.h"

// boost
#include <boost/bind.hpp>
#include <boost/function.hpp>

// GNU Radio
#include <volk/volk.h>

namespace gr {
namespace pulsed_power {

typedef boost::function<void(int32_t, uint32_t, int16_t)> streaming_callback_function_t;

static void invoke_streaming_callback(int16_t handle,
                                      int32_t noOfSamples,
                                      uint32_t startIndex,
                                      int16_t overflow,
                                      uint32_t triggerAt,
                                      int16_t triggered,
                                      int16_t autoStop,
                                      void* pParameter)
{
    (*static_cast<streaming_callback_function_t*>(pParameter))(
        noOfSamples, startIndex, overflow);
}

/*!
 * \brief Common functionality shared between different PicoScope devices
 */
class picoscope_base : public digitizer_source
{
protected:
    std::string d_serial_number; // if empty first available device is used
    int16_t d_max_value;         // maximum ADC count used for ADC conversion
    float d_vertical_precision;  // vertical precision of AI channels

    meta_range_t d_ranges;
    std::vector<std::string> d_channel_ids;

    streaming_callback_function_t d_streaming_callback;

    // Driver buffers
    std::vector<std::vector<int16_t>> d_buffers;
    std::vector<std::vector<int16_t>> d_buffers_min;
    std::vector<std::vector<int16_t>> d_port_buffers;

    // Tmp buffer
    app_buffer_t::data_chunk_t* d_tmp_buffer;
    size_t d_tmp_buffer_size; // number of samples in the buffer

    int d_lost_count;

public:
    picoscope_base(std::string serial_number,
                   int max_ai_channels,
                   int max_di_ports,
                   bool auto_arm,
                   int16_t max_raw_analog_value,
                   float vertical_precision);

    ~picoscope_base();

    std::vector<std::string> get_aichan_ids() override;

    meta_range_t get_aichan_ranges() override;

protected:
    void
    streaming_callback(int32_t no_of_samples, uint32_t start_index, int16_t overflow);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_PICOSCOPE_BASE_H */
