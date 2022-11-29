/********************
GNU Radio C++ Flow Graph Source File

Title: three_phases_simulation
Author: buchner
GNU Radio version: 3.10.4.0
********************/

#include "three_phases_simulation.hpp"


namespace po = boost::program_options;

using namespace gr;


three_phases_simulation::three_phases_simulation (int bp_decimantion, double bp_high_cut, double bp_low_cut, double bp_trans, double current_correction_factor, double in_samp_rate, int lp_decimantion, double out_samp_rate, double voltage_correction_factor)
: QWidget(),
  bp_decimantion(bp_decimantion),
  bp_high_cut(bp_high_cut),
  bp_low_cut(bp_low_cut),
  bp_trans(bp_trans),
  current_correction_factor(current_correction_factor),
  in_samp_rate(in_samp_rate),
  lp_decimantion(lp_decimantion),
  out_samp_rate(out_samp_rate),
  voltage_correction_factor(voltage_correction_factor) {

    this->setWindowTitle("three_phases_simulation");
    // check_set_qss
    // set icon
    this->top_scroll_layout = new QVBoxLayout();
    this->setLayout(this->top_scroll_layout);
    this->top_scroll = new QScrollArea();
    this->top_scroll->setFrameStyle(QFrame::NoFrame);
    this->top_scroll_layout->addWidget(this->top_scroll);
    this->top_scroll->setWidgetResizable(true);
    this->top_widget = new QWidget();
    this->top_scroll->setWidget(this->top_widget);
    this->top_layout = new QVBoxLayout(this->top_widget);
    this->top_grid_layout = new QGridLayout();
    this->top_layout->addLayout(this->top_grid_layout);

    this->settings = new QSettings("GNU Radio", "three_phases_simulation");

    this->tb = gr::make_top_block("three_phases_simulation");

// Blocks:
        qtgui_time_sink_x_2_1 = qtgui::time_sink_f::make(
            1024, // size
            out_samp_rate, // samp_rate
            """", // name
            3 // number of inputs
        );

        QWidget* _qtgui_time_sink_x_2_1_win;
        qtgui_time_sink_x_2_1->set_update_time(0.10);
        qtgui_time_sink_x_2_1->set_y_axis(-1, 1);

        qtgui_time_sink_x_2_1->set_y_label("Amplitude", """");

        qtgui_time_sink_x_2_1->enable_tags(true);
        qtgui_time_sink_x_2_1->set_trigger_mode(gr::qtgui::TRIG_MODE_FREE, gr::qtgui::TRIG_SLOPE_POS, 0.0, 0,0, "");

        qtgui_time_sink_x_2_1->enable_autoscale(false);
        qtgui_time_sink_x_2_1->enable_grid(false);
        qtgui_time_sink_x_2_1->enable_axis_labels(true);
        qtgui_time_sink_x_2_1->enable_control_panel(true);
        qtgui_time_sink_x_2_1->enable_stem_plot(false);

        {
            std::string labels[10] = {"P_acc", "Q_acc", "S_acc", "Signal 4", "Signal 5",
                "Signal 6", "Signal 7", "Signal 8", "Signal 9", "Signal 10"};
            int widths[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};
            std::string colors[10] = {"blue", "red", "green", "black", "cyan",
                "magenta", "yellow", "dark red", "dark green", "dark blue"};
            double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0};
            int markers[10] = {-1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1};
            int styles[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};

            for (int i = 0; i < 3; i++) {
                if (sizeof(labels[i]) == 0) {
                    qtgui_time_sink_x_2_1->set_line_label(i, "Data " + std::to_string(i));
                } else {
                    qtgui_time_sink_x_2_1->set_line_label(i, labels[i]);
                }
                qtgui_time_sink_x_2_1->set_line_width(i, widths[i]);
                qtgui_time_sink_x_2_1->set_line_color(i, colors[i]);
                qtgui_time_sink_x_2_1->set_line_style(i, styles[i]);
                qtgui_time_sink_x_2_1->set_line_marker(i, markers[i]);
                qtgui_time_sink_x_2_1->set_line_alpha(i, alphas[i]);
            }
        }

        _qtgui_time_sink_x_2_1_win = this->qtgui_time_sink_x_2_1->qwidget();

        top_layout->addWidget(_qtgui_time_sink_x_2_1_win);

        qtgui_time_sink_x_2_0 = qtgui::time_sink_f::make(
            1024, // size
            out_samp_rate, // samp_rate
            """", // name
            3 // number of inputs
        );

        QWidget* _qtgui_time_sink_x_2_0_win;
        qtgui_time_sink_x_2_0->set_update_time(0.10);
        qtgui_time_sink_x_2_0->set_y_axis(-1, 1);

        qtgui_time_sink_x_2_0->set_y_label("Amplitude", """");

        qtgui_time_sink_x_2_0->enable_tags(true);
        qtgui_time_sink_x_2_0->set_trigger_mode(gr::qtgui::TRIG_MODE_FREE, gr::qtgui::TRIG_SLOPE_POS, 0.0, 0,0, "");

        qtgui_time_sink_x_2_0->enable_autoscale(false);
        qtgui_time_sink_x_2_0->enable_grid(false);
        qtgui_time_sink_x_2_0->enable_axis_labels(true);
        qtgui_time_sink_x_2_0->enable_control_panel(true);
        qtgui_time_sink_x_2_0->enable_stem_plot(false);

        {
            std::string labels[10] = {"Phi_1", "Phi_2", "Phi_3", "Signal 4", "Signal 5",
                "Signal 6", "Signal 7", "Signal 8", "Signal 9", "Signal 10"};
            int widths[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};
            std::string colors[10] = {"blue", "red", "green", "black", "cyan",
                "magenta", "yellow", "dark red", "dark green", "dark blue"};
            double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0};
            int markers[10] = {-1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1};
            int styles[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};

            for (int i = 0; i < 3; i++) {
                if (sizeof(labels[i]) == 0) {
                    qtgui_time_sink_x_2_0->set_line_label(i, "Data " + std::to_string(i));
                } else {
                    qtgui_time_sink_x_2_0->set_line_label(i, labels[i]);
                }
                qtgui_time_sink_x_2_0->set_line_width(i, widths[i]);
                qtgui_time_sink_x_2_0->set_line_color(i, colors[i]);
                qtgui_time_sink_x_2_0->set_line_style(i, styles[i]);
                qtgui_time_sink_x_2_0->set_line_marker(i, markers[i]);
                qtgui_time_sink_x_2_0->set_line_alpha(i, alphas[i]);
            }
        }

        _qtgui_time_sink_x_2_0_win = this->qtgui_time_sink_x_2_0->qwidget();

        top_layout->addWidget(_qtgui_time_sink_x_2_0_win);

        qtgui_time_sink_x_2 = qtgui::time_sink_f::make(
            1024, // size
            out_samp_rate, // samp_rate
            """", // name
            3 // number of inputs
        );

        QWidget* _qtgui_time_sink_x_2_win;
        qtgui_time_sink_x_2->set_update_time(0.10);
        qtgui_time_sink_x_2->set_y_axis(-1, 1);

        qtgui_time_sink_x_2->set_y_label("Amplitude", """");

        qtgui_time_sink_x_2->enable_tags(true);
        qtgui_time_sink_x_2->set_trigger_mode(gr::qtgui::TRIG_MODE_FREE, gr::qtgui::TRIG_SLOPE_POS, 0.0, 0,0, "");

        qtgui_time_sink_x_2->enable_autoscale(false);
        qtgui_time_sink_x_2->enable_grid(false);
        qtgui_time_sink_x_2->enable_axis_labels(true);
        qtgui_time_sink_x_2->enable_control_panel(true);
        qtgui_time_sink_x_2->enable_stem_plot(false);

        {
            std::string labels[10] = {"P_3", "Q_3", "S_3", "Signal 4", "Signal 5",
                "Signal 6", "Signal 7", "Signal 8", "Signal 9", "Signal 10"};
            int widths[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};
            std::string colors[10] = {"blue", "red", "green", "black", "cyan",
                "magenta", "yellow", "dark red", "dark green", "dark blue"};
            double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0};
            int markers[10] = {-1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1};
            int styles[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};

            for (int i = 0; i < 3; i++) {
                if (sizeof(labels[i]) == 0) {
                    qtgui_time_sink_x_2->set_line_label(i, "Data " + std::to_string(i));
                } else {
                    qtgui_time_sink_x_2->set_line_label(i, labels[i]);
                }
                qtgui_time_sink_x_2->set_line_width(i, widths[i]);
                qtgui_time_sink_x_2->set_line_color(i, colors[i]);
                qtgui_time_sink_x_2->set_line_style(i, styles[i]);
                qtgui_time_sink_x_2->set_line_marker(i, markers[i]);
                qtgui_time_sink_x_2->set_line_alpha(i, alphas[i]);
            }
        }

        _qtgui_time_sink_x_2_win = this->qtgui_time_sink_x_2->qwidget();

        top_layout->addWidget(_qtgui_time_sink_x_2_win);

        qtgui_time_sink_x_1 = qtgui::time_sink_f::make(
            1024, // size
            out_samp_rate, // samp_rate
            """", // name
            3 // number of inputs
        );

        QWidget* _qtgui_time_sink_x_1_win;
        qtgui_time_sink_x_1->set_update_time(0.10);
        qtgui_time_sink_x_1->set_y_axis(-1, 1);

        qtgui_time_sink_x_1->set_y_label("Amplitude", """");

        qtgui_time_sink_x_1->enable_tags(true);
        qtgui_time_sink_x_1->set_trigger_mode(gr::qtgui::TRIG_MODE_FREE, gr::qtgui::TRIG_SLOPE_POS, 0.0, 0,0, "");

        qtgui_time_sink_x_1->enable_autoscale(false);
        qtgui_time_sink_x_1->enable_grid(false);
        qtgui_time_sink_x_1->enable_axis_labels(true);
        qtgui_time_sink_x_1->enable_control_panel(true);
        qtgui_time_sink_x_1->enable_stem_plot(false);

        {
            std::string labels[10] = {"P_1", "Q_1", "S_1", "Signal 4", "Signal 5",
                "Signal 6", "Signal 7", "Signal 8", "Signal 9", "Signal 10"};
            int widths[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};
            std::string colors[10] = {"blue", "red", "green", "black", "cyan",
                "magenta", "yellow", "dark red", "dark green", "dark blue"};
            double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0};
            int markers[10] = {-1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1};
            int styles[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};

            for (int i = 0; i < 3; i++) {
                if (sizeof(labels[i]) == 0) {
                    qtgui_time_sink_x_1->set_line_label(i, "Data " + std::to_string(i));
                } else {
                    qtgui_time_sink_x_1->set_line_label(i, labels[i]);
                }
                qtgui_time_sink_x_1->set_line_width(i, widths[i]);
                qtgui_time_sink_x_1->set_line_color(i, colors[i]);
                qtgui_time_sink_x_1->set_line_style(i, styles[i]);
                qtgui_time_sink_x_1->set_line_marker(i, markers[i]);
                qtgui_time_sink_x_1->set_line_alpha(i, alphas[i]);
            }
        }

        _qtgui_time_sink_x_1_win = this->qtgui_time_sink_x_1->qwidget();

        top_layout->addWidget(_qtgui_time_sink_x_1_win);

        qtgui_time_sink_x_0 = qtgui::time_sink_f::make(
            1024, // size
            out_samp_rate, // samp_rate
            """", // name
            3 // number of inputs
        );

        QWidget* _qtgui_time_sink_x_0_win;
        qtgui_time_sink_x_0->set_update_time(0.10);
        qtgui_time_sink_x_0->set_y_axis(-1, 1);

        qtgui_time_sink_x_0->set_y_label("Amplitude", """");

        qtgui_time_sink_x_0->enable_tags(true);
        qtgui_time_sink_x_0->set_trigger_mode(gr::qtgui::TRIG_MODE_FREE, gr::qtgui::TRIG_SLOPE_POS, 0.0, 0,0, "");

        qtgui_time_sink_x_0->enable_autoscale(false);
        qtgui_time_sink_x_0->enable_grid(false);
        qtgui_time_sink_x_0->enable_axis_labels(true);
        qtgui_time_sink_x_0->enable_control_panel(true);
        qtgui_time_sink_x_0->enable_stem_plot(false);

        {
            std::string labels[10] = {"P_2", "Q_2", "S_2", "Signal 4", "Signal 5",
                "Signal 6", "Signal 7", "Signal 8", "Signal 9", "Signal 10"};
            int widths[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};
            std::string colors[10] = {"blue", "red", "green", "black", "cyan",
                "magenta", "yellow", "dark red", "dark green", "dark blue"};
            double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
                1.0, 1.0, 1.0, 1.0, 1.0};
            int markers[10] = {-1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1};
            int styles[10] = {1, 1, 1, 1, 1,
                1, 1, 1, 1, 1};

            for (int i = 0; i < 3; i++) {
                if (sizeof(labels[i]) == 0) {
                    qtgui_time_sink_x_0->set_line_label(i, "Data " + std::to_string(i));
                } else {
                    qtgui_time_sink_x_0->set_line_label(i, labels[i]);
                }
                qtgui_time_sink_x_0->set_line_width(i, widths[i]);
                qtgui_time_sink_x_0->set_line_color(i, colors[i]);
                qtgui_time_sink_x_0->set_line_style(i, styles[i]);
                qtgui_time_sink_x_0->set_line_marker(i, markers[i]);
                qtgui_time_sink_x_0->set_line_alpha(i, alphas[i]);
            }
        }

        _qtgui_time_sink_x_0_win = this->qtgui_time_sink_x_0->qwidget();

        top_layout->addWidget(_qtgui_time_sink_x_0_win);

        this->pulsed_power_daq_power_calc_mul_ph_ff_0_0 = pulsed_power_daq::power_calc_mul_ph_ff::make(0.0001);

        this->low_pass_filter_0_1_3_1_0_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_3_1_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_3_1 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_2_0_1_0_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_2_0_1_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_2_0_1 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_1_0_1_0_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_1_0_1_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_1_0_1 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_0_0_1_0_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_0_0_1_0 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                window::WIN_HAMMING,
                6.76));

        this->low_pass_filter_0_1_0_0_1 = filter::fir_filter_fff::make(
            lp_decimantion,
            gr::filter::firdes::low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window::WIN_HAMMING,
                6.76));

        this->blocks_transcendental_1_1_0_0 = blocks::transcendental::make("atan", "$type");

        this->blocks_transcendental_1_1_0 = blocks::transcendental::make("atan", "$type");

        this->blocks_transcendental_1_1 = blocks::transcendental::make("atan", "$type");

        this->blocks_transcendental_0_0_0_1_0_0 = blocks::transcendental::make("atan", "$type");

        this->blocks_transcendental_0_0_0_1_0 = blocks::transcendental::make("atan", "$type");

        this->blocks_transcendental_0_0_0_1 = blocks::transcendental::make("atan", "$type");

        this->blocks_sub_xx_0_0_1_0_0 = blocks::sub_ff::make(1);

        this->blocks_sub_xx_0_0_1_0 = blocks::sub_ff::make(1);

        this->blocks_sub_xx_0_0_1 = blocks::sub_ff::make(1);

        this->blocks_multiply_xx_0_3_1_0_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_3_1_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_3_1 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_2_0_1_0_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_2_0_1_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_2_0_1 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_1_0_1_0_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_1_0_1_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_1_0_1 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_0_0_1_0_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_0_0_1_0 = blocks::multiply_ff::make(1);

        this->blocks_multiply_xx_0_0_0_1 = blocks::multiply_ff::make(1);

        this->blocks_divide_xx_0_1_1_0_0 = blocks::divide_ff::make(1);

        this->blocks_divide_xx_0_1_1_0 = blocks::divide_ff::make(1);

        this->blocks_divide_xx_0_1_1 = blocks::divide_ff::make(1);

        this->blocks_divide_xx_0_0_0_1_0_0 = blocks::divide_ff::make(1);

        this->blocks_divide_xx_0_0_0_1_0 = blocks::divide_ff::make(1);

        this->blocks_divide_xx_0_0_0_1 = blocks::divide_ff::make(1);

        this->band_pass_filter_0_1_1_0_0 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->band_pass_filter_0_1_1_0 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->band_pass_filter_0_1_1 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->band_pass_filter_0_0_0_1_0_0 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->band_pass_filter_0_0_0_1_0 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->band_pass_filter_0_0_0_1 = filter::fir_filter_fff::make(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                //window.WIN_HANN,
                6.76));

        this->analog_sig_source_x_1_0_1_0_0 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0,0.2);

        this->analog_sig_source_x_1_0_1_0 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0,0.2);

        this->analog_sig_source_x_1_0_1 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 2, 0,0.2);

        this->analog_sig_source_x_0_1_0_1_0_0 = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0,0);

        this->analog_sig_source_x_0_1_0_1_0 = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0,0);

        this->analog_sig_source_x_0_1_0_1 = analog::sig_source_f::make(out_samp_rate, analog::GR_SIN_WAVE, 55, 1, 0,0);

        this->analog_sig_source_x_0_0_1_0_0 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0,0);

        this->analog_sig_source_x_0_0_1_0 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0,0);

        this->analog_sig_source_x_0_0_1 = analog::sig_source_f::make(in_samp_rate, analog::GR_SIN_WAVE, 50, 325, 0,0);

        this->analog_sig_source_x_0_0_0_0_1_0_0 = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0,0);

        this->analog_sig_source_x_0_0_0_0_1_0 = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0,0);

        this->analog_sig_source_x_0_0_0_0_1 = analog::sig_source_f::make(out_samp_rate, analog::GR_COS_WAVE, 55, 1, 0,0);


