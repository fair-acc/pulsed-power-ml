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

#include <gnuradio/pulsed_power/mains_frequency_calc.h>
#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/pulsed_power/picoscope_4000a_source.h>
#include <gnuradio/pulsed_power/power_calc_ff.h>
#include <gnuradio/pulsed_power/power_calc_mul_ph_ff.h>

const float PI = 3.141592653589793238463f;

class GRFlowGraph {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraph(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // flowgraph setup
        float samp_rate = 4'000.0f;
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

class FlowgraphSimulated {
private:
    gr::top_block_sptr top;

public:
    FlowgraphSimulated(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float source_samp_rate = 200'000.0f;
        float freq_samp_rate   = 32'000.0f;
        // parameters band pass filter
        int   decimation_bpf    = 200;
        float bpf_out_samp_rate = source_samp_rate / (float) decimation_bpf;
        float bpf_high_cut      = 80;
        float bpf_low_cut       = 20;
        float bpf_trans         = 10;
        // parameters low pass filter
        int   decimation_lpf    = 1;
        float lpf_in_samp_rate  = 1'000.0f;
        float lpf_out_samp_rate = 1'000.0f;
        // parameters decimation
        float out_samp_rate_high        = 1'000.0f;
        float out_samp_rate_low         = 100.0f;
        int   decimation_out_raw        = 200;
        int   decimation_out_mains_freq = 2'000;
        int   decimation_out_short_term = 10;
        int   decimation_out_mid_term   = 1'000;
        int   decimation_out_long_term  = 60'000;

        // blocks
        auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
        auto analog_sig_source_current0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);  // I_raw

        auto calc_mains_frequency       = gr::pulsed_power::mains_frequency_calc::make(source_samp_rate, -100.0f, 100.0f);

        auto band_pass_filter_current0  = gr::filter::fft_filter_fff::make(
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

        auto analog_sig_source_phase0_sin = gr::analog::sig_source_f::make(out_samp_rate_high, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase0_cos = gr::analog::sig_source_f::make(out_samp_rate_high, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

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
                          10,
                          gr::fft::window::win_type::WIN_HAMMING,
                          6.76));
        auto low_pass_filter_current0_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_0 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_1 = gr::filter::fft_filter_fff::make(
                decimation_lpf,
                gr::filter::firdes::low_pass(
                        1,
                        lpf_in_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase0_0         = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase0_1         = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase0_0 = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase0_1 = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase0              = gr::blocks::sub_ff::make(1);

        auto pulsed_power_power_calc_ff_0_0 = gr::pulsed_power::power_calc_ff::make(0.0001);

        auto throttle_block_raw_current0    = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
        auto throttle_block_raw_voltage0    = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

        auto out_decimation_current0        = gr::filter::fft_filter_fff::make(
                       decimation_out_raw,
                       gr::filter::firdes::low_pass(
                               1,
                               source_samp_rate,
                               400,
                               10,
                               gr::fft::window::win_type::WIN_HAMMING,
                               6.76));
        auto out_decimation_voltage0 = gr::filter::fft_filter_fff::make(
                decimation_out_raw,
                gr::filter::firdes::low_pass(
                        1,
                        source_samp_rate,
                        400,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto out_decimation_mains_frequency = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq);
        auto out_decimation_current_bpf     = gr::filter::fft_filter_fff::make(
                    1,
                    gr::filter::firdes::low_pass(
                            1,
                            bpf_out_samp_rate,
                            400,
                            10,
                            gr::fft::window::win_type::WIN_HAMMING,
                            6.76));
        auto out_decimation_voltage_bpf = gr::filter::fft_filter_fff::make(
                1,
                gr::filter::firdes::low_pass(
                        1,
                        bpf_out_samp_rate,
                        400,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto out_decimation_p_shortterm           = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_q_shortterm           = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_s_shortterm           = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_phi_shortterm         = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto pulsed_power_opencmw_time_sink_raw_0 = gr::pulsed_power::opencmw_time_sink::make(
                { "U", "I" },
                { "V", "A" },
                out_samp_rate_high);
        pulsed_power_opencmw_time_sink_raw_0->set_max_noutput_items(noutput_items);
        auto pulsed_power_opencmw_time_sink_mains_freq = gr::pulsed_power::opencmw_time_sink::make(
                { "mains_freq" },
                { "Hz" },
                out_samp_rate_low);
        auto pulsed_power_opencmw_time_sink_bpf_0 = gr::pulsed_power::opencmw_time_sink::make(
                { "U_bpf", "I_bpf" },
                { "V", "A" },
                out_samp_rate_high);
        pulsed_power_opencmw_time_sink_bpf_0->set_max_noutput_items(noutput_items);
        auto pulsed_power_opencmw_time_sink_power = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "rad" },
                out_samp_rate_low);
        pulsed_power_opencmw_time_sink_power->set_max_noutput_items(noutput_items);

        auto multiply_voltaga_current = gr::blocks::multiply<float>::make(1);
        auto frequency_spec_one_in_n  = gr::blocks::keep_one_in_n::make(sizeof(float), 400);
        auto frequency_spec_low_pass  = gr::filter::fft_filter_fff::make(
                 10,
                 gr::filter::firdes::low_pass(
                         1,
                         500,
                         20,
                         100,
                         gr::fft::window::win_type::WIN_HAMMING,
                         6.76));
        auto frequency_spec_stream_to_vec                  = gr::blocks::stream_to_vector::make(sizeof(float), 512);
        auto frequency_spec_fft                            = gr::fft::fft_v<float, true>::make(512, gr::fft::window::rectangular(512), true, 1);
        auto frequency_multiply_const                      = gr::blocks::multiply_const<gr_complex>::make(2.0 / (512.0), 512);
        auto frequency_spec_complex_to_mag                 = gr::blocks::complex_to_mag::make(512);
        auto frequency_spec_pulsed_power_opencmw_freq_sink = gr::pulsed_power::opencmw_freq_sink::make({ "sinus_fft" }, { "W" }, 50, 50, 512);

        // Connections:
        // signal
        top->hier_block2::connect(analog_sig_source_voltage0, 0, throttle_block_raw_voltage0, 0);
        top->hier_block2::connect(analog_sig_source_current0, 0, throttle_block_raw_current0, 0);
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, out_decimation_voltage0, 0);
        top->hier_block2::connect(throttle_block_raw_current0, 0, out_decimation_current0, 0);
        top->hier_block2::connect(out_decimation_voltage0, 0, pulsed_power_opencmw_time_sink_raw_0, 0); // U_raw
        top->hier_block2::connect(out_decimation_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_raw
        // Mains frequency
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, calc_mains_frequency, 0);
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency, 0);
        top->hier_block2::connect(out_decimation_mains_frequency, 0, pulsed_power_opencmw_time_sink_mains_freq, 0); // mains_freq
        // Bandpass filter
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(throttle_block_raw_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_opencmw_time_sink_bpf_0, 0); // U_bpf
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_opencmw_time_sink_bpf_0, 1); // I_bpf
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
        top->hier_block2::connect(out_decimation_p_shortterm, 0, pulsed_power_opencmw_time_sink_power, 0);   // P short-term
        top->hier_block2::connect(out_decimation_q_shortterm, 0, pulsed_power_opencmw_time_sink_power, 1);   // Q short-term
        top->hier_block2::connect(out_decimation_s_shortterm, 0, pulsed_power_opencmw_time_sink_power, 2);   // S short-term
        top->hier_block2::connect(out_decimation_phi_shortterm, 0, pulsed_power_opencmw_time_sink_power, 3); // phi short-term
        // Calculate frequency specturm
        top->hier_block2::connect(throttle_block_raw_current0, 0, multiply_voltaga_current, 0);
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, multiply_voltaga_current, 1);
        top->hier_block2::connect(multiply_voltaga_current, 0, frequency_spec_one_in_n, 0);
        top->hier_block2::connect(frequency_spec_one_in_n, 0, frequency_spec_low_pass, 0);
        top->hier_block2::connect(frequency_spec_low_pass, 0, frequency_spec_stream_to_vec, 0);
        top->hier_block2::connect(frequency_spec_stream_to_vec, 0, frequency_spec_fft, 0);
        top->hier_block2::connect(frequency_spec_fft, 0, frequency_multiply_const, 0);
        top->hier_block2::connect(frequency_multiply_const, 0, frequency_spec_complex_to_mag, 0);
        top->hier_block2::connect(frequency_spec_complex_to_mag, 0, frequency_spec_pulsed_power_opencmw_freq_sink, 0);
    }
    ~FlowgraphSimulated() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

class FlowgraphPicoscope {
private:
    gr::top_block_sptr top;

public:
    FlowgraphPicoscope(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float                                 in_samp_rate                = 20'000.0f;
        float                                 in_samp_rate_2              = 32'000.0f;
        float                                 out_samp_rate               = 1'000.0f;
        int                                   bp_decimation               = 20;
        double                                bp_high_cut                 = 80;
        double                                bp_low_cut                  = 20;
        double                                bp_trans                    = 10;
        int                                   lp_decimation               = 1;
        float                                 current_correction_factor   = 2.5f;
        float                                 voltage_correction_factor   = 100.0f;
        gr::pulsed_power::downsampling_mode_t picoscope_downsampling_mode = gr::pulsed_power::DOWNSAMPLING_MODE_NONE;
        gr::pulsed_power::coupling_t          picoscope_coupling          = gr::pulsed_power::AC_1M;
        gr::pulsed_power::trigger_direction_t picoscope_trigger_direction = gr::pulsed_power::TRIGGER_DIRECTION_RISING;

        // blocks
        auto picoscope_source = gr::pulsed_power::picoscope_4000a_source::make("", true);
        picoscope_source->set_trigger_once(false);
        picoscope_source->set_samp_rate(in_samp_rate);
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

        auto null_sink_picoscope       = gr::blocks::null_sink::make(sizeof(float));

        auto blocks_multiply_current0  = gr::blocks::multiply_const_ff::make(current_correction_factor);
        auto blocks_multiply_voltage0  = gr::blocks::multiply_const_ff::make(voltage_correction_factor);

        auto band_pass_filter_current0 = gr::filter::fft_filter_fff::make(
                bp_decimation,
                gr::filter::firdes::band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto band_pass_filter_voltage0 = gr::filter::fft_filter_fff::make(
                bp_decimation,
                gr::filter::firdes::band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto analog_sig_source_phase0_sin = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase0_cos = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

        auto blocks_multiply_phase0_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_3     = gr::blocks::multiply_ff::make(1);

        auto low_pass_filter_current0_0   = gr::filter::fft_filter_fff::make(
                  lp_decimation,
                  gr::filter::firdes::low_pass(
                          1,
                          out_samp_rate,
                          60,
                          10,
                          gr::fft::window::win_type::WIN_HAMMING,
                          6.76));
        auto low_pass_filter_current0_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_0 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage0_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase0_0               = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase0_1               = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase0_0       = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase0_1       = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase0                    = gr::blocks::sub_ff::make(1);

        auto pulsed_power_power_calc_ff_0_0       = gr::pulsed_power::power_calc_ff::make(0.0001);

        auto pulsed_power_opencmw_time_sink_power = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                { "W", "Var", "VA", "deg" },
                out_samp_rate);
        pulsed_power_opencmw_time_sink_power->set_max_noutput_items(noutput_items);
        auto pulsed_power_opencmw_time_sink_raw_0 = gr::pulsed_power::opencmw_time_sink::make(
                { "U", "I" },
                { "V", "A" },
                in_samp_rate);
        pulsed_power_opencmw_time_sink_raw_0->set_max_noutput_items(noutput_items);
        auto pulsed_power_opencmw_time_sink_bpf_0 = gr::pulsed_power::opencmw_time_sink::make(
                { "U_bpf", "I_bpf" },
                { "V", "A" },
                out_samp_rate);
        pulsed_power_opencmw_time_sink_bpf_0->set_max_noutput_items(noutput_items);

        // Connections:
        // Phase 0:
        top->hier_block2::connect(picoscope_source, 0, blocks_multiply_voltage0, 0);
        top->hier_block2::connect(picoscope_source, 2, blocks_multiply_current0, 0);
        // top->hier_block2::connect(blocks_multiply_voltage0, 0, pulsed_power_opencmw_time_sink_raw_0, 0); // U_0
        // top->hier_block2::connect(blocks_multiply_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_0
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
        top->hier_block2::connect(blocks_multiply_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(blocks_multiply_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_power_calc_ff_0_0, 1);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_opencmw_time_sink_bpf_0, 1);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_power_calc_ff_0_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_opencmw_time_sink_bpf_0, 0);
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

        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 0, pulsed_power_opencmw_time_sink_power, 0);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 1, pulsed_power_opencmw_time_sink_power, 1);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 2, pulsed_power_opencmw_time_sink_power, 2);
        top->hier_block2::connect(pulsed_power_power_calc_ff_0_0, 3, pulsed_power_opencmw_time_sink_power, 3);
    }
    ~FlowgraphPicoscope() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

