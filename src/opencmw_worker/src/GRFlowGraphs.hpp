#ifndef GR_FLOWGRAPHS_HPP
#define GR_FLOWGRAPHS_HPP

#include <gnuradio/analog/noise_source.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/add_blk.h>
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
    explicit GRFlowGraph(int noutput_items)
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
    PulsedPowerFlowgraph(int noutput_items, bool use_picoscope = false, bool add_noise = false)
        : top(gr::make_top_block("GNURadio")) {
        float source_samp_rate          = 2'000'000.0f;
        auto  source_interface_voltage0 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current0 = gr::blocks::multiply_const_ff::make(1);
        // signal sources phase 1, 2
        auto  source_interface_voltage1 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current1 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_voltage2 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current2 = gr::blocks::multiply_const_ff::make(1);
        if (use_picoscope) {
                // picoscope part previously (not relevant at the moment)
                return;
        } else { 
        // U, I signal simulation
            if (add_noise) {
                //I
                auto analog_sig_source_voltage0        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
                //u
                auto analog_sig_source_current0        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 5, 0, 0.2f);  // I_raw

                
                auto analog_sig_source_freq_modulation = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 2, 1, 0, 0.2f);

                auto noise_source_current0             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 0.25f);
                auto noise_source_voltage0             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 16.25f);

                auto throttle_voltage0                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current0                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_freq_modulation          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_voltage0           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_current0           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                auto multiply_freq_modulation          = gr::blocks::multiply_ff::make();

                auto add_noise_current0                = gr::blocks::add_ff::make(1);
                auto add_noise_voltage0                = gr::blocks::add_ff::make(1);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage0, 0, throttle_voltage0, 0);
                top->hier_block2::connect(analog_sig_source_current0, 0, throttle_current0, 0);
                top->hier_block2::connect(analog_sig_source_freq_modulation, 0, throttle_freq_modulation, 0);
                top->hier_block2::connect(noise_source_voltage0, 0, throttle_noise_voltage0, 0);
                top->hier_block2::connect(noise_source_current0, 0, throttle_noise_current0, 0);
                // multiply frequency modulation
                top->hier_block2::connect(throttle_current0, 0, multiply_freq_modulation, 0);
                top->hier_block2::connect(throttle_freq_modulation, 0, multiply_freq_modulation, 1);
                // add noise
                top->hier_block2::connect(throttle_voltage0, 0, add_noise_voltage0, 0);
                top->hier_block2::connect(throttle_noise_voltage0, 0, add_noise_voltage0, 1);
                top->hier_block2::connect(multiply_freq_modulation, 0, add_noise_current0, 0);
                top->hier_block2::connect(throttle_noise_current0, 0, add_noise_current0, 1);
                // connect to interface
                top->hier_block2::connect(add_noise_voltage0, 0, source_interface_voltage0, 0);
                top->hier_block2::connect(add_noise_current0, 0, source_interface_current0, 0);
            } else {
                // blocks

                //--SIGNAL SOURCE PHASE 0--//
                //change for testing of different phases etc.
                //                                                                                                        Freq, Amplitude
                auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
                auto analog_sig_source_current0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 1.57f);  // I_raw
                //--END SIGNAL SOURCES PHASE 0--//

                auto throttle_voltage0          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current0          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage0, 0, throttle_voltage0, 0);
                top->hier_block2::connect(analog_sig_source_current0, 0, throttle_current0, 0);
                // connect to interface
                top->hier_block2::connect(throttle_voltage0, 0, source_interface_voltage0, 0);
                top->hier_block2::connect(throttle_current0, 0, source_interface_current0, 0);
            }
            // U, I simulation phase 2, 3
            phase1_signal_simulation(add_noise, source_samp_rate, source_interface_voltage1, source_interface_current1);
            phase2_signal_simulation(add_noise, source_samp_rate, source_interface_voltage2, source_interface_current2);
        }

        // parameters
        const float samp_rate_delta_phi_calc = 1'000.0f;
        // parameters decimation
        const float out_samp_rate_ui                     = 1'000.0f;
        const float out_samp_rate_power_shortterm        = 100.0f;
        const float out_samp_rate_power_midterm          = 1.0f;
        const float out_samp_rate_power_longterm         = 1.0f / 60.0f;
        int         decimation_out_raw                   = static_cast<int>(roundf(source_samp_rate / out_samp_rate_ui));
        int         decimation_out_bpf                   = static_cast<int>(roundf(source_samp_rate / out_samp_rate_ui));
        int         decimation_out_mains_freq_short_term = static_cast<int>(roundf(source_samp_rate / out_samp_rate_power_shortterm));
        int         decimation_out_mains_freq_mid_term   = static_cast<int>(roundf(source_samp_rate / out_samp_rate_power_midterm));
        int         decimation_out_mains_freq_long_term  = static_cast<int>(roundf(source_samp_rate / out_samp_rate_power_longterm));
        int         decimation_out_short_term            = static_cast<int>(roundf(samp_rate_delta_phi_calc / out_samp_rate_power_shortterm));
        int         decimation_out_mid_term              = static_cast<int>(roundf(samp_rate_delta_phi_calc / out_samp_rate_power_midterm));
        int         decimation_out_long_term             = static_cast<int>(roundf(samp_rate_delta_phi_calc / out_samp_rate_power_longterm));
        // parameters band pass filter
        const int   decimation_bpf            = static_cast<int>(roundf(source_samp_rate / out_samp_rate_ui));
        const float bpf_high_cut              = 80.0f;
        const float bpf_low_cut               = 20.0f;
        const float bpf_trans                 = 1000.0f;
        const int   decimation_delta_phi_calc = static_cast<int>(roundf(out_samp_rate_ui / samp_rate_delta_phi_calc));
        // parameters low pass filter
        const int   decimation_lpf   = 1;
        const float lpf_in_samp_rate = samp_rate_delta_phi_calc;
        const float lpf_trans        = 10.0f;
        // parameters frequency spectra
        const int fft_size_ppem        = 512;
        size_t    fft_vector_size_ppem = static_cast<size_t>(fft_size_ppem);
        const int fft_size_nilm        = 131072;
        //size_t    fft_vector_size_nilm = static_cast<size_t>(fft_size_nilm);
        //float     bandwidth_nilm       = source_samp_rate;

        auto multiply_voltage_current = gr::blocks::multiply_ff::make(1);
        auto frequency_spec_one_in_n  = gr::blocks::keep_one_in_n::make(sizeof(float), 4000);
        auto frequency_spec_low_pass  = gr::filter::fft_filter_fff::make(
                10,
                gr::filter::firdes::low_pass(
                        1,
                        500,
                        20,
                        100,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto frequency_spec_stream_to_vec  = gr::blocks::stream_to_vector::make(sizeof(float), fft_size_ppem);
        auto frequency_spec_fft            = gr::fft::fft_v<float, true>::make(fft_size_ppem, gr::fft::window::rectangular(fft_size_ppem), true, 1);
        auto frequency_multiply_const      = gr::blocks::multiply_const<gr_complex>::make(2.0 / static_cast<double>(fft_size_ppem), fft_vector_size_ppem);
        auto frequency_spec_complex_to_mag = gr::blocks::complex_to_mag::make(fft_vector_size_ppem);

        auto calc_mains_frequency          = gr::pulsed_power::mains_frequency_calc::make(source_samp_rate, -100.0f, 100.0f);

        auto integrate_S_day               = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::DAY, "SDay.txt");
        auto integrate_S_week              = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::WEEK, "SWeek.txt");
        auto integrate_S_month             = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::MONTH, "SMonth.txt");
        auto integrate_P_day               = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::DAY, "PDay.txt");
        auto integrate_P_week              = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::WEEK, "PWeek.txt");
        auto integrate_P_month             = gr::pulsed_power::integration::make(1000, 1000, gr::pulsed_power::INTEGRATION_DURATION::MONTH, "PMonth.txt");
        // Filter for phi phase calculation
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
         auto band_pass_filter_current1     = gr::filter::fft_filter_fff::make(
                    decimation_bpf,
                    gr::filter::firdes::band_pass(
                            1,
                            source_samp_rate,
                            bpf_low_cut,
                            bpf_high_cut,
                            bpf_trans,
                            gr::fft::window::win_type::WIN_HANN,
                            6.76));
         auto band_pass_filter_current2     = gr::filter::fft_filter_fff::make(
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
        auto band_pass_filter_voltage1 = gr::filter::fft_filter_fff::make(
                decimation_bpf,
                gr::filter::firdes::band_pass(
                        1,
                        source_samp_rate,
                        bpf_low_cut,
                        bpf_high_cut,
                        bpf_trans,
                        gr::fft::window::win_type::WIN_HANN,
                        6.76));
        auto band_pass_filter_voltage2 = gr::filter::fft_filter_fff::make(
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

        // Three phase block
        auto pulsed_power_calc_mul_ph_ff              = gr::pulsed_power::power_calc_mul_ph_ff::make(0.001);

        // Statistics
        auto out_decimation_current0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        // auto out_decimation_current_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);
        // auto out_decimation_voltage_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);
        auto out_decimation_mains_frequency_shortterm = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_short_term);
        auto out_decimation_mains_frequency_midterm   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_mid_term);
        auto out_decimation_mains_frequency_longterm  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_long_term);

        auto decimation_block_current_bpf0            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);
        auto decimation_block_voltage_bpf0            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);

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

        // Statistics for accumulated P, Q, S
        auto opencmw_time_sink_signals                = gr::pulsed_power::opencmw_time_sink::make(
                               { "U", "I", "U_bpf", "I_bpf" },
                               { "V", "A", "V", "A" },
                               out_samp_rate_ui);
        opencmw_time_sink_signals->set_max_noutput_items(noutput_items);
        // Mains frequency sinks, only for phase 0
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
        // Reinserted longterm power sinks (no data came through)
        auto opencmw_time_sink_power_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "rad" },
                out_samp_rate_power_longterm);
        opencmw_time_sink_power_longterm->set_max_noutput_items(noutput_items);
        // Integral sinks
        auto opencmw_time_sink_int_day = gr::pulsed_power::opencmw_time_sink::make(
                { "P_Int_Day", "S_Int_Day" },
                { "Wh", "VAh" },
                1.0f);
        opencmw_time_sink_int_day->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_int_week = gr::pulsed_power::opencmw_time_sink::make(
                { "P_Int_Week", "S_Int_Week" },
                { "Wh", "VAh" },
                1.0f);
        opencmw_time_sink_int_week->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_int_month = gr::pulsed_power::opencmw_time_sink::make(
                { "P_Int_Month", "S_Int_Month" },
                { "Wh", "VAh" },
                1.0f);
        opencmw_time_sink_int_month->set_max_noutput_items(noutput_items);

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
        auto null_sink_stats_longterm = gr::blocks::null_sink::make(sizeof(float));

        // Frequency spectra sinks
        auto frequency_spec_pulsed_power_opencmw_freq_sink = gr::pulsed_power::opencmw_freq_sink::make(
                 { "sinus_fft" },
                 { "W" }, 50, 50, 512);
        frequency_spec_pulsed_power_opencmw_freq_sink->set_max_noutput_items(1);
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
        //  Calculate phase shift
        top->hier_block2::connect(band_pass_filter_voltage0, 0, decimation_block_voltage_bpf0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, decimation_block_current_bpf0, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(decimation_block_current_bpf0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(decimation_block_current_bpf0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf0, 0, pulsed_power_calc_mul_ph_ff, 0);
        top->hier_block2::connect(decimation_block_current_bpf0, 0, pulsed_power_calc_mul_ph_ff, 1);
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
        top->hier_block2::connect(blocks_sub_phase0, 0, pulsed_power_calc_mul_ph_ff, 2);
        // Calculate P, Q, S
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, out_decimation_p_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, out_decimation_q_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, out_decimation_s_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, out_decimation_phi_shortterm, 0);
        top->hier_block2::connect(out_decimation_p_shortterm, 0, opencmw_time_sink_power_shortterm, 0);   // P short-term
        top->hier_block2::connect(out_decimation_q_shortterm, 0, opencmw_time_sink_power_shortterm, 1);   // Q short-term
        top->hier_block2::connect(out_decimation_s_shortterm, 0, opencmw_time_sink_power_shortterm, 2);   // S short-term
        top->hier_block2::connect(out_decimation_phi_shortterm, 0, opencmw_time_sink_power_shortterm, 3); // phi short-term
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, out_decimation_p_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, out_decimation_q_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, out_decimation_s_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, out_decimation_phi_midterm, 0);
        top->hier_block2::connect(out_decimation_p_midterm, 0, opencmw_time_sink_power_midterm, 0);   // P mid-term
        top->hier_block2::connect(out_decimation_q_midterm, 0, opencmw_time_sink_power_midterm, 1);   // Q mid-term
        top->hier_block2::connect(out_decimation_s_midterm, 0, opencmw_time_sink_power_midterm, 2);   // S mid-term
        top->hier_block2::connect(out_decimation_phi_midterm, 0, opencmw_time_sink_power_midterm, 3); // phi mid-term
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, out_decimation_p_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, out_decimation_q_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, out_decimation_s_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, out_decimation_phi_longterm, 0);
        top->hier_block2::connect(out_decimation_p_longterm, 0, opencmw_time_sink_power_longterm, 0);   // P long-term
        top->hier_block2::connect(out_decimation_q_longterm, 0, opencmw_time_sink_power_longterm, 1);   // Q long-term
        top->hier_block2::connect(out_decimation_s_longterm, 0, opencmw_time_sink_power_longterm, 2);   // S long-term
        top->hier_block2::connect(out_decimation_phi_longterm, 0, opencmw_time_sink_power_longterm, 3); // phi long-term
        // Integrals (ppem: usage active and apparent power)
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 0, integrate_P_day, 0);
        top->hier_block2::connect(integrate_P_day, 0, opencmw_time_sink_int_day, 0); // int P day
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 2, integrate_S_day, 0);
        top->hier_block2::connect(integrate_S_day, 0, opencmw_time_sink_int_day, 1); // int S day
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 0, integrate_P_week, 0);
        top->hier_block2::connect(integrate_P_week, 0, opencmw_time_sink_int_week, 0); // int P week
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 2, integrate_S_week, 0);
        top->hier_block2::connect(integrate_S_week, 0, opencmw_time_sink_int_week, 1); // int S week
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 0, integrate_P_month, 0);
        top->hier_block2::connect(integrate_P_month, 0, opencmw_time_sink_int_month, 0); // int P month
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 2, integrate_S_month, 0);
        top->hier_block2::connect(integrate_S_month, 0, opencmw_time_sink_int_month, 1); // int S month
        // Statistics
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, statistics_p_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, statistics_q_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, statistics_s_shortterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, statistics_phi_shortterm, 0);
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
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, statistics_p_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, statistics_q_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, statistics_s_midterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, statistics_phi_midterm, 0);
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
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 12, statistics_p_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 13, statistics_q_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 14, statistics_s_longterm, 0);
        top->hier_block2::connect(pulsed_power_calc_mul_ph_ff, 3, statistics_phi_longterm, 0);
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
        top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current, 0);
        top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current, 1);
        top->hier_block2::connect(multiply_voltage_current, 0, frequency_spec_one_in_n, 0);
        top->hier_block2::connect(frequency_spec_one_in_n, 0, frequency_spec_low_pass, 0);
        top->hier_block2::connect(frequency_spec_low_pass, 0, frequency_spec_stream_to_vec, 0);
        top->hier_block2::connect(frequency_spec_stream_to_vec, 0, frequency_spec_fft, 0);
        top->hier_block2::connect(frequency_spec_fft, 0, frequency_multiply_const, 0);
        top->hier_block2::connect(frequency_multiply_const, 0, frequency_spec_complex_to_mag, 0);
        top->hier_block2::connect(frequency_spec_complex_to_mag, 0, frequency_spec_pulsed_power_opencmw_freq_sink, 0); // freq_spectra apparent power + ppem: Power Spectrum
        top->hier_block2::connect(source_interface_current1, 0, band_pass_filter_current1, 0);
        top->hier_block2::connect(source_interface_current2, 0, band_pass_filter_current2, 0);
        phase1_phi_phase_calculation(band_pass_filter_current1, band_pass_filter_voltage1, source_interface_current1, pulsed_power_calc_mul_ph_ff, source_samp_rate, bpf_low_cut, bpf_high_cut, bpf_trans, decimation_bpf, source_interface_voltage1, samp_rate_delta_phi_calc, decimation_lpf, lpf_in_samp_rate, lpf_trans, decimation_delta_phi_calc);
        phase2_phi_phase_calculation(band_pass_filter_current2, band_pass_filter_voltage2, source_interface_current2, pulsed_power_calc_mul_ph_ff, source_samp_rate, bpf_low_cut, bpf_high_cut, bpf_trans, decimation_bpf, source_interface_voltage2, samp_rate_delta_phi_calc, decimation_lpf, lpf_in_samp_rate, lpf_trans, decimation_delta_phi_calc);
        
        // statistics U_0, U_1, U_2 (-> voltages dashboard)
        statistics_connection_voltage(band_pass_filter_voltage1, band_pass_filter_voltage2, source_interface_voltage1, source_interface_voltage2, decimation_out_raw, decimation_out_bpf,
                out_samp_rate_ui, noutput_items, out_decimation_voltage0, band_pass_filter_voltage0);
        statistics_connection_current(band_pass_filter_current1, band_pass_filter_current2, source_interface_current1, source_interface_current2, decimation_out_raw, decimation_out_bpf,
                out_samp_rate_ui, noutput_items, out_decimation_current0, band_pass_filter_current0);
    }
    ~PulsedPowerFlowgraph() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }

    void phase1_signal_simulation(bool add_noise, float source_samp_rate, std::shared_ptr<gr::blocks::multiply_const<float> > source_interface_voltage1, std::shared_ptr<gr::blocks::multiply_const<float> > source_interface_current1){
        float phase_shift_0_1 = 2 * PI / 3;
        // same as phase 0
        if(add_noise){
                //I
                auto analog_sig_source_voltage1        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f + phase_shift_0_1); // U_raw
                //u
                auto analog_sig_source_current1        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 5, 0, 0.2f + phase_shift_0_1);  // I_raw
                auto analog_sig_source_freq_modulation = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 2, 1, 0, 0.2f + phase_shift_0_1);

                auto noise_source_current1             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 0.25f);
                auto noise_source_voltage1             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 16.25f);

                auto throttle_voltage1                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current1                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_freq_modulation          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_voltage1           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_current1           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                auto multiply_freq_modulation          = gr::blocks::multiply_ff::make();

                auto add_noise_current1                = gr::blocks::add_ff::make(1);
                auto add_noise_voltage1                = gr::blocks::add_ff::make(1);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage1, 0, throttle_voltage1, 0);
                top->hier_block2::connect(analog_sig_source_current1, 0, throttle_current1, 0);
                top->hier_block2::connect(analog_sig_source_freq_modulation, 0, throttle_freq_modulation, 0);
                top->hier_block2::connect(noise_source_voltage1, 0, throttle_noise_voltage1, 0);
                top->hier_block2::connect(noise_source_current1, 0, throttle_noise_current1, 0);
                // multiply frequency modulation
                top->hier_block2::connect(throttle_current1, 0, multiply_freq_modulation, 0);
                top->hier_block2::connect(throttle_freq_modulation, 0, multiply_freq_modulation, 1);
                // add noise
                top->hier_block2::connect(throttle_voltage1, 0, add_noise_voltage1, 0);
                top->hier_block2::connect(throttle_noise_voltage1, 0, add_noise_voltage1, 1);
                top->hier_block2::connect(multiply_freq_modulation, 0, add_noise_current1, 0);
                top->hier_block2::connect(throttle_noise_current1, 0, add_noise_current1, 1);
                // connect to interface
                top->hier_block2::connect(add_noise_voltage1, 0, source_interface_voltage1, 0);
                top->hier_block2::connect(add_noise_current1, 0, source_interface_current1, 0);

        }
        else{

                //--SIGNAL SOURCE PHASE 1--//
                auto analog_sig_source_voltage1 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f + phase_shift_0_1); // U_raw
                auto analog_sig_source_current1 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 0, 0, 0.0f + phase_shift_0_1);  // I_raw
                //--END SIGNAL SOURCE PHASE 1--//

                auto throttle_voltage1          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current1          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage1, 0, throttle_voltage1, 0);
                top->hier_block2::connect(analog_sig_source_current1, 0, throttle_current1, 0);
                // connect to interface
                top->hier_block2::connect(throttle_voltage1, 0, source_interface_voltage1, 0);
                top->hier_block2::connect(throttle_current1, 0, source_interface_current1, 0);
        }

    }
    void phase2_signal_simulation(bool add_noise, float source_samp_rate, std::shared_ptr<gr::blocks::multiply_const<float> > source_interface_voltage2, std::shared_ptr<gr::blocks::multiply_const<float> > source_interface_current2){
        float phase_shift_1_2 = 4 * PI / 3;
        if(add_noise){
                // I
                auto analog_sig_source_voltage2        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f + phase_shift_1_2); // U_raw
                // u
                auto analog_sig_source_current2        = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 5, 0, 0.2f + phase_shift_1_2);  // I_raw
                auto analog_sig_source_freq_modulation = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 2, 1, 0, 0.2f + phase_shift_1_2);

                auto noise_source_current2             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 0.25f);
                auto noise_source_voltage2             = gr::analog::noise_source_f::make(gr::analog::GR_GAUSSIAN, 16.25f);

                auto throttle_voltage2                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current2                 = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_freq_modulation          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_voltage2           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_noise_current2           = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                auto multiply_freq_modulation          = gr::blocks::multiply_ff::make();

                auto add_noise_current2                = gr::blocks::add_ff::make(1);
                auto add_noise_voltage2                = gr::blocks::add_ff::make(1);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage2, 0, throttle_voltage2, 0);
                top->hier_block2::connect(analog_sig_source_current2, 0, throttle_current2, 0);
                top->hier_block2::connect(analog_sig_source_freq_modulation, 0, throttle_freq_modulation, 0);
                top->hier_block2::connect(noise_source_voltage2, 0, throttle_noise_voltage2, 0);
                top->hier_block2::connect(noise_source_current2, 0, throttle_noise_current2, 0);
                // multiply frequency modulation
                top->hier_block2::connect(throttle_current2, 0, multiply_freq_modulation, 0);
                top->hier_block2::connect(throttle_freq_modulation, 0, multiply_freq_modulation, 1);
                // add noise
                top->hier_block2::connect(throttle_voltage2, 0, add_noise_voltage2, 0);
                top->hier_block2::connect(throttle_noise_voltage2, 0, add_noise_voltage2, 1);
                top->hier_block2::connect(multiply_freq_modulation, 0, add_noise_current2, 0);
                top->hier_block2::connect(throttle_noise_current2, 0, add_noise_current2, 1);
                // connect to interface
                top->hier_block2::connect(add_noise_voltage2, 0, source_interface_voltage2, 0);
                top->hier_block2::connect(add_noise_current2, 0, source_interface_current2, 0);

        }
        else{

                //--SIGNAL SOURCE PHASE 2--//
                auto analog_sig_source_voltage2 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f + phase_shift_1_2); // U_raw
                auto analog_sig_source_current2 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 0, 0, 0.0f + phase_shift_1_2);  // I_raw
                //--END SIGNAL SOURCE PHASE 2--//

                auto throttle_voltage2          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
                auto throttle_current2          = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

                // connections
                top->hier_block2::connect(analog_sig_source_voltage2, 0, throttle_voltage2, 0);
                top->hier_block2::connect(analog_sig_source_current2, 0, throttle_current2, 0);
                // connect to interface
                top->hier_block2::connect(throttle_voltage2, 0, source_interface_voltage2, 0);
                top->hier_block2::connect(throttle_current2, 0, source_interface_current2, 0);
        }
    }
    void phase1_phi_phase_calculation(gr::filter::fft_filter_fff::sptr band_pass_filter_current1, gr::filter::fft_filter_fff::sptr band_pass_filter_voltage1, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current1, gr::pulsed_power::power_calc_mul_ph_ff::sptr pulsed_power_power_calc_ff_0_0, const float source_samp_rate, const float bpf_low_cut, const float bpf_high_cut, const int bpf_trans,
        const int decimation_bpf, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage1, const float samp_rate_delta_phi_calc,
        const int decimation_lpf, const float lpf_in_samp_rate, const float lpf_trans, const int decimation_delta_phi_calc){

        auto analog_sig_source_phase1_sin = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase1_cos = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

        auto blocks_multiply_phase1_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_3     = gr::blocks::multiply_ff::make(1);

        auto low_pass_filter_current1_0   = gr::filter::fft_filter_fff::make(
                  decimation_lpf,
                  gr::filter::firdes::low_pass(
                          1,
                          lpf_in_samp_rate,
                          60,
                          lpf_trans,
                          gr::fft::window::win_type::WIN_HAMMING,
                          6.76));
        auto low_pass_filter_current1_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage1_0 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage1_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase1_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase1_1                   = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase1_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase1_1           = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase1                        = gr::blocks::sub_ff::make(1);
        auto decimation_block_current_bpf1            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);
        auto decimation_block_voltage_bpf1            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);
        phase1_calc_connections(pulsed_power_power_calc_ff_0_0, source_interface_voltage1, source_interface_current1, band_pass_filter_voltage1, band_pass_filter_current1,
                decimation_block_current_bpf1, decimation_block_voltage_bpf1, blocks_multiply_phase1_0, blocks_multiply_phase1_1, blocks_multiply_phase1_2,
                blocks_multiply_phase1_3, analog_sig_source_phase1_sin, analog_sig_source_phase1_cos,
                low_pass_filter_voltage1_0, low_pass_filter_voltage1_1, low_pass_filter_current1_0, low_pass_filter_current1_1,
                blocks_divide_phase1_0, blocks_divide_phase1_1, blocks_transcendental_phase1_0, blocks_transcendental_phase1_1, blocks_sub_phase1);
    }

    void phase1_calc_connections(gr::pulsed_power::power_calc_mul_ph_ff::sptr pulsed_power_power_calc_ff_0_0,
        std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage1, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current1,
        gr::filter::fft_filter_fff::sptr band_pass_filter_voltage1, gr::filter::fft_filter_fff::sptr band_pass_filter_current1,
        gr::blocks::keep_one_in_n::sptr decimation_block_current_bpf1, gr::blocks::keep_one_in_n::sptr decimation_block_voltage_bpf1,
        std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase1_0, std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase1_1,
        std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase1_2, std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase1_3,
        std::shared_ptr<gr::analog::sig_source_f> analog_sig_source_phase1_sin, std::shared_ptr<gr::analog::sig_source_f> analog_sig_source_phase1_cos,
        gr::filter::fft_filter_fff::sptr low_pass_filter_voltage1_0,
        gr::filter::fft_filter_fff::sptr low_pass_filter_voltage1_1, gr::filter::fft_filter_fff::sptr low_pass_filter_current1_0,
        gr::filter::fft_filter_fff::sptr low_pass_filter_current1_1,  std::shared_ptr<gr::blocks::divide_ff> blocks_divide_phase1_0,
        std::shared_ptr<gr::blocks::divide_ff> blocks_divide_phase1_1, gr::blocks::transcendental::sptr blocks_transcendental_phase1_0,
        gr::blocks::transcendental::sptr blocks_transcendental_phase1_1, std::shared_ptr<gr::blocks::sub_ff> blocks_sub_phase1)
    {
        auto null_sink_p_q_s_phi_1 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_p_q_s_phi_2 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_p_q_s_phi_acc = gr::blocks::null_sink::make(sizeof(float));
        // Bandpass filter
        top->hier_block2::connect(source_interface_voltage1, 0, band_pass_filter_voltage1, 0);
        // Calculate phase shift
        top->hier_block2::connect(band_pass_filter_voltage1, 0, decimation_block_voltage_bpf1, 0);
        top->hier_block2::connect(band_pass_filter_current1, 0, decimation_block_current_bpf1, 0);

        top->hier_block2::connect(decimation_block_voltage_bpf1, 0, blocks_multiply_phase1_0, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf1, 0, blocks_multiply_phase1_1, 0);
        top->hier_block2::connect(decimation_block_current_bpf1, 0, blocks_multiply_phase1_2, 0);
        top->hier_block2::connect(decimation_block_current_bpf1, 0, blocks_multiply_phase1_3, 0);
        // connection U_1, I_1 to power_calc_mul_ph
        top->hier_block2::connect(decimation_block_voltage_bpf1, 0, pulsed_power_power_calc_ff_0_0, 3);
        top->hier_block2::connect(decimation_block_current_bpf1, 0, pulsed_power_power_calc_ff_0_0, 4);
        top->hier_block2::connect(analog_sig_source_phase1_sin, 0, blocks_multiply_phase1_0, 1);
        top->hier_block2::connect(analog_sig_source_phase1_sin, 0, blocks_multiply_phase1_2, 1);
        top->hier_block2::connect(analog_sig_source_phase1_cos, 0, blocks_multiply_phase1_1, 1);
        top->hier_block2::connect(analog_sig_source_phase1_cos, 0, blocks_multiply_phase1_3, 1);
        top->hier_block2::connect(blocks_multiply_phase1_0, 0, low_pass_filter_voltage1_0, 0);
        top->hier_block2::connect(blocks_multiply_phase1_1, 0, low_pass_filter_voltage1_1, 0);
        top->hier_block2::connect(blocks_multiply_phase1_2, 0, low_pass_filter_current1_0, 0);
        top->hier_block2::connect(blocks_multiply_phase1_3, 0, low_pass_filter_current1_1, 0);
        top->hier_block2::connect(low_pass_filter_voltage1_0, 0, blocks_divide_phase1_0, 0);
        top->hier_block2::connect(low_pass_filter_voltage1_1, 0, blocks_divide_phase1_0, 1);
        top->hier_block2::connect(low_pass_filter_current1_0, 0, blocks_divide_phase1_1, 0);
        top->hier_block2::connect(low_pass_filter_current1_1, 0, blocks_divide_phase1_1, 1);
        top->hier_block2::connect(blocks_divide_phase1_0, 0, blocks_transcendental_phase1_0, 0);
        top->hier_block2::connect(blocks_divide_phase1_1, 0, blocks_transcendental_phase1_1, 0);
        top->hier_block2::connect(blocks_transcendental_phase1_0, 0, blocks_sub_phase1, 0);
        top->hier_block2::connect(blocks_transcendental_phase1_1, 0, blocks_sub_phase1, 1);
        // phase connection to power_calc_mul_ph
        top->hier_block2::connect(blocks_sub_phase1, 0, pulsed_power_power_calc_ff_0_0, 5); 
        // Calculate P, Q, S, phi
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 4, null_sink_p_q_s_phi_1, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 5, null_sink_p_q_s_phi_1, 1);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 6, null_sink_p_q_s_phi_1, 2);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 7, null_sink_p_q_s_phi_1, 3);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 8, null_sink_p_q_s_phi_2, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 9, null_sink_p_q_s_phi_2, 1);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 10, null_sink_p_q_s_phi_2, 2);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 11, null_sink_p_q_s_phi_2, 3);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, null_sink_p_q_s_phi_acc, 0); //pqs phase 0
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, null_sink_p_q_s_phi_acc, 1);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, null_sink_p_q_s_phi_acc, 2);
    }
    void phase2_phi_phase_calculation(gr::filter::fft_filter_fff::sptr band_pass_filter_current2, gr::filter::fft_filter_fff::sptr band_pass_filter_voltage2, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current2,
        gr::pulsed_power::power_calc_mul_ph_ff::sptr pulsed_power_power_calc_ff_0_0, const float source_samp_rate, const float bpf_low_cut,
        const float bpf_high_cut, const int bpf_trans, const int decimation_bpf, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage2,
        const float samp_rate_delta_phi_calc, const int decimation_lpf, const float lpf_in_samp_rate, const float lpf_trans,
        const int decimation_delta_phi_calc)
        {

        auto analog_sig_source_phase2_sin = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase2_cos = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

        auto blocks_multiply_phase2_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_3     = gr::blocks::multiply_ff::make(1);

        auto low_pass_filter_current2_0   = gr::filter::fft_filter_fff::make(
                  decimation_lpf,
                  gr::filter::firdes::low_pass(
                          1,
                          lpf_in_samp_rate,
                          60,
                          lpf_trans,
                          gr::fft::window::win_type::WIN_HAMMING,
                          6.76));
        auto low_pass_filter_current2_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage2_0 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage2_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        lpf_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase2_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase2_1                   = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase2_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase2_1           = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase2                        = gr::blocks::sub_ff::make(1);
        auto decimation_block_current_bpf2            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);
        auto decimation_block_voltage_bpf2            = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_delta_phi_calc);
        phase2_calc_connections(pulsed_power_power_calc_ff_0_0, source_interface_voltage2, source_interface_current2, band_pass_filter_voltage2,
                band_pass_filter_current2, decimation_block_current_bpf2, decimation_block_voltage_bpf2, blocks_multiply_phase2_0,
                blocks_multiply_phase2_1, blocks_multiply_phase2_2, blocks_multiply_phase2_3, analog_sig_source_phase2_sin, analog_sig_source_phase2_cos,
                low_pass_filter_voltage2_0, low_pass_filter_voltage2_1, low_pass_filter_current2_0, low_pass_filter_current2_1,
                blocks_divide_phase2_0, blocks_divide_phase2_1, blocks_transcendental_phase2_0, blocks_transcendental_phase2_1, blocks_sub_phase2
                );
    }

    
    void phase2_calc_connections(gr::pulsed_power::power_calc_mul_ph_ff::sptr pulsed_power_power_calc_ff_0_0,
        std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage2, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current2,
        gr::filter::fft_filter_fff::sptr band_pass_filter_voltage2, gr::filter::fft_filter_fff::sptr band_pass_filter_current2,
        gr::blocks::keep_one_in_n::sptr decimation_block_current_bpf2, gr::blocks::keep_one_in_n::sptr decimation_block_voltage_bpf2,
        std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase2_0, std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase2_1,
        std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase2_2, std::shared_ptr<gr::blocks::multiply_ff> blocks_multiply_phase2_3,
        std::shared_ptr<gr::analog::sig_source_f> analog_sig_source_phase2_sin, std::shared_ptr<gr::analog::sig_source_f> analog_sig_source_phase2_cos,
        gr::filter::fft_filter_fff::sptr low_pass_filter_voltage2_0, gr::filter::fft_filter_fff::sptr low_pass_filter_voltage2_1,
        gr::filter::fft_filter_fff::sptr low_pass_filter_current2_0, gr::filter::fft_filter_fff::sptr low_pass_filter_current2_1,
        std::shared_ptr<gr::blocks::divide_ff> blocks_divide_phase2_0, std::shared_ptr<gr::blocks::divide_ff> blocks_divide_phase2_1,
        gr::blocks::transcendental::sptr blocks_transcendental_phase2_0, gr::blocks::transcendental::sptr blocks_transcendental_phase2_1,
        std::shared_ptr<gr::blocks::sub_ff> blocks_sub_phase2)
        {
        auto null_sink_p_q_s_phi_1 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_p_q_s_phi_2 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_p_q_s_phi_acc = gr::blocks::null_sink::make(sizeof(float));
        // Bandpass filter
        top->hier_block2::connect(source_interface_voltage2, 0, band_pass_filter_voltage2, 0);
        // Calculate phase shift
        top->hier_block2::connect(band_pass_filter_voltage2, 0, decimation_block_voltage_bpf2, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, decimation_block_current_bpf2, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf2, 0, blocks_multiply_phase2_0, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf2, 0, blocks_multiply_phase2_1, 0);
        top->hier_block2::connect(decimation_block_current_bpf2, 0, blocks_multiply_phase2_2, 0);
        top->hier_block2::connect(decimation_block_current_bpf2, 0, blocks_multiply_phase2_3, 0);
        top->hier_block2::connect(decimation_block_voltage_bpf2, 0, pulsed_power_power_calc_ff_0_0, 6);
        top->hier_block2::connect(decimation_block_current_bpf2, 0, pulsed_power_power_calc_ff_0_0, 7);
        top->hier_block2::connect(analog_sig_source_phase2_sin, 0, blocks_multiply_phase2_0, 1);
        top->hier_block2::connect(analog_sig_source_phase2_sin, 0, blocks_multiply_phase2_2, 1);
        top->hier_block2::connect(analog_sig_source_phase2_cos, 0, blocks_multiply_phase2_1, 1);
        top->hier_block2::connect(analog_sig_source_phase2_cos, 0, blocks_multiply_phase2_3, 1);
        top->hier_block2::connect(blocks_multiply_phase2_0, 0, low_pass_filter_voltage2_0, 0);
        top->hier_block2::connect(blocks_multiply_phase2_1, 0, low_pass_filter_voltage2_1, 0);
        top->hier_block2::connect(blocks_multiply_phase2_2, 0, low_pass_filter_current2_0, 0);
        top->hier_block2::connect(blocks_multiply_phase2_3, 0, low_pass_filter_current2_1, 0);
        top->hier_block2::connect(low_pass_filter_voltage2_0, 0, blocks_divide_phase2_0, 0);
        top->hier_block2::connect(low_pass_filter_voltage2_1, 0, blocks_divide_phase2_0, 1);
        top->hier_block2::connect(low_pass_filter_current2_0, 0, blocks_divide_phase2_1, 0);
        top->hier_block2::connect(low_pass_filter_current2_1, 0, blocks_divide_phase2_1, 1);
        top->hier_block2::connect(blocks_divide_phase2_0, 0, blocks_transcendental_phase2_0, 0);
        top->hier_block2::connect(blocks_divide_phase2_1, 0, blocks_transcendental_phase2_1, 0);
        top->hier_block2::connect(blocks_transcendental_phase2_0, 0, blocks_sub_phase2, 0);
        top->hier_block2::connect(blocks_transcendental_phase2_1, 0, blocks_sub_phase2, 1);
        top->hier_block2::connect(blocks_sub_phase2, 0, pulsed_power_power_calc_ff_0_0, 8);
    }

    void statistics_connection_voltage(gr::filter::fft_filter_fff::sptr band_pass_filter_voltage1, gr::filter::fft_filter_fff::sptr band_pass_filter_voltage2,
        std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage1, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_voltage2,
        int decimation_out_raw, int decimation_out_bpf, float out_samp_rate_ui, int noutput_items, gr::blocks::keep_one_in_n::sptr out_decimation_voltage0,
        gr::filter::fft_filter_fff::sptr band_pass_filter_voltage0)
    {
        auto opencmw_time_sink_signals_voltages       = gr::pulsed_power::opencmw_time_sink::make(
                               { "U_0", "U_1", "U_2", "U_0_bpf", "U_1_bpf", "U_2_bpf" },
                               { "V", "V", "V", "V", "V", "V" },
                               //exchanged "sample_rate_ui" with 500
                               500);
        opencmw_time_sink_signals_voltages->set_max_noutput_items(noutput_items);
        
        auto out_decimation_voltage1                   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage2                   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        top->hier_block2::connect(source_interface_voltage1, 0, out_decimation_voltage1, 0);
        top->hier_block2::connect(source_interface_voltage2, 0, out_decimation_voltage2, 0);
        top->hier_block2::connect(out_decimation_voltage0, 0, opencmw_time_sink_signals_voltages, 0); // U_0_raw
        top->hier_block2::connect(out_decimation_voltage1, 0, opencmw_time_sink_signals_voltages, 1); // U_1_raw
        top->hier_block2::connect(out_decimation_voltage2, 0, opencmw_time_sink_signals_voltages, 2); // U_2_raw
        top->hier_block2::connect(band_pass_filter_voltage0, 0, opencmw_time_sink_signals_voltages, 3); // U_0_bpf 
        top->hier_block2::connect(band_pass_filter_voltage1, 0, opencmw_time_sink_signals_voltages, 4); // U_1_bpf
        top->hier_block2::connect(band_pass_filter_voltage2, 0, opencmw_time_sink_signals_voltages, 5); // U_1_bpf
    }
     void statistics_connection_current(gr::filter::fft_filter_fff::sptr band_pass_filter_current1, gr::filter::fft_filter_fff::sptr band_pass_filter_current2,
        std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current1, std::shared_ptr<gr::blocks::multiply_const_ff> source_interface_current2,
        int decimation_out_raw, int decimation_out_bpf, float out_samp_rate_ui, int noutput_items, gr::blocks::keep_one_in_n::sptr out_decimation_current0,
        gr::filter::fft_filter_fff::sptr band_pass_filter_current0)
    {
        auto opencmw_time_sink_signals_currents       = gr::pulsed_power::opencmw_time_sink::make(
                               { "I_0", "I_1", "I_2", "I_0_bpf", "I_1_bpf", "I_2_bpf" },
                               { "A", "A", "A", "A", "A", "A" },
                               500);
        opencmw_time_sink_signals_currents->set_max_noutput_items(noutput_items);
        
        auto out_decimation_current1                   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_current2                   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        top->hier_block2::connect(source_interface_current1, 0, out_decimation_current1, 0);
        top->hier_block2::connect(source_interface_current2, 0, out_decimation_current2, 0);
        top->hier_block2::connect(out_decimation_current0, 0, opencmw_time_sink_signals_currents, 0); // U_0_raw
        top->hier_block2::connect(out_decimation_current1, 0, opencmw_time_sink_signals_currents, 1); // U_1_raw
        top->hier_block2::connect(out_decimation_current2, 0, opencmw_time_sink_signals_currents, 2); // U_2_raw
        top->hier_block2::connect(band_pass_filter_current0, 0, opencmw_time_sink_signals_currents, 3); // U_0_bpf 
        top->hier_block2::connect(band_pass_filter_current1, 0, opencmw_time_sink_signals_currents, 4); // U_1_bpf
        top->hier_block2::connect(band_pass_filter_current2, 0, opencmw_time_sink_signals_currents, 5); // U_1_bpf
    }
};

#endif /* GR_FLOWGRAPHS_HPP */