// Connections:
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1, 0, this->blocks_multiply_xx_0_0_0_1, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1, 0, this->blocks_multiply_xx_0_1_0_1, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1_0, 0, this->blocks_multiply_xx_0_0_0_1_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1_0, 0, this->blocks_multiply_xx_0_1_0_1_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1_0_0, 0, this->blocks_multiply_xx_0_0_0_1_0_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_0_0_1_0_0, 0, this->blocks_multiply_xx_0_1_0_1_0_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_1, 0, this->band_pass_filter_0_0_0_1, 0);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_1_0, 0, this->band_pass_filter_0_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_0_1_0_0, 0, this->band_pass_filter_0_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1, 0, this->blocks_multiply_xx_0_2_0_1, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1, 0, this->blocks_multiply_xx_0_3_1, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1_0, 0, this->blocks_multiply_xx_0_2_0_1_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1_0, 0, this->blocks_multiply_xx_0_3_1_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1_0_0, 0, this->blocks_multiply_xx_0_2_0_1_0_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_0_1_0_1_0_0, 0, this->blocks_multiply_xx_0_3_1_0_0, 1);
    this->tb->hier_block2::connect(this->analog_sig_source_x_1_0_1, 0, this->band_pass_filter_0_1_1, 0);
    this->tb->hier_block2::connect(this->analog_sig_source_x_1_0_1_0, 0, this->band_pass_filter_0_1_1_0, 0);
    this->tb->hier_block2::connect(this->analog_sig_source_x_1_0_1_0_0, 0, this->band_pass_filter_0_1_1_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1, 0, this->blocks_multiply_xx_0_1_0_1, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1, 0, this->blocks_multiply_xx_0_3_1, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 6);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0, 0, this->blocks_multiply_xx_0_1_0_1_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0, 0, this->blocks_multiply_xx_0_3_1_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 3);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0_0, 0, this->blocks_multiply_xx_0_1_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0_0, 0, this->blocks_multiply_xx_0_3_1_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_0_0_1_0_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1, 0, this->blocks_multiply_xx_0_0_0_1, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1, 0, this->blocks_multiply_xx_0_2_0_1, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 7);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0, 0, this->blocks_multiply_xx_0_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0, 0, this->blocks_multiply_xx_0_2_0_1_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 4);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0_0, 0, this->blocks_multiply_xx_0_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0_0, 0, this->blocks_multiply_xx_0_2_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->band_pass_filter_0_1_1_0_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 1);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_0_0_1, 0, this->blocks_transcendental_0_0_0_1, 0);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_0_0_1_0, 0, this->blocks_transcendental_0_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_0_0_1_0_0, 0, this->blocks_transcendental_0_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_1_1, 0, this->blocks_transcendental_1_1, 0);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_1_1_0, 0, this->blocks_transcendental_1_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_divide_xx_0_1_1_0_0, 0, this->blocks_transcendental_1_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_0_0_1, 0, this->low_pass_filter_0_1_0_0_1, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_0_0_1_0, 0, this->low_pass_filter_0_1_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_0_0_1_0_0, 0, this->low_pass_filter_0_1_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_1_0_1, 0, this->low_pass_filter_0_1_1_0_1, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_1_0_1_0, 0, this->low_pass_filter_0_1_1_0_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_1_0_1_0_0, 0, this->low_pass_filter_0_1_1_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_2_0_1, 0, this->low_pass_filter_0_1_2_0_1, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_2_0_1_0, 0, this->low_pass_filter_0_1_2_0_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_2_0_1_0_0, 0, this->low_pass_filter_0_1_2_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_3_1, 0, this->low_pass_filter_0_1_3_1, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_3_1_0, 0, this->low_pass_filter_0_1_3_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_multiply_xx_0_3_1_0_0, 0, this->low_pass_filter_0_1_3_1_0_0, 0);
    this->tb->hier_block2::connect(this->blocks_sub_xx_0_0_1, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 8);
    this->tb->hier_block2::connect(this->blocks_sub_xx_0_0_1_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 5);
    this->tb->hier_block2::connect(this->blocks_sub_xx_0_0_1_0_0, 0, this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 2);
    this->tb->hier_block2::connect(this->blocks_transcendental_0_0_0_1, 0, this->blocks_sub_xx_0_0_1, 1);
    this->tb->hier_block2::connect(this->blocks_transcendental_0_0_0_1_0, 0, this->blocks_sub_xx_0_0_1_0, 1);
    this->tb->hier_block2::connect(this->blocks_transcendental_0_0_0_1_0_0, 0, this->blocks_sub_xx_0_0_1_0_0, 1);
    this->tb->hier_block2::connect(this->blocks_transcendental_1_1, 0, this->blocks_sub_xx_0_0_1, 0);
    this->tb->hier_block2::connect(this->blocks_transcendental_1_1_0, 0, this->blocks_sub_xx_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->blocks_transcendental_1_1_0_0, 0, this->blocks_sub_xx_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_0_0_1, 0, this->blocks_divide_xx_0_0_0_1, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_0_0_1_0, 0, this->blocks_divide_xx_0_0_0_1_0, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_0_0_1_0_0, 0, this->blocks_divide_xx_0_0_0_1_0_0, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_1_0_1, 0, this->blocks_divide_xx_0_1_1, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_1_0_1_0, 0, this->blocks_divide_xx_0_1_1_0, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_1_0_1_0_0, 0, this->blocks_divide_xx_0_1_1_0_0, 1);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_2_0_1, 0, this->blocks_divide_xx_0_0_0_1, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_2_0_1_0, 0, this->blocks_divide_xx_0_0_0_1_0, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_2_0_1_0_0, 0, this->blocks_divide_xx_0_0_0_1_0_0, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_3_1, 0, this->blocks_divide_xx_0_1_1, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_3_1_0, 0, this->blocks_divide_xx_0_1_1_0, 0);
    this->tb->hier_block2::connect(this->low_pass_filter_0_1_3_1_0_0, 0, this->blocks_divide_xx_0_1_1_0_0, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 11, this->qtgui_time_sink_x_0, 2);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 9, this->qtgui_time_sink_x_0, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 10, this->qtgui_time_sink_x_0, 1);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 7, this->qtgui_time_sink_x_1, 2);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 1, this->qtgui_time_sink_x_1, 1);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 0, this->qtgui_time_sink_x_1, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 13, this->qtgui_time_sink_x_2, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 14, this->qtgui_time_sink_x_2, 1);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 2, this->qtgui_time_sink_x_2, 2);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 12, this->qtgui_time_sink_x_2_0, 1);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 8, this->qtgui_time_sink_x_2_0, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 3, this->qtgui_time_sink_x_2_0, 2);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 6, this->qtgui_time_sink_x_2_1, 2);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 4, this->qtgui_time_sink_x_2_1, 0);
    this->tb->hier_block2::connect(this->pulsed_power_daq_power_calc_mul_ph_ff_0_0, 5, this->qtgui_time_sink_x_2_1, 1);
}

