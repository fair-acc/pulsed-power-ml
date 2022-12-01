/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_STATISTICS_H
#define INCLUDED_PULSED_POWER_STATISTICS_H

#include <gnuradio/block.h>
#include <gnuradio/pulsed_power/api.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API statistics : virtual public gr::block
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
    static sptr make();
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_STATISTICS_H */
