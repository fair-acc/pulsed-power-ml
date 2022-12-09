/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */


#ifndef INCLUDED_PULSED_POWER_TAGS_H
#define INCLUDED_PULSED_POWER_TAGS_H

// Digitizer
#include "api.h"

// GNU Radio
#include <gnuradio/tags.h>

// Build-in
#include <cassert>

namespace gr {
namespace pulsed_power {

// ################################################################################################################
// ################################################################################################################

/*!
 * \brief A convenience structure holding information about the measurement.
 * \ingroup pulsed_power
 *
 * General use:
 *
 * Digitizers are expected to attach the 'acq_info' tag to all the enabled channels
 * including digital ports. In streaming mode this should be done whenever a new chunk of
 * data is obtained from the device.
 *
 * If trigger detection is enabled in streaming mode, then the acq_info tag should be
 * attached to the output streams whenever a trigger is detected. In this case the
 * triggered_data field should be set to True allowing other post-processing modules (i.e.
 * B.2 Demux) to work with triggered data only.
 *
 *
 * The acq_info tag contains two delays, namely the user_delay and the actual_delay. The
 * following ascii figure depicts relation between different terms:
 *
 * user_delay                 |--->          (3 us)
 * realignment_delay          |--------->    (9 us)
 * actual_delay               |------------> (3 + 9 = 12 us)
 *
 * Those delays are used for two purposes: (TODO: This doc. is outdated, fix it)
 *
 * 1) Time synchronization:
 *    User delay is added to the actual timestamp of a given sample the tag is attached
 * to. Field timestamp therefore contains timestamp calculated like this:
 *
 *    timestamp = <actual acquisition timestamp> + user_delay
 *
 * 2) Extraction of triggered data
 *    Actual delay, that is user delay and edge-trigger based delay synchronization is
 * accounted for when extracting triggered data. See B.2 block description for more
 * details (extractor.h).
 */

char const* const acq_info_tag_name = "acq_info";
struct PULSED_POWER_API acq_info_t {
    int64_t timestamp;   // timestamp (UTC nanoseconds), used as fallback if no trigger is
                         // available
    double timebase;     // distance between samples in seconds
    double user_delay;   // see description above
    double actual_delay; // see description above
    uint32_t status;     // acquisition status
};

inline gr::tag_t make_acq_info_tag(const acq_info_t& acq_info, uint64_t offset)
{
    gr::tag_t tag;
    tag.key = pmt::intern(acq_info_tag_name);
    tag.value =
        pmt::make_tuple(pmt::from_uint64(static_cast<uint64_t>(acq_info.timestamp)),
                        pmt::from_double(acq_info.timebase),
                        pmt::from_double(acq_info.user_delay),
                        pmt::from_double(acq_info.actual_delay),
                        pmt::from_long(static_cast<long>(acq_info.status)));
    tag.offset = offset;
    return tag;
}

inline acq_info_t decode_acq_info_tag(const gr::tag_t& tag)
{
    assert(pmt::symbol_to_string(tag.key) == acq_info_tag_name);

    if (!pmt::is_tuple(tag.value) || pmt::length(tag.value) != 5) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid acq_info tag format";
        throw std::runtime_error(message.str());
    }

    acq_info_t acq_info;
    auto tag_tuple = pmt::to_tuple(tag.value);
    acq_info.timestamp = static_cast<int64_t>(pmt::to_uint64(tuple_ref(tag_tuple, 0)));
    acq_info.timebase = pmt::to_double(tuple_ref(tag_tuple, 1));
    acq_info.user_delay = pmt::to_double(tuple_ref(tag_tuple, 2));
    acq_info.actual_delay = pmt::to_double(tuple_ref(tag_tuple, 3));
    acq_info.status = static_cast<uint32_t>(pmt::to_long(tuple_ref(tag_tuple, 4)));
    return acq_info;
}

// ################################################################################################################
// ################################################################################################################

char const* const trigger_tag_name = "trigger";
struct PULSED_POWER_API trigger_t {
    uint32_t downsampling_factor;
    int64_t timestamp;
    uint32_t status;
};

inline gr::tag_t make_trigger_tag(trigger_t& trigger_tag_data, uint64_t offset)
{
    gr::tag_t tag;
    tag.key = pmt::intern(trigger_tag_name);
    tag.value = pmt::make_tuple(
        pmt::from_long(static_cast<long>(trigger_tag_data.downsampling_factor)),
        pmt::from_uint64(static_cast<uint64_t>(trigger_tag_data.timestamp)),
        pmt::from_long(static_cast<long>(trigger_tag_data.status)));
    tag.offset = offset;
    return tag;
}

inline gr::tag_t make_trigger_tag(uint32_t downsampling_factor,
                                  int64_t timestamp,
                                  uint64_t offset,
                                  uint32_t status)
{
    gr::tag_t tag;
    tag.key = pmt::intern(trigger_tag_name);
    tag.value = pmt::make_tuple(pmt::from_long(static_cast<long>(downsampling_factor)),
                                pmt::from_uint64(static_cast<uint64_t>(timestamp)),
                                pmt::from_long(static_cast<long>(status)));
    tag.offset = offset;
    return tag;
}

// e.g. used for streaming
inline gr::tag_t make_trigger_tag(uint64_t offset)
{
    gr::tag_t tag;
    tag.key = pmt::intern(trigger_tag_name);
    tag.value = pmt::make_tuple(pmt::from_long(static_cast<long>(0)),
                                pmt::from_uint64(static_cast<uint64_t>(0)),
                                pmt::from_long(static_cast<long>(0)));
    tag.offset = offset;
    return tag;
}

inline trigger_t decode_trigger_tag(const gr::tag_t& tag)
{
    assert(pmt::symbol_to_string(tag.key) == trigger_tag_name);

    if (!pmt::is_tuple(tag.value) || pmt::length(tag.value) != 3) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid trigger tag format";
        throw std::runtime_error(message.str());
    }

