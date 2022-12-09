// Gnu Radio includes
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/power_calc_mul_ph_ff.h>
#include <gnuradio/top_block.h>

// Johanna
#include <gnuradio/fft/window.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/firdes.h>

class GRFlowGraphThreePhaseSimulated {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraphThreePhaseSimulated(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // parameters
        float  in_samp_rate              = 4'000.0f;
        float  out_samp_rate             = 4'000.0f;
        double bp_decimation             = 20;
        double bp_high_cut               = 80;
        double bp_low_cut                = 20;
        double bp_trans                  = 10;
        double current_correction_factor = 2.5;
        double voltage_correction_factor = 100;
        double lp_decimantion            = 1;

        // blocks
        auto pulsed_power_power_calc_mul_ph_ff_0_0 = gr::pulsed_power::power_calc_mul_ph_ff::make(0.0001);

        auto low_pass_filter_0_1_3_1_0_0           = gr::filter::fft_filter_fff::make(
                          lp_decimantion,
                          gr::filter::firdes::low_pass(
                                  1,
                                  out_samp_rate,
                                  60,
                                  10,
                                  gr::fft::window::win_type::WIN_HAMMING,
                                  6.76));

        auto low_pass_filter_0_1_3_1_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_3_1 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_2_0_1_0_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_2_0_1_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_2_0_1 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_1_0_1_0_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_1_0_1_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_1_0_1 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_0_0_1_0_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_0_0_1_0 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto low_pass_filter_0_1_0_0_1 = gr::filter::fft_filter_fff::make(
                lp_decimantion,
                gr::filter::firdes::low_pass(
                        1,
                        out_samp_rate,
                        60,
                        10,
                        gr::fft::window::win_type::WIN_HAMMING,
                        6.76));

        auto blocks_transcendental_1_1_0_0     = blocks::transcendental::make("atan", "$type");

        auto blocks_transcendental_1_1_0       = blocks::transcendental::make("atan", "$type");

        auto blocks_transcendental_1_1         = blocks::transcendental::make("atan", "$type");

        auto blocks_transcendental_0_0_0_1_0_0 = blocks::transcendental::make("atan", "$type");

        auto blocks_transcendental_0_0_0_1_0   = blocks::transcendental::make("atan", "$type");

        auto blocks_transcendental_0_0_0_1     = blocks::transcendental::make("atan", "$type");

        auto blocks_sub_xx_0_0_1_0_0           = blocks::sub_ff::make(1);

        auto blocks_sub_xx_0_0_1_0             = blocks::sub_ff::make(1);

        auto blocks_sub_xx_0_0_1               = blocks::sub_ff::make(1);

        auto blocks_multiply_xx_0_3_1_0_0      = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_3_1_0        = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_3_1          = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_2_0_1_0_0    = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_2_0_1_0      = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_2_0_1        = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_1_0_1_0_0    = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_1_0_1_0      = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_1_0_1        = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_0_0_1_0_0    = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_0_0_1_0      = blocks::multiply_ff::make(1);

        auto blocks_multiply_xx_0_0_0_1        = blocks::multiply_ff::make(1);

        auto blocks_divide_xx_0_1_1_0_0        = blocks::divide_ff::make(1);

        auto blocks_divide_xx_0_1_1_0          = blocks::divide_ff::make(1);

        auto blocks_divide_xx_0_1_1            = blocks::divide_ff::make(1);

        auto blocks_divide_xx_0_0_0_1_0_0      = blocks::divide_ff::make(1);

        auto blocks_divide_xx_0_0_0_1_0        = blocks::divide_ff::make(1);

        auto blocks_divide_xx_0_0_0_1          = blocks::divide_ff::make(1);

        auto band_pass_filter_0_1_1_0_0        = gr::filter::fft_filter_fff::make(
                       bp_decimantion,
                       firdes.band_pass(
                               1,
                               in_samp_rate,
                               bp_low_cut,
                               bp_high_cut,
                               bp_trans,
                               // window.WIN_HANN,
                               6.76));

        auto band_pass_filter_0_1_1_0 = gr::filter::fft_filter_fff::make(
                bp_decimantion,
                firdes.band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        // window.WIN_HANN,
                        6.76));

