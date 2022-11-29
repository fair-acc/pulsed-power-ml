#ifndef THREE_PHASES_SIMULATION_HPP
#define THREE_PHASES_SIMULATION_HPP
/********************
GNU Radio C++ Flow Graph Header File

Title: three_phases_simulation
Author: buchner
GNU Radio version: 3.10.4.0
********************/

/********************
** Create includes
********************/
#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/blocks/divide.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/transcendental.h>
#include <gnuradio/filter/interp_fir_filter.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <pulsed_power_daq/power_calc_mul_ph_ff.h>
#include <gnuradio/qtgui/time_sink_f.h> //#include <qapplication.h> not known

#include <QVBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <QGridLayout>
#include <QSettings>
#include <QApplication>

#include <boost/program_options.hpp>

using namespace gr;



class three_phases_simulation : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout *top_scroll_layout;
    QScrollArea *top_scroll;
    QWidget *top_widget;
    QVBoxLayout *top_layout;
    QGridLayout *top_grid_layout;
    QSettings *settings;


    qtgui::time_sink_f::sptr qtgui_time_sink_x_2_1;
    qtgui::time_sink_f::sptr qtgui_time_sink_x_2_0;
    qtgui::time_sink_f::sptr qtgui_time_sink_x_2;
    qtgui::time_sink_f::sptr qtgui_time_sink_x_1;
    qtgui::time_sink_f::sptr qtgui_time_sink_x_0;
    pulsed_power_daq::power_calc_mul_ph_ff::sptr pulsed_power_daq_power_calc_mul_ph_ff_0_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_3_1_0_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_3_1_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_3_1;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_2_0_1_0_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_2_0_1_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_2_0_1;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_1_0_1_0_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_1_0_1_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_1_0_1;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_0_0_1_0_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_0_0_1_0;
    gr::filter::fir_filter_fff::sptr low_pass_filter_0_1_0_0_1;
    blocks::transcendental::sptr blocks_transcendental_1_1_0_0;
    blocks::transcendental::sptr blocks_transcendental_1_1_0;
    blocks::transcendental::sptr blocks_transcendental_1_1;
    blocks::transcendental::sptr blocks_transcendental_0_0_0_1_0_0;
    blocks::transcendental::sptr blocks_transcendental_0_0_0_1_0;
    blocks::transcendental::sptr blocks_transcendental_0_0_0_1;
    blocks::sub_ff::sptr blocks_sub_xx_0_0_1_0_0;
    blocks::sub_ff::sptr blocks_sub_xx_0_0_1_0;
    blocks::sub_ff::sptr blocks_sub_xx_0_0_1;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_3_1_0_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_3_1_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_3_1;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_2_0_1_0_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_2_0_1_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_2_0_1;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_1_0_1_0_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_1_0_1_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_1_0_1;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_0_0_1_0_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_0_0_1_0;
    blocks::multiply_ff::sptr blocks_multiply_xx_0_0_0_1;
    blocks::divide_ff::sptr blocks_divide_xx_0_1_1_0_0;
    blocks::divide_ff::sptr blocks_divide_xx_0_1_1_0;
    blocks::divide_ff::sptr blocks_divide_xx_0_1_1;
    blocks::divide_ff::sptr blocks_divide_xx_0_0_0_1_0_0;
    blocks::divide_ff::sptr blocks_divide_xx_0_0_0_1_0;
    blocks::divide_ff::sptr blocks_divide_xx_0_0_0_1;
    filter::firdes::vector<float> band_pass_filter_0_1_1_0_0;
    filter::firdes::band_pass band_pass_filter_0_1_1_0;
    filter::firdes::std::vector<float> band_pass_filter_0_1_1;
    filter::firdes::std::vector<float> band_pass_filter_0_0_0_1_0_0;
    filter::firdes::std::vector<float> band_pass_filter_0_0_0_1_0;
    filter::firdes::std::vector<float> band_pass_filter_0_0_0_1;
    analog::sig_source_f::sptr analog_sig_source_x_1_0_1_0_0;
    analog::sig_source_f::sptr analog_sig_source_x_1_0_1_0;
    analog::sig_source_f::sptr analog_sig_source_x_1_0_1;
    analog::sig_source_f::sptr analog_sig_source_x_0_1_0_1_0_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_1_0_1_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_1_0_1;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_1_0_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_1_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_1;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_0_0_1_0_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_0_0_1_0;
    analog::sig_source_f::sptr analog_sig_source_x_0_0_0_0_1;

// Parameters:
    int bp_decimantion = 20;
    double bp_high_cut = 80;
    double bp_low_cut = 20;
    double bp_trans = 10;
    double current_correction_factor = 2.5;
    double in_samp_rate = 200000;
    int lp_decimantion = 1;
    double out_samp_rate = 10000;
    double voltage_correction_factor = 100;


public:
    top_block_sptr tb;
    three_phases_simulation(int bp_decimantion, double bp_high_cut, double bp_low_cut, double bp_trans, double current_correction_factor, double in_samp_rate, int lp_decimantion, double out_samp_rate, double voltage_correction_factor);
    ~three_phases_simulation();

    int get_bp_decimantion () const;
    void set_bp_decimantion(int bp_decimantion);
    double get_bp_high_cut () const;
    void set_bp_high_cut(double bp_high_cut);
    double get_bp_low_cut () const;
    void set_bp_low_cut(double bp_low_cut);
    double get_bp_trans () const;
    void set_bp_trans(double bp_trans);
    double get_current_correction_factor () const;
    void set_current_correction_factor(double current_correction_factor);
    double get_in_samp_rate () const;
    void set_in_samp_rate(double in_samp_rate);
    int get_lp_decimantion () const;
    void set_lp_decimantion(int lp_decimantion);
    double get_out_samp_rate () const;
    void set_out_samp_rate(double out_samp_rate);
    double get_voltage_correction_factor () const;
    void set_voltage_correction_factor(double voltage_correction_factor);

};


#endif

