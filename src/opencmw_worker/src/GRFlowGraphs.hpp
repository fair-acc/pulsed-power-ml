#ifndef GR_FLOWGRAPHS_HPP
#define GR_FLOWGRAPHS_HPP

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/divide.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/blocks/transcendental.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/top_block.h>

#include <gnuradio/pulsed_power/integration.h>
#include <gnuradio/pulsed_power/mains_frequency_calc.h>
#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/pulsed_power/picoscope_4000a_source.h>
#include <gnuradio/pulsed_power/power_calc_ff.h>
#include <gnuradio/pulsed_power/power_calc_mul_ph_ff.h>
#include <gnuradio/pulsed_power/statistics.h>

const float PI = 3.141592653589793238463f;

class GRFlowGraph {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraph(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // flowgraph setup
        const float samp_rate = 4'000.0f;
        // sinus_signal --> throttle --> opencmw_time_sink
        auto signal_source_0             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 0.5, 5, 0, 0);
        auto throttle_block_0            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_0 = gr::pulsed_power::opencmw_time_sink::make({ "sinus", "square" }, { "V", "A" }, samp_rate);
        pulsed_power_opencmw_sink_0->set_max_noutput_items(noutput_items);

        // saw_signal --> throttle --> opencmw_time_sink
        auto signal_source_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        auto throttle_block_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_1 = gr::pulsed_power::opencmw_time_sink::make({ "saw" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_1->set_max_noutput_items(noutput_items);

        // square_signal --> throttle --> opencmw_time_sink
        auto signal_source_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        auto throttle_block_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_2 = gr::pulsed_power::opencmw_time_sink::make({ "square" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_2->set_max_noutput_items(noutput_items);

        // sinus_signal --> throttle --> stream_to_vector --> fft --> fast_multiply_constant --> complex_to_mag^2 --> log10 --> opencmw_freq_sink
        const float  samp_rate_2                      = 32'000.0f;
        const size_t vec_length                       = 1024;
        const size_t fft_size                         = vec_length;
        const auto   bandwidth                        = samp_rate_2;
        auto         signal_source_3                  = gr::analog::sig_source_f::make(samp_rate_2, gr::analog::GR_SIN_WAVE, 3000.0f, 220.0);
        auto         throttle_block_3                 = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate_2, true);
        auto         stream_to_vector_0               = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vec_length);
        auto         fft_vxx_0                        = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(1024), true, 1);
        auto         multiply_const_xx_0              = gr::blocks::multiply_const_cc::make(1 / static_cast<float>(vec_length), vec_length);
        auto         complex_to_mag_squared_0         = gr::blocks::complex_to_mag_squared::make(vec_length);
        auto         nlog10_ff_0                      = gr::blocks::nlog10_ff::make(10, vec_length, 0);
        auto         pulsed_power_opencmw_freq_sink_0 = gr::pulsed_power::opencmw_freq_sink::make({ "sinus_fft" }, { "dB" }, samp_rate_2, bandwidth);

        // nilm worker (time and frequency sink)
        auto nilm_time_sink = gr::pulsed_power::opencmw_time_sink::make({ "P", "Q", "S", "Phi" }, { "W", "Var", "VA", "deg" }, samp_rate);
        nilm_time_sink->set_max_noutput_items(noutput_items);

        auto nilm_freq_sink = gr::pulsed_power::opencmw_freq_sink::make({ "S", "U", "I" }, { "dB", "dB", "dB" }, samp_rate, samp_rate);
        nilm_freq_sink->set_max_noutput_items(noutput_items);

        // connections
        // time-domain sinks
        top->hier_block2::connect(signal_source_0, 0, throttle_block_0, 0);
        top->hier_block2::connect(throttle_block_0, 0, pulsed_power_opencmw_sink_0, 0);

        top->hier_block2::connect(signal_source_1, 0, throttle_block_1, 0);
        top->hier_block2::connect(throttle_block_1, 0, pulsed_power_opencmw_sink_1, 0);

        top->hier_block2::connect(signal_source_2, 0, throttle_block_2, 0);
        top->hier_block2::connect(throttle_block_2, 0, pulsed_power_opencmw_sink_0, 1);

        // frequency-domain sinks
        top->hier_block2::connect(signal_source_3, 0, throttle_block_3, 0);
        top->hier_block2::connect(throttle_block_3, 0, stream_to_vector_0, 0);
        top->hier_block2::connect(stream_to_vector_0, 0, fft_vxx_0, 0);
        top->hier_block2::connect(fft_vxx_0, 0, multiply_const_xx_0, 0);
        top->hier_block2::connect(multiply_const_xx_0, 0, complex_to_mag_squared_0, 0);
        top->hier_block2::connect(complex_to_mag_squared_0, 0, nlog10_ff_0, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, pulsed_power_opencmw_freq_sink_0, 0);

        // nilm worker (time and frequency sink)
        top->hier_block2::connect(throttle_block_0, 0, nilm_time_sink, 0);
        top->hier_block2::connect(throttle_block_1, 0, nilm_time_sink, 1);
        top->hier_block2::connect(throttle_block_2, 0, nilm_time_sink, 2);
        top->hier_block2::connect(throttle_block_3, 0, nilm_time_sink, 3);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 1);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 2);
    }

    ~GRFlowGraph() { top->stop(); }

    void start() { top->start(); }
};

