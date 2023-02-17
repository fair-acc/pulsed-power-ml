#ifndef INCLUDED_PULSED_POWER_POWER_CALC_MUL_PH_FF_H
#define INCLUDED_PULSED_POWER_POWER_CALC_MUL_PH_FF_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API power_calc_mul_ph_ff : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<power_calc_mul_ph_ff> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::power_calc_mul_ph_ff.
     *
     * To avoid accidental use of raw pointers, pulsed_power::power_calc_mul_ph_ff's
     * constructor is in a private implementation
     * class. pulsed_power::power_calc_mul_ph_ff::make is the public interface for
     * creating new instances.
     */
    static sptr make(double alpha = 0.0000001);

    virtual void calc_active_power(float* out,
                                   float* voltage,
                                   float* current,
                                   float* phi_out,
                                   int noutput_items) = 0;

    virtual void calc_reactive_power(float* out,
                                     float* voltage,
                                     float* current,
                                     float* phi_out,
                                     int noutput_items) = 0;

    virtual void calc_apparent_power(float* out,
                                     float* voltage,
                                     float* current,
                                     int noutput_items) = 0;

    virtual void calc_phi_phase_correction(float* phi_out,
                                           const float* dalta_phi,
                                           int noutput_items) = 0;

    virtual void calc_rms_u(float* output, const float* input, int noutput_items) = 0;
    virtual void calc_rms_i(float* output, const float* input, int noutput_items) = 0;
    virtual void get_timestamp_ms(float* out) = 0;

    virtual void calc_acc_val_active_power(float* p_out_acc,
                                           float* p_out_1,
                                           float* p_out_2,
                                           float* p_out_3,
                                           int noutput_items) = 0;

    virtual void calc_acc_val_reactive_power(float* q_out_acc,
                                             float* q_out_1,
                                             float* q_out_2,
                                             float* q_out_3,
                                             int noutput_items) = 0;

    virtual void calc_acc_val_apparent_power(float* out,
                                             float* acc_active_power,
                                             float* acc_reactive_power,
                                             int noutput_items) = 0;

    virtual void set_alpha(double alpha) = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_POWER_CALC_MUL_PH_FF_H */
