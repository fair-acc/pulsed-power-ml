#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: three_phases_simulation
# Author: buchner
# GNU Radio version: 3.10.4.0

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import blocks
from gnuradio import filter
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
import pulsed_power_daq



from gnuradio import qtgui

class three_phases_simulation(gr.top_block, Qt.QWidget):

    def __init__(self, bp_decimantion=20, bp_high_cut=80, bp_low_cut=20, bp_trans=10, current_correction_factor=2.5, in_samp_rate=200000, lp_decimantion=1, out_samp_rate=10000, voltage_correction_factor=100):
        gr.top_block.__init__(self, "three_phases_simulation", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("three_phases_simulation")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "three_phases_simulation")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Parameters
        ##################################################
        self.bp_decimantion = bp_decimantion
        self.bp_high_cut = bp_high_cut
        self.bp_low_cut = bp_low_cut
        self.bp_trans = bp_trans
        self.current_correction_factor = current_correction_factor
        self.in_samp_rate = in_samp_rate
        self.lp_decimantion = lp_decimantion
        self.out_samp_rate = out_samp_rate
        self.voltage_correction_factor = voltage_correction_factor

        ##################################################
        # Blocks
        ##################################################
        self.qtgui_time_sink_x_2_1 = qtgui.time_sink_f(
            1024, #size
            out_samp_rate, #samp_rate
            "", #name
            3, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_2_1.set_update_time(0.10)
        self.qtgui_time_sink_x_2_1.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_2_1.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_2_1.enable_tags(True)
        self.qtgui_time_sink_x_2_1.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_2_1.enable_autoscale(False)
        self.qtgui_time_sink_x_2_1.enable_grid(False)
        self.qtgui_time_sink_x_2_1.enable_axis_labels(True)
        self.qtgui_time_sink_x_2_1.enable_control_panel(True)
        self.qtgui_time_sink_x_2_1.enable_stem_plot(False)


        labels = ['P_acc', 'Q_acc', 'S_acc', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(3):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_2_1.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_2_1.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_2_1.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_2_1.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_2_1.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_2_1.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_2_1.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_2_1_win = sip.wrapinstance(self.qtgui_time_sink_x_2_1.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_2_1_win)
        self.qtgui_time_sink_x_2_0 = qtgui.time_sink_f(
            1024, #size
            out_samp_rate, #samp_rate
            "", #name
            3, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_2_0.set_update_time(0.10)
        self.qtgui_time_sink_x_2_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_2_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_2_0.enable_tags(True)
        self.qtgui_time_sink_x_2_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_2_0.enable_autoscale(False)
        self.qtgui_time_sink_x_2_0.enable_grid(False)
        self.qtgui_time_sink_x_2_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_2_0.enable_control_panel(True)
        self.qtgui_time_sink_x_2_0.enable_stem_plot(False)


        labels = ['Phi_1', 'Phi_2', 'Phi_3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(3):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_2_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_2_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_2_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_2_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_2_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_2_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_2_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_2_0_win = sip.wrapinstance(self.qtgui_time_sink_x_2_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_2_0_win)
        self.qtgui_time_sink_x_2 = qtgui.time_sink_f(
            1024, #size
            out_samp_rate, #samp_rate
            "", #name
            3, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_2.set_update_time(0.10)
        self.qtgui_time_sink_x_2.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_2.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_2.enable_tags(True)
        self.qtgui_time_sink_x_2.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_2.enable_autoscale(False)
        self.qtgui_time_sink_x_2.enable_grid(False)
        self.qtgui_time_sink_x_2.enable_axis_labels(True)
        self.qtgui_time_sink_x_2.enable_control_panel(True)
        self.qtgui_time_sink_x_2.enable_stem_plot(False)


        labels = ['P_3', 'Q_3', 'S_3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(3):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_2.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_2.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_2.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_2.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_2.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_2.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_2.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_2_win = sip.wrapinstance(self.qtgui_time_sink_x_2.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_2_win)
        self.qtgui_time_sink_x_1 = qtgui.time_sink_f(
            1024, #size
            out_samp_rate, #samp_rate
            "", #name
            3, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_1.set_update_time(0.10)
        self.qtgui_time_sink_x_1.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_1.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_1.enable_tags(True)
        self.qtgui_time_sink_x_1.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_1.enable_autoscale(False)
        self.qtgui_time_sink_x_1.enable_grid(False)
        self.qtgui_time_sink_x_1.enable_axis_labels(True)
        self.qtgui_time_sink_x_1.enable_control_panel(True)
        self.qtgui_time_sink_x_1.enable_stem_plot(False)


        labels = ['P_1', 'Q_1', 'S_1', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(3):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_1.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_1.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_1.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_1.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_1.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_1.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_1.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_1_win = sip.wrapinstance(self.qtgui_time_sink_x_1.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_1_win)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
            1024, #size
            out_samp_rate, #samp_rate
            "", #name
            3, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0.enable_grid(False)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(True)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)


        labels = ['P_2', 'Q_2', 'S_2', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(3):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_win)
        self.pulsed_power_daq_power_calc_mul_ph_ff_0_0 = pulsed_power_daq.power_calc_mul_ph_ff(0.0001)
        self.low_pass_filter_0_1_3_1 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_3_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_3 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_2_0_1 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_2_0_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_2_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_1_0_1 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_1_0_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_1_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_0_0_1 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_0_0_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_0_0 = filter.fir_filter_fff(
            lp_decimantion,
            firdes.low_pass(
                1,
                out_samp_rate,
                60,
                10,
                window.WIN_HAMMING,
                6.76))
        self.blocks_transcendental_1_1 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_1_0 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_1 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_0_0_0_1 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_0_0_0_0 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_0_0_0 = blocks.transcendental('atan', "float")
        self.blocks_sub_xx_0_0_1 = blocks.sub_ff(1)
        self.blocks_sub_xx_0_0_0 = blocks.sub_ff(1)
        self.blocks_sub_xx_0_0 = blocks.sub_ff(1)
        self.blocks_multiply_xx_0_3_1 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_3_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_3 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_2_0_1 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_2_0_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_2_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_1_0_1 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_1_0_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_1_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_0_0_1 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_0_0_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_0_0 = blocks.multiply_vff(1)
        self.blocks_divide_xx_0_1_1 = blocks.divide_ff(1)
        self.blocks_divide_xx_0_1_0 = blocks.divide_ff(1)
        self.blocks_divide_xx_0_1 = blocks.divide_ff(1)
        self.blocks_divide_xx_0_0_0_1 = blocks.divide_ff(1)
        self.blocks_divide_xx_0_0_0_0 = blocks.divide_ff(1)
        self.blocks_divide_xx_0_0_0 = blocks.divide_ff(1)
        self.band_pass_filter_0_1_1 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_0_1_0 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_0_1 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_0_0_0_1 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_0_0_0_0 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_0_0_0 = filter.fir_filter_fff(
            bp_decimantion,
            firdes.band_pass(
                1,
                in_samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.analog_sig_source_x_1_0_1 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 2, 0, 0.2)
        self.analog_sig_source_x_1_0_0 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 2, 0, 0.2)
        self.analog_sig_source_x_1_0 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 2, 0, 0.2)
        self.analog_sig_source_x_0_1_0_1 = analog.sig_source_f(out_samp_rate, analog.GR_SIN_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_1_0_0 = analog.sig_source_f(out_samp_rate, analog.GR_SIN_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_1_0 = analog.sig_source_f(out_samp_rate, analog.GR_SIN_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_0_1 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 325, 0, 0)
        self.analog_sig_source_x_0_0_0_0_1 = analog.sig_source_f(out_samp_rate, analog.GR_COS_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_0_0_0_0 = analog.sig_source_f(out_samp_rate, analog.GR_COS_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_0_0_0 = analog.sig_source_f(out_samp_rate, analog.GR_COS_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_0_0 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 325, 0, 0)
        self.analog_sig_source_x_0_0 = analog.sig_source_f(in_samp_rate, analog.GR_SIN_WAVE, 50, 325, 0, 0)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0_0, 0), (self.band_pass_filter_0_0_0, 0))
        self.connect((self.analog_sig_source_x_0_0_0, 0), (self.band_pass_filter_0_0_0_0, 0))
        self.connect((self.analog_sig_source_x_0_0_0_0, 0), (self.blocks_multiply_xx_0_0_0, 1))
        self.connect((self.analog_sig_source_x_0_0_0_0, 0), (self.blocks_multiply_xx_0_1_0, 1))
        self.connect((self.analog_sig_source_x_0_0_0_0_0, 0), (self.blocks_multiply_xx_0_0_0_0, 1))
        self.connect((self.analog_sig_source_x_0_0_0_0_0, 0), (self.blocks_multiply_xx_0_1_0_0, 1))
        self.connect((self.analog_sig_source_x_0_0_0_0_1, 0), (self.blocks_multiply_xx_0_0_0_1, 1))
        self.connect((self.analog_sig_source_x_0_0_0_0_1, 0), (self.blocks_multiply_xx_0_1_0_1, 1))
        self.connect((self.analog_sig_source_x_0_0_1, 0), (self.band_pass_filter_0_0_0_1, 0))
        self.connect((self.analog_sig_source_x_0_1_0, 0), (self.blocks_multiply_xx_0_2_0, 1))
        self.connect((self.analog_sig_source_x_0_1_0, 0), (self.blocks_multiply_xx_0_3, 1))
        self.connect((self.analog_sig_source_x_0_1_0_0, 0), (self.blocks_multiply_xx_0_2_0_0, 1))
        self.connect((self.analog_sig_source_x_0_1_0_0, 0), (self.blocks_multiply_xx_0_3_0, 1))
        self.connect((self.analog_sig_source_x_0_1_0_1, 0), (self.blocks_multiply_xx_0_2_0_1, 1))
        self.connect((self.analog_sig_source_x_0_1_0_1, 0), (self.blocks_multiply_xx_0_3_1, 1))
        self.connect((self.analog_sig_source_x_1_0, 0), (self.band_pass_filter_0_1, 0))
        self.connect((self.analog_sig_source_x_1_0_0, 0), (self.band_pass_filter_0_1_0, 0))
        self.connect((self.analog_sig_source_x_1_0_1, 0), (self.band_pass_filter_0_1_1, 0))
        self.connect((self.band_pass_filter_0_0_0, 0), (self.blocks_multiply_xx_0_1_0, 0))
        self.connect((self.band_pass_filter_0_0_0, 0), (self.blocks_multiply_xx_0_3, 0))
        self.connect((self.band_pass_filter_0_0_0, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 3))
        self.connect((self.band_pass_filter_0_0_0_0, 0), (self.blocks_multiply_xx_0_1_0_0, 0))
        self.connect((self.band_pass_filter_0_0_0_0, 0), (self.blocks_multiply_xx_0_3_0, 0))
        self.connect((self.band_pass_filter_0_0_0_0, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 0))
        self.connect((self.band_pass_filter_0_0_0_1, 0), (self.blocks_multiply_xx_0_1_0_1, 0))
        self.connect((self.band_pass_filter_0_0_0_1, 0), (self.blocks_multiply_xx_0_3_1, 0))
        self.connect((self.band_pass_filter_0_0_0_1, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 6))
        self.connect((self.band_pass_filter_0_1, 0), (self.blocks_multiply_xx_0_0_0, 0))
        self.connect((self.band_pass_filter_0_1, 0), (self.blocks_multiply_xx_0_2_0, 0))
        self.connect((self.band_pass_filter_0_1, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 4))
        self.connect((self.band_pass_filter_0_1_0, 0), (self.blocks_multiply_xx_0_0_0_0, 0))
        self.connect((self.band_pass_filter_0_1_0, 0), (self.blocks_multiply_xx_0_2_0_0, 0))
        self.connect((self.band_pass_filter_0_1_0, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 1))
        self.connect((self.band_pass_filter_0_1_1, 0), (self.blocks_multiply_xx_0_0_0_1, 0))
        self.connect((self.band_pass_filter_0_1_1, 0), (self.blocks_multiply_xx_0_2_0_1, 0))
        self.connect((self.band_pass_filter_0_1_1, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 7))
        self.connect((self.blocks_divide_xx_0_0_0, 0), (self.blocks_transcendental_0_0_0, 0))
        self.connect((self.blocks_divide_xx_0_0_0_0, 0), (self.blocks_transcendental_0_0_0_0, 0))
        self.connect((self.blocks_divide_xx_0_0_0_1, 0), (self.blocks_transcendental_0_0_0_1, 0))
        self.connect((self.blocks_divide_xx_0_1, 0), (self.blocks_transcendental_1, 0))
        self.connect((self.blocks_divide_xx_0_1_0, 0), (self.blocks_transcendental_1_0, 0))
        self.connect((self.blocks_divide_xx_0_1_1, 0), (self.blocks_transcendental_1_1, 0))
        self.connect((self.blocks_multiply_xx_0_0_0, 0), (self.low_pass_filter_0_1_0_0, 0))
        self.connect((self.blocks_multiply_xx_0_0_0_0, 0), (self.low_pass_filter_0_1_0_0_0, 0))
        self.connect((self.blocks_multiply_xx_0_0_0_1, 0), (self.low_pass_filter_0_1_0_0_1, 0))
        self.connect((self.blocks_multiply_xx_0_1_0, 0), (self.low_pass_filter_0_1_1_0, 0))
        self.connect((self.blocks_multiply_xx_0_1_0_0, 0), (self.low_pass_filter_0_1_1_0_0, 0))
        self.connect((self.blocks_multiply_xx_0_1_0_1, 0), (self.low_pass_filter_0_1_1_0_1, 0))
        self.connect((self.blocks_multiply_xx_0_2_0, 0), (self.low_pass_filter_0_1_2_0, 0))
        self.connect((self.blocks_multiply_xx_0_2_0_0, 0), (self.low_pass_filter_0_1_2_0_0, 0))
        self.connect((self.blocks_multiply_xx_0_2_0_1, 0), (self.low_pass_filter_0_1_2_0_1, 0))
        self.connect((self.blocks_multiply_xx_0_3, 0), (self.low_pass_filter_0_1_3, 0))
        self.connect((self.blocks_multiply_xx_0_3_0, 0), (self.low_pass_filter_0_1_3_0, 0))
        self.connect((self.blocks_multiply_xx_0_3_1, 0), (self.low_pass_filter_0_1_3_1, 0))
        self.connect((self.blocks_sub_xx_0_0, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 5))
        self.connect((self.blocks_sub_xx_0_0_0, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 2))
        self.connect((self.blocks_sub_xx_0_0_1, 0), (self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 8))
        self.connect((self.blocks_transcendental_0_0_0, 0), (self.blocks_sub_xx_0_0, 1))
        self.connect((self.blocks_transcendental_0_0_0_0, 0), (self.blocks_sub_xx_0_0_0, 1))
        self.connect((self.blocks_transcendental_0_0_0_1, 0), (self.blocks_sub_xx_0_0_1, 1))
        self.connect((self.blocks_transcendental_1, 0), (self.blocks_sub_xx_0_0, 0))
        self.connect((self.blocks_transcendental_1_0, 0), (self.blocks_sub_xx_0_0_0, 0))
        self.connect((self.blocks_transcendental_1_1, 0), (self.blocks_sub_xx_0_0_1, 0))
        self.connect((self.low_pass_filter_0_1_0_0, 0), (self.blocks_divide_xx_0_0_0, 1))
        self.connect((self.low_pass_filter_0_1_0_0_0, 0), (self.blocks_divide_xx_0_0_0_0, 1))
        self.connect((self.low_pass_filter_0_1_0_0_1, 0), (self.blocks_divide_xx_0_0_0_1, 1))
        self.connect((self.low_pass_filter_0_1_1_0, 0), (self.blocks_divide_xx_0_1, 1))
        self.connect((self.low_pass_filter_0_1_1_0_0, 0), (self.blocks_divide_xx_0_1_0, 1))
        self.connect((self.low_pass_filter_0_1_1_0_1, 0), (self.blocks_divide_xx_0_1_1, 1))
        self.connect((self.low_pass_filter_0_1_2_0, 0), (self.blocks_divide_xx_0_0_0, 0))
        self.connect((self.low_pass_filter_0_1_2_0_0, 0), (self.blocks_divide_xx_0_0_0_0, 0))
        self.connect((self.low_pass_filter_0_1_2_0_1, 0), (self.blocks_divide_xx_0_0_0_1, 0))
        self.connect((self.low_pass_filter_0_1_3, 0), (self.blocks_divide_xx_0_1, 0))
        self.connect((self.low_pass_filter_0_1_3_0, 0), (self.blocks_divide_xx_0_1_0, 0))
        self.connect((self.low_pass_filter_0_1_3_1, 0), (self.blocks_divide_xx_0_1_1, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 10), (self.qtgui_time_sink_x_0, 1))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 11), (self.qtgui_time_sink_x_0, 2))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 9), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 7), (self.qtgui_time_sink_x_1, 2))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 0), (self.qtgui_time_sink_x_1, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 1), (self.qtgui_time_sink_x_1, 1))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 2), (self.qtgui_time_sink_x_2, 2))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 14), (self.qtgui_time_sink_x_2, 1))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 13), (self.qtgui_time_sink_x_2, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 12), (self.qtgui_time_sink_x_2_0, 1))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 3), (self.qtgui_time_sink_x_2_0, 2))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 8), (self.qtgui_time_sink_x_2_0, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 5), (self.qtgui_time_sink_x_2_1, 1))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 4), (self.qtgui_time_sink_x_2_1, 0))
        self.connect((self.pulsed_power_daq_power_calc_mul_ph_ff_0_0, 6), (self.qtgui_time_sink_x_2_1, 2))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "three_phases_simulation")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_bp_decimantion(self):
        return self.bp_decimantion

    def set_bp_decimantion(self, bp_decimantion):
        self.bp_decimantion = bp_decimantion

    def get_bp_high_cut(self):
        return self.bp_high_cut

    def set_bp_high_cut(self, bp_high_cut):
        self.bp_high_cut = bp_high_cut
        self.band_pass_filter_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_low_cut(self):
        return self.bp_low_cut

    def set_bp_low_cut(self, bp_low_cut):
        self.bp_low_cut = bp_low_cut
        self.band_pass_filter_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_trans(self):
        return self.bp_trans

    def set_bp_trans(self, bp_trans):
        self.bp_trans = bp_trans
        self.band_pass_filter_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_current_correction_factor(self):
        return self.current_correction_factor

    def set_current_correction_factor(self, current_correction_factor):
        self.current_correction_factor = current_correction_factor

    def get_in_samp_rate(self):
        return self.in_samp_rate

    def set_in_samp_rate(self, in_samp_rate):
        self.in_samp_rate = in_samp_rate
        self.analog_sig_source_x_0_0.set_sampling_freq(self.in_samp_rate)
        self.analog_sig_source_x_0_0_0.set_sampling_freq(self.in_samp_rate)
        self.analog_sig_source_x_0_0_1.set_sampling_freq(self.in_samp_rate)
        self.analog_sig_source_x_1_0.set_sampling_freq(self.in_samp_rate)
        self.analog_sig_source_x_1_0_0.set_sampling_freq(self.in_samp_rate)
        self.analog_sig_source_x_1_0_1.set_sampling_freq(self.in_samp_rate)
        self.band_pass_filter_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_0_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_0.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_0_1_1.set_taps(firdes.band_pass(1, self.in_samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_lp_decimantion(self):
        return self.lp_decimantion

    def set_lp_decimantion(self, lp_decimantion):
        self.lp_decimantion = lp_decimantion

    def get_out_samp_rate(self):
        return self.out_samp_rate

    def set_out_samp_rate(self, out_samp_rate):
        self.out_samp_rate = out_samp_rate
        self.analog_sig_source_x_0_0_0_0.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_0_0_0_0.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_0_0_0_1.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_1_0.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_1_0_0.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_1_0_1.set_sampling_freq(self.out_samp_rate)
        self.low_pass_filter_0_1_0_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0_0_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0_0_1.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1_0_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1_0_1.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2_0_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2_0_1.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_3.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_3_0.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_3_1.set_taps(firdes.low_pass(1, self.out_samp_rate, 60, 10, window.WIN_HAMMING, 6.76))
        self.qtgui_time_sink_x_0.set_samp_rate(self.out_samp_rate)
        self.qtgui_time_sink_x_1.set_samp_rate(self.out_samp_rate)
        self.qtgui_time_sink_x_2.set_samp_rate(self.out_samp_rate)
        self.qtgui_time_sink_x_2_0.set_samp_rate(self.out_samp_rate)
        self.qtgui_time_sink_x_2_1.set_samp_rate(self.out_samp_rate)

    def get_voltage_correction_factor(self):
        return self.voltage_correction_factor

    def set_voltage_correction_factor(self, voltage_correction_factor):
        self.voltage_correction_factor = voltage_correction_factor



def argument_parser():
    parser = ArgumentParser()
    parser.add_argument(
        "--bp-decimantion", dest="bp_decimantion", type=intx, default=20,
        help="Set band pass decimantion [default=%(default)r]")
    parser.add_argument(
        "--bp-high-cut", dest="bp_high_cut", type=eng_float, default=eng_notation.num_to_str(float(80)),
        help="Set band pass high cutoff frequency [default=%(default)r]")
    parser.add_argument(
        "--bp-low-cut", dest="bp_low_cut", type=eng_float, default=eng_notation.num_to_str(float(20)),
        help="Set band pass low cutoff frequency [default=%(default)r]")
    parser.add_argument(
        "--bp-trans", dest="bp_trans", type=eng_float, default=eng_notation.num_to_str(float(10)),
        help="Set band pass transition width [default=%(default)r]")
    parser.add_argument(
        "--current-correction-factor", dest="current_correction_factor", type=eng_float, default=eng_notation.num_to_str(float(2.5)),
        help="Set Current Correction Factor [default=%(default)r]")
    parser.add_argument(
        "--in-samp-rate", dest="in_samp_rate", type=eng_float, default=eng_notation.num_to_str(float(200000)),
        help="Set in-coming samp rate [default=%(default)r]")
    parser.add_argument(
        "--lp-decimantion", dest="lp_decimantion", type=intx, default=1,
        help="Set low pass decimantion [default=%(default)r]")
    parser.add_argument(
        "--out-samp-rate", dest="out_samp_rate", type=eng_float, default=eng_notation.num_to_str(float(10000)),
        help="Set out-going samp rate [default=%(default)r]")
    parser.add_argument(
        "--voltage-correction-factor", dest="voltage_correction_factor", type=eng_float, default=eng_notation.num_to_str(float(100)),
        help="Set Voltage Correction Factor [default=%(default)r]")
    return parser


def main(top_block_cls=three_phases_simulation, options=None):
    if options is None:
        options = argument_parser().parse_args()

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls(bp_decimantion=options.bp_decimantion, bp_high_cut=options.bp_high_cut, bp_low_cut=options.bp_low_cut, bp_trans=options.bp_trans, current_correction_factor=options.current_correction_factor, in_samp_rate=options.in_samp_rate, lp_decimantion=options.lp_decimantion, out_samp_rate=options.out_samp_rate, voltage_correction_factor=options.voltage_correction_factor)

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
