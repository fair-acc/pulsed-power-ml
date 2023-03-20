/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_INTEGRATION_H
#define INCLUDED_PULSED_POWER_INTEGRATION_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
namespace pulsed_power {

enum INTEGRATION_DURATION { DAY, WEEK, MONTH };

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API integration : virtual public gr::sync_decimator
{
public:
    typedef std::shared_ptr<integration> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::integration.
     *
     * To avoid accidental use of raw pointers, pulsed_power::integration's
     * constructor is in a private implementation
     * class. pulsed_power::integration::make is the public interface for
     * creating new instances.
     *
     * @param decimation Decimation
     * @param sample_rate Sample rate in Hz
     * @param duration The time duration before reset.
     */
    static sptr make(int decimation, int sample_rate, INTEGRATION_DURATION duration);

    /*!
     * @brief Calculates the integral of the samples given in Watt
     *
     * @param out Result pointer to integrated values in Watt/hour
     * @param sample Pointer to samples that shall be integrated
     * @param n_samples Number of samples over that is integrated
     * @param calculate_with_last_value if true the last value of the last sequence is
     * used to calculate the value of the integral since then. Only do this if the
     * last_value variable was set.
     */
    virtual void integrate(float& out,
                           const float* sample,
                           int n_samples,
                           bool calculate_with_last_value) = 0;

    /*!
     * @brief Calculate integrals of samples and save the sum of the last integral in a
     * file
     *
     * @param out Result pointer
     * @param sample Pointer to samples that shall be integrated
     * @param noutput_items Number of outputs
     */

    virtual void add_new_steps(float* out, const float* sample, int noutput_items) = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_INTEGRATION_H */
