/* -*- c++ -*- */
/*
 * Copyright 2022 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "integration_impl.h"
#include <gnuradio/io_signature.h>
#include <fstream>
#include <iostream>
#include <fstream>

namespace gr {
namespace pulsed_power {

using input_type = float;
using output_type = float;
integration::sptr integration::make(int decimation, int sample_rate)
{
    return gnuradio::make_block_sptr<integration_impl>(decimation, sample_rate);
}


/*
 * The private constructor
 */
  integration_impl::integration_impl(int decimation, int sample_rate)
    : gr::sync_decimator("integration",
                gr::io_signature::make(
                    1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                gr::io_signature::make(
		    1 /* min outputs */, 1 /*max outputs */, sizeof(output_type)),
	   decimation)
{
    d_decimation = decimation;
    d_step_size = 1.0/sample_rate;
}

/*
 * Our virtual destructor.
 */
integration_impl::~integration_impl() {}

int integration_impl::work(int noutput_items,
			   gr_vector_const_void_star& input_items,
			   gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    add_new_steps(out, in, noutput_items);


    // Tell runtime system how many output items we produced.
    return noutput_items;
}

void integration_impl::add_new_steps(float* out, const float* sample, int number_of_calculations) {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    bool calculate_with_last_value = true;
    bool rewrite_save_file;
    // new startup
    if (last_save.time_since_epoch() == std::chrono::seconds(0) ||
        last_reset.time_since_epoch() == std::chrono::seconds(0)) {
        calculate_with_last_value = false;
        int result = get_values_from_file(last_reset, last_save, sum);
        if (result == -1) {
            std::cout << "Failed to read file - write a new one" << std::endl;
            rewrite_save_file = true;
        }
    }

    // write current sum to file every 10 minutes
    if (now - last_save > std::chrono::minutes(10)) {
        rewrite_save_file = true;
	last_save = now;
    }
    // reset every 30 days (also if no vlaues from file?)
    if (now - last_reset > std::chrono::hours(24 * 30)) {
        rewrite_save_file = true;
	last_save = now;
	last_reset = now;
	sum = 0;
    }

    for (int i = 0; i < number_of_calculations; i++) {
        float int_value;
	integrate(int_value, &sample[i*d_decimation], d_decimation, calculate_with_last_value);
	sum = sum + int_value;
	out[i] = sum;
	last_value = sample[((i+1)*d_decimation)-1];
	calculate_with_last_value = true;
    }

    if (rewrite_save_file) {
        int result = write_to_file(last_reset, last_save, sum);
	if (result == -1) {
	    std::cout << "Failed to write to file" << std::endl;
	}
    }
}

int integration_impl::get_values_from_file(std::chrono::time_point<std::chrono::system_clock>& last_reset, std::chrono::time_point<std::chrono::system_clock>& last_save, float& sum) {
    try {
        long last_reset_duration;
        long last_save_duration;
        std::ifstream read_stream;
        read_stream.open("t.txt", std::ifstream::in);
        if (!read_stream.is_open()) {
            return -1;
        }
        read_stream >> last_reset_duration;
        read_stream >> last_save_duration;
        read_stream >> sum;
        read_stream.close();
        std::chrono::seconds last_reset_seconds(last_reset_duration);
        std::chrono::seconds last_save_seconds(last_reset_duration);
        std::chrono::time_point<std::chrono::system_clock> last_reset_obj(
            last_reset_seconds);
        std::chrono::time_point<std::chrono::system_clock> last_save_obj(
            last_save_seconds);
        last_reset = last_reset_obj;
        last_save = last_save_obj;
        return 0;
    } catch (...) {
        return -1;
    }
}

int integration_impl::write_to_file(
    std::chrono::time_point<std::chrono::system_clock> last_reset,
    std::chrono::time_point<std::chrono::system_clock> last_save,
    float sum)
{
    try {
        auto last_reset_s =
            std::chrono::time_point_cast<std::chrono::seconds>(last_reset);
        auto last_save_s = std::chrono::time_point_cast<std::chrono::seconds>(last_save);
        long last_reset_duration = last_reset_s.time_since_epoch().count();
        long last_save_duration = last_save_s.time_since_epoch().count();
        std::ofstream write_stream;
        write_stream.open("t.txt", std::ofstream::out | std::ofstream::trunc);
        if (!write_stream.is_open()) {
            return -1;
        }
        write_stream << last_reset_duration << '\n';
        write_stream << last_save_duration << '\n';
        write_stream << sum;
        return 0;
    } catch (...) {
        return -1;
    }
}

void integration_impl::integrate(float& out, const float* sample, int n_samples, bool calculate_with_last_value)
{
    double value = 0;
    if (calculate_with_last_value) {
        value += d_step_size * ((last_value + sample[0]) / 2);
    }
    for (int i = 1; i < n_samples; i++) {
        value += d_step_size * ((sample[i - 1] + sample[i]) / 2);
    }
    out = value;
}

} /* namespace pulsed_power */
} /* namespace gr */