        auto band_pass_filter_0_1_1 = gr::filter::fft_filter_fff::make(
                bp_decimantion,
                firdes.band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        // window.WIN_HANN,
                        6.76));

        auto band_pass_filter_0_0_0_1_0_0 = gr::filter::fft_filter_fff::make(
                bp_decimantion,
                firdes.band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        // window.WIN_HANN,
                        6.76));

        auto band_pass_filter_0_0_0_1_0 = gr::filter::fft_filter_fff::make(
                bp_decimantion,
                firdes.band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        // window.WIN_HANN,
                        6.76));

        auto band_pass_filter_0_0_0_1 = gr::filter::fft_filter_fff::make(
                bp_decimantion,
                firdes.band_pass(
                        1,
                        in_samp_rate,
                        bp_low_cut,
                        bp_high_cut,
                        bp_trans,
                        // window.WIN_HANN,
                        6.76));

        auto analog_sig_source_x_1_0_1_0_0     = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0, 0.2);

        auto analog_sig_source_x_1_0_1_0       = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0, 0.2);

        auto analog_sig_source_x_1_0_1         = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0, 0.2);

        auto analog_sig_source_x_0_1_0_1_0_0   = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0, 0);

        auto analog_sig_source_x_0_1_0_1_0     = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0, 0);

        auto analog_sig_source_x_0_1_0_1       = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0, 0);

        auto analog_sig_source_x_0_0_1_0_0     = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0, 0);

        auto analog_sig_source_x_0_0_1_0       = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0, 0);

        auto analog_sig_source_x_0_0_1         = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0, 0);

        auto analog_sig_source_x_0_0_0_0_1_0_0 = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0, 0);

        auto analog_sig_source_x_0_0_0_0_1_0   = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0, 0);

        auto analog_sig_source_x_0_0_0_0_1     = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0, 0);

        // Connections:
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1, 0, auto blocks_multiply_xx_0_0_0_1, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1, 0, auto blocks_multiply_xx_0_1_0_1, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1_0, 0, auto blocks_multiply_xx_0_0_0_1_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1_0, 0, auto blocks_multiply_xx_0_1_0_1_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1_0_0, 0, auto blocks_multiply_xx_0_0_0_1_0_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_0_0_1_0_0, 0, auto blocks_multiply_xx_0_1_0_1_0_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_1, 0, auto band_pass_filter_0_0_0_1, 0);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_1_0, 0, auto band_pass_filter_0_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_0_1_0_0, 0, auto band_pass_filter_0_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1, 0, auto blocks_multiply_xx_0_2_0_1, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1, 0, auto blocks_multiply_xx_0_3_1, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1_0, 0, auto blocks_multiply_xx_0_2_0_1_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1_0, 0, auto blocks_multiply_xx_0_3_1_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1_0_0, 0, auto blocks_multiply_xx_0_2_0_1_0_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_0_1_0_1_0_0, 0, auto blocks_multiply_xx_0_3_1_0_0, 1);
        auto tb->hier_block2::connect(auto analog_sig_source_x_1_0_1, 0, auto band_pass_filter_0_1_1, 0);
        auto tb->hier_block2::connect(auto analog_sig_source_x_1_0_1_0, 0, auto band_pass_filter_0_1_1_0, 0);
        auto tb->hier_block2::connect(auto analog_sig_source_x_1_0_1_0_0, 0, auto band_pass_filter_0_1_1_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1, 0, auto blocks_multiply_xx_0_1_0_1, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1, 0, auto blocks_multiply_xx_0_3_1, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 6);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0, 0, auto blocks_multiply_xx_0_1_0_1_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0, 0, auto blocks_multiply_xx_0_3_1_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 3);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0_0, 0, auto blocks_multiply_xx_0_1_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0_0, 0, auto blocks_multiply_xx_0_3_1_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_0_0_1_0_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1, 0, auto blocks_multiply_xx_0_0_0_1, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1, 0, auto blocks_multiply_xx_0_2_0_1, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 7);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0, 0, auto blocks_multiply_xx_0_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0, 0, auto blocks_multiply_xx_0_2_0_1_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 4);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0_0, 0, auto blocks_multiply_xx_0_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0_0, 0, auto blocks_multiply_xx_0_2_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto band_pass_filter_0_1_1_0_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 1);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_0_0_1, 0, auto blocks_transcendental_0_0_0_1, 0);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_0_0_1_0, 0, auto blocks_transcendental_0_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_0_0_1_0_0, 0, auto blocks_transcendental_0_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_1_1, 0, auto blocks_transcendental_1_1, 0);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_1_1_0, 0, auto blocks_transcendental_1_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_divide_xx_0_1_1_0_0, 0, auto blocks_transcendental_1_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_0_0_1, 0, auto low_pass_filter_0_1_0_0_1, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_0_0_1_0, 0, auto low_pass_filter_0_1_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_0_0_1_0_0, 0, auto low_pass_filter_0_1_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_1_0_1, 0, auto low_pass_filter_0_1_1_0_1, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_1_0_1_0, 0, auto low_pass_filter_0_1_1_0_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_1_0_1_0_0, 0, auto low_pass_filter_0_1_1_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_2_0_1, 0, auto low_pass_filter_0_1_2_0_1, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_2_0_1_0, 0, auto low_pass_filter_0_1_2_0_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_2_0_1_0_0, 0, auto low_pass_filter_0_1_2_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_3_1, 0, auto low_pass_filter_0_1_3_1, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_3_1_0, 0, auto low_pass_filter_0_1_3_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_multiply_xx_0_3_1_0_0, 0, auto low_pass_filter_0_1_3_1_0_0, 0);
        auto tb->hier_block2::connect(auto blocks_sub_xx_0_0_1, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 8);
        auto tb->hier_block2::connect(auto blocks_sub_xx_0_0_1_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 5);
        auto tb->hier_block2::connect(auto blocks_sub_xx_0_0_1_0_0, 0, auto pulsed_power_power_calc_mul_ph_ff_0_0, 2);
        auto tb->hier_block2::connect(auto blocks_transcendental_0_0_0_1, 0, auto blocks_sub_xx_0_0_1, 1);
        auto tb->hier_block2::connect(auto blocks_transcendental_0_0_0_1_0, 0, auto blocks_sub_xx_0_0_1_0, 1);
        auto tb->hier_block2::connect(auto blocks_transcendental_0_0_0_1_0_0, 0, auto blocks_sub_xx_0_0_1_0_0, 1);
        auto tb->hier_block2::connect(auto blocks_transcendental_1_1, 0, auto blocks_sub_xx_0_0_1, 0);
        auto tb->hier_block2::connect(auto blocks_transcendental_1_1_0, 0, auto blocks_sub_xx_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto blocks_transcendental_1_1_0_0, 0, auto blocks_sub_xx_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_0_0_1, 0, auto blocks_divide_xx_0_0_0_1, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_0_0_1_0, 0, auto blocks_divide_xx_0_0_0_1_0, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_0_0_1_0_0, 0, auto blocks_divide_xx_0_0_0_1_0_0, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_1_0_1, 0, auto blocks_divide_xx_0_1_1, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_1_0_1_0, 0, auto blocks_divide_xx_0_1_1_0, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_1_0_1_0_0, 0, auto blocks_divide_xx_0_1_1_0_0, 1);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_2_0_1, 0, auto blocks_divide_xx_0_0_0_1, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_2_0_1_0, 0, auto blocks_divide_xx_0_0_0_1_0, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_2_0_1_0_0, 0, auto blocks_divide_xx_0_0_0_1_0_0, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_3_1, 0, auto blocks_divide_xx_0_1_1, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_3_1_0, 0, auto blocks_divide_xx_0_1_1_0, 0);
        auto tb->hier_block2::connect(auto low_pass_filter_0_1_3_1_0_0, 0, auto blocks_divide_xx_0_1_1_0_0, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 11, auto qtgui_time_sink_x_0, 2);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 9, auto qtgui_time_sink_x_0, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 10, auto qtgui_time_sink_x_0, 1);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 7, auto qtgui_time_sink_x_1, 2);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 1, auto qtgui_time_sink_x_1, 1);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 0, auto qtgui_time_sink_x_1, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 13, auto qtgui_time_sink_x_2, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 14, auto qtgui_time_sink_x_2, 1);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 2, auto qtgui_time_sink_x_2, 2);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 12, auto qtgui_time_sink_x_2_0, 1);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 8, auto qtgui_time_sink_x_2_0, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 3, auto qtgui_time_sink_x_2_0, 2);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 6, auto qtgui_time_sink_x_2_1, 2);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 4, auto qtgui_time_sink_x_2_1, 0);
        auto tb->hier_block2::connect(auto pulsed_power_power_calc_mul_ph_ff_0_0, 5, auto qtgui_time_sink_x_2_1, 1);
    }

    ~GRFlowGraphThreePhaseSimulated() { top->stop(); }

    // start gnuradio flowgraph
    void start() { top->start(); }
};
