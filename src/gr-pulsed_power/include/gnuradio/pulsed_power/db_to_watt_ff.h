/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_PULSED_POWER_DB_TO_WATT_FF_H
#define INCLUDED_PULSED_POWER_DB_TO_WATT_FF_H

#include <gnuradio/sync_block.h>
#include <pulsed_power/api.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API db_to_watt_ff : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<db_to_watt_ff> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::db_to_watt_ff.
     *
     * To avoid accidental use of raw pointers, pulsed_power::db_to_watt_ff's
     * constructor is in a private implementation
     * class. pulsed_power::db_to_watt_ff::make is the public interface for
     * creating new instances.
     */
    static sptr make();
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_DB_TO_WATT_FF_H */
