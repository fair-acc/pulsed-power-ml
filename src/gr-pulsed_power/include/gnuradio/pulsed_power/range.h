/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */

#ifndef INCLUDED_PULSED_POWER_RANGE_H
#define INCLUDED_PULSED_POWER_RANGE_H

// GNU Radio
#include "api.h"

// Build-in
#include <gnuradio/runtime_types.h>
#include <iostream>
#include <limits>


// boost
#include <boost/math/special_functions/round.hpp>

namespace gr {
namespace pulsed_power {

static const double default_step = 0.0000001;

/*!
 * \brief Represents available ranges.
 *
 * \ingroup pulsed_power
 *
 * Based on: https://github.com/EttusResearch/uhd/blob/maint/host/lib/types/ranges.cpp
 */
class PULSED_POWER_API range_t
{
public:
    range_t(double value = 0.0) : d_start(value), d_stop(value), d_step(0.0) {}

    range_t(double start, double stop, double step = default_step)
        : d_start(start), d_stop(stop), d_step(step)
    {
        if (stop < start) {
            std::ostringstream message;
            message << "Exception in " << __FILE__ << ":" << __LINE__
                    << ":  stop should be larger or equal to start, start: " << start
                    << ", stop: " << stop;
            throw std::invalid_argument(message.str());
        }
        if (step <= 0.0) {
            std::ostringstream message;
            message << "Exception in " << __FILE__ << ":" << __LINE__
                    << ":  invalid step: " << step;
            throw std::invalid_argument(message.str());
        }
    }

    double start() const { return d_start; };

    double stop() const { return d_stop; };

    double step() const { return d_step; };

private:
    double d_start;
    double d_stop;
    double d_step;
};

class PULSED_POWER_API meta_range_t : public std::vector<range_t>
{
public:
    meta_range_t() {}
    meta_range_t(double start, double stop, double step = default_step)
        : std::vector<range_t>(1, range_t(start, stop, step))
    {
    }

    template <typename InputIterator>
    meta_range_t(InputIterator first, InputIterator last, std::input_iterator_tag)
        : std::vector<range_t>(first, last)
    { /* NOP */
    }

    double start() const
    {
        check_meta_range_monotonic();
        return back().start();
    }

    double stop() const
    {
        check_meta_range_monotonic();
        return front().stop();
    }

    double clip(double value) const
    {
        check_meta_range_monotonic();

        // less or equal to min
        if (value <= front().start()) {
            return front().start();
        }

        // Greater or equal to max
        if (value >= back().stop()) {
            return back().stop();
        }

        // find appropriate range, always clip up! Note reverse iterator is used in order
        // to simplify the implementation...
        auto it = std::find_if(
            begin(), end(), [value](const range_t& r) { return value <= r.start(); });

        // this should not happen.... ever.
        if (it == end()) {
            std::ostringstream message;
            message << "Exception in " << __FILE__ << ":" << __LINE__
                    << ": failed to find appropriate range for the value: " << value;
            throw std::runtime_error(message.str());
        }

        // lest find setting
        if (it->start() == it->stop()) {
            return it->start();
        } else if (it->step() != 0.0) {
            return round((value - it->start()) / it->step()) * it->step() + it->start();
        } else {
            // lower-bound check done as part of find_if
            assert(value >= it->start());
            return value >= it->stop() ? it->stop() : value;
        }
    };

private:
    void check_meta_range_monotonic() const
    {
        if (empty()) {
            std::ostringstream message;
            message << "Exception in " << __FILE__ << ":" << __LINE__
                    << ": meta-range cannot be empty";
            throw std::runtime_error(message.str());
        }
        for (size_type i = 1; i < size(); i++) {
            if (at(i).start() < at(i - 1).stop()) {
                std::ostringstream message;
                message << "Exception in " << __FILE__ << ":" << __LINE__
                        << ": meta-range is not monotonic";
                throw std::runtime_error(message.str());
            }
        }
    }
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_RANGE_H */
