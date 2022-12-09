#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 fair.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, gr_unittest
from gnuradio import analog
from gnuradio import blocks
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
import pulsed_power
import time

# from gnuradio import blocks
from power_calc_ff_prepper import power_calc_ff_prepper

class qa_power_calc_ff_prepper(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()
        
        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000
        
        ##################################################
        # Blocks
        ##################################################
        
        self.pulsed_power_power_calc_ff_prepper_0 = pulsed_power.power_calc_ff_prepper()
        self.pulsed_power_power_calc_ff_0 = pulsed_power.power_calc_ff(0.0001)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_float*1)
        self.analog_sig_source_current = analog.sig_source_f(self.samp_rate, analog.GR_COS_WAVE, 50, 2, 0, 0)
        self.analog_sig_source_voltage = analog.sig_source_f(self.samp_rate, analog.GR_COS_WAVE, 50, 5, 0, 0)
        self.blocks_vector_sink_p = blocks.vector_sink_f(1, 1024)
        self.blocks_vector_sink_q = blocks.vector_sink_f(1, 1024)
        self.blocks_vector_sink_s = blocks.vector_sink_f(1, 1024)
        self.blocks_vector_sink_phi = blocks.vector_sink_f(1, 1024)

        ##################################################
        # Connections
        ##################################################
        self.tb.connect((self.pulsed_power_power_calc_ff_prepper_0, 3), (self.blocks_null_sink_0, 0))
        self.tb.connect((self.pulsed_power_power_calc_ff_prepper_0, 4), (self.blocks_null_sink_0, 1))
        self.tb.connect((self.pulsed_power_power_calc_ff_prepper_0, 0), (self.pulsed_power_power_calc_ff_0, 2))
        self.tb.connect((self.pulsed_power_power_calc_ff_prepper_0, 2), (self.pulsed_power_power_calc_ff_0, 0))
        self.tb.connect((self.pulsed_power_power_calc_ff_prepper_0, 1), (self.pulsed_power_power_calc_ff_0, 1))
        self.tb.connect((self.pulsed_power_power_calc_ff_0, 1), (self.blocks_vector_sink_q, 0))
        self.tb.connect((self.pulsed_power_power_calc_ff_0, 0), (self.blocks_vector_sink_p, 0))
        self.tb.connect((self.pulsed_power_power_calc_ff_0, 3), (self.blocks_vector_sink_phi, 0))
        self.tb.connect((self.pulsed_power_power_calc_ff_0, 2), (self.blocks_vector_sink_s, 0))

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.analog_sig_source_voltage.set_sampling_freq(self.samp_rate)
        self.analog_sig_source_current.set_sampling_freq(self.samp_rate)
        
    def tearDown(self):
        self.tb = None
        self.pulsed_power_power_calc_ff_prepper_0 = None
        self.pulsed_power_power_calc_ff_0 = None
        self.blocks_null_sink_0 = None
        self.analog_sig_source_current = None
        self.analog_sig_source_voltage = None

    @staticmethod
    def Fill_List(size, value):
        return_list = []
        for x in range(0,size):
            return_list.append(value)
        return return_list

    def test_signal_source_offset_point5(self):
        
        
        self.tb.connect((self.analog_sig_source_voltage, 0), (self.pulsed_power_power_calc_ff_prepper_0, 0))
        self.tb.connect((self.analog_sig_source_current, 0), (self.pulsed_power_power_calc_ff_prepper_0, 1))
        dst = blocks.vector_sink_f()
        self.analog_sig_source_current.set_offset(0.5)
        
        seconds_to_discard = 5
        discarded_Samples = self.samp_rate * seconds_to_discard
        
        seconds_to_test = 2
        samples_to_test = self.samp_rate * seconds_to_test
        self.tb.start(samples_to_test+discarded_Samples)
        time.sleep(seconds_to_discard+seconds_to_test)
        result_vector_q   = self.blocks_vector_sink_q.data()
        result_vector_p   = self.blocks_vector_sink_p.data()
        result_vector_s   = self.blocks_vector_sink_s.data()
        result_vector_phi = self.blocks_vector_sink_phi.data()
        expected_value_p   = self.Fill_List(samples_to_test - discarded_Samples, 0)
        expected_value_q   = self.Fill_List(samples_to_test - discarded_Samples, 0)
        expected_value_s   = self.Fill_List(samples_to_test - discarded_Samples, 0)
        expected_value_phi = self.Fill_List(samples_to_test - discarded_Samples, 0)
        
        temp_q   = result_vector_p[discarded_Samples:samples_to_test]
        temp_p   = result_vector_q[discarded_Samples:samples_to_test]
        temp_s   = result_vector_s[discarded_Samples:samples_to_test]
        temp_phi = result_vector_phi[discarded_Samples:samples_to_test]
        #length_temp_q = len(temp_q)
        #length_q = len(expected_value_q)
        self.assertFloatTuplesAlmostEqual(temp_q,   expected_value_p,   1)
        self.assertFloatTuplesAlmostEqual(temp_p,   expected_value_q,   1)
        self.assertFloatTuplesAlmostEqual(temp_s,   expected_value_s,   1)
        self.assertFloatTuplesAlmostEqual(temp_phi, expected_value_phi, 1)
    
        
    #    for x in (discarded_Samples, samples_to_test):
    #        temp_q   = result_vector_p[x]
    #        temp_p   = result_vector_q[x]
    #        temp_s   = result_vector_s[x]
    #        temp_phi = result_vector_phi[x]
    #        self.assertFloatTuplesAlmostEqual(temp_q,   expected_value_p, 1)
    #        self.assertFloatTuplesAlmostEqual(temp_p,   expected_value_q, 1)
    #        self.assertFloatTuplesAlmostEqual(temp_s,   expected_value_s, 1)
    #        self.assertFloatTuplesAlmostEqual(temp_phi, expected_value_phi, 1)

if __name__ == '__main__':
    gr_unittest.run(qa_power_calc_ff_prepper)
