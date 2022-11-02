/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_H
#define INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief GNU Radio OpenCMW sink for exporting frequency-domain data into OpenCMW.
 *
 * On each incoming data package a callback is called which allows the host
 * application to copy the data to its internal buffers
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API opencmw_freq_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<opencmw_freq_sink> sptr;
    using cb_copy_data_t = std::function<void(
        const float*, int&, size_t, const std::string&, float, int64_t)>;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::opencmw_freq_sink.
     *
     * To avoid accidental use of raw pointers, pulsed_power::opencmw_freq_sink's
     * constructor is in a private implementation
     * class. pulsed_power::opencmw_freq_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make(std::string signal_name = "signal_1",
                     std::string signal_unit = "",
                     float sample_rate = 0.0F,
                     float bandwidth = 0.0F,
                     size_t vector_size = 1024);

    /*!
     * \brief Registers a callback which is called whenever a predefined number of samples
     * is available.
     *
     * \param cb_copy_data callback in which the host application can copy the data
     */
    virtual void set_callback(cb_copy_data_t cb_copy_data) = 0;

    /*!
     * \brief Returns bandwidth in Hz.
     */
    virtual float get_bandwidth() = 0;

    /*!
     * \brief Returns sample rate in Hz.
     */
    virtual float get_sample_rate() = 0;

    /*!
     * \brief Returns signal name.
     */
    virtual std::string get_signal_name() = 0;

    /*!
     * \brief Returns signal unit.
     */
    virtual std::string get_signal_unit() = 0;

    /*!
     * \brief Returns vector size.
     */
    virtual size_t get_vector_size() = 0;
};

extern PULSED_POWER_API std::mutex globalFrequencySinksRegistryMutex;
extern PULSED_POWER_API std::vector<opencmw_freq_sink*> globalFrequencySinksRegistry;

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_FREQ_SINK_H */
