/* -*- c++ -*- */
/* Copyright (C) 2018 GSI Darmstadt, Germany - All Rights Reserved
 * co-developed with: Cosylab, Ljubljana, Slovenia and CERN, Geneva, Switzerland
 * You may use, distribute and modify this code under the terms of the GPL v.3  license.
 */


#ifndef INCLUDED_PULSED_POWER_UTILS_H
#define INCLUDED_PULSED_POWER_UTILS_H

// Digitizer
#include "tags.h"

// boost
#include <boost/call_traits.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/pthread/condition_variable.hpp>
#include <boost/thread/pthread/mutex.hpp>

// GNU Radio
#include <gnuradio/tags.h>

// Build-in
#include <math.h>
#include <pmt/pmt.h>
#include <system_error>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <list>
#include <queue>

namespace gr {
namespace pulsed_power {

inline uint64_t get_timestamp_nano_utc()
{
    timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    return (start_time.tv_sec * 1000000000) + (start_time.tv_nsec);
}

inline uint64_t get_timestamp_milli_utc()
{
    return uint64_t(get_timestamp_nano_utc() / 1000000);
}

/*!
 * \brief Converts an integer value to hex (string).
 */
inline std::string to_hex_string(int value)
{
    std::stringstream sstream;
    sstream << "0x" << std::setfill('0') << std::setw(2) << std::hex << (int)value;
    return sstream.str();
}

/*!
 * \brief Converts error code to string message.
 */
inline std::string to_string(std::error_code ec) { return ec.message(); }

template <typename T>
class concurrent_queue
{
private:
    mutable boost::mutex mut;
    std::queue<T> data_queue;
    boost::condition_variable data_cond;

public:
    concurrent_queue() {}

    void push(T new_value)
    {
        {
            boost::lock_guard<boost::mutex> lg(mut);
            data_queue.push(new_value);
        }
        data_cond.notify_one();
    }

    /*!
     *\returns false if no data is available in time, otherwiese true.
     */
    bool wait_and_pop(T& value, std::chrono::milliseconds timeout)
    {

        boost::unique_lock<boost::mutex> lk(mut);
        auto retval =
            data_cond.wait_for(lk, timeout, [this] { return !data_queue.empty(); });
        if (retval) { // true is returned if condition evaluates to true...
            value = data_queue.front();
            data_queue.pop();
        }

        return retval;
    }

    /*!
     *\returns false if no data is available, otherwiese true.
     */
    bool pop(T& value)
    {
        boost::unique_lock<boost::mutex> lk(mut);

        if (data_queue.size() == 0) {
            return false;
        }

        value = data_queue.front();
        data_queue.pop();

        return true;
    }

    bool empty() const
    {
        boost::lock_guard<boost::mutex> lk(mut);
        return data_queue.empty();
    }

    void clear()
    {
        boost::lock_guard<boost::mutex> lk(mut);
        while (!data_queue.empty()) {
            data_queue.pop();
        }
    }

    size_t size() const
    {
        boost::lock_guard<boost::mutex> lk(mut);
        return data_queue.size();
    }
};

template <class T>
class circular_buffer
{
public:
    typedef boost::circular_buffer<T> container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename boost::call_traits<value_type>::param_type param_type;

    explicit circular_buffer(size_type capacity) : m_missed(0), m_container(capacity) {}

    size_type size() const { return m_container.size(); }

    size_type capacity() const { return m_container.capacity(); }

    uint64_t missed_count() const { return m_missed; }

    const_iterator begin() { return m_container.begin(); }

    const_iterator end() { return m_container.end(); }

    void push_back(const value_type* items, size_type nitems)
    {
        for (size_type i = 0; i < nitems; i++) {
            if (m_container.full()) {
                m_missed++;
            }

            m_container.push_back(items[i]);
        }
    }

    void pop_front(value_type* pItem, size_type nitems)
    {
        assert(m_container.size() >= nitems);

        for (size_type i = 0; i < nitems; i++) {
            pItem[i] = m_container.front();
            m_container.pop_front();
        }
    }

    void pop_front() { m_container.pop_front(); }

private:
    circular_buffer(const circular_buffer&);            // Disabled copy constructor.
    circular_buffer& operator=(const circular_buffer&); // Disabled assign operator.

