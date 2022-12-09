# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: power_calc_ff_prepper
# Author: neumann
# GNU Radio version: 3.10.1.1

from gnuradio import analog
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal







class power_calc_ff_prepper(gr.hier_block2):
    def __init__(self, bp_decimation = 20, samp_rate = 2000000):
        gr.hier_block2.__init__(
            self, "power_calc_ff_prepper",
                gr.io_signature.makev(2, 2, [gr.sizeof_float*1, gr.sizeof_float*1]),
                gr.io_signature.makev(5, 5, [gr.sizeof_float*1, gr.sizeof_float*1, gr.sizeof_float*1, gr.sizeof_float*1, gr.sizeof_float*1]),
        )

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate
        self.lp_trans = lp_trans = 10
        self.lp_gain = lp_gain = 1
        self.lp_decimation = lp_decimation = 1
        self.lp_cut = lp_cut = 60
        self.bp_trans = bp_trans = 10
        self.bp_low_cut = bp_low_cut = 20
        self.bp_high_cut = bp_high_cut = 80
        self.bp_gain = bp_gain = 1
        self.bp_decimation = bp_decimation = 20
        self.out_samp_rate = out_samp_rate = samp_rate/bp_decimation

        ##################################################
        # Blocks
        ##################################################
        self.rational_resampler_xxx_0_0 = filter.rational_resampler_fff(
                interpolation=1,
                decimation=bp_decimation,
                taps=[],
                fractional_bw=0)
        self.rational_resampler_xxx_0 = filter.rational_resampler_fff(
                interpolation=1,
                decimation=bp_decimation,
                taps=[],
                fractional_bw=0)
        self.low_pass_filter_0_1_2 = filter.fir_filter_fff(
            lp_decimation,
            firdes.low_pass(
                lp_gain,
                out_samp_rate,
                lp_cut,
                lp_trans,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_1 = filter.fir_filter_fff(
            lp_decimation,
            firdes.low_pass(
                lp_gain,
                out_samp_rate,
                lp_cut,
                lp_trans,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1_0 = filter.fir_filter_fff(
            lp_decimation,
            firdes.low_pass(
                lp_gain,
                out_samp_rate,
                lp_cut,
                lp_trans,
                window.WIN_HAMMING,
                6.76))
        self.low_pass_filter_0_1 = filter.fir_filter_fff(
            lp_decimation,
            firdes.low_pass(
                lp_gain,
                out_samp_rate,
                lp_cut,
                lp_trans,
                window.WIN_HAMMING,
                6.76))
        self.blocks_transcendental_0_0 = blocks.transcendental('atan', "float")
        self.blocks_transcendental_0 = blocks.transcendental('atan', "float")
        self.blocks_sub_xx_0 = blocks.sub_ff(1)
        self.blocks_multiply_xx_0_2 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_1 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0_0 = blocks.multiply_vff(1)
        self.blocks_multiply_xx_0 = blocks.multiply_vff(1)
        self.blocks_multiply_const_vxx_current = blocks.multiply_const_ff(2.5)
        self.blocks_multiply_const_vxx_voltage = blocks.multiply_const_ff(100)
        self.blocks_divide_xx_0_0 = blocks.divide_ff(1)
        self.blocks_divide_xx_0 = blocks.divide_ff(1)
        print("decimation: " + str(bp_decimation) + " bp_gain: " + str(bp_gain) + " sample rate: " 
              + str(samp_rate) + " bp low cut: " + str(bp_low_cut) + " bp high cut: " + str(bp_high_cut) + 
              " bp trans: " + str(bp_trans))
        self.band_pass_filter_voltage = filter.fir_filter_fff(
            bp_decimation,
            firdes.band_pass(
                bp_gain,
                samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.band_pass_filter_current = filter.fir_filter_fff(
            bp_decimation,
            firdes.band_pass(
                bp_gain,
                samp_rate,
                bp_low_cut,
                bp_high_cut,
                bp_trans,
                window.WIN_HANN,
                6.76))
        self.analog_sig_source_x_0_1 = analog.sig_source_f(out_samp_rate, analog.GR_SIN_WAVE, 55, 1, 0, 0)
        self.analog_sig_source_x_0_0_0 = analog.sig_source_f(out_samp_rate, analog.GR_COS_WAVE, 55, 1, 0, 0)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0_0_0, 0), (self.blocks_multiply_xx_0_0, 1))
        self.connect((self.analog_sig_source_x_0_0_0, 0), (self.blocks_multiply_xx_0_1, 1))
        self.connect((self.analog_sig_source_x_0_1, 0), (self.blocks_multiply_xx_0, 1))
        self.connect((self.analog_sig_source_x_0_1, 0), (self.blocks_multiply_xx_0_2, 1))
        self.connect((self.band_pass_filter_current, 0), (self.blocks_multiply_xx_0_0, 0))
        self.connect((self.band_pass_filter_current, 0), (self.blocks_multiply_xx_0_2, 0))
        self.connect((self.band_pass_filter_current, 0), (self, 1))
        self.connect((self.band_pass_filter_voltage, 0), (self.blocks_multiply_xx_0, 0))
        self.connect((self.band_pass_filter_voltage, 0), (self.blocks_multiply_xx_0_1, 0))
        self.connect((self.band_pass_filter_voltage, 0), (self, 2))
        self.connect((self.blocks_divide_xx_0, 0), (self.blocks_transcendental_0, 0))
        self.connect((self.blocks_divide_xx_0_0, 0), (self.blocks_transcendental_0_0, 0))
        self.connect((self.blocks_multiply_const_vxx_voltage, 0), (self.band_pass_filter_voltage, 0))
        self.connect((self.blocks_multiply_const_vxx_voltage, 0), (self.rational_resampler_xxx_0, 0))
        self.connect((self.blocks_multiply_const_vxx_current, 0), (self.band_pass_filter_current, 0))
        self.connect((self.blocks_multiply_const_vxx_current, 0), (self.rational_resampler_xxx_0_0, 0))
        self.connect((self.blocks_multiply_xx_0, 0), (self.low_pass_filter_0_1, 0))
        self.connect((self.blocks_multiply_xx_0_0, 0), (self.low_pass_filter_0_1_0, 0))
        self.connect((self.blocks_multiply_xx_0_1, 0), (self.low_pass_filter_0_1_1, 0))
        self.connect((self.blocks_multiply_xx_0_2, 0), (self.low_pass_filter_0_1_2, 0))
        self.connect((self.blocks_sub_xx_0, 0), (self, 0))
        self.connect((self.blocks_transcendental_0, 0), (self.blocks_sub_xx_0, 0))
        self.connect((self.blocks_transcendental_0_0, 0), (self.blocks_sub_xx_0, 1))
        self.connect((self.low_pass_filter_0_1, 0), (self.blocks_divide_xx_0, 0))
        self.connect((self.low_pass_filter_0_1_0, 0), (self.blocks_divide_xx_0_0, 1))
        self.connect((self.low_pass_filter_0_1_1, 0), (self.blocks_divide_xx_0, 1))
        self.connect((self.low_pass_filter_0_1_2, 0), (self.blocks_divide_xx_0_0, 0))
        self.connect((self, 1), (self.blocks_multiply_const_vxx_current, 0))
        self.connect((self, 0), (self.blocks_multiply_const_vxx_voltage, 0))
        self.connect((self.rational_resampler_xxx_0, 0), (self, 4))
        self.connect((self.rational_resampler_xxx_0_0, 0), (self, 3))


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.band_pass_filter_current.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_voltage.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.set_out_samp_rate(self, samp_rate/self.bp_decimation)
        
    def get_out_samp_rate(self):
        return self.out_samp_rate

    def set_out_samp_rate(self, out_samp_rate):
        self.out_samp_rate = out_samp_rate
        self.analog_sig_source_x_0_0_0.set_sampling_freq(self.out_samp_rate)
        self.analog_sig_source_x_0_1.set_sampling_freq(self.out_samp_rate)
        self.low_pass_filter_0_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))

    def get_lp_trans(self):
        return self.lp_trans

    def set_lp_trans(self, lp_trans):
        self.lp_trans = lp_trans
        self.low_pass_filter_0_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))

    def get_lp_gain(self):
        return self.lp_gain

    def set_lp_gain(self, lp_gain):
        self.lp_gain = lp_gain
        self.low_pass_filter_0_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))

    def get_lp_decimation(self):
        return self.lp_decimation

    def set_lp_decimation(self, lp_decimation):
        self.lp_decimation = lp_decimation

    def get_lp_cut(self):
        return self.lp_cut

    def set_lp_cut(self, lp_cut):
        self.lp_cut = lp_cut
        self.low_pass_filter_0_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_0.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_1.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_1_2.set_taps(firdes.low_pass(self.lp_gain, self.out_samp_rate, self.lp_cut, self.lp_trans, window.WIN_HAMMING, 6.76))

    def get_bp_trans(self):
        return self.bp_trans

    def set_bp_trans(self, bp_trans):
        self.bp_trans = bp_trans
        self.band_pass_filter_current.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_voltage.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_low_cut(self):
        return self.bp_low_cut

    def set_bp_low_cut(self, bp_low_cut):
        self.bp_low_cut = bp_low_cut
        self.band_pass_filter_current.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_voltage.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_high_cut(self):
        return self.bp_high_cut

    def set_bp_high_cut(self, bp_high_cut):
        self.bp_high_cut = bp_high_cut
        self.band_pass_filter_current.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_voltage.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_gain(self):
        return self.bp_gain

    def set_bp_gain(self, bp_gain):
        self.bp_gain = bp_gain
        self.band_pass_filter_current.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))
        self.band_pass_filter_voltage.set_taps(firdes.band_pass(self.bp_gain, self.samp_rate, self.bp_low_cut, self.bp_high_cut, self.bp_trans, window.WIN_HANN, 6.76))

    def get_bp_decimation(self):
        return self.bp_decimation

    def set_bp_decimation(self, bp_decimation):
        self.bp_decimation = bp_decimation
        self.set_out_samp_rate(self.samp_rate/bp_decimation)