class PulsedPowerFlowgraph {
private:
    gr::top_block_sptr top;

public:
    PulsedPowerFlowgraph(int noutput_items, bool use_picoscope = false)
        : top(gr::make_top_block("GNURadio")) {
        float source_samp_rate          = 200'000.0f;
        auto  source_interface_voltage0 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current0 = gr::blocks::multiply_const_ff::make(1);
        if (use_picoscope) {
            // source_samp_rate                                                  = 2'000'000.0f;
            const float                           current_correction_factor   = 2.5f;
            const float                           voltage_correction_factor   = 100.0f;
            gr::pulsed_power::downsampling_mode_t picoscope_downsampling_mode = gr::pulsed_power::DOWNSAMPLING_MODE_NONE;
            gr::pulsed_power::coupling_t          picoscope_coupling          = gr::pulsed_power::AC_1M;

            // blocks
            auto picoscope_source = gr::pulsed_power::picoscope_4000a_source::make("", true);
            picoscope_source->set_trigger_once(false);
            picoscope_source->set_samp_rate(source_samp_rate);
            picoscope_source->set_downsampling(picoscope_downsampling_mode, 1);
            picoscope_source->set_aichan_a(true, 5, picoscope_coupling, 0.0);
            picoscope_source->set_aichan_b(true, 1, picoscope_coupling, 0.0);
            picoscope_source->set_aichan_c(false, 5.0, picoscope_coupling, 5.0);
            picoscope_source->set_aichan_d(false, 5.0, picoscope_coupling, 0.0);
            picoscope_source->set_aichan_e(false, 5, picoscope_coupling, 0.0);
            picoscope_source->set_aichan_f(false, 5, picoscope_coupling, 0.0);
            picoscope_source->set_aichan_g(false, 5.0, picoscope_coupling, 5.0);
            picoscope_source->set_aichan_h(false, 5.0, picoscope_coupling, 0.0);

            // mode = streaming
            picoscope_source->set_nr_buffers(64);
            picoscope_source->set_driver_buffer_size(102400);
            picoscope_source->set_streaming(0.0005);
            picoscope_source->set_buffer_size(204800);

            auto null_sink_picoscope = gr::blocks::null_sink::make(sizeof(float));
            auto voltage0            = gr::blocks::multiply_const_ff::make(voltage_correction_factor);
            auto current0            = gr::blocks::multiply_const_ff::make(current_correction_factor);

            // connections
            top->hier_block2::connect(picoscope_source, 0, voltage0, 0);
            top->hier_block2::connect(picoscope_source, 2, current0, 0);
            top->hier_block2::connect(picoscope_source, 1, null_sink_picoscope, 0);
            top->hier_block2::connect(picoscope_source, 8, null_sink_picoscope, 1);
            top->hier_block2::connect(picoscope_source, 3, null_sink_picoscope, 2);
            top->hier_block2::connect(picoscope_source, 4, null_sink_picoscope, 3);
            top->hier_block2::connect(picoscope_source, 5, null_sink_picoscope, 4);
            top->hier_block2::connect(picoscope_source, 6, null_sink_picoscope, 5);
            top->hier_block2::connect(picoscope_source, 7, null_sink_picoscope, 6);
            top->hier_block2::connect(picoscope_source, 9, null_sink_picoscope, 7);
            top->hier_block2::connect(picoscope_source, 10, null_sink_picoscope, 8);
            top->hier_block2::connect(picoscope_source, 11, null_sink_picoscope, 9);
            top->hier_block2::connect(picoscope_source, 12, null_sink_picoscope, 10);
            top->hier_block2::connect(picoscope_source, 13, null_sink_picoscope, 11);
            top->hier_block2::connect(picoscope_source, 14, null_sink_picoscope, 12);
            top->hier_block2::connect(picoscope_source, 15, null_sink_picoscope, 13);
            top->hier_block2::connect(voltage0, 0, source_interface_voltage0, 0);
            top->hier_block2::connect(current0, 0, source_interface_current0, 0);

        } else {
            // blocks
            auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
            auto analog_sig_source_current0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);  // I_raw

            auto voltage0                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            auto current0                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

            // connections
            top->hier_block2::connect(analog_sig_source_voltage0, 0, voltage0, 0);
            top->hier_block2::connect(analog_sig_source_current0, 0, current0, 0);
            top->hier_block2::connect(voltage0, 0, source_interface_voltage0, 0);
            top->hier_block2::connect(current0, 0, source_interface_current0, 0);
        }

