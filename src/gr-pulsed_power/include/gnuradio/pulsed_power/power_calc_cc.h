#ifndef INCLUDED_PULSED_POWER_POWER_CALC_CC_H
#define INCLUDED_PULSED_POWER_POWER_CALC_CC_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief <+description of block+>
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API power_calc_cc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<power_calc_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::power_calc_cc.
     *
     * To avoid accidental use of raw pointers, pulsed_power::power_calc_cc's
     * constructor is in a private implementation
     * class. pulsed_power::power_calc_cc::make is the public interface for
     * creating new instances.
     */
    static sptr make(double alpha = 0.0000001);

    virtual void set_alpha(double alpha) = 0;

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

    virtual void calc_phi(float* phi_out,
                          const gr_complex* u_in,
                          const gr_complex* i_in,
                          int noutput_items) = 0;

    virtual void
    calc_rms_u(float* output, const gr_complex* input, int noutput_items) = 0;

    virtual void
    calc_rms_i(float* output, const gr_complex* input, int noutput_items) = 0;
};

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_POWER_CALC_CC_H */
