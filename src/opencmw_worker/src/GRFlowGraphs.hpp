// Gnu Radio includes
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/divide.h>
#include <gnuradio/blocks/file_sink.h>
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

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/pulsed_power/picoscope_4000a_source.h>
#include <gnuradio/pulsed_power/power_calc_ff.h>
#include <gnuradio/pulsed_power/power_calc_mul_ph_ff.h>

const float PI = 3.141592653589793238463f;

class GRFlowGraphThreePhaseSimulated {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraphThreePhaseSimulated(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float in_samp_rate   = 20'000.0f;
        float in_samp_rate_2 = 32'000.0f;
        float out_samp_rate  = 1'000.0f;
        // float  bandwidth      = in_samp_rate_2;
        int    bp_decimation = 20;
        double bp_high_cut   = 80;
        double bp_low_cut    = 20;
        double bp_trans      = 10;
        // double current_correction_factor = 2.5;
        // double voltage_correction_factor = 100;
        int lp_decimation = 1;

        // blocks
        auto analog_sig_source_current0 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);
        auto analog_sig_source_current1 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 2, 0, 0.2f + PI / 3.0f);
        auto analog_sig_source_current2 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 2, 0, 0.2f + PI * 2.0f / 3.0f);

        auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f);
        auto analog_sig_source_voltage1 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, PI / 3.0f);
        auto analog_sig_source_voltage2 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 2.0f * PI / 3.0f);

        auto band_pass_filter_current0  = gr::filter::fft_filter_fff::make(
                 bp_decimation,
                 gr::filter::firdes::band_pass(
                         1,
                         in_samp_rate,
                         bp_low_cut,
                         bp_high_cut,
                         bp_trans,
                         gr::fft::window::win_type::WIN_HAMMING,
                         6.76));
        auto band_pass_filter_current1 = gr::filter::fft_filter_fff::make(
                bp_decimation,
                gr::filter::firdes::band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto band_pass_filter_current2 = gr::filter::fft_filter_fff::make(
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
        auto band_pass_filter_voltage1 = gr::filter::fft_filter_fff::make(
                bp_decimation,
                gr::filter::firdes::band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto band_pass_filter_voltage2 = gr::filter::fft_filter_fff::make(
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
        auto analog_sig_source_phase1_sin = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase2_sin = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase0_cos = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase1_cos = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase2_cos = gr::analog::sig_source_f::make(out_samp_rate, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

        auto blocks_multiply_phase0_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase0_3     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase1_3     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_0     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_1     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_2     = gr::blocks::multiply_ff::make(1);
        auto blocks_multiply_phase2_3     = gr::blocks::multiply_ff::make(1);

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
        auto low_pass_filter_current1_0 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_current1_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage1_0 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage1_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_current2_0 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_current2_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage2_0 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));
        auto low_pass_filter_voltage2_1 = gr::filter::fft_filter_fff::make(
                lp_decimation,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_divide_phase0_0                = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase0_1                = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase1_0                = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase1_1                = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase2_0                = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase2_1                = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase0_0        = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase0_1        = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase1_0        = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase1_1        = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase2_0        = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase2_1        = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase0                     = gr::blocks::sub_ff::make(1);
        auto blocks_sub_phase1                     = gr::blocks::sub_ff::make(1);
        auto blocks_sub_phase2                     = gr::blocks::sub_ff::make(1);

        auto pulsed_power_power_calc_mul_ph_ff_0_0 = gr::pulsed_power::power_calc_mul_ph_ff::make(0.0001);

        auto throttle_block_p                      = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_q                      = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_s                      = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_phi                    = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_raw_current0           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_voltage0           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_current1           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_voltage1           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_current2           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_voltage2           = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_bandpass_current0      = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_bandpass_voltage0      = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);

        auto pulsed_power_opencmw_time_sink_power  = gr::pulsed_power::opencmw_time_sink::make(
                 { "P", "Q", "S", "phi" },
                 {},
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

        auto null_sink_0  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_1  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_2  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_3  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_4  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_5  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_6  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_7  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_8  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_9  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_10 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_11 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_12 = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_13 = gr::blocks::null_sink::make(sizeof(float));

        // Connections:
        // Phase 0:
        top->hier_block2::connect(analog_sig_source_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(analog_sig_source_current0, 0, throttle_block_raw_current0, 0);
        top->hier_block2::connect(analog_sig_source_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(analog_sig_source_voltage0, 0, throttle_block_raw_voltage0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 1);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_opencmw_time_sink_bpf_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_opencmw_time_sink_bpf_0, 1);
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, pulsed_power_opencmw_time_sink_raw_0, 0); // U_0
        top->hier_block2::connect(throttle_block_raw_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_0
        // top->hier_block2::connect(throttle_block_raw_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_0
        // top->hier_block2::connect(throttle_block_raw_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_0
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
        top->hier_block2::connect(blocks_sub_phase0, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 2);

        // Phase 1:
        top->hier_block2::connect(analog_sig_source_current1, 0, band_pass_filter_current1, 0);
        // top->hier_block2::connect(analog_sig_source_current1, 0, throttle_block_raw_current1, 0);
        top->hier_block2::connect(analog_sig_source_voltage1, 0, band_pass_filter_voltage1, 0);
        // top->hier_block2::connect(analog_sig_source_voltage1, 0, throttle_block_raw_voltage1, 0);
        top->hier_block2::connect(band_pass_filter_current1, 0, blocks_multiply_phase1_0, 0);
        top->hier_block2::connect(band_pass_filter_current1, 0, blocks_multiply_phase1_1, 0);
        // top->hier_block2::connect(throttle_block_raw_voltage1, 0, pulsed_power_opencmw_time_sink_raw, 0);   // U_2
        // top->hier_block2::connect(throttle_block_raw_current1, 0, pulsed_power_opencmw_time_sink_power, 1); // I_2
        top->hier_block2::connect(band_pass_filter_current1, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 4);
        top->hier_block2::connect(band_pass_filter_voltage1, 0, blocks_multiply_phase1_2, 0);
        top->hier_block2::connect(band_pass_filter_voltage1, 0, blocks_multiply_phase1_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage1, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 3);
        top->hier_block2::connect(analog_sig_source_phase1_sin, 0, blocks_multiply_phase1_0, 1);
        top->hier_block2::connect(analog_sig_source_phase1_sin, 0, blocks_multiply_phase1_2, 1);
        top->hier_block2::connect(analog_sig_source_phase1_cos, 0, blocks_multiply_phase1_1, 1);
        top->hier_block2::connect(analog_sig_source_phase1_cos, 0, blocks_multiply_phase1_3, 1);
        top->hier_block2::connect(blocks_multiply_phase1_0, 0, low_pass_filter_current1_0, 0);
        top->hier_block2::connect(blocks_multiply_phase1_1, 0, low_pass_filter_current1_1, 0);
        top->hier_block2::connect(blocks_multiply_phase1_2, 0, low_pass_filter_voltage1_0, 0);
        top->hier_block2::connect(blocks_multiply_phase1_3, 0, low_pass_filter_voltage1_1, 0);
        top->hier_block2::connect(low_pass_filter_current1_0, 0, blocks_divide_phase1_0, 0);
        top->hier_block2::connect(low_pass_filter_current1_1, 0, blocks_divide_phase1_0, 1);
        top->hier_block2::connect(low_pass_filter_voltage1_0, 0, blocks_divide_phase1_1, 0);
        top->hier_block2::connect(low_pass_filter_voltage1_1, 0, blocks_divide_phase1_1, 1);
        top->hier_block2::connect(blocks_divide_phase1_0, 0, blocks_transcendental_phase1_0, 0);
        top->hier_block2::connect(blocks_divide_phase1_1, 0, blocks_transcendental_phase1_1, 0);
        top->hier_block2::connect(blocks_transcendental_phase1_0, 0, blocks_sub_phase1, 0);
        top->hier_block2::connect(blocks_transcendental_phase1_1, 0, blocks_sub_phase1, 1);
        top->hier_block2::connect(blocks_sub_phase1, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 5);

        // Phase 2:
        top->hier_block2::connect(analog_sig_source_current2, 0, band_pass_filter_current2, 0);
        top->hier_block2::connect(analog_sig_source_voltage2, 0, band_pass_filter_voltage2, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, blocks_multiply_phase2_0, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, blocks_multiply_phase2_1, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 7);
        top->hier_block2::connect(band_pass_filter_voltage2, 0, blocks_multiply_phase2_2, 0);
        top->hier_block2::connect(band_pass_filter_voltage2, 0, blocks_multiply_phase2_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage2, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 6);
        top->hier_block2::connect(analog_sig_source_phase2_sin, 0, blocks_multiply_phase2_0, 1);
        top->hier_block2::connect(analog_sig_source_phase2_sin, 0, blocks_multiply_phase2_2, 1);
        top->hier_block2::connect(analog_sig_source_phase2_cos, 0, blocks_multiply_phase2_1, 1);
        top->hier_block2::connect(analog_sig_source_phase2_cos, 0, blocks_multiply_phase2_3, 1);
        top->hier_block2::connect(blocks_multiply_phase2_3, 0, low_pass_filter_voltage2_1, 0);
        top->hier_block2::connect(blocks_multiply_phase2_2, 0, low_pass_filter_voltage2_0, 0);
        top->hier_block2::connect(blocks_multiply_phase2_1, 0, low_pass_filter_current2_1, 0);
        top->hier_block2::connect(blocks_multiply_phase2_0, 0, low_pass_filter_current2_0, 0);
        top->hier_block2::connect(low_pass_filter_current2_0, 0, blocks_divide_phase2_0, 0);
        top->hier_block2::connect(low_pass_filter_current2_1, 0, blocks_divide_phase2_0, 1);
        top->hier_block2::connect(low_pass_filter_voltage2_0, 0, blocks_divide_phase2_1, 0);
        top->hier_block2::connect(low_pass_filter_voltage2_1, 0, blocks_divide_phase2_1, 1);
        top->hier_block2::connect(blocks_divide_phase2_0, 0, blocks_transcendental_phase2_0, 0);
        top->hier_block2::connect(blocks_divide_phase2_1, 0, blocks_transcendental_phase2_1, 0);
        top->hier_block2::connect(blocks_transcendental_phase2_0, 0, blocks_sub_phase2, 0);
        top->hier_block2::connect(blocks_transcendental_phase2_1, 0, blocks_sub_phase2, 1);
        top->hier_block2::connect(blocks_sub_phase2, 0, pulsed_power_power_calc_mul_ph_ff_0_0, 8);

        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 0, pulsed_power_opencmw_time_sink_power, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 1, pulsed_power_opencmw_time_sink_power, 1);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 2, pulsed_power_opencmw_time_sink_power, 2);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 3, pulsed_power_opencmw_time_sink_power, 3);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 4, null_sink_0, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 5, null_sink_1, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 6, null_sink_2, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 7, null_sink_3, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 8, null_sink_4, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 9, null_sink_5, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 10, null_sink_6, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 11, null_sink_7, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 12, null_sink_8, 0);  // P_acc
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 13, null_sink_9, 0);  // Q_acc
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_ff_0_0, 14, null_sink_10, 0); // S_acc
    }
    ~GRFlowGraphThreePhaseSimulated() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

