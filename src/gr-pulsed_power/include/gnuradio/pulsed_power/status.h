/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */


#ifndef INCLUDED_PULSED_POWER_STATUS_H
#define INCLUDED_PULSED_POWER_STATUS_H

#include "api.h"

namespace gr {
namespace pulsed_power {

/*!
 * \brief Channel-related status flags (bit-enum).
 */
enum PULSED_POWER_API channel_status_t {
    // Overvoltage has occurred on the channel.
    CHANNEL_STATUS_OVERFLOW = 0x01,

    // Not enough pre- or post-trigger samples available to perform realignment or/and
    // user delay.
    CHANNEL_STATUS_REALIGNMENT_ERROR = 0x02,

    // Insufficient buffer size to extract all samples
    CHANNEL_STATUS_NOT_ALL_DATA_EXTRACTED = 0x04,

    CHANNEL_STATUS_TIMEOUT_WAITING_WR_OR_REALIGNMENT_EVENT = 0x08
};

enum PULSED_POWER_API algorithm_id_t {
    FIR_LP = 0,
    FIR_BP,
    FIR_CUSTOM,
    FIR_CUSTOM_FFT,
    IIR_LP,
    IIR_HP,
    IIR_CUSTOM,
    AVERAGE
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_STATUS_H */
