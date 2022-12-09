#ifndef INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_H
#define INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_H

#include <gnuradio/pulsed_power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace pulsed_power {

/*!
 * \brief GNU Radio OpenCMW sink for exporting time-domain data into OpenCMW.
 *
 * On each incoming data package a callback is called which allows the host
 * application to copy the data to its internal buffers
 * \ingroup pulsed_power
 *
 */
class PULSED_POWER_API opencmw_time_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<opencmw_time_sink> sptr;
    using cb_copy_data_t = std::function<void(std::vector<const void*>&,
                                              int&,
                                              const std::vector<std::string>&,
                                              float,
                                              int64_t)>;

    /*!
     * \brief Return a shared_ptr to a new instance of pulsed_power::opencmw_time_sink.
     *
     * To avoid accidental use of raw pointers, pulsed_power::opencmw_time_sink's
     * constructor is in a private implementation
     * class. pulsed_power::opencmw_time_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make(float sample_rate,
                     std::vector<std::string> signal_names,
                     std::vector<std::string> signal_units);

    /*!
     * \brief Registers a callback which is called whenever a predefined number of samples
     * is available.
     *
     * \param cb_copy_data callback in which the host application can copy the data
     */
    virtual void set_callback(cb_copy_data_t cb_copy_data) = 0;

    /*!
     * \brief Returns expected sample rate in Hz.
     */
    virtual float get_sample_rate() = 0;

    /*!
     * \brief Returns signal names.
     */
    virtual std::vector<std::string> get_signal_names() = 0;

    /*!
     * \brief Returns signal units.
     */
    virtual std::vector<std::string> get_signal_units() = 0;
};

extern PULSED_POWER_API std::mutex globalTimeSinksRegistryMutex;
extern PULSED_POWER_API std::vector<opencmw_time_sink*> globalTimeSinksRegistry;

} // namespace pulsed_power
} // namespace gr

#endif /* INCLUDED_PULSED_POWER_OPENCMW_TIME_SINK_H */