three_phases_simulation::~three_phases_simulation () {
}

// Callbacks:
int three_phases_simulation::get_bp_decimantion () const {
    return this->bp_decimantion;
}

void three_phases_simulation::set_bp_decimantion (int bp_decimantion) {
    this->bp_decimantion = bp_decimantion;
}

double three_phases_simulation::get_bp_high_cut () const {
    return this->bp_high_cut;
}

void three_phases_simulation::set_bp_high_cut (double bp_high_cut) {
    this->bp_high_cut = bp_high_cut;
    this->band_pass_filter_0_0_0_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
}

double three_phases_simulation::get_bp_low_cut () const {
    return this->bp_low_cut;
}

void three_phases_simulation::set_bp_low_cut (double bp_low_cut) {
    this->bp_low_cut = bp_low_cut;
    this->band_pass_filter_0_0_0_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
}

double three_phases_simulation::get_bp_trans () const {
    return this->bp_trans;
}

void three_phases_simulation::set_bp_trans (double bp_trans) {
    this->bp_trans = bp_trans;
    this->band_pass_filter_0_0_0_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
}

double three_phases_simulation::get_current_correction_factor () const {
    return this->current_correction_factor;
}

void three_phases_simulation::set_current_correction_factor (double current_correction_factor) {
    this->current_correction_factor = current_correction_factor;
}

