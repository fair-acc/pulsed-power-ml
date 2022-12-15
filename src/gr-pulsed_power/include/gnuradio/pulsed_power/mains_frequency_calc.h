/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_H
#define INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief Calculates Frequency of main from signal.
 * \ingroup pulsed_power
 * \details Expects sine wave.
 */
class PULSED_POWER_API mains_frequency_calc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<mains_frequency_calc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of
     * pulsed_power::mains_frequency_calc. blubb
     *
     * \param expected_sample_rate This Block needs to know the sample rate to accurately
     * calculate the signal's frequency. \param low_threshold Low Threshold at which to
     * start counting the half period. Ideally mirrored with high_threshold at zero. Needs
     * to be smaller than the expected amplitude. \param high_threshold High Threshold at
     * which to start counting the other half period. Ideally mirrored with low_threshold
     * at zero. Needs to be smaller than the expected amplitude.
     */
    static sptr
    make(float expected_sample_rate, float low_threshold, float high_threshold);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_H */
