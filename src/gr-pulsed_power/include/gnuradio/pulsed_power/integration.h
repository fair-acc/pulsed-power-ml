/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_INTEGRATION_H
#define INCLUDED_PULSED_POWER_INTEGRATION_H

#include <gnuradio/block.h>
#include <gnuradio/pulsed_power/api.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API integration : virtual public gr::block
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
     */
    static sptr make(float step_size);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_INTEGRATION_H */
