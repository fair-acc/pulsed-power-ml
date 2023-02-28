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
        ///flowgraph setup (dummy test data setup)
        const float samp_rate = 4'000.0f; //check sampel rate & decimation before merge

        ///sinus_signal --> throttle --> opencmw_time_sink
        // first phase:
        auto signal_source_0_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 0.5, 5, 0, 0);
        auto throttle_block_0_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_0_1 = gr::pulsed_power::opencmw_time_sink::make({ "sinus", "square" }, { "V", "A" }, samp_rate);
        pulsed_power_opencmw_sink_0_1->set_max_noutput_items(noutput_items);
        // // second phase:
        // auto signal_source_0_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 0.5, 5, 0, 0);
        // auto throttle_block_0_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_0_2 = gr::pulsed_power::opencmw_time_sink::make({ "sinus", "square" }, { "V", "A" }, samp_rate);
        // pulsed_power_opencmw_sink_0_2->set_max_noutput_items(noutput_items);
        // // third phase:
        // auto signal_source_0_3             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 0.5, 5, 0, 0);
        // auto throttle_block_0_3            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_0_3 = gr::pulsed_power::opencmw_time_sink::make({ "sinus", "square" }, { "V", "A" }, samp_rate);
        // pulsed_power_opencmw_sink_0_3->set_max_noutput_items(noutput_items);

        ///saw_signal --> throttle --> opencmw_time_sink
        // first phase:
        auto signal_source_1_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        auto throttle_block_1_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_1_1 = gr::pulsed_power::opencmw_time_sink::make({ "saw" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_1_1->set_max_noutput_items(noutput_items);
        // // second phase: saw_signal --> throttle --> opencmw_time_sink
        // auto signal_source_1_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        // auto throttle_block_1_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_1_2 = gr::pulsed_power::opencmw_time_sink::make({ "saw" }, { "A" }, samp_rate);
        // pulsed_power_opencmw_sink_1_2->set_max_noutput_items(noutput_items);
        // // third phase: saw_signal --> throttle --> opencmw_time_sink
        // auto signal_source_1_3             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        // auto throttle_block_1_3            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_1_3 = gr::pulsed_power::opencmw_time_sink::make({ "saw" }, { "A" }, samp_rate);
        // pulsed_power_opencmw_sink_1_3->set_max_noutput_items(noutput_items);

        ///square_signal --> throttle --> opencmw_time_sink
        //first phase:
        auto signal_source_2_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        auto throttle_block_2_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_2_1 = gr::pulsed_power::opencmw_time_sink::make({ "square" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_2_1->set_max_noutput_items(noutput_items);
        // //second phase:
        // auto signal_source_2_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        // auto throttle_block_2_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_2_2 = gr::pulsed_power::opencmw_time_sink::make({ "square" }, { "A" }, samp_rate);
        // pulsed_power_opencmw_sink_2_2->set_max_noutput_items(noutput_items);
        // //third phase:
        // auto signal_source_2_3             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        // auto throttle_block_2_3            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        // auto pulsed_power_opencmw_sink_2_3 = gr::pulsed_power::opencmw_time_sink::make({ "square" }, { "A" }, samp_rate);
        // pulsed_power_opencmw_sink_2_3->set_max_noutput_items(noutput_items);

        ///sinus_signal --> throttle --> stream_to_vector --> fft --> fast_multiply_constant --> complex_to_mag^2 --> log10 --> opencmw_freq_sink
        //-- todo (three phases)
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

        ///nilm worker (time and frequency sink) //expanded for three phases
        auto nilm_time_sink = gr::pulsed_power::opencmw_time_sink::make({ "P_1", "Q_1", "S_1", "Phi_1", "P_2", "Q_2", "S_2", "Phi_2", "P_3", "Q_3", "S_3", "Phi_3", "P_acc", "Q_acc", "S_acc" }, { "W", "Var", "VA", "deg", "W", "Var", "VA", "deg","W", "Var", "VA", "deg","W", "Var", "VA" }, samp_rate);
        nilm_time_sink->set_max_noutput_items(noutput_items);
        
        auto nilm_freq_sink = gr::pulsed_power::opencmw_freq_sink::make({ "S", "U", "I" }, { "dB", "dB", "dB" }, samp_rate, samp_rate);
        nilm_freq_sink->set_max_noutput_items(noutput_items);

        ///connections
        ///time-domain sinks
        //first phase
        top->hier_block2::connect(signal_source_0_1, 0, throttle_block_0_1, 0);
        top->hier_block2::connect(throttle_block_0_1, 0, pulsed_power_opencmw_sink_0_1, 0);

        top->hier_block2::connect(signal_source_1_1, 0, throttle_block_1_1, 0);
        top->hier_block2::connect(throttle_block_1_1, 0, pulsed_power_opencmw_sink_1_1, 0);

        top->hier_block2::connect(signal_source_2_1, 0, throttle_block_2_1, 0);
        top->hier_block2::connect(throttle_block_2_1, 0, pulsed_power_opencmw_sink_0_1, 1);

        // //second phase
        // top->hier_block2::connect(signal_source_0_2, 0, throttle_block_0_2, 0);
        // top->hier_block2::connect(throttle_block_0_2, 0, pulsed_power_opencmw_sink_0_2, 0);

        // top->hier_block2::connect(signal_source_1_2, 0, throttle_block_1_2, 0);
        // top->hier_block2::connect(throttle_block_1_2, 0, pulsed_power_opencmw_sink_1_2, 0);

        // top->hier_block2::connect(signal_source_2_2, 0, throttle_block_2_2, 0);
        // top->hier_block2::connect(throttle_block_2_2, 0, pulsed_power_opencmw_sink_0_2, 1);

        // //third Phase
        // top->hier_block2::connect(signal_source_0_3, 0, throttle_block_0_3, 0);
        // top->hier_block2::connect(throttle_block_0_3, 0, pulsed_power_opencmw_sink_0_3, 0);
        
        // top->hier_block2::connect(signal_source_1_3, 0, throttle_block_1_3, 0);
        // top->hier_block2::connect(throttle_block_1_3, 0, pulsed_power_opencmw_sink_1_3, 0);

        // top->hier_block2::connect(signal_source_2_3, 0, throttle_block_2_3, 0);
        // top->hier_block2::connect(throttle_block_2_3, 0, pulsed_power_opencmw_sink_0_3, 1);

        // frequency-domain sinks //-- todo (three phases)
        top->hier_block2::connect(signal_source_3, 0, throttle_block_3, 0);
        top->hier_block2::connect(throttle_block_3, 0, stream_to_vector_0, 0);
        top->hier_block2::connect(stream_to_vector_0, 0, fft_vxx_0, 0);
        top->hier_block2::connect(fft_vxx_0, 0, multiply_const_xx_0, 0);
        top->hier_block2::connect(multiply_const_xx_0, 0, complex_to_mag_squared_0, 0);
        top->hier_block2::connect(complex_to_mag_squared_0, 0, nlog10_ff_0, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, pulsed_power_opencmw_freq_sink_0, 0);

        // nilm worker (time and frequency sink)
        top->hier_block2::connect(throttle_block_0_1, 0, nilm_time_sink, 0);
        top->hier_block2::connect(throttle_block_1_1, 0, nilm_time_sink, 1);
        top->hier_block2::connect(throttle_block_2_1, 0, nilm_time_sink, 2);
        top->hier_block2::connect(throttle_block_3, 0, nilm_time_sink, 3);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 1);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 2);
    }

    ~GRFlowGraph() { top->stop(); }

    void start() { top->start(); }
};

