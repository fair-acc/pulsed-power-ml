#ifndef INCLUDED_PULSED_POWER_POWER_CALC_FF_IMPL_H
#define INCLUDED_PULSED_POWER_POWER_CALC_FF_IMPL_H

#include <gnuradio/math.h>
#include <gnuradio/pulsed_power/power_calc_ff.h>
#include <volk/volk.h>
#include <cstdlib>

namespace gr {
namespace pulsed_power {

class power_calc_ff_impl : public power_calc_ff
{
private:
    double d_alpha, d_beta, d_avg_u, d_avg_i, d_avg_phi, d_last_valid_phi;

public:
    power_calc_ff_impl(double alpha = 0.0000001); // 100n
    ~power_calc_ff_impl() override;

    void calc_active_power(
        float* out, float* voltage, float* current, float* phi_out, int noutput_items);
    void calc_reactive_power(
        float* out, float* voltage, float* current, float* phi_out, int noutput_items);
    void
    calc_apparent_power(float* out, float* voltage, float* current, int noutput_items);
    void
    calc_phi_phase_correction(float* phi_out, const float* dalta_phi, int noutput_items);
    void calc_rms_u(float* output, const float* input, int noutput_items);
    void calc_rms_i(float* output, const float* input, int noutput_items);
    void get_timestamp_ms(float* out);

    void set_alpha(double alpha) override; // step-length

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_DIGITIZERS_39_POWER_CALC_FF_IMPL_H */