        // parameters
        const float samp_rate_delta_phi_calc = 1'000.0f;
        // parameters band pass filter
        const int   decimation_bpf = static_cast<int>(source_samp_rate / samp_rate_delta_phi_calc);
        const float bpf_high_cut   = 80.0f;
        const float bpf_low_cut    = 20.0f;
        const float bpf_trans      = 1000.0f;
        // parameters low pass filter
        const int   decimation_lpf   = 1;
        const float lpf_in_samp_rate = samp_rate_delta_phi_calc;
        const float lpf_trans        = 10.0f;
        // parameters decimation
        const float out_samp_rate_ui                     = 1'000.0f;
        const float out_samp_rate_power_shortterm        = 100.0f;
        const float out_samp_rate_power_midterm          = 1.0f;
        const float out_samp_rate_power_longterm         = 1.0f / 60.0f;
        int         decimation_out_raw                   = static_cast<int>(source_samp_rate / out_samp_rate_ui);
        int         decimation_out_bpf                   = static_cast<int>(samp_rate_delta_phi_calc / out_samp_rate_ui);
        int         decimation_out_mains_freq_short_term = static_cast<int>(source_samp_rate / out_samp_rate_power_shortterm);
        int         decimation_out_mains_freq_mid_term   = static_cast<int>(source_samp_rate / out_samp_rate_power_midterm);
        int         decimation_out_mains_freq_long_term  = static_cast<int>(source_samp_rate / out_samp_rate_power_longterm);
        int         decimation_out_short_term            = static_cast<int>(samp_rate_delta_phi_calc / out_samp_rate_power_shortterm);
        int         decimation_out_mid_term              = static_cast<int>(samp_rate_delta_phi_calc / out_samp_rate_power_midterm);
        int         decimation_out_long_term             = static_cast<int>(samp_rate_delta_phi_calc / out_samp_rate_power_longterm);
        // parameters nilm
        const int fft_size    = 131072;
        size_t    vector_size = static_cast<size_t>(fft_size);
        float     bandwidth   = source_samp_rate;

        // blocks
        auto multiply_voltage_current_nilm = gr::blocks::multiply_ff::make(1);

        auto stream_to_vector_U            = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_U                         = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_U              = gr::blocks::complex_to_mag_squared::make(vector_size);

        auto stream_to_vector_I            = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_I                         = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_I              = gr::blocks::complex_to_mag_squared::make(vector_size);

        auto stream_to_vector_S            = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_S                         = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_S              = gr::blocks::complex_to_mag_squared::make(vector_size);