double three_phases_simulation::get_in_samp_rate () const {
    return this->in_samp_rate;
}

void three_phases_simulation::set_in_samp_rate (double in_samp_rate) {
    this->in_samp_rate = in_samp_rate;
    this->analog_sig_source_x_0_0_1->set_sampling_freq(this->in_samp_rate);
    this->analog_sig_source_x_0_0_1_0->set_sampling_freq(this->in_samp_rate);
    this->analog_sig_source_x_0_0_1_0_0->set_sampling_freq(this->in_samp_rate);
    this->analog_sig_source_x_1_0_1->set_sampling_freq(this->in_samp_rate);
    this->analog_sig_source_x_1_0_1_0->set_sampling_freq(this->in_samp_rate);
    this->analog_sig_source_x_1_0_1_0_0->set_sampling_freq(this->in_samp_rate);
    this->band_pass_filter_0_0_0_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_0_0_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
    this->band_pass_filter_0_1_1_0_0->set_taps(firdes::band_pass(1, this->in_samp_rate, this->bp_low_cut, this->bp_high_cut, this->bp_trans, window.WIN_HANN, 6.76));
}

int three_phases_simulation::get_lp_decimantion () const {
    return this->lp_decimantion;
}

void three_phases_simulation::set_lp_decimantion (int lp_decimantion) {
    this->lp_decimantion = lp_decimantion;
}

