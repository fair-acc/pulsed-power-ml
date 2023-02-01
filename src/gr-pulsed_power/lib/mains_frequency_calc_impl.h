#ifndef INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_IMPL_H
#define INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_IMPL_H

#include <gnuradio/pulsed_power/mains_frequency_calc.h>

namespace gr {
namespace pulsed_power {

class mains_frequency_calc_impl : public mains_frequency_calc
{
private:
    float d_expected_sample_rate;
    float d_lo, d_hi;
    double current_half_frequency, average_frequency, d_alpha;
    int no_low, no_high;
    bool d_last_state;

public:
    mains_frequency_calc_impl(float expected_sample_rate = 2000000,
                              float low_threshold = -100,
                              float high_threshold = 100);
    ~mains_frequency_calc_impl();

    void calc_frequency_per_halfed_period(int current_count);
    void calc_current_average();
    void detect_mains_frequency_over_half_period(float* f_out,
                                                 const float* samples_in,
                                                 int noutput_items);
    void reset_no_low();
    void reset_no_high();


    bool thresholdNotCrossedForGivenSeconds(int seconds);
    bool isAboveThresholdDuringUpperPeriod(float sample);
    bool isBelowThresholdDuringUpperPeriod(float sample);
    bool hasCrossedThresholdForUpperPeriod(float sample);
    bool hasCrossedThresholdForLowerPeriod(float sample);
    bool isBelowThresholdDuringLowerPeriod(float sample);
    bool isAboveThresholdDuringLowerPeriod(float sample);

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_MAINS_FREQUENCY_CALC_IMPL_H */