        auto multiply_voltage_current      = gr::blocks::multiply_ff::make(1);
        auto frequency_spec_one_in_n       = gr::blocks::keep_one_in_n::make(sizeof(float), 400);
        auto frequency_spec_low_pass       = gr::filter::fft_filter_fff::make(
                      10,
                      gr::filter::firdes::low_pass(
                              1,
                              500,
                              20,
                              100,
                              gr::fft::window::win_type::WIN_HAMMING,
                              6.76));
        auto frequency_spec_stream_to_vec  = gr::blocks::stream_to_vector::make(sizeof(float), 512);
        auto frequency_spec_fft            = gr::fft::fft_v<float, true>::make(512, gr::fft::window::rectangular(512), true, 1);
        auto frequency_multiply_const      = gr::blocks::multiply_const<gr_complex>::make(2.0 / (512.0), 512);
        auto frequency_spec_complex_to_mag = gr::blocks::complex_to_mag::make(512);

        auto calc_mains_frequency          = gr::pulsed_power::mains_frequency_calc::make(source_samp_rate, -100.0f, 100.0f);

        auto integrate                     = gr::pulsed_power::integration::make(10, 1000, gr::pulsed_power::INTEGRATION_DURATION::DAY);

        auto band_pass_filter_current0     = gr::filter::fft_filter_fff::make(
                    decimation_bpf,
                    gr::filter::firdes::band_pass(
                            1,
                            source_samp_rate,
                            bpf_low_cut,
                            bpf_high_cut,
                            bpf_trans,
                            gr::fft::window::win_type::WIN_HANN,
                            6.76));
        auto band_pass_filter_voltage0 = gr::filter::fft_filter_fff::make(
                decimation_bpf,
                gr::filter::firdes::band_pass(
                        1,
                        source_samp_rate,
                        bpf_low_cut,
                        bpf_high_cut,
                        bpf_trans,
                        gr::fft::window::win_type::WIN_HANN,
                        6.76));

        auto analog_sig_source_phase0_sin = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase0_cos = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

        auto blocks_multiply_phase0_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_3     = gr::blocks::multiply_ff::make(1);

        auto low_pass_filter_current0_0   = gr::filter::fft_filter_fff::make(
                  decimation_lpf,
                  gr::filter::firdes::low_pass(
                          1,
                          lpf_in_samp_rate,
                          60,
                          lpf_trans,
                          gr::fft::window::win_type::WIN_HAMMING,
                          6.76));
        auto low_pass_filter_current0_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_0 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase0_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase0_1                   = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase0_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase0_1           = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase0                        = gr::blocks::sub_ff::make(1);

        auto pulsed_power_power_calc_ff_0_0           = gr::pulsed_power::power_calc_ff::make(0.001);