    uint64_t m_missed;
    container_type m_container;
};

inline gr::tag_t make_peak_info_tag(double frequency, double stdev)
{
    gr::tag_t tag;
    tag.key = pmt::intern("peak_info");
    tag.value = pmt::make_tuple(pmt::from_double(frequency), pmt::from_double(stdev));
    return tag;
}

inline void decode_peak_info_tag(const gr::tag_t& tag, double& frequency, double& stdev)
{
    assert(pmt::symbol_to_string(tag.key) == "peak_info");

    auto values = pmt::to_tuple(tag.value);
    frequency = pmt::to_double(tuple_ref(values, 0));
    stdev = pmt::to_double(tuple_ref(values, 1));
}

static const double fwhm2stdev = 0.5 / sqrt(2 * log(2));
static const double whm2stdev = 2.0 * fwhm2stdev;


template <typename T>
class median_filter
{
private:
    std::queue<T> vals;
    std::list<T> ord_vals;
    int num;
    int middle;

public:
    median_filter(int n) : num(n), middle(n / 2)
    {
        // fill wit zeroes as starter.
        for (int i = 0; i < num; i++) {
            vals.push(0.0);
            ord_vals.push_back(0.0);
        }
    }

    T add(T new_el)
    {
        // track sample chronological order and remove oldest one
        vals.push(new_el);
        float oldest = vals.front();
        vals.pop();

        // remove from ordered list the oldest value
        for (auto it = ord_vals.begin(); it != ord_vals.end(); ++it) {
            if (*it == oldest) {
                ord_vals.erase(it);
                break;
            }
        }
        // add to the ordered list by insertion
        bool added_in = false;
        for (auto it = ord_vals.begin(); it != ord_vals.end(); ++it) {
            if (*it <= new_el) {
                ord_vals.insert(it, new_el);
                added_in = true;
                break;
            }
        }
        // the value hasn't been inserted into the list.
        // add it to end, i.e. biggest sample in this window.
        if (!added_in) {
            ord_vals.push_back(new_el);
        }

        // middle value of the new ordered list is
        // the median of the last n samples
        auto mean_val = ord_vals.begin();
        std::advance(mean_val, middle);
        return *mean_val;
    }
};

template <typename T>
class average_filter
{
private:
    std::queue<T> vals;
    int num;
    T running_avg;
    int iterations_to_fixing;

    // fixes small errors that may occur in average estimation
    bool fix_runinng_average()
    {
        iterations_to_fixing++;
        if (iterations_to_fixing == 100000) {
            // calculate a fresh average
            running_avg = 0.0;
            for (int i = 0; i < num; i++) {
                // iteration through a queue.
                double val = vals.front();
                vals.pop();
                vals.push(val);

                // sum up all elements in queue
                running_avg += val;
            }
            // average the sum
            running_avg /= num;

            // prepare for new estimations
            iterations_to_fixing = 0;
            return true;
        } else {
            return false;
        }
    }

public:
    average_filter(int n) : num(n), running_avg(0.0), iterations_to_fixing(0)
    {
        for (int i = 0; i < num; i++) {
            vals.push(0.0);
        }
        fix_runinng_average();
    }

    T add(T val)
    {
        float old_el = vals.front();
        vals.pop();
        vals.push(val);

        // if running average has not been freshly calculated,
        // estimate it
        if (!fix_runinng_average()) {
            running_avg = (num * running_avg - old_el + val) / num;
        }

        return running_avg;
    }

    T get_avg_value() const { return running_avg; }

    size_t size() const { return vals.size(); }
};


template <typename T>
class measurement_buffer_t
{
public:
    using data_chunk_sptr = boost::shared_ptr<T>;

    measurement_buffer_t(std::size_t num_buffers)
        : d_free_data_chunks(num_buffers), d_data_chunks(num_buffers)
    {
    }

    void add_measurement(const data_chunk_sptr& data_chunk)
    {
        boost::mutex::scoped_lock lock(d_mutex);
        d_data_chunks.push_back(data_chunk);
    }

    void return_free_buffer(const data_chunk_sptr& data_chunk)
    {
        boost::mutex::scoped_lock lock(d_mutex);
        d_free_data_chunks.push_back(data_chunk);
    }

    data_chunk_sptr get_free_buffer()
    {
        boost::mutex::scoped_lock lock(d_mutex);

        if (d_free_data_chunks.empty()) {
            return nullptr;
        } else {
            auto ptr = d_free_data_chunks.back();
            d_free_data_chunks.pop_back();
            return ptr;
        }
    }

    data_chunk_sptr get_measurement()
    {
        boost::unique_lock<boost::mutex> lock(d_mutex);
        if (d_data_chunks.empty()) {
            return nullptr;
        }
        // get the oldest data chunk
        auto sptr = d_data_chunks.front();
        d_data_chunks.pop_front();

        return sptr;
    }

private:
    // For simplify we use circular buffers for managing free pool of data chunks
    boost::circular_buffer<data_chunk_sptr> d_free_data_chunks;
    boost::circular_buffer<data_chunk_sptr> d_data_chunks;
    boost::mutex d_mutex;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_UTILS_H */