double three_phases_simulation::get_out_samp_rate () const {
    return this->out_samp_rate;
}

void three_phases_simulation::set_out_samp_rate (double out_samp_rate) {
    this->out_samp_rate = out_samp_rate;
    this->analog_sig_source_x_0_0_0_0_1->set_sampling_freq(this->out_samp_rate);
    this->analog_sig_source_x_0_0_0_0_1_0->set_sampling_freq(this->out_samp_rate);
    this->analog_sig_source_x_0_0_0_0_1_0_0->set_sampling_freq(this->out_samp_rate);
    this->analog_sig_source_x_0_1_0_1->set_sampling_freq(this->out_samp_rate);
    this->analog_sig_source_x_0_1_0_1_0->set_sampling_freq(this->out_samp_rate);
    this->analog_sig_source_x_0_1_0_1_0_0->set_sampling_freq(this->out_samp_rate);                          //error when deleted
    this->low_pass_filter_0_1_0_0_1->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_0_0_1_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_0_0_1_0_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_1_0_1->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_1_0_1_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_1_0_1_0_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_2_0_1->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_2_0_1_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_2_0_1_0_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_3_1->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_3_1_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->low_pass_filter_0_1_3_1_0_0->set_taps(gr::filter::firdes::low_pass(1, this->out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76));
    this->qtgui_time_sink_x_0->set_samp_rate(this->out_samp_rate);
    this->qtgui_time_sink_x_1->set_samp_rate(this->out_samp_rate);
    this->qtgui_time_sink_x_2->set_samp_rate(this->out_samp_rate);
    this->qtgui_time_sink_x_2_0->set_samp_rate(this->out_samp_rate);
    this->qtgui_time_sink_x_2_1->set_samp_rate(this->out_samp_rate);
}

