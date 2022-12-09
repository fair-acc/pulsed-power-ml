/* -*- c++ -*- */
/*
 * Copyright 2021 fair.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/io_signature.h>
#include "power_calc_mul_ph_ff_impl.h"

namespace gr {
  namespace pulsed_power_daq {

    power_calc_mul_ph_ff::sptr

    /**
     * @brief Construct a new power calc::make object
     * 
     * @param alpha A value > 0 < 1
     */
    power_calc_mul_ph_ff::make(double alpha)
    {
      return gnuradio::make_block_sptr<power_calc_mul_ph_ff_impl>(alpha);
    }

    /**
     * @brief Construct a new power calc impl::power calc impl object (private constructor) //todo: change to 9 input vals, 11 output vals
     * 
     * @param alpha A value > 0 < 1
     */
    power_calc_mul_ph_ff_impl::power_calc_mul_ph_ff_impl(double alpha)
      : gr::sync_block("power_calc",
              gr::io_signature::make(9 /* min inputs */, 9 /* max inputs */, sizeof(float)), //input: voltage, current, deltaphi for all three phases
              gr::io_signature::make(15 /* min outputs */, 15 /*max outputs */, sizeof(float)))
    {
      set_alpha(alpha);
    }

    /**
     * @brief Destroy the power calc impl::power calc impl object (virtual destructor)
     * 
     */
    power_calc_mul_ph_ff_impl::~power_calc_mul_ph_ff_impl()
    {                                                                                                                                                                                                                                                                                                         
    }

    /**
     * @brief Generates a timestamp (per routine) of milliseconds since New York 1970 UTC and splits them into two floats | convert back uint64_t int64 = ((long long) out[0] << 32) | out[1];
     * 
     * @param out The pointer containing 2 values, the first 4 byte (high) and the last 4 byte (low) | out[0]=>high; out[1]=>low
     */
    void power_calc_mul_ph_ff_impl::get_timestamp_ms(float* out)
    {
      uint64_t milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      out[0] = (float)(milliseconds_since_epoch >> 32);
      out[1] = (float)(milliseconds_since_epoch);
    }

    /**
     * @brief Calculates RMS for voltage for the number of items currently available
     * 
     * @param output The input pointer for peak voltage, over number of input items
     * @param input The input pointer for raw voltage, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_rms_u(float* output, const float* input, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++) 
        {
          double mag_sqrd = input[i] * input[i];
          d_avg_u = d_beta * d_avg_u + d_alpha * mag_sqrd;
          output[i] = sqrt(d_avg_u);
        }
    }

    /**
     * @brief Calculates RMS for current for the number of items currently available
     * 
     * @param output The input pointer for peak current, over number of input items
     * @param input The input pointer for raw current, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_rms_i(float* output, const float* input, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++) 
        {
          double mag_sqrd = input[i] * input[i];// + input[i].imag() * input[i].imag();
          d_avg_i = d_beta * d_avg_i + d_alpha * mag_sqrd;
          output[i] = sqrt(d_avg_i);
        }
    }

    /**
     * @brief Calculates phase correction; Flattens the value | Phi = PHIu - PHIi | for the number of items currently available
     * 
     * @param phi_out The output pointer containing Phi over all concurrent Phi voltage and Phi current values
     * @param u_in The input pointer for raw voltage, over number of input items
     * @param i_in The input pointer for raw current, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_phi_phase_correction(float* phi_out, const float* delta_phi, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++)
        { 
          float temp = 0;
          if (!isnan(delta_phi[i]) && delta_phi[i] >= 0 && delta_phi[i] <= 2 * M_PI) //check for valid value range -> always > 0?
          {
            temp = delta_phi[i];
            d_last_valid_phi = temp;
          }
          else 
          {
            temp = d_last_valid_phi;
          }
          // Phase correction
          if (temp <= (M_PI_2 * -1))
          {
            phi_out[i] = temp + M_PI;
          }
          else if (temp >= M_PI_2)
          {
            phi_out[i] = temp - M_PI;
          } 
          else
          {
            phi_out[i] = temp;
          }

          // Single Pole IIR Filter
          d_avg_phi = d_alpha * phi_out[i] + d_beta * d_avg_phi;
          phi_out[i] = d_avg_phi;
        }
    }

    /**
     * @brief Calculates active power | P = RMSu * RMSi * cos(Phi) | for the number of items currently available
     * 
     * @param out The output pointer containing P over all concurrent RMS and Phi values
     * @param voltage The input pointer for peak voltage, over number of input items
     * @param current The input pointer for peak current, over number of input items
     * @param phi_out The input pointer for phi, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_active_power(float* out, float* voltage, float* current, float* phi_out, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++) 
        {
          out[i] = (float)(voltage[i] * current[i] * cos(phi_out[i]));
        }
    }

    /**
     * @brief Calculates reactive power | Q = RMSu * RMSi * sin(Phi) | for the number of items currently available
     * 
     * @param out The output pointer containing Q over all concurrent RMS and Phi values
     * @param voltage The input pointer for peak voltage, over number of input items
     * @param current The input pointer for peak current, over number of input items
     * @param phi_out The input pointer for phi, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_reactive_power(float* out, float* voltage, float* current, float* phi_out, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++) 
        {
          out[i] = (float)(voltage[i] * current[i] * sin(phi_out[i]));
        }
    }

    /**
     * @brief Calculates apperent power | S = RMSu * RMSi | for the number of items currently available
     * 
     * @param out The output pointer containing S over all concurrent RMS values
     * @param voltage The input pointer for peak voltage, over number of input items
     * @param current The input pointer for peak current, over number of input items
     * @param noutput_items The samples currently available for cumputation
     */
    void power_calc_mul_ph_ff_impl::calc_apparent_power(float* out, float* voltage, float* current, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++)
        {
          out[i] = (float)(voltage[i] * current[i]);
        }
    }
    //calculate accumulated values
    void power_calc_mul_ph_ff_impl::calc_acc_val_active_power(float* p_out_acc, float* p_out_1, float* p_out_2, float* p_out_3, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++)
        {
          p_out_acc[i] = p_out_1[i] + p_out_2[i] + p_out_3[i];
        }
    }

    void power_calc_mul_ph_ff_impl::calc_acc_val_reactive_power(float* q_out_acc, float* q_out_1, float* q_out_2, float* q_out_3, int noutput_items)
    {
        for (int i = 0; i < noutput_items; i++)
        {
          q_out_acc[i] = q_out_1[i] + q_out_2[i] + q_out_3[i];
        }
    }

    void power_calc_mul_ph_ff_impl::calc_acc_val_apparent_power(float* out, float* acc_active_power, float* acc_reactive_power, int noutput_items)
    {
        //solve w pythagoras -> same result when calculated with complex values?
        if(acc_active_power == NULL || acc_reactive_power == NULL){
          return;
        }
        if(acc_active_power[0] == NULL || acc_reactive_power[0] == NULL){
          return; 
        }
        for (int i = 0; i < noutput_items; i++)
        {
          out[i] = sqrt(acc_active_power[i]*acc_active_power[i] + acc_reactive_power[i]*acc_reactive_power[i]);
        }
    }

    //output funct?

    /**
     * @brief Sets global alpha, beta und average for all RMS calculations
     * 
     * @param alpha A value > 0 < 1
     */
    void power_calc_mul_ph_ff_impl::set_alpha(double alpha)
    {
        d_alpha = alpha; ///< impacts the "flattening" | default value 0.00001
        d_beta = 1 - d_alpha;
        d_avg_u = 0; ///< RMS average for voltage
        d_avg_i = 0; ///< RMS average for current
        d_avg_phi = 0; ///< RMS | single point iir filter average
        d_last_valid_phi = 0;
    }

    /**
     * @brief Main | Core block routine
     * 
     * @param noutput_items The samples currently available for cumputation
     * @param input_items The item vector containing the input items
     * @param output_items  The item vector that will contain the output items
     * @return number of output items
     */
    int power_calc_mul_ph_ff_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {

      //calc first phase
      const float* u_in_1 =  (const float*)input_items[0];
      const float* i_in_1 = (const float*)input_items[1];

      const float* delta_phase_in_1 =  (const float*)input_items[2];

      float* p_out_1 = (float*)output_items[0];
      float* q_out_1 = (float*)output_items[1];
      float* s_out_1 = (float*)output_items[2];
      float* phi_out_1 = (float*)output_items[3];
      
      float* rms_u_1 = (float*)malloc(noutput_items*sizeof(float));
      float* rms_i_1 = (float*)malloc(noutput_items*sizeof(float));

      calc_rms_u(rms_u_1, u_in_1, noutput_items);
      calc_rms_i(rms_i_1, i_in_1, noutput_items);

      calc_phi_phase_correction(phi_out_1, delta_phase_in_1, noutput_items);

      calc_active_power(p_out_1, rms_u_1, rms_i_1, phi_out_1, noutput_items);
      calc_reactive_power(q_out_1, rms_u_1, rms_i_1, phi_out_1, noutput_items);
      calc_apparent_power(s_out_1, rms_u_1, rms_i_1, noutput_items);

      free(rms_u_1);
      free(rms_i_1);
      
      //calc sec phase
      const float* u_in_2 =  (const float*)input_items[3];
      const float* i_in_2 = (const float*)input_items[4];

      const float* delta_phase_in_2 =  (const float*)input_items[5];

      float* p_out_2 = (float*)output_items[4];
      float* q_out_2 = (float*)output_items[5];
      float* s_out_2 = (float*)output_items[6];
      float* phi_out_2 = (float*)output_items[7];
      
      float* rms_u_2 = (float*)malloc(noutput_items*sizeof(float));
      float* rms_i_2 = (float*)malloc(noutput_items*sizeof(float));

      calc_rms_u(rms_u_2, u_in_2, noutput_items);
      calc_rms_i(rms_i_2, i_in_2, noutput_items);

      calc_phi_phase_correction(phi_out_2, delta_phase_in_2, noutput_items);

      calc_active_power(p_out_2, rms_u_2, rms_i_2, phi_out_2, noutput_items);
      calc_reactive_power(q_out_2, rms_u_2, rms_i_2, phi_out_2, noutput_items);
      calc_apparent_power(s_out_2, rms_u_2, rms_i_2, noutput_items);

      free(rms_u_2);
      free(rms_i_2);

      //calc third phase
      const float* u_in_3 =  (const float*)input_items[6];
      const float* i_in_3 = (const float*)input_items[7];

      const float* delta_phase_in_3 =  (const float*)input_items[8];

      float* p_out_3 = (float*)output_items[8];
      float* q_out_3 = (float*)output_items[9];
      float* s_out_3 = (float*)output_items[10];
      float* phi_out_3 = (float*)output_items[11];
      
      float* rms_u_3 = (float*)malloc(noutput_items*sizeof(float));
      float* rms_i_3 = (float*)malloc(noutput_items*sizeof(float));

      calc_rms_u(rms_u_3, u_in_3, noutput_items);
      calc_rms_i(rms_i_3, i_in_3, noutput_items);

      calc_phi_phase_correction(phi_out_3, delta_phase_in_3, noutput_items);

      calc_active_power(p_out_3, rms_u_3, rms_i_3, phi_out_3, noutput_items);
      calc_reactive_power(q_out_3, rms_u_3, rms_i_3, phi_out_3, noutput_items);
      calc_apparent_power(s_out_3, rms_u_3, rms_i_3, noutput_items);

      free(rms_u_3);
      free(rms_i_3);
      
      float* p_out_acc = (float*)output_items[12];
      float* q_out_acc = (float*)output_items[13];
      float* s_out_acc = (float*)output_items[14];

      //call calc function for each phase
      calc_acc_val_active_power(p_out_acc, p_out_1, p_out_2, p_out_3, noutput_items);
      calc_acc_val_reactive_power(q_out_acc, q_out_1, q_out_2, q_out_3, noutput_items);
      calc_acc_val_apparent_power(s_out_acc, p_out_acc, q_out_acc, noutput_items);

      return noutput_items;
    }
  } /* namespace digitizers_39 */
} /* namespace gr */
