#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# Author: neumann
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
from gnuradio.fft import logpwrfft
from gnuradio.qtgui import Range, GrRangeWidget
from PyQt5 import QtCore



from gnuradio import qtgui

class Scheinleistung_Low_Frequency(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Not titled yet", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Not titled yet")
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

        self.settings = Qt.QSettings("GNU Radio", "Scheinleistung_Low_Frequency")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.source_samp_rate = source_samp_rate = 2000
        self.decimation = decimation = 20
        self.percentage_voltage = percentage_voltage = (325 * 0.05)
        self.percentage_current = percentage_current = (5 * 0.05)
        self.low_pass_samp_rate = low_pass_samp_rate = (source_samp_rate / decimation)
        self.items = items = 1000
        self.freq_modul = freq_modul = 2

        ##################################################
        # Blocks
        ##################################################
        self._freq_modul_range = Range(0, 50, 0.1, 2, 200)
        self._freq_modul_win = GrRangeWidget(self._freq_modul_range, self.set_freq_modul, "'freq_modul'", "counter_slider", float, QtCore.Qt.Horizontal, "value")

        self.top_layout.addWidget(self._freq_modul_win)
        self.qtgui_vector_sink_f_0 = qtgui.vector_sink_f(
            items,
            0,
            0.1,
            "x-Axis",
            "y-Axis",
            "",
            1, # Number of inputs
            None # parent
        )
        self.qtgui_vector_sink_f_0.set_update_time(0.10)
        self.qtgui_vector_sink_f_0.set_y_axis((-20), 60)
        self.qtgui_vector_sink_f_0.enable_autoscale(False)
        self.qtgui_vector_sink_f_0.enable_grid(False)
        self.qtgui_vector_sink_f_0.set_x_axis_units("")
        self.qtgui_vector_sink_f_0.set_y_axis_units("")
        self.qtgui_vector_sink_f_0.set_ref_level(0)


        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_vector_sink_f_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_vector_sink_f_0.set_line_label(i, labels[i])
            self.qtgui_vector_sink_f_0.set_line_width(i, widths[i])
            self.qtgui_vector_sink_f_0.set_line_color(i, colors[i])
            self.qtgui_vector_sink_f_0.set_line_alpha(i, alphas[i])

        self._qtgui_vector_sink_f_0_win = sip.wrapinstance(self.qtgui_vector_sink_f_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_vector_sink_f_0_win)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
            1024, #size
            source_samp_rate, #samp_rate
            "", #name
            1, #number of inputs
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


        labels = ['Signal 1', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
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


        for i in range(1):
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
        self.low_pass_filter_0_0 = filter.fir_filter_fff(
            decimation,
            firdes.low_pass(
                1,
                source_samp_rate,
                20,
                100,
                window.WIN_HAMMING,
                6.76))
        self.logpwrfft_x_0 = logpwrfft.logpwrfft_f(
            sample_rate=low_pass_samp_rate,
            fft_size=items,
            ref_scale=2,
            frame_rate=30,
            avg_alpha=1.0,
            average=False,
            shift=False)
        self.blocks_multiply_xx_0_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0 = blocks.multiply_vff(1)
        self.blocks_add_xx_0_0 = blocks.add_vff(1)
        self.blocks_add_xx_0 = blocks.add_vff(1)
        self.analog_sig_source_x_0_0_0 = analog.sig_source_f(source_samp_rate, analog.GR_SIN_WAVE, freq_modul, 1, 0, 0)
        self.analog_sig_source_x_0_0 = analog.sig_source_f(source_samp_rate, analog.GR_SIN_WAVE, 50, 5, 0, 0)
        self.analog_sig_source_x_0 = analog.sig_source_f(source_samp_rate, analog.GR_SIN_WAVE, 50, 325, 0, 0)
        self.analog_noise_source_x_0_0 = analog.noise_source_f(analog.GR_GAUSSIAN, percentage_voltage, 10)
        self.analog_noise_source_x_0 = analog.noise_source_f(analog.GR_GAUSSIAN, percentage_current, 0)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_noise_source_x_0, 0), (self.blocks_add_xx_0_0, 1))
        self.connect((self.analog_noise_source_x_0_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.analog_sig_source_x_0_0, 0), (self.blocks_multiply_xx_0_0, 0))
        self.connect((self.analog_sig_source_x_0_0_0, 0), (self.blocks_multiply_xx_0_0, 1))
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_multiply_xx_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.blocks_add_xx_0_0, 0), (self.blocks_multiply_xx_0, 1))
        self.connect((self.blocks_multiply_xx_0, 0), (self.low_pass_filter_0_0, 0))
        self.connect((self.blocks_multiply_xx_0_0, 0), (self.blocks_add_xx_0_0, 0))
        self.connect((self.logpwrfft_x_0, 0), (self.qtgui_vector_sink_f_0, 0))
        self.connect((self.low_pass_filter_0_0, 0), (self.logpwrfft_x_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "Scheinleistung_Low_Frequency")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_source_samp_rate(self):
        return self.source_samp_rate

    def set_source_samp_rate(self, source_samp_rate):
        self.source_samp_rate = source_samp_rate
        self.set_low_pass_samp_rate((self.source_samp_rate / self.decimation))
        self.analog_sig_source_x_0.set_sampling_freq(self.source_samp_rate)
        self.analog_sig_source_x_0_0.set_sampling_freq(self.source_samp_rate)
        self.analog_sig_source_x_0_0_0.set_sampling_freq(self.source_samp_rate)
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.source_samp_rate, 20, 100, window.WIN_HAMMING, 6.76))
        self.qtgui_time_sink_x_0.set_samp_rate(self.source_samp_rate)

    def get_decimation(self):
        return self.decimation

    def set_decimation(self, decimation):
        self.decimation = decimation
        self.set_low_pass_samp_rate((self.source_samp_rate / self.decimation))

    def get_percentage_voltage(self):
        return self.percentage_voltage

    def set_percentage_voltage(self, percentage_voltage):
        self.percentage_voltage = percentage_voltage
        self.analog_noise_source_x_0_0.set_amplitude(self.percentage_voltage)

    def get_percentage_current(self):
        return self.percentage_current

    def set_percentage_current(self, percentage_current):
        self.percentage_current = percentage_current
        self.analog_noise_source_x_0.set_amplitude(self.percentage_current)

    def get_low_pass_samp_rate(self):
        return self.low_pass_samp_rate

    def set_low_pass_samp_rate(self, low_pass_samp_rate):
        self.low_pass_samp_rate = low_pass_samp_rate
        self.logpwrfft_x_0.set_sample_rate(self.low_pass_samp_rate)

    def get_items(self):
        return self.items

    def set_items(self, items):
        self.items = items

    def get_freq_modul(self):
        return self.freq_modul

    def set_freq_modul(self, freq_modul):
        self.freq_modul = freq_modul
        self.analog_sig_source_x_0_0_0.set_frequency(self.freq_modul)




def main(top_block_cls=Scheinleistung_Low_Frequency, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

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