double three_phases_simulation::get_voltage_correction_factor () const {
    return this->voltage_correction_factor;
}

void three_phases_simulation::set_voltage_correction_factor (double voltage_correction_factor) {
    this->voltage_correction_factor = voltage_correction_factor;
}


int main (int argc, char **argv) {
    int bp_decimantion = 20;
    double bp_high_cut = 80;
    double bp_low_cut = 20;
    double bp_trans = 10;
    double current_correction_factor = 2.5;
    double in_samp_rate = 200000;
    int lp_decimantion = 1;
    double out_samp_rate = 10000;
    double voltage_correction_factor = 100;

    po::options_description desc("Options");
    desc.add_options()
    ("help", "display help")
    ("bp_decimantion", po::value<int>(&bp_decimantion), "Parameter")
    ("bp_high_cut", po::value<double>(&bp_high_cut), "Parameter")
    ("bp_low_cut", po::value<double>(&bp_low_cut), "Parameter")
    ("bp_trans", po::value<double>(&bp_trans), "Parameter")
    ("current_correction_factor", po::value<double>(&current_correction_factor), "Parameter")
    ("in_samp_rate", po::value<double>(&in_samp_rate), "Parameter")
    ("lp_decimantion", po::value<int>(&lp_decimantion), "Parameter")
    ("out_samp_rate", po::value<double>(&out_samp_rate), "Parameter")
    ("voltage_correction_factor", po::value<double>(&voltage_correction_factor), "Parameter")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    QApplication app(argc, argv);

    three_phases_simulation* top_block = new three_phases_simulation(bp_decimantion, bp_high_cut, bp_low_cut, bp_trans, current_correction_factor, in_samp_rate, lp_decimantion, out_samp_rate, voltage_correction_factor);

    top_block->tb->start();
    top_block->show();
    app.exec();


    return 0;
}
#include "moc_three_phases_simulation.cpp"