class PulsedPowerThreePhaseFlowgraph {
private:
    gr::top_block_sptr top;

public:
    PulsedPowerThreePhaseFlowgraph(int noutput_items, bool use_picoscope = false)
        : top(gr::make_top_block("GNURadio")) {
        float source_samp_rate          = 200'000.0f;
        //first phase
        auto  source_interface_voltage0 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current0 = gr::blocks::multiply_const_ff::make(1);
        //second phase
        auto  source_interface_voltage1 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current1 = gr::blocks::multiply_const_ff::make(1);
        //third phase
        auto  source_interface_voltage2 = gr::blocks::multiply_const_ff::make(1);
        auto  source_interface_current2 = gr::blocks::multiply_const_ff::make(1);
        if (use_picoscope) { //-- todo: opencmw sinks, mains frequency --//
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
            picoscope_source->set_aichan_a(true, 5, picoscope_coupling, 0.0); //i_1
            picoscope_source->set_aichan_b(true, 1, picoscope_coupling, 0.0); //u_1
            picoscope_source->set_aichan_c(true, 5, picoscope_coupling, 0.0); //i_2
            picoscope_source->set_aichan_d(true, 1, picoscope_coupling, 0.0); //u_2
            picoscope_source->set_aichan_e(true, 5, picoscope_coupling, 0.0); //i_3
            picoscope_source->set_aichan_f(true, 1, picoscope_coupling, 0.0); //u_3
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
            auto voltage1            = gr::blocks::multiply_const_ff::make(voltage_correction_factor);
            auto current1            = gr::blocks::multiply_const_ff::make(current_correction_factor);
            auto voltage2            = gr::blocks::multiply_const_ff::make(voltage_correction_factor);
            auto current2            = gr::blocks::multiply_const_ff::make(current_correction_factor);

            // connections
            top->hier_block2::connect(picoscope_source, 0, voltage0, 0);  //i_1
            top->hier_block2::connect(picoscope_source, 2, current0, 0);  //u_1
            top->hier_block2::connect(picoscope_source, 4, voltage1, 0);  //i_2
            top->hier_block2::connect(picoscope_source, 6, current1, 0);  //u_2
            top->hier_block2::connect(picoscope_source, 8, voltage2, 0);  //i_3
            top->hier_block2::connect(picoscope_source, 10, current2, 0); //u_3
            top->hier_block2::connect(picoscope_source, 1, null_sink_picoscope, 0);
            top->hier_block2::connect(picoscope_source, 3, null_sink_picoscope, 1);
            top->hier_block2::connect(picoscope_source, 5, null_sink_picoscope, 2);
            top->hier_block2::connect(picoscope_source, 7, null_sink_picoscope, 3);
            top->hier_block2::connect(picoscope_source, 9, null_sink_picoscope, 4);
            top->hier_block2::connect(picoscope_source, 11, null_sink_picoscope, 5);
            top->hier_block2::connect(picoscope_source, 12, null_sink_picoscope, 6);
            top->hier_block2::connect(picoscope_source, 13, null_sink_picoscope, 7);
            top->hier_block2::connect(picoscope_source, 14, null_sink_picoscope, 8);
            top->hier_block2::connect(picoscope_source, 15, null_sink_picoscope, 9);
            top->hier_block2::connect(voltage0, 0, source_interface_voltage0, 0);
            top->hier_block2::connect(current0, 0, source_interface_current0, 0);
            top->hier_block2::connect(voltage1, 0, source_interface_voltage1, 0);
            top->hier_block2::connect(current1, 0, source_interface_current1, 0);
            top->hier_block2::connect(voltage2, 0, source_interface_voltage2, 0);
            top->hier_block2::connect(current2, 0, source_interface_current2, 0);
        } else {
            ///blocks
            // first phase
            auto analog_sig_source_voltage0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
            auto analog_sig_source_current0 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);  // I_raw
            // second phase
            auto analog_sig_source_voltage1 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
            auto analog_sig_source_current1 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);  // I_raw
            // third phase
            auto analog_sig_source_voltage2 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 325, 0, 0.0f); // U_raw
            auto analog_sig_source_current2 = gr::analog::sig_source_f::make(source_samp_rate, gr::analog::GR_SIN_WAVE, 50, 50, 0, 0.2f);  // I_raw
            // first phase
            auto voltage0                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            auto current0                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            // second phase
            auto voltage1                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            auto current1                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            // third phase
            auto voltage2                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);
            auto current2                   = gr::blocks::throttle::make(sizeof(float) * 1, source_samp_rate, true);

            // connections
            top->hier_block2::connect(analog_sig_source_voltage0, 0, voltage0, 0);
            top->hier_block2::connect(analog_sig_source_current0, 0, current0, 0);
            top->hier_block2::connect(analog_sig_source_voltage1, 0, voltage0, 0);
            top->hier_block2::connect(analog_sig_source_current1, 0, current0, 0);
            top->hier_block2::connect(analog_sig_source_voltage2, 0, voltage0, 0);
            top->hier_block2::connect(analog_sig_source_current2, 0, current0, 0);

            top->hier_block2::connect(voltage0, 0, source_interface_voltage0, 0);
            top->hier_block2::connect(current0, 0, source_interface_current0, 0);
            top->hier_block2::connect(voltage1, 0, source_interface_voltage1, 0);
            top->hier_block2::connect(current1, 0, source_interface_current1, 0);
            top->hier_block2::connect(voltage2, 0, source_interface_voltage2, 0);
            top->hier_block2::connect(current2, 0, source_interface_current2, 0);
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
        //--todo/ask--//
        // parameters nilm
        const int fft_size    = 131072;
        size_t    vector_size = static_cast<size_t>(fft_size);
        float     bandwidth   = source_samp_rate;

        // blocks (nilm? no extension done here)
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

        auto integrate                     = gr::pulsed_power::integration::make(10, 1000);
        //--end todo/ask--//
        // phase 0
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
        // phase 1
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
        // phase 2
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
        // phase 0
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
        // phase 1
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
        // phase 2
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
        auto analog_sig_source_phase1_sin = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase1_cos = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase2_sin = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_SIN_WAVE, 55, 1, 0, 0.0f);
        auto analog_sig_source_phase2_cos = gr::analog::sig_source_f::make(samp_rate_delta_phi_calc, gr::analog::GR_COS_WAVE, 55, 1, 0, 0.0f);

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
        //phase 0
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
        //phase 1
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
        //phase 2
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
        //phase 0
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
        //phase 1
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
        //phase 2
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

        auto blocks_divide_phase0_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase0_1                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase1_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase1_1                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase2_0                   = gr::blocks::divide_ff::make(1);
        auto blocks_divide_phase2_1                   = gr::blocks::divide_ff::make(1);

        auto blocks_transcendental_phase0_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase0_1           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase1_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase1_1           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase2_0           = gr::blocks::transcendental::make("atan");
        auto blocks_transcendental_phase2_1           = gr::blocks::transcendental::make("atan");

        auto blocks_sub_phase0                        = gr::blocks::sub_ff::make(1);
        auto blocks_sub_phase1                        = gr::blocks::sub_ff::make(1);
        auto blocks_sub_phase2                        = gr::blocks::sub_ff::make(1);

        auto pulsed_power_power_calc_mul_ph_0_0           = gr::pulsed_power::power_calc_mul_ph_ff::make(0.001);

        auto out_decimation_current0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage0                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_current1                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage1                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_current2                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        auto out_decimation_voltage2                  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_raw);
        //--todo/ask--//
        auto out_decimation_current_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);
        auto out_decimation_voltage_bpf               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_bpf);

        auto out_decimation_mains_frequency_shortterm = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_short_term);
        auto out_decimation_mains_frequency_midterm   = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_mid_term);
        auto out_decimation_mains_frequency_longterm  = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mains_freq_long_term);
        //--end todo/ask--//
        //phase 0
        auto out_decimation_p_0_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_q_0_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_s_0_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_phi_0_shortterm             = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        auto out_decimation_p_0_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_q_0_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_s_0_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_phi_0_midterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        auto out_decimation_p_0_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_q_0_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_s_0_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        auto out_decimation_phi_0_longterm              = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // //phase 1
        // auto out_decimation_p_1_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_q_1_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_s_1_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_phi_1_shortterm             = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_p_1_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_q_1_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_s_1_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_phi_1_midterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_p_1_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_q_1_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_s_1_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_phi_1_longterm              = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // //phase 2
        // auto out_decimation_p_2_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_q_2_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_s_2_shortterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_phi_2_shortterm             = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_short_term);
        // auto out_decimation_p_2_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_q_2_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_s_2_midterm                 = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_phi_2_midterm               = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_mid_term);
        // auto out_decimation_p_2_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_q_2_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_s_2_longterm                = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // auto out_decimation_phi_2_longterm              = gr::blocks::keep_one_in_n::make(sizeof(float), decimation_out_long_term);
        // phase 0
        auto statistics_p_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_q_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_s_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_phi_0_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_p_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_q_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_s_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_phi_0_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_p_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_q_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_s_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_phi_0_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // //phase 1
        // auto statistics_p_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_q_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_s_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_phi_1_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_p_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_q_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_s_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_phi_1_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_p_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_q_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_s_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // //phase 2
        // auto statistics_p_2_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_q_2_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_s_2_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_phi_2_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_p_2_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_q_2_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_s_2_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_phi_2_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_p_2_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_q_2_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_s_2_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_phi_2_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        //phase 0
        auto statistics_phi_0_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_p_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_q_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_s_0_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_phi_0_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        auto statistics_p_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_q_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_s_0_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_phi_0_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        auto statistics_p_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_q_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_s_0_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        auto statistics_phi_0_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // //pase 1
        // auto statistics_phi_1_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_p_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_q_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_s_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_phi_1_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_p_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_q_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_s_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_phi_1_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_p_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_q_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_s_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_phi_1_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // //phase 2
        // auto statistics_phi_1_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_p_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_q_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_s_1_shortterm                   = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_phi_1_shortterm                 = gr::pulsed_power::statistics::make(decimation_out_short_term);
        // auto statistics_p_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_q_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_s_1_midterm                     = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_phi_1_midterm                   = gr::pulsed_power::statistics::make(decimation_out_mid_term);
        // auto statistics_p_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_q_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_s_1_longterm                    = gr::pulsed_power::statistics::make(decimation_out_long_term);
        // auto statistics_phi_1_longterm                  = gr::pulsed_power::statistics::make(decimation_out_long_term);

        //--todo/ask--//
        auto opencmw_time_sink_signals                = gr::pulsed_power::opencmw_time_sink::make(
                               { 
                                "U_0", "I_0", "U_0_bpf", "I_0_bpf"
                                // ,"U_1", "I_1", "U_1_bpf", "I_1_bpf",
                                // "U_2", "I_2", "U_2_bpf", "I_2_bpf"
                               },
                               { 
                                "V", "A", "V", "A"
                                // ,"V", "A", "V", "A",
                                // "V", "A", "V", "A"
                                },
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
                { "P_0", "Q_0", "S_0", "phi_0" /*,"P_1", "Q_1", "S_1", "phi_1", "P_2", "Q_2", "S_2", "phi_2", "P_acc", "Q_acc", "S_acc"*/ },
                { "W", "Var", "VA", "rad", "W" /* ,"Var", "VA", "rad", "W", "Var", "VA", "rad","W", "Var", "VA"*/ },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_power_shortterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_midterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P_0", "Q_0", "S_0", "phi_0", /*"P_1", "Q_1", "S_1", "phi_1", "P_2", "Q_2", "S_2", "phi_2", "P_acc", "Q_acc", "S_acc"*/ },
                { "W", "Var", "VA", "rad", "W", /*"Var", "VA", "rad", "W", "Var", "VA", "rad", "W", "Var", "VA" */},
                out_samp_rate_power_midterm);
        opencmw_time_sink_power_midterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { "P_0", "Q_0", "S_0", "phi_0", /*"P_1", "Q_1", "S_1", "phi_1", "P_2", "Q_2", "S_2", "phi_2", "P_acc", "Q_acc", "S_acc"*/ },
                { "W", "Var", "VA", "rad", "W", /*"Var", "VA", "rad", "W", "Var", "VA", "rad", "W", "Var", "VA" */},
                out_samp_rate_power_longterm);
        opencmw_time_sink_power_longterm->set_max_noutput_items(noutput_items);
        //-- todo/ask --//
        // Integral sink
        auto opencmw_time_sink_int_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { "S_Int" },
                { "Wh" },
                out_samp_rate_power_shortterm);
        
        // Statistic sinks
        //one sink for all? /one per phase? + statistics acc values needed?
        auto opencmw_time_sink_power_stats_shortterm = gr::pulsed_power::opencmw_time_sink::make(
                { 
                        "P_0_mean", "P_0_min", "P_0_max", "Q_0_mean", "Q_0_min", "Q_0_max", "S_0_mean", "S_0_min", "S_0_max", "phi_0_mean", "phi_0_min", "phi_0_max"
                        // ,"P_1_mean", "P_1_min", "P_1_max", "Q_1_mean", "Q_1_min", "Q_1_max", "S_1_mean", "S_1_min", "S_1_max", "phi_1_mean", "phi_1_min", "phi_1_max"
                        // ,"P_2_mean", "P_2_min", "P_2_max", "Q_2_mean", "Q_2_min", "Q_2_max", "S_2_mean", "S_2_min", "S_2_max", "phi_2_mean", "phi_2_min", "phi_2_max" 
                },
                { 
                        "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad",
                        // ,"W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad"
                        // ,"W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" 
                },
                out_samp_rate_power_shortterm);
        opencmw_time_sink_power_stats_shortterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_stats_midterm = gr::pulsed_power::opencmw_time_sink::make(
                { 
                        "P_0_mean", "P_0_min", "P_0_max", "Q_0_mean", "Q_0_min", "Q_0_max", "S_0_mean", "S_0_min", "S_0_max", "phi_0_mean", "phi_0_min", "phi_0_max"
                        // ,"P_1_mean", "P_1_min", "P_1_max", "Q_1_mean", "Q_1_min", "Q_1_max", "S_1_mean", "S_1_min", "S_1_max", "phi_1_mean", "phi_1_min", "phi_1_max" 
                        // ,"P_2_mean", "P_2_min", "P_2_max", "Q_2_mean", "Q_2_min", "Q_2_max", "S_2_mean", "S_2_min", "S_2_max", "phi_2_mean", "phi_2_min", "phi_2_max" 
                },
                { 
                        "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad"
                        // ,"W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" 
                        // ,"W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" 
                },
                out_samp_rate_power_midterm);
        opencmw_time_sink_power_stats_midterm->set_max_noutput_items(noutput_items);
        auto opencmw_time_sink_power_stats_longterm = gr::pulsed_power::opencmw_time_sink::make(
                { 
                        "P_0_mean", "P_0_min", "P_0_max", "Q_0_mean", "Q_0_min", "Q_0_max", "S_0_mean", "S_0_min", "S_0_max", "phi_0_mean", "phi_0_min", "phi_0_max" 
                        // "P_1_mean", "P_1_min", "P_1_max", "Q_1_mean", "Q_1_min", "Q_1_max", "S_1_mean", "S_1_min", "S_1_max", "phi_1_mean", "phi_1_min", "phi_1_max" 
                        // "P_2_mean", "P_2_min", "P_2_max", "Q_2_mean", "Q_2_min", "Q_2_max", "S_2_mean", "S_2_min", "S_2_max", "phi_2_mean", "phi_2_min", "phi_2_max" 
                },
                { 
                        "W", "W", "W", "Var", "Var", "Var", "VA", "VA", "VA", "rad", "rad", "rad" 
                },
                out_samp_rate_power_longterm);
        opencmw_time_sink_power_stats_longterm->set_max_noutput_items(noutput_items);
        
        auto null_sink_stats_shortterm = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_stats_midterm   = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_stats_longterm  = gr::blocks::null_sink::make(sizeof(float));
        auto null_sink_phase_1_2 = gr::blocks::null_sink::make(sizeof(float));
        //-- todo/ask --//
        // Frequency spectra sinks
        auto frequency_spec_pulsed_power_opencmw_freq_sink = gr::pulsed_power::opencmw_freq_sink::make(
                { "sinus_fft" },
                { "W" }, 50, 50, 512);
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
        //-- end todo/ask --//
        //-- todo: connect rest of outputs (mul_ph_ff) with null sinks
        ///Connections:
        ///signal
        // phase 0
        top->hier_block2::connect(source_interface_voltage0, 0, out_decimation_voltage0, 0);
        top->hier_block2::connect(source_interface_current0, 0, out_decimation_current0, 0);
        top->hier_block2::connect(out_decimation_voltage0, 0, opencmw_time_sink_signals, 0); // U_raw 
        top->hier_block2::connect(out_decimation_current0, 0, opencmw_time_sink_signals, 1); // I_raw
        // phase 1
        top->hier_block2::connect(source_interface_voltage1, 0, out_decimation_voltage1, 0);
        top->hier_block2::connect(source_interface_current1, 0, out_decimation_current1, 0);
        //-- todo/ask: opencmw time sink extension --//
        // top->hier_block2::connect(out_decimation_voltage1, 0, opencmw_time_sink_signals, 4); // U_raw
        // top->hier_block2::connect(out_decimation_current1, 0, opencmw_time_sink_signals, 5); // I_raw
        // phase 2
        top->hier_block2::connect(source_interface_voltage2, 0, out_decimation_voltage2, 0);
        top->hier_block2::connect(source_interface_current2, 0, out_decimation_current2, 0);
        // top->hier_block2::connect(out_decimation_voltage2, 0, opencmw_time_sink_signals, 8); // U_raw
        // top->hier_block2::connect(out_decimation_current2, 0, opencmw_time_sink_signals, 9); // I_raw

        ///Mains frequency //-- todo
        // phase 0
        top->hier_block2::connect(source_interface_voltage0, 0, calc_mains_frequency, 0);
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_shortterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_shortterm, 0, opencmw_time_sink_mains_freq_shortterm, 0); // mains_freq short-term
        // // phase 1
        // top->hier_block2::connect(source_interface_voltage1, 0, calc_mains_frequency, 0);
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_shortterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_shortterm, 0, opencmw_time_sink_mains_freq_shortterm, 0); // mains_freq short-term
        // // phase 2
        // top->hier_block2::connect(source_interface_voltage2, 0, calc_mains_frequency, 0);
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_shortterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_shortterm, 0, opencmw_time_sink_mains_freq_shortterm, 0); // mains_freq short-term
        // phase 0
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_midterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_midterm, 0, opencmw_time_sink_mains_freq_midterm, 0); // mains_freq mid-term
        top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_longterm, 0);
        top->hier_block2::connect(out_decimation_mains_frequency_longterm, 0, opencmw_time_sink_mains_freq_longterm, 0); // mains_freq long-term
        // // phase 1
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_midterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_midterm, 0, opencmw_time_sink_mains_freq_midterm, 0); // mains_freq mid-term
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_longterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_longterm, 0, opencmw_time_sink_mains_freq_longterm, 0); // mains_freq long-term
        // // phase 2
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_midterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_midterm, 0, opencmw_time_sink_mains_freq_midterm, 0); // mains_freq mid-term
        // top->hier_block2::connect(calc_mains_frequency, 0, out_decimation_mains_frequency_longterm, 0);
        // top->hier_block2::connect(out_decimation_mains_frequency_longterm, 0, opencmw_time_sink_mains_freq_longterm, 0); // mains_freq long-term

        ///Bandpass filter
        // phase 0
        top->hier_block2::connect(source_interface_voltage0, 0, band_pass_filter_voltage0, 0);
        top->hier_block2::connect(source_interface_current0, 0, band_pass_filter_current0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, opencmw_time_sink_signals, 2); // U_bpf
        top->hier_block2::connect(band_pass_filter_current0, 0, opencmw_time_sink_signals, 3); // I_bpf
        // phase 1
        top->hier_block2::connect(source_interface_voltage1, 0, band_pass_filter_voltage1, 0);
        top->hier_block2::connect(source_interface_current1, 0, band_pass_filter_current1, 0);
        // top->hier_block2::connect(band_pass_filter_voltage1, 0, opencmw_time_sink_signals, 2); // U_bpf
        // top->hier_block2::connect(band_pass_filter_current1, 0, opencmw_time_sink_signals, 3); // I_bpf
        // phase 2
        top->hier_block2::connect(source_interface_voltage2, 0, band_pass_filter_voltage2, 0);
        top->hier_block2::connect(source_interface_current2, 0, band_pass_filter_current2, 0);
        // top->hier_block2::connect(band_pass_filter_voltage2, 0, opencmw_time_sink_signals, 2); // U_bpf
        // top->hier_block2::connect(band_pass_filter_current2, 0, opencmw_time_sink_signals, 3); // I_bpf
        ///Calculate phase shift
        ///connection with power_calc_mul_ph_ff
        // phase 0
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, blocks_multiply_phase0_1, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_2, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, blocks_multiply_phase0_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage0, 0, pulsed_power_power_calc_mul_ph_0_0, 0);
        top->hier_block2::connect(band_pass_filter_current0, 0, pulsed_power_power_calc_mul_ph_0_0, 1);
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
        top->hier_block2::connect(blocks_sub_phase0, 0, pulsed_power_power_calc_mul_ph_0_0, 2);
        // phase 1
        top->hier_block2::connect(band_pass_filter_voltage1, 0, blocks_multiply_phase1_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage1, 0, blocks_multiply_phase1_1, 0);
        top->hier_block2::connect(band_pass_filter_current1, 0, blocks_multiply_phase1_2, 0);
        top->hier_block2::connect(band_pass_filter_current1, 0, blocks_multiply_phase1_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage1, 0, pulsed_power_power_calc_mul_ph_0_0, 3);
        top->hier_block2::connect(band_pass_filter_current1, 0, pulsed_power_power_calc_mul_ph_0_0, 4);
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
        top->hier_block2::connect(blocks_sub_phase1, 0, pulsed_power_power_calc_mul_ph_0_0, 5);
        // phase 2
        top->hier_block2::connect(band_pass_filter_voltage2, 0, blocks_multiply_phase2_0, 0);
        top->hier_block2::connect(band_pass_filter_voltage2, 0, blocks_multiply_phase2_1, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, blocks_multiply_phase2_2, 0);
        top->hier_block2::connect(band_pass_filter_current2, 0, blocks_multiply_phase2_3, 0);
        top->hier_block2::connect(band_pass_filter_voltage2, 0, pulsed_power_power_calc_mul_ph_0_0, 6);
        top->hier_block2::connect(band_pass_filter_current2, 0, pulsed_power_power_calc_mul_ph_0_0, 7);
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
        top->hier_block2::connect(blocks_sub_phase2, 0, pulsed_power_power_calc_mul_ph_0_0, 8);
        ///Calculate P, Q, S, phi
        // phase 0
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, out_decimation_p_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, out_decimation_q_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, out_decimation_s_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, out_decimation_phi_0_shortterm, 0);
        top->hier_block2::connect(out_decimation_p_0_shortterm, 0, opencmw_time_sink_power_shortterm, 0);   // P short-term
        top->hier_block2::connect(out_decimation_q_0_shortterm, 0, opencmw_time_sink_power_shortterm, 1);   // Q short-term
        top->hier_block2::connect(out_decimation_s_0_shortterm, 0, opencmw_time_sink_power_shortterm, 2);   // S short-term
        top->hier_block2::connect(out_decimation_phi_0_shortterm, 0, opencmw_time_sink_power_shortterm, 3); // phi short-term
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, out_decimation_p_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, out_decimation_q_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, out_decimation_s_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, out_decimation_phi_0_midterm, 0);
        top->hier_block2::connect(out_decimation_p_0_midterm, 0, opencmw_time_sink_power_midterm, 0);   // P mid-term
        top->hier_block2::connect(out_decimation_q_0_midterm, 0, opencmw_time_sink_power_midterm, 1);   // Q mid-term
        top->hier_block2::connect(out_decimation_s_0_midterm, 0, opencmw_time_sink_power_midterm, 2);   // S mid-term
        top->hier_block2::connect(out_decimation_phi_0_midterm, 0, opencmw_time_sink_power_midterm, 3); // phi mid-term
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, out_decimation_p_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, out_decimation_q_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, out_decimation_s_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, out_decimation_phi_0_longterm, 0);
        top->hier_block2::connect(out_decimation_p_0_longterm, 0, opencmw_time_sink_power_longterm, 0);   // P long-term
        top->hier_block2::connect(out_decimation_q_0_longterm, 0, opencmw_time_sink_power_longterm, 1);   // Q long-term
        top->hier_block2::connect(out_decimation_s_0_longterm, 0, opencmw_time_sink_power_longterm, 2);   // S long-term
        top->hier_block2::connect(out_decimation_phi_0_longterm, 0, opencmw_time_sink_power_longterm, 3); // phi long-term
        // phase 1 //--todo: connect with null sink
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 4, null_sink_phase_1_2, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 5, null_sink_phase_1_2, 1);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 6, null_sink_phase_1_2, 2);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 7, null_sink_phase_1_2, 3);
        // top->hier_block2::connect(out_decimation_p_1_shortterm, 0, opencmw_time_sink_power_shortterm, 0);   // P short-term //--todo: connect opencmw time sinks correct
        // top->hier_block2::connect(out_decimation_q_1_shortterm, 0, opencmw_time_sink_power_shortterm, 1);   // Q short-term
        // top->hier_block2::connect(out_decimation_s_1_shortterm, 0, opencmw_time_sink_power_shortterm, 2);   // S short-term
        // top->hier_block2::connect(out_decimation_phi_1_shortterm, 0, opencmw_time_sink_power_shortterm, 3); // phi short-term
        // top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, out_decimation_p_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, out_decimation_q_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, out_decimation_s_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, out_decimation_phi_1_midterm, 0);
        // top->hier_block2::connect(out_decimation_p_1_midterm, 0, opencmw_time_sink_power_midterm, 0);   // P mid-term
        // top->hier_block2::connect(out_decimation_q_1_midterm, 0, opencmw_time_sink_power_midterm, 1);   // Q mid-term
        // top->hier_block2::connect(out_decimation_s_1_midterm, 0, opencmw_time_sink_power_midterm, 2);   // S mid-term
        // top->hier_block2::connect(out_decimation_phi_1_midterm, 0, opencmw_time_sink_power_midterm, 3); // phi mid-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 0, out_decimation_p_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 1, out_decimation_q_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 2, out_decimation_s_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 3, out_decimation_phi_1_longterm, 0);
        // top->hier_block2::connect(out_decimation_p_1_longterm, 0, opencmw_time_sink_power_longterm, 0);   // P long-term
        // top->hier_block2::connect(out_decimation_q_1_longterm, 0, opencmw_time_sink_power_longterm, 1);   // Q long-term
        // top->hier_block2::connect(out_decimation_s_1_longterm, 0, opencmw_time_sink_power_longterm, 2);   // S long-term
        // top->hier_block2::connect(out_decimation_phi_1_longterm, 0, opencmw_time_sink_power_longterm, 3); // phi long-term
        // phase 2
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 8, null_sink_phase_1_2, 4);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 9, null_sink_phase_1_2, 5);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 10, null_sink_phase_1_2, 6);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 11, null_sink_phase_1_2, 7);
        // top->hier_block2::connect(out_decimation_p_2_shortterm, 0, opencmw_time_sink_power_shortterm, 0);   // P short-term
        // top->hier_block2::connect(out_decimation_q_2_shortterm, 0, opencmw_time_sink_power_shortterm, 1);   // Q short-term
        // top->hier_block2::connect(out_decimation_s_2_shortterm, 0, opencmw_time_sink_power_shortterm, 2);   // S short-term
        // top->hier_block2::connect(out_decimation_phi_2_shortterm, 0, opencmw_time_sink_power_shortterm, 3); // phi short-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 0, out_decimation_p_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 1, out_decimation_q_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, out_decimation_s_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 3, out_decimation_phi_2_midterm, 0);
        // top->hier_block2::connect(out_decimation_p_2_midterm, 0, opencmw_time_sink_power_midterm, 0);   // P mid-term
        // top->hier_block2::connect(out_decimation_q_2_midterm, 0, opencmw_time_sink_power_midterm, 1);   // Q mid-term
        // top->hier_block2::connect(out_decimation_s_2_midterm, 0, opencmw_time_sink_power_midterm, 2);   // S mid-term
        // top->hier_block2::connect(out_decimation_phi_2_midterm, 0, opencmw_time_sink_power_midterm, 3); // phi mid-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 0, out_decimation_p_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 1, out_decimation_q_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, out_decimation_s_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 3, out_decimation_phi_2_longterm, 0);
        // top->hier_block2::connect(out_decimation_p_2_longterm, 0, opencmw_time_sink_power_longterm, 0);   // P long-term
        // top->hier_block2::connect(out_decimation_q_2_longterm, 0, opencmw_time_sink_power_longterm, 1);   // Q long-term
        // top->hier_block2::connect(out_decimation_s_2_longterm, 0, opencmw_time_sink_power_longterm, 2);   // S long-term
        // top->hier_block2::connect(out_decimation_phi_2_longterm, 0, opencmw_time_sink_power_longterm, 3); // phi long-term
        ///integral S
        // phase 0
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, integrate, 0);
        top->hier_block2::connect(integrate, 0, opencmw_time_sink_int_shortterm, 0); // int S short-term
        // // phase 1
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 2, integrate, 0);
        // top->hier_block2::connect(integrate, 0, opencmw_time_sink_int_shortterm, 0); // int S short-term
        // //phase 2
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, integrate, 0);
        // top->hier_block2::connect(integrate, 0, opencmw_time_sink_int_shortterm, 0); // int S short-term
        ///Statistics
        // phase 0
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, statistics_p_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, statistics_q_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, statistics_s_0_shortterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, statistics_phi_0_shortterm, 0);
        top->hier_block2::connect(statistics_p_0_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 0);    // P_mean short-term
        top->hier_block2::connect(statistics_p_0_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 1);    // P_min short-term
        top->hier_block2::connect(statistics_p_0_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 2);    // P_max short-term
        top->hier_block2::connect(statistics_p_0_shortterm, 3, null_sink_stats_shortterm, 0);                  // P_std_dev short-term
        top->hier_block2::connect(statistics_q_0_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 3);    // Q_mean short-term
        top->hier_block2::connect(statistics_q_0_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 4);    // Q_min short-term
        top->hier_block2::connect(statistics_q_0_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 5);    // Q_max short-term
        top->hier_block2::connect(statistics_q_0_shortterm, 3, null_sink_stats_shortterm, 1);                  // Q_std_dev short-term
        top->hier_block2::connect(statistics_s_0_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 6);    // S_mean short-term
        top->hier_block2::connect(statistics_s_0_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 7);    // S_min short-term
        top->hier_block2::connect(statistics_s_0_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 8);    // S_max short-term
        top->hier_block2::connect(statistics_s_0_shortterm, 3, null_sink_stats_shortterm, 2);                  // S_std_dev short-term
        top->hier_block2::connect(statistics_phi_0_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 9);  // phi_mean short-term
        top->hier_block2::connect(statistics_phi_0_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 10); // phi_min short-term
        top->hier_block2::connect(statistics_phi_0_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 11); // phi_max short-term
        top->hier_block2::connect(statistics_phi_0_shortterm, 3, null_sink_stats_shortterm, 3);                // phi_std_dev short-term
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, statistics_p_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, statistics_q_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, statistics_s_0_midterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, statistics_phi_0_midterm, 0);
        top->hier_block2::connect(statistics_p_0_midterm, 0, opencmw_time_sink_power_stats_midterm, 0);    // P_mean mid-term
        top->hier_block2::connect(statistics_p_0_midterm, 1, opencmw_time_sink_power_stats_midterm, 1);    // P_min mid-term
        top->hier_block2::connect(statistics_p_0_midterm, 2, opencmw_time_sink_power_stats_midterm, 2);    // P_max mid-term
        top->hier_block2::connect(statistics_p_0_midterm, 3, null_sink_stats_midterm, 0);                  // P_std_dev mid-term
        top->hier_block2::connect(statistics_q_0_midterm, 0, opencmw_time_sink_power_stats_midterm, 3);    // Q_mean mid-term
        top->hier_block2::connect(statistics_q_0_midterm, 1, opencmw_time_sink_power_stats_midterm, 4);    // Q_min mid-term
        top->hier_block2::connect(statistics_q_0_midterm, 2, opencmw_time_sink_power_stats_midterm, 5);    // Q_max mid-term
        top->hier_block2::connect(statistics_q_0_midterm, 3, null_sink_stats_midterm, 1);                  // Q_std_dev mid-term
        top->hier_block2::connect(statistics_s_0_midterm, 0, opencmw_time_sink_power_stats_midterm, 6);    // S_mean mid-term
        top->hier_block2::connect(statistics_s_0_midterm, 1, opencmw_time_sink_power_stats_midterm, 7);    // S_min mid-term
        top->hier_block2::connect(statistics_s_0_midterm, 2, opencmw_time_sink_power_stats_midterm, 8);    // S_max mid-term
        top->hier_block2::connect(statistics_s_0_midterm, 3, null_sink_stats_midterm, 2);                  // S_std_dev mid-term
        top->hier_block2::connect(statistics_phi_0_midterm, 0, opencmw_time_sink_power_stats_midterm, 9);  // phi_mean mid-term
        top->hier_block2::connect(statistics_phi_0_midterm, 1, opencmw_time_sink_power_stats_midterm, 10); // phi_min mid-term
        top->hier_block2::connect(statistics_phi_0_midterm, 2, opencmw_time_sink_power_stats_midterm, 11); // phi_max mid-term
        top->hier_block2::connect(statistics_phi_0_midterm, 3, null_sink_stats_midterm, 3);                // phi_std_dev mid-term
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 0, statistics_p_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 1, statistics_q_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 2, statistics_s_0_longterm, 0);
        top->hier_block2::connect(pulsed_power_power_calc_mul_ph_0_0, 3, statistics_phi_0_longterm, 0);
        top->hier_block2::connect(statistics_p_0_longterm, 0, opencmw_time_sink_power_stats_longterm, 0);    // P_mean long-term
        top->hier_block2::connect(statistics_p_0_longterm, 1, opencmw_time_sink_power_stats_longterm, 1);    // P_min long-term
        top->hier_block2::connect(statistics_p_0_longterm, 2, opencmw_time_sink_power_stats_longterm, 2);    // P_max long-term
        top->hier_block2::connect(statistics_p_0_longterm, 3, null_sink_stats_longterm, 0);                  // P_std_dev long-term
        top->hier_block2::connect(statistics_q_0_longterm, 0, opencmw_time_sink_power_stats_longterm, 3);    // Q_mean long-term
        top->hier_block2::connect(statistics_q_0_longterm, 1, opencmw_time_sink_power_stats_longterm, 4);    // Q_min long-term
        top->hier_block2::connect(statistics_q_0_longterm, 2, opencmw_time_sink_power_stats_longterm, 5);    // Q_max long-term
        top->hier_block2::connect(statistics_q_0_longterm, 3, null_sink_stats_longterm, 1);                  // Q_std_dev long-term
        top->hier_block2::connect(statistics_s_0_longterm, 0, opencmw_time_sink_power_stats_longterm, 6);    // S_mean long-term
        top->hier_block2::connect(statistics_s_0_longterm, 1, opencmw_time_sink_power_stats_longterm, 7);    // S_min long-term
        top->hier_block2::connect(statistics_s_0_longterm, 2, opencmw_time_sink_power_stats_longterm, 8);    // S_max long-term
        top->hier_block2::connect(statistics_s_0_longterm, 3, null_sink_stats_longterm, 2);                  // S_std_dev long-term
        top->hier_block2::connect(statistics_phi_0_longterm, 0, opencmw_time_sink_power_stats_longterm, 9);  // phi_mean long-term
        top->hier_block2::connect(statistics_phi_0_longterm, 1, opencmw_time_sink_power_stats_longterm, 10); // phi_min long-term
        top->hier_block2::connect(statistics_phi_0_longterm, 2, opencmw_time_sink_power_stats_longterm, 11); // phi_max long-term
        top->hier_block2::connect(statistics_phi_0_longterm, 3, null_sink_stats_longterm, 3);                // phi_std_dev long-term
        // // phase 1
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 0, statistics_p_1_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 1, statistics_q_1_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 2, statistics_s_1_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 3, statistics_phi_1_shortterm, 0);           //-- todo: connect opencmw sinks correct
        // top->hier_block2::connect(statistics_p_1_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 0);    // P_mean short-term
        // top->hier_block2::connect(statistics_p_1_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 1);    // P_min short-term
        // top->hier_block2::connect(statistics_p_1_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 2);    // P_max short-term
        // top->hier_block2::connect(statistics_p_1_shortterm, 3, null_sink_stats_shortterm, 0);                  // P_std_dev short-term
        // top->hier_block2::connect(statistics_q_1_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 3);    // Q_mean short-term
        // top->hier_block2::connect(statistics_q_1_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 4);    // Q_min short-term
        // top->hier_block2::connect(statistics_q_1_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 5);    // Q_max short-term
        // top->hier_block2::connect(statistics_q_1_shortterm, 3, null_sink_stats_shortterm, 1);                  // Q_std_dev short-term
        // top->hier_block2::connect(statistics_s_1_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 6);    // S_mean short-term
        // top->hier_block2::connect(statistics_s_1_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 7);    // S_min short-term
        // top->hier_block2::connect(statistics_s_1_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 8);    // S_max short-term
        // top->hier_block2::connect(statistics_s_1_shortterm, 3, null_sink_stats_shortterm, 2);                  // S_std_dev short-term
        // top->hier_block2::connect(statistics_phi_1_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 9);  // phi_mean short-term
        // top->hier_block2::connect(statistics_phi_1_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 10); // phi_min short-term
        // top->hier_block2::connect(statistics_phi_1_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 11); // phi_max short-term
        // top->hier_block2::connect(statistics_phi_1_shortterm, 3, null_sink_stats_shortterm, 3);                // phi_std_dev short-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 0, statistics_p_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 1, statistics_q_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 2, statistics_s_1_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 3, statistics_phi_1_midterm, 0);
        // top->hier_block2::connect(statistics_p_1_midterm, 0, opencmw_time_sink_power_stats_midterm, 0);    // P_mean mid-term
        // top->hier_block2::connect(statistics_p_1_midterm, 1, opencmw_time_sink_power_stats_midterm, 1);    // P_min mid-term
        // top->hier_block2::connect(statistics_p_1_midterm, 2, opencmw_time_sink_power_stats_midterm, 2);    // P_max mid-term
        // top->hier_block2::connect(statistics_p_1_midterm, 3, null_sink_stats_midterm, 0);                  // P_std_dev mid-term
        // top->hier_block2::connect(statistics_q_1_midterm, 0, opencmw_time_sink_power_stats_midterm, 3);    // Q_mean mid-term
        // top->hier_block2::connect(statistics_q_1_midterm, 1, opencmw_time_sink_power_stats_midterm, 4);    // Q_min mid-term
        // top->hier_block2::connect(statistics_q_1_midterm, 2, opencmw_time_sink_power_stats_midterm, 5);    // Q_max mid-term
        // top->hier_block2::connect(statistics_q_1_midterm, 3, null_sink_stats_midterm, 1);                  // Q_std_dev mid-term
        // top->hier_block2::connect(statistics_s_1_midterm, 0, opencmw_time_sink_power_stats_midterm, 6);    // S_mean mid-term
        // top->hier_block2::connect(statistics_s_1_midterm, 1, opencmw_time_sink_power_stats_midterm, 7);    // S_min mid-term
        // top->hier_block2::connect(statistics_s_1_midterm, 2, opencmw_time_sink_power_stats_midterm, 8);    // S_max mid-term
        // top->hier_block2::connect(statistics_s_1_midterm, 3, null_sink_stats_midterm, 2);                  // S_std_dev mid-term
        // top->hier_block2::connect(statistics_phi_1_midterm, 0, opencmw_time_sink_power_stats_midterm, 9);  // phi_mean mid-term
        // top->hier_block2::connect(statistics_phi_1_midterm, 1, opencmw_time_sink_power_stats_midterm, 10); // phi_min mid-term
        // top->hier_block2::connect(statistics_phi_1_midterm, 2, opencmw_time_sink_power_stats_midterm, 11); // phi_max mid-term
        // top->hier_block2::connect(statistics_phi_1_midterm, 3, null_sink_stats_midterm, 3);                // phi_std_dev mid-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 0, statistics_p_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 1, statistics_q_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 2, statistics_s_1_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_1_0, 3, statistics_phi_1_longterm, 0);
        // top->hier_block2::connect(statistics_p_1_longterm, 0, opencmw_time_sink_power_stats_longterm, 0);    // P_mean long-term
        // top->hier_block2::connect(statistics_p_1_longterm, 1, opencmw_time_sink_power_stats_longterm, 1);    // P_min long-term
        // top->hier_block2::connect(statistics_p_1_longterm, 2, opencmw_time_sink_power_stats_longterm, 2);    // P_max long-term
        // top->hier_block2::connect(statistics_p_1_longterm, 3, null_sink_stats_longterm, 0);                  // P_std_dev long-term
        // top->hier_block2::connect(statistics_q_1_longterm, 0, opencmw_time_sink_power_stats_longterm, 3);    // Q_mean long-term
        // top->hier_block2::connect(statistics_q_1_longterm, 1, opencmw_time_sink_power_stats_longterm, 4);    // Q_min long-term
        // top->hier_block2::connect(statistics_q_1_longterm, 2, opencmw_time_sink_power_stats_longterm, 5);    // Q_max long-term
        // top->hier_block2::connect(statistics_q_1_longterm, 3, null_sink_stats_longterm, 1);                  // Q_std_dev long-term
        // top->hier_block2::connect(statistics_s_1_longterm, 0, opencmw_time_sink_power_stats_longterm, 6);    // S_mean long-term
        // top->hier_block2::connect(statistics_s_1_longterm, 1, opencmw_time_sink_power_stats_longterm, 7);    // S_min long-term
        // top->hier_block2::connect(statistics_s_1_longterm, 2, opencmw_time_sink_power_stats_longterm, 8);    // S_max long-term
        // top->hier_block2::connect(statistics_s_1_longterm, 3, null_sink_stats_longterm, 2);                  // S_std_dev long-term
        // top->hier_block2::connect(statistics_phi_1_longterm, 0, opencmw_time_sink_power_stats_longterm, 9);  // phi_mean long-term
        // top->hier_block2::connect(statistics_phi_1_longterm, 1, opencmw_time_sink_power_stats_longterm, 10); // phi_min long-term
        // top->hier_block2::connect(statistics_phi_1_longterm, 2, opencmw_time_sink_power_stats_longterm, 11); // phi_max long-term
        // top->hier_block2::connect(statistics_phi_1_longterm, 3, null_sink_stats_longterm, 3);                // phi_std_dev long-term
        // //phase 2
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 0, statistics_p_2_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 1, statistics_q_2_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, statistics_s_2_shortterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 3, statistics_phi_2_shortterm, 0);
        // top->hier_block2::connect(statistics_p_2_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 0);    // P_mean short-term
        // top->hier_block2::connect(statistics_p_2_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 1);    // P_min short-term
        // top->hier_block2::connect(statistics_p_2_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 2);    // P_max short-term
        // top->hier_block2::connect(statistics_p_2_shortterm, 3, null_sink_stats_shortterm, 0);                  // P_std_dev short-term
        // top->hier_block2::connect(statistics_q_2_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 3);    // Q_mean short-term
        // top->hier_block2::connect(statistics_q_2_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 4);    // Q_min short-term
        // top->hier_block2::connect(statistics_q_2_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 5);    // Q_max short-term
        // top->hier_block2::connect(statistics_q_2_shortterm, 3, null_sink_stats_shortterm, 1);                  // Q_std_dev short-term
        // top->hier_block2::connect(statistics_s_2_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 6);    // S_mean short-term
        // top->hier_block2::connect(statistics_s_2_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 7);    // S_min short-term
        // top->hier_block2::connect(statistics_s_2_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 8);    // S_max short-term
        // top->hier_block2::connect(statistics_s_2_shortterm, 3, null_sink_stats_shortterm, 2);                  // S_std_dev short-term
        // top->hier_block2::connect(statistics_phi_2_shortterm, 0, opencmw_time_sink_power_stats_shortterm, 9);  // phi_mean short-term
        // top->hier_block2::connect(statistics_phi_2_shortterm, 1, opencmw_time_sink_power_stats_shortterm, 10); // phi_min short-term
        // top->hier_block2::connect(statistics_phi_2_shortterm, 2, opencmw_time_sink_power_stats_shortterm, 11); // phi_max short-term
        // top->hier_block2::connect(statistics_phi_2_shortterm, 3, null_sink_stats_shortterm, 3);                // phi_std_dev short-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 0, statistics_p_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 1, statistics_q_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, statistics_s_2_midterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 3, statistics_phi_2_midterm, 0);
        // top->hier_block2::connect(statistics_p_2_midterm, 0, opencmw_time_sink_power_stats_midterm, 0);    // P_mean mid-term
        // top->hier_block2::connect(statistics_p_2_midterm, 1, opencmw_time_sink_power_stats_midterm, 1);    // P_min mid-term
        // top->hier_block2::connect(statistics_p_2_midterm, 2, opencmw_time_sink_power_stats_midterm, 2);    // P_max mid-term
        // top->hier_block2::connect(statistics_p_2_midterm, 3, null_sink_stats_midterm, 0);                  // P_std_dev mid-term
        // top->hier_block2::connect(statistics_q_2_midterm, 0, opencmw_time_sink_power_stats_midterm, 3);    // Q_mean mid-term
        // top->hier_block2::connect(statistics_q_2_midterm, 1, opencmw_time_sink_power_stats_midterm, 4);    // Q_min mid-term
        // top->hier_block2::connect(statistics_q_2_midterm, 2, opencmw_time_sink_power_stats_midterm, 5);    // Q_max mid-term
        // top->hier_block2::connect(statistics_q_2_midterm, 3, null_sink_stats_midterm, 1);                  // Q_std_dev mid-term
        // top->hier_block2::connect(statistics_s_2_midterm, 0, opencmw_time_sink_power_stats_midterm, 6);    // S_mean mid-term
        // top->hier_block2::connect(statistics_s_2_midterm, 1, opencmw_time_sink_power_stats_midterm, 7);    // S_min mid-term
        // top->hier_block2::connect(statistics_s_2_midterm, 2, opencmw_time_sink_power_stats_midterm, 8);    // S_max mid-term
        // top->hier_block2::connect(statistics_s_2_midterm, 3, null_sink_stats_midterm, 2);                  // S_std_dev mid-term
        // top->hier_block2::connect(statistics_phi_2_midterm, 0, opencmw_time_sink_power_stats_midterm, 9);  // phi_mean mid-term
        // top->hier_block2::connect(statistics_phi_2_midterm, 1, opencmw_time_sink_power_stats_midterm, 10); // phi_min mid-term
        // top->hier_block2::connect(statistics_phi_2_midterm, 2, opencmw_time_sink_power_stats_midterm, 11); // phi_max mid-term
        // top->hier_block2::connect(statistics_phi_2_midterm, 3, null_sink_stats_midterm, 3);                // phi_std_dev mid-term
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 0, statistics_p_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 1, statistics_q_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 2, statistics_s_2_longterm, 0);
        // top->hier_block2::connect(pulsed_power_power_calc_ff_2_0, 3, statistics_phi_2_longterm, 0);
        // top->hier_block2::connect(statistics_p_2_longterm, 0, opencmw_time_sink_power_stats_longterm, 0);    // P_mean long-term
        // top->hier_block2::connect(statistics_p_2_longterm, 1, opencmw_time_sink_power_stats_longterm, 1);    // P_min long-term
        // top->hier_block2::connect(statistics_p_2_longterm, 2, opencmw_time_sink_power_stats_longterm, 2);    // P_max long-term
        // top->hier_block2::connect(statistics_p_2_longterm, 3, null_sink_stats_longterm, 0);                  // P_std_dev long-term
        // top->hier_block2::connect(statistics_q_2_longterm, 0, opencmw_time_sink_power_stats_longterm, 3);    // Q_mean long-term
        // top->hier_block2::connect(statistics_q_2_longterm, 1, opencmw_time_sink_power_stats_longterm, 4);    // Q_min long-term
        // top->hier_block2::connect(statistics_q_2_longterm, 2, opencmw_time_sink_power_stats_longterm, 5);    // Q_max long-term
        // top->hier_block2::connect(statistics_q_2_longterm, 3, null_sink_stats_longterm, 1);                  // Q_std_dev long-term
        // top->hier_block2::connect(statistics_s_2_longterm, 0, opencmw_time_sink_power_stats_longterm, 6);    // S_mean long-term
        // top->hier_block2::connect(statistics_s_2_longterm, 1, opencmw_time_sink_power_stats_longterm, 7);    // S_min long-term
        // top->hier_block2::connect(statistics_s_2_longterm, 2, opencmw_time_sink_power_stats_longterm, 8);    // S_max long-term
        // top->hier_block2::connect(statistics_s_2_longterm, 3, null_sink_stats_longterm, 2);                  // S_std_dev long-term
        // top->hier_block2::connect(statistics_phi_2_longterm, 0, opencmw_time_sink_power_stats_longterm, 9);  // phi_mean long-term
        // top->hier_block2::connect(statistics_phi_2_longterm, 1, opencmw_time_sink_power_stats_longterm, 10); // phi_min long-term
        // top->hier_block2::connect(statistics_phi_2_longterm, 2, opencmw_time_sink_power_stats_longterm, 11); // phi_max long-term
        // top->hier_block2::connect(statistics_phi_2_longterm, 3, null_sink_stats_longterm, 3);                // phi_std_dev long-term
        ///Frequency spectras
        // phase 0
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
        // // phase 1
        // top->hier_block2::connect(source_interface_voltage1, 0, stream_to_vector_U, 0); //-- todo: connect streams correct
        // top->hier_block2::connect(stream_to_vector_U, 0, fft_U, 0);
        // top->hier_block2::connect(fft_U, 0, complex_to_mag_U, 0);
        // top->hier_block2::connect(complex_to_mag_U, 0, opencmw_freq_sink_nilm_U, 0); // freq_spectra voltage
        // top->hier_block2::connect(source_interface_current0, 0, stream_to_vector_I, 0);
        // top->hier_block2::connect(stream_to_vector_I, 0, fft_I, 0);
        // top->hier_block2::connect(fft_I, 0, complex_to_mag_I, 0);
        // top->hier_block2::connect(complex_to_mag_I, 0, opencmw_freq_sink_nilm_I, 0); // freq_spectra current
        // top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current_nilm, 0);
        // top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current_nilm, 1);
        // top->hier_block2::connect(multiply_voltage_current_nilm, 0, stream_to_vector_S, 0);
        // top->hier_block2::connect(stream_to_vector_S, 0, fft_S, 0);
        // top->hier_block2::connect(fft_S, 0, complex_to_mag_S, 0);
        // top->hier_block2::connect(complex_to_mag_S, 0, opencmw_freq_sink_nilm_S, 0); // freq_spectra apparent power (nilm)
        // top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current, 0);
        // top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current, 1);
        // top->hier_block2::connect(multiply_voltage_current, 0, frequency_spec_one_in_n, 0);
        // top->hier_block2::connect(frequency_spec_one_in_n, 0, frequency_spec_low_pass, 0);
        // top->hier_block2::connect(frequency_spec_low_pass, 0, frequency_spec_stream_to_vec, 0);
        // top->hier_block2::connect(frequency_spec_stream_to_vec, 0, frequency_spec_fft, 0);
        // top->hier_block2::connect(frequency_spec_fft, 0, frequency_multiply_const, 0);
        // top->hier_block2::connect(frequency_multiply_const, 0, frequency_spec_complex_to_mag, 0);
        // top->hier_block2::connect(frequency_spec_complex_to_mag, 0, frequency_spec_pulsed_power_opencmw_freq_sink, 0); // freq_spectra apparent power
        // // phase 2
        // top->hier_block2::connect(source_interface_voltage0, 0, stream_to_vector_U, 0);
        // top->hier_block2::connect(stream_to_vector_U, 0, fft_U, 0);
        // top->hier_block2::connect(fft_U, 0, complex_to_mag_U, 0);
        // top->hier_block2::connect(complex_to_mag_U, 0, opencmw_freq_sink_nilm_U, 0); // freq_spectra voltage
        // top->hier_block2::connect(source_interface_current0, 0, stream_to_vector_I, 0);
        // top->hier_block2::connect(stream_to_vector_I, 0, fft_I, 0);
        // top->hier_block2::connect(fft_I, 0, complex_to_mag_I, 0);
        // top->hier_block2::connect(complex_to_mag_I, 0, opencmw_freq_sink_nilm_I, 0); // freq_spectra current
        // top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current_nilm, 0);
        // top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current_nilm, 1);
        // top->hier_block2::connect(multiply_voltage_current_nilm, 0, stream_to_vector_S, 0);
        // top->hier_block2::connect(stream_to_vector_S, 0, fft_S, 0);
        // top->hier_block2::connect(fft_S, 0, complex_to_mag_S, 0);
        // top->hier_block2::connect(complex_to_mag_S, 0, opencmw_freq_sink_nilm_S, 0); // freq_spectra apparent power (nilm)
        // top->hier_block2::connect(source_interface_current0, 0, multiply_voltage_current, 0);
        // top->hier_block2::connect(source_interface_voltage0, 0, multiply_voltage_current, 1);
        // top->hier_block2::connect(multiply_voltage_current, 0, frequency_spec_one_in_n, 0);
        // top->hier_block2::connect(frequency_spec_one_in_n, 0, frequency_spec_low_pass, 0);
        // top->hier_block2::connect(frequency_spec_low_pass, 0, frequency_spec_stream_to_vec, 0);
        // top->hier_block2::connect(frequency_spec_stream_to_vec, 0, frequency_spec_fft, 0);
        // top->hier_block2::connect(frequency_spec_fft, 0, frequency_multiply_const, 0);
        // top->hier_block2::connect(frequency_multiply_const, 0, frequency_spec_complex_to_mag, 0);
        // top->hier_block2::connect(frequency_spec_complex_to_mag, 0, frequency_spec_pulsed_power_opencmw_freq_sink, 0); // freq_spectra apparent power
    }
    ~PulsedPowerThreePhaseFlowgraph() { top->stop(); }
    // start gnuradio flowgraph
    void start() { top->start(); }
};

#endif /* GR_THREEPHASEFLOWGRAPHS_HPP */