class GRFlowGraphOnePhasePicoscopeNilm {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraphOnePhasePicoscopeNilm(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float in_samp_rate = 2'000'000.0f;

        // gr::pulsed_power::trigger_direction_t picoscope_trigger_direction = gr::pulsed_power::TRIGGER_DIRECTION_RISING;

        // blocks
        auto picoscope_source = gr::pulsed_power::picoscope_4000a_source::make("", true);
        // picoscope parameters

        // acquisition mode = streaming
        picoscope_source->set_nr_buffers(64);
        picoscope_source->set_driver_buffer_size(102400);
        picoscope_source->set_streaming(0.0005);
        picoscope_source->set_buffer_size(204800);

        // S U I, fft
        int    fft_size    = 131072;
        size_t vector_size = static_cast<size_t>(fft_size);
        float  bandwidth   = in_samp_rate;
        // S
        auto multiply_voltage_current = gr::blocks::multiply_ff::make(1);
        auto stream_to_vector_S       = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_S                    = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_S         = gr::blocks::complex_to_mag_squared::make(vector_size);

        // U
        auto stream_to_vector_U = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_U              = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_U   = gr::blocks::complex_to_mag_squared::make(vector_size);

        // I
        auto stream_to_vector_I = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vector_size);
        auto fft_I              = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(fft_size), true, 1);
        auto complex_to_mag_I   = gr::blocks::complex_to_mag_squared::make(vector_size);

        // nilm frequency sink
        auto nilm_freq_sink = gr::pulsed_power::opencmw_freq_sink::make({ "S", "U", "I" }, { "dB", "dB", "dB" }, in_samp_rate, bandwidth, vector_size);
        nilm_freq_sink->set_max_noutput_items(noutput_items);

        // nilm connections
        // S
        top->hier_block2::connect(picoscope_source, 0, multiply_voltage_current, 0); // picoscope voltage
        top->hier_block2::connect(picoscope_source, 2, multiply_voltage_current, 1); // picoscope current
        top->hier_block2::connect(multiply_voltage_current, 0, stream_to_vector_S, 0);
        top->hier_block2::connect(stream_to_vector_S, 0, fft_S, 0);
        top->hier_block2::connect(fft_S, 0, complex_to_mag_S, 0);
        top->hier_block2::connect(complex_to_mag_S, 0, nilm_freq_sink, 0);
        // U
        top->hier_block2::connect(picoscope_source, 0, stream_to_vector_U, 0); // picoscope voltage
        top->hier_block2::connect(stream_to_vector_U, 0, fft_U, 0);
        top->hier_block2::connect(fft_U, 0, complex_to_mag_U, 0);
        top->hier_block2::connect(complex_to_mag_U, 0, nilm_freq_sink, 1);
        // I
        top->hier_block2::connect(picoscope_source, 2, stream_to_vector_I, 0); // picoscope current
        top->hier_block2::connect(stream_to_vector_I, 0, fft_I, 0);
        top->hier_block2::connect(fft_I, 0, complex_to_mag_I, 0);
        top->hier_block2::connect(complex_to_mag_I, 0, nilm_freq_sink, 2);
    }
    ~GRFlowGraphOnePhasePicoscopeNilm() {
        top->stop();
    }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

#endif /* GR_FLOWGRAPHS_HPP */