class GRFlowGraphThreePhasePicoscope {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraphThreePhasePicoscope(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {}

    ~GRFlowGraphThreePhasePicoscope() {
        top->stop();
    }
    // start gnuradio flowgraph
    void
    start() {
        top->start();
    }
};

class GRFlowGraphOnePhaseSimulated {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraphOnePhaseSimulated(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float  in_samp_rate   = 20'000.0f;
        float  in_samp_rate_2 = 32'000.0f;
        float  out_samp_rate  = 1'000.0f;
        int    bp_decimation  = 20;
        double bp_high_cut    = 80;
        double bp_low_cut     = 20;
        double bp_trans       = 10;
        int    lp_decimation  = 1;

        // blocks
        // U_raw
        auto analog_sig_source_current0 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);
        // I_raw
        auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(in_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f);

        auto band_pass_filter_current0  = gr::filter::fft_filter_fff::make(
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

        auto throttle_block_p                     = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_q                     = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_s                     = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_phi                   = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_raw_current0          = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_raw_voltage0          = gr::blocks::throttle::make(sizeof(float) * 1, in_samp_rate, true);
        auto throttle_block_bandpass_current0     = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);
        auto throttle_block_bandpass_voltage0     = gr::blocks::throttle::make(sizeof(float) * 1, out_samp_rate, true);

        auto pulsed_power_opencmw_time_sink_power = gr::pulsed_power::opencmw_time_sink::make(
                { "P", "Q", "S", "phi" },
                {},
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
        top->hier_block2::connect(analog_sig_source_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(analog_sig_source_current0, 0, throttle_block_raw_current0, 0);
        top->hier_block2::connect(analog_sig_source_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(analog_sig_source_voltage0, 0, throttle_block_raw_voltage0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_power_calc_ff_0_0, 1);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_power_calc_ff_0_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_opencmw_time_sink_bpf_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_opencmw_time_sink_bpf_0, 1);
        top->hier_block2::connect(throttle_block_raw_voltage0, 0, pulsed_power_opencmw_time_sink_raw_0, 0); // U_0
        top->hier_block2::connect(throttle_block_raw_current0, 0, pulsed_power_opencmw_time_sink_raw_0, 1); // I_0
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
    ~GRFlowGraphOnePhaseSimulated() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};
