/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_OPENCMW_SINK_H
#define INCLUDED_PULSED_POWER_OPENCMW_SINK_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief GNU Radio OpenCMW sink for exporting time-domain data into OpenCMW.
 *
 * On each incoming data package a callback is called which allows the host 
 * application to copy the data to its internal buffers
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API opencmw_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<opencmw_sink> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::opencmw_sink.
     *
     * To avoid accidental use of raw pointers, pulsed_power::opencmw_sink's
     * constructor is in a private implementation
     * class. pulsed_power::opencmw_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make();

    /*!
     * \brief Registers a callback which is called whenever a predefined number of samples is available.
     *
     * \param cb_copy_data callback in which the host application can copy the data
     */
    virtual void set_callback(std::function<void(const float*, int)> cb_copy_data) = 0;
};

extern PULSED_POWER_API std::mutex globalSinksRegistryMutex;
extern PULSED_POWER_API std::vector<opencmw_sink*> globalSinksRegistry;

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_SINK_H */