    trigger_t trigger_tag;
    auto tag_tuple = pmt::to_tuple(tag.value);
    trigger_tag.downsampling_factor =
        static_cast<uint32_t>(pmt::to_long(tuple_ref(tag_tuple, 0)));
    trigger_tag.timestamp = static_cast<int64_t>(pmt::to_uint64(tuple_ref(tag_tuple, 1)));
    trigger_tag.status = static_cast<uint32_t>(pmt::to_long(tuple_ref(tag_tuple, 2)));

    return trigger_tag;
}

// ################################################################################################################
// ################################################################################################################

char const* const timebase_info_tag_name = "timebase_info";

/*!
 * \brief Factory function for creating timebase_info tags.
 */
inline gr::tag_t make_timebase_info_tag(double timebase)
{
    gr::tag_t tag;
    tag.key = pmt::intern(timebase_info_tag_name);
    tag.value = pmt::from_double(timebase);
    return tag;
}

/*!
 * \brief Returns timebase stored within the timebase_info tag.
 */
inline double decode_timebase_info_tag(const gr::tag_t& tag)
{
    assert(pmt::symbol_to_string(tag.key) == timebase_info_tag_name);
    return pmt::to_double(tag.value);
}

// ################################################################################################################
// ################################################################################################################

/*!
 * \brief Name of the WR event tag.
 */
char const* const wr_event_tag_name = "wr_event";

/*!
 * \brief A convenience structure holding information about the WR timing event.
 * \ingroup pulsed_power
 */
struct PULSED_POWER_API wr_event_t {
    std::string event_id;
    int64_t wr_trigger_stamp;     // timestamp of the wr-event (TAI nanoseconds)
    int64_t wr_trigger_stamp_utc; // timestamp of the wr-event (UTC nanoseconds)
};

/*!
 * \brief Factory function for creating wr event tags.
 */
inline gr::tag_t make_wr_event_tag(const wr_event_t& event, uint64_t offset)
{
    gr::tag_t tag;
    tag.key = pmt::intern(wr_event_tag_name);
    tag.value = pmt::make_tuple(
        pmt::string_to_symbol(event.event_id),
        pmt::from_uint64(static_cast<uint64_t>(event.wr_trigger_stamp)),
        pmt::from_uint64(static_cast<uint64_t>(event.wr_trigger_stamp_utc)));
    tag.offset = offset;
    return tag;
}

/*!
 * \brief Converts wr event tag into wr_event_t struct.
 */
inline wr_event_t decode_wr_event_tag(const gr::tag_t& tag)
{
    assert(pmt::symbol_to_string(tag.key) == wr_event_tag_name);

    if (!pmt::is_tuple(tag.value) || pmt::length(tag.value) != 3) {
        std::ostringstream message;
        message << "Exception in " << __FILE__ << ":" << __LINE__
                << ": invalid wr_event tag format";
        throw std::runtime_error(message.str());
    }

    wr_event_t event;

    auto tag_tuple = pmt::to_tuple(tag.value);
    event.event_id = pmt::symbol_to_string(tuple_ref(tag_tuple, 0));
    event.wr_trigger_stamp =
        static_cast<int64_t>(pmt::to_uint64(tuple_ref(tag_tuple, 1)));
    event.wr_trigger_stamp_utc =
        static_cast<int64_t>(pmt::to_uint64(tuple_ref(tag_tuple, 2)));

    return event;
}

// ################################################################################################################
// ################################################################################################################

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_TAGS_H */