        auto out_decimation_current0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_current_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);
        auto out_decimation_voltage_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);

        auto out_decimation_mains_frequency_shortterm = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_short_term);
        auto out_decimation_mains_frequency_midterm   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_mid_term);
        auto out_decimation_mains_frequency_longterm  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_long_term);

        auto out_decimation_p_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_q_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_s_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_phi_shortterm             = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_p_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_q_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_s_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_phi_midterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_p_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_q_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_s_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_phi_longterm              = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);

        auto statistics_p_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_q_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_s_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_phi_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_p_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_q_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_s_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_phi_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_p_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_q_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_s_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_phi_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);

        auto opencmw_time_sink_signals                = gr::pulsed_power::opencmw_time_sink::make(
                               { "U", "I", "U_bpf", "I_bpf" },
                               { "V", "A", "V", "A" },
                               out_samp_rate_ui);
        opencmw_time_sink_signals->set_max_noutput_items(noutput_items);

        // Mains frequency sinks
        auto opencmw_time_sink_mains_freq_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { "mains_freq" },
                { "Hz" },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_mains_freq_shortterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_mains_freq_midterm = gr::pulsed_power::opencmw_time_sink::make(
                { "mains_freq" },
                { "Hz" },
                out_samp_rate_power_midterm);
        opencmw_time_sink_mains_freq_midterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_mains_freq_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { "mains_freq" },
                { "Hz" },
                out_samp_rate_power_longterm);
        opencmw_time_sink_mains_freq_longterm->set_max_noutput_items(noutput_items);

        // Power sinks
        auto opencmw_time_sink_power_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "rad" },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_power_shortterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_midterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "rad" },
                out_samp_rate_power_midterm);
        opencmw_time_sink_power_midterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "rad" },
                out_samp_rate_power_longterm);
        opencmw_time_sink_power_longterm->set_max_noutput_items(noutput_items);

        // Integral sink
        auto opencmw_time_sink_int_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { "S_Int" },
                { "Wh" },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_int_shortterm->set_max_noutput_items(noutput_items);

        // Statistic sinks
        auto opencmw_time_sink_power_stats_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P_mean", "P_min", "P_max", "Q_mean", "Q_min", "Q_max", "S_mean", "S_min", "S_max", "phi_mean", "phi_min", "phi_max" },
                { "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_power_stats_shortterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_stats_midterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P_mean", "P_min", "P_max", "Q_mean", "Q_min", "Q_max", "S_mean", "S_min", "S_max", "phi_mean", "phi_min", "phi_max" },
                { "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" },
                out_samp_rate_power_midterm);
        opencmw_time_sink_power_stats_midterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_stats_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P_mean", "P_min", "P_max", "Q_mean", "Q_min", "Q_max", "S_mean", "S_min", "S_max", "phi_mean", "phi_min", "phi_max" },
                { "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" },
                out_samp_rate_power_longterm);
        opencmw_time_sink_power_stats_longterm->set_max_noutput_items(noutput_items);

        auto null_sink_stats_shortterm = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_stats_midterm   = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_stats_longterm  = gr::blocks::null_sink::make(sizeof(float));

        // Frequency spectra sinks
        auto frequency_spec_pulsed_power_opencmw_freq_sink = gr::pulsed_power::opencmw_freq_sink::make(
                { "sinus_fft" },
                { "W" }, 50, 50, 512);
        frequency_spec_pulsed_power_opencmw_freq_sink->set_max_noutput_items(noutput_items);
        auto opencmw_freq_sink_nilm_U = gr::pulsed_power::opencmw_freq_sink::make(
                { "U" },
                { "V" },
                source_samp_rate,
                bandwidth,
                vector_size);
        opencmw_freq_sink_nilm_U->set_max_noutput_items(noutput_items);
        auto opencmw_freq_sink_nilm_I = gr::pulsed_power::opencmw_freq_sink::make(
                { "I" },
                { "A" },
                source_samp_rate,
                bandwidth,
                vector_size);
        opencmw_freq_sink_nilm_I->set_max_noutput_items(noutput_items);
        auto opencmw_freq_sink_nilm_S = gr::pulsed_power::opencmw_freq_sink::make(
                { "S" },
                { "VA" },
                source_samp_rate,
                bandwidth,
                vector_size);
        opencmw_freq_sink_nilm_S->set_max_noutput_items(noutput_items);

        // Connections:
        // signal
        top->hier_block2::connect(source_interface_voltage0, 0, out_decimation_voltage0, 0);
        top->hier_block2::connect(source_interface_current0, 0, out_decimation_current0, 0);
        top->hier_block2::connect(out_decimation_voltage0, 0, opencmw_time_sink_signals, 0); // U_raw
        top->hier_block2::connect(out_decimation_current0, 0, opencmw_time_sink_signals, 1); // I_raw
        // Mains frequency
        top->hier_block2::connect(source_interface_voltage0, 0, calc_mains_frequency, 0);
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_shortterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_shortterm, 0, opencmw_time_sink_mains_freq_shortterm, 0); // mains_freq short-term
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_midterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_midterm, 0, opencmw_time_sink_mains_freq_midterm, 0); // mains_freq mid-term
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_longterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_longterm, 0, opencmw_time_sink_mains_freq_longterm, 0); // mains_freq long-term
        // Bandpass filter
        top->hier_block2::connect(source_interface_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(source_interface_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, opencmw_time_sink_signals, 2); // U_bpf
        top->hier_block2::connect(band_pass_filter_current0, 0, opencmw_time_sink_signals, 3); // I_bpf
        // Calculate phase shift
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_power_calc_ff_0_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_power_calc_ff_0_0, 1);
        top->hier_block2::connect(analog_sig_source_phase0_sin, 0, blocks_multiply_phase0_0, 1);
        top->hier_block2::connect(analog_sig_source_phase0_sin, 0, blocks_multiply_phase0_2, 1);
        top->hier_block2::connect(analog_sig_source_phase0_cos, 0, blocks_multiply_phase0_1, 1);
        top->hier_block2::connect(analog_sig_source_phase0_cos, 0, blocks_multiply_phase0_3, 1);
        top->hier_block2::connect(blocks_multiply_phase0_0, 0, low_pass_filter_voltage0_0, 0);
        top->hier_block2::connect(blocks_multiply_phase0_1, 0, low_pass_filter_voltage0_1, 0);
        top->hier_block2::connect(blocks_multiply_phase0_2, 0, low_pass_filter_current0_0, 0);
        top->hier_block2::connect(blocks_multiply_phase0_3, 0, low_pass_filter_current0_1, 0);
        top->hier_block2::connect(low_pass_filter_voltage0_0, 0, blocks_divide_phase0_0, 0);
        top->hier_block2::connect(low_pass_filter_voltage0_1, 0, blocks_divide_phase0_0, 1);
        top->hier_block2::connect(low_pass_filter_current0_0, 0, blocks_divide_phase0_1, 0);
        top->hier_block2::connect(low_pass_filter_current0_1, 0, blocks_divide_phase0_1, 1);
        top->hier_block2::connect(blocks_divide_phase0_0, 0, blocks_transcendental_phase0_0, 0);
        top->hier_block2::connect(blocks_divide_phase0_1, 0, blocks_transcendental_phase0_1, 0);
        top->hier_block2::connect(blocks_transcendental_phase0_0, 0, blocks_sub_phase0, 0);
        top->hier_block2::connect(blocks_transcendental_phase0_1, 0, blocks_sub_phase0, 1);
        top->hier_block2::connect(blocks_sub_phase0, 0, pulsed_power_power_calc_ff_0_0, 2);
        // Calculate P, Q, S, phi
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, out_decimation_p_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, out_decimation_q_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, out_decimation_s_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, out_decimation_phi_shortterm, 0);
        top->hier_block2::connect(out_decimation_p_shortterm, 0, opencmw_time_sink_power_shortterm, 0);   // P short-term
        top->hier_block2::connect(out_decimation_q_shortterm, 0, opencmw_time_sink_power_shortterm, 1);   // Q short-term
        top->hier_block2::connect(out_decimation_s_shortterm, 0, opencmw_time_sink_power_shortterm, 2);   // S short-term
        top->hier_block2::connect(out_decimation_phi_shortterm, 0, opencmw_time_sink_power_shortterm, 3); // phi short-term
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, out_decimation_p_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, out_decimation_q_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, out_decimation_s_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, out_decimation_phi_midterm, 0);
        top->hier_block2::connect(out_decimation_p_midterm, 0, opencmw_time_sink_power_midterm, 0);   // P mid-term
        top->hier_block2::connect(out_decimation_q_midterm, 0, opencmw_time_sink_power_midterm, 1);   // Q mid-term
        top->hier_block2::connect(out_decimation_s_midterm, 0, opencmw_time_sink_power_midterm, 2);   // S mid-term
        top->hier_block2::connect(out_decimation_phi_midterm, 0, opencmw_time_sink_power_midterm, 3); // phi mid-term
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, out_decimation_p_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, out_decimation_q_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, out_decimation_s_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, out_decimation_phi_longterm, 0);
        top->hier_block2::connect(out_decimation_p_longterm, 0, opencmw_time_sink_power_longterm, 0);   // P long-term
        top->hier_block2::connect(out_decimation_q_longterm, 0, opencmw_time_sink_power_longterm, 1);   // Q long-term
        top->hier_block2::connect(out_decimation_s_longterm, 0, opencmw_time_sink_power_longterm, 2);   // S long-term
        top->hier_block2::connect(out_decimation_phi_longterm, 0, opencmw_time_sink_power_longterm, 3); // phi long-term
                                                                                                        // integral S
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, integrate, 0);
        top->hier_block2::connect(integrate, 0, opencmw_time_sink_int_shortterm, 0); // int S short-term
        // Statistics
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, statistics_p_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, statistics_q_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, statistics_s_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, statistics_phi_shortterm, 0);
        top->hier_block2::connect(statistics_p_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 0);    // P_mean short-term
        top->hier_block2::connect(statistics_p_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 1);    // P_min short-term
        top->hier_block2::connect(statistics_p_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 2);    // P_max short-term
        top->hier_block2::connect(statistics_p_shortterm, 3, null_sink_stats_shortterm, 0);                  // P_std_dev short-term
        top->hier_block2::connect(statistics_q_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 3);    // Q_mean short-term
        top->hier_block2::connect(statistics_q_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 4);    // Q_min short-term
        top->hier_block2::connect(statistics_q_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 5);    // Q_max short-term
        top->hier_block2::connect(statistics_q_shortterm, 3, null_sink_stats_shortterm, 1);                  // Q_std_dev short-term
        top->hier_block2::connect(statistics_s_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 6);    // S_mean short-term
        top->hier_block2::connect(statistics_s_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 7);    // S_min short-term
        top->hier_block2::connect(statistics_s_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 8);    // S_max short-term
        top->hier_block2::connect(statistics_s_shortterm, 3, null_sink_stats_shortterm, 2);                  // S_std_dev short-term
        top->hier_block2::connect(statistics_phi_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 9);  // phi_mean short-term
        top->hier_block2::connect(statistics_phi_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 10); // phi_min short-term
        top->hier_block2::connect(statistics_phi_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 11); // phi_max short-term
        top->hier_block2::connect(statistics_phi_shortterm, 3, null_sink_stats_shortterm, 3);                // phi_std_dev short-term
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, statistics_p_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, statistics_q_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, statistics_s_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, statistics_phi_midterm, 0);
        top->hier_block2::connect(statistics_p_midterm, 0, opencmw_time_sink_power_stats_midterm, 0);    // P_mean mid-term
        top->hier_block2::connect(statistics_p_midterm, 1, opencmw_time_sink_power_stats_midterm, 1);    // P_min mid-term
        top->hier_block2::connect(statistics_p_midterm, 2, opencmw_time_sink_power_stats_midterm, 2);    // P_max mid-term
        top->hier_block2::connect(statistics_p_midterm, 3, null_sink_stats_midterm, 0);                  // P_std_dev mid-term
        top->hier_block2::connect(statistics_q_midterm, 0, opencmw_time_sink_power_stats_midterm, 3);    // Q_mean mid-term
        top->hier_block2::connect(statistics_q_midterm, 1, opencmw_time_sink_power_stats_midterm, 4);    // Q_min mid-term
        top->hier_block2::connect(statistics_q_midterm, 2, opencmw_time_sink_power_stats_midterm, 5);    // Q_max mid-term
        top->hier_block2::connect(statistics_q_midterm, 3, null_sink_stats_midterm, 1);                  // Q_std_dev mid-term
        top->hier_block2::connect(statistics_s_midterm, 0, opencmw_time_sink_power_stats_midterm, 6);    // S_mean mid-term
        top->hier_block2::connect(statistics_s_midterm, 1, opencmw_time_sink_power_stats_midterm, 7);    // S_min mid-term
        top->hier_block2::connect(statistics_s_midterm, 2, opencmw_time_sink_power_stats_midterm, 8);    // S_max mid-term
        top->hier_block2::connect(statistics_s_midterm, 3, null_sink_stats_midterm, 2);                  // S_std_dev mid-term
        top->hier_block2::connect(statistics_phi_midterm, 0, opencmw_time_sink_power_stats_midterm, 9);  // phi_mean mid-term
        top->hier_block2::connect(statistics_phi_midterm, 1, opencmw_time_sink_power_stats_midterm, 10); // phi_min mid-term
        top->hier_block2::connect(statistics_phi_midterm, 2, opencmw_time_sink_power_stats_midterm, 11); // phi_max mid-term
        top->hier_block2::connect(statistics_phi_midterm, 3, null_sink_stats_midterm, 3);                // phi_std_dev mid-term
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, statistics_p_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, statistics_q_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, statistics_s_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, statistics_phi_longterm, 0);
        top->hier_block2::connect(statistics_p_longterm, 0, opencmw_time_sink_power_stats_longterm, 0);    // P_mean long-term
        top->hier_block2::connect(statistics_p_longterm, 1, opencmw_time_sink_power_stats_longterm, 1);    // P_min long-term
        top->hier_block2::connect(statistics_p_longterm, 2, opencmw_time_sink_power_stats_longterm, 2);    // P_max long-term
        top->hier_block2::connect(statistics_p_longterm, 3, null_sink_stats_longterm, 0);                  // P_std_dev long-term
        top->hier_block2::connect(statistics_q_longterm, 0, opencmw_time_sink_power_stats_longterm, 3);    // Q_mean long-term
        top->hier_block2::connect(statistics_q_longterm, 1, opencmw_time_sink_power_stats_longterm, 4);    // Q_min long-term
        top->hier_block2::connect(statistics_q_longterm, 2, opencmw_time_sink_power_stats_longterm, 5);    // Q_max long-term
        top->hier_block2::connect(statistics_q_longterm, 3, null_sink_stats_longterm, 1);                  // Q_std_dev long-term
        top->hier_block2::connect(statistics_s_longterm, 0, opencmw_time_sink_power_stats_longterm, 6);    // S_mean long-term
        top->hier_block2::connect(statistics_s_longterm, 1, opencmw_time_sink_power_stats_longterm, 7);    // S_min long-term
        top->hier_block2::connect(statistics_s_longterm, 2, opencmw_time_sink_power_stats_longterm, 8);    // S_max long-term
        top->hier_block2::connect(statistics_s_longterm, 3, null_sink_stats_longterm, 2);                  // S_std_dev long-term
        top->hier_block2::connect(statistics_phi_longterm, 0, opencmw_time_sink_power_stats_longterm, 9);  // phi_mean long-term
        top->hier_block2::connect(statistics_phi_longterm, 1, opencmw_time_sink_power_stats_longterm, 10); // phi_min long-term
        top->hier_block2::connect(statistics_phi_longterm, 2, opencmw_time_sink_power_stats_longterm, 11); // phi_max long-term
        top->hier_block2::connect(statistics_phi_longterm, 3, null_sink_stats_longterm, 3);                // phi_std_dev long-term
        // Frequency spectras
        top->hier_block2::connect(source_interface_voltage0, 0, stream_to_vector_U, 0);
        top->hier_block2::connect(stream_to_vector_U, 0, fft_U, 0);
        top->hier_block2::connect(fft_U, 0, complex_to_mag_U, 0);
        top->hier_block2::connect(complex_to_mag_U, 0, opencmw_freq_sink_nilm_U, 0); // freq_spectra voltage
        top->hier_block2::connect(source_interface_current0, 0, stream_to_vector_I, 0);
        top->hier_block2::connect(stream_to_vector_I, 0, fft_I, 0);
        top->hier_block2::connect(fft_I, 0, complex_to_mag_I, 0);
        top->hier_block2::connect(complex_to_mag_I, 0, opencmw_freq_sink_nilm_I, 0); // freq_spectra current
        top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current_nilm, 0);
        top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current_nilm, 1);
        top->hier_block2::connect(multiply_voltage_current_nilm, 0, stream_to_vector_S, 0);
        top->hier_block2::connect(stream_to_vector_S, 0, fft_S, 0);
        top->hier_block2::connect(fft_S, 0, complex_to_mag_S, 0);
        top->hier_block2::connect(complex_to_mag_S, 0, opencmw_freq_sink_nilm_S, 0); // freq_spectra apparent power (nilm)
        top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current, 0);
        top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current, 1);
        top->hier_block2::connect(multiply_voltage_current, 0, frequency_spec_one_in_n, 0);
        top->hier_block2::connect(frequency_spec_one_in_n, 0, frequency_spec_low_pass, 0);
        top->hier_block2::connect(frequency_spec_low_pass, 0, frequency_spec_stream_to_vec, 0);
        top->hier_block2::connect(frequency_spec_stream_to_vec, 0, frequency_spec_fft, 0);
        top->hier_block2::connect(frequency_spec_fft, 0, frequency_multiply_const, 0);
        top->hier_block2::connect(frequency_multiply_const, 0, frequency_spec_complex_to_mag, 0);
        top->hier_block2::connect(frequency_spec_complex_to_mag, 0, frequency_spec_pulsed_power_opencmw_freq_sink, 0); // freq_spectra apparent power
    }
    ~PulsedPowerFlowgraph() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

#endif /* GR_FLOWGRAPHS_HPP */
