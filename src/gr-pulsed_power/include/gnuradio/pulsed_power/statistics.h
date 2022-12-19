/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_STATISTICS_H
#define INCLUDED_PULSED_POWER_STATISTICS_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API statistics : virtual public gr::sync_decimator
{
public:
    typedef std::shared_ptr<statistics> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::statistics.
     *
     * To avoid accidental use of raw pointers, pulsed_power::statistics's
     * constructor is in a private implementation
     * class. pulsed_power::statistics::make is the public interface for
     * creating new instances.
     */
    static sptr make(int decimation);

    /**
     * @brief Returns the statistics of the samples
     *
     * @param mean The result of the mean calculation
     * @param min The result of the min calculation
     * @param max The result of the max calculation
     * @param std_deviation The result of the std_deviation calculation
     * @param in The inputs
     * @param ninput_items Number of samples
     */
    virtual void calculate_statistics(float& mean,
                                      float& min,
                                      float& max,
                                      float& std_deviation,
                                      const float* in,
                                      int ninput_items) = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_STATISTICS_H */
