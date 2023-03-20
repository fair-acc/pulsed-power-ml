#include <gnuradio/analog/noise_source.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/attributes.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/pulsed_power/mains_frequency_calc.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

namespace gr {
namespace pulsed_power {

BOOST_AUTO_TEST_SUITE(mains_frequency_calc_testing);

bool isSimilar(float first, float second, float decimals)
{
    float difference = 1 / (decimals * 10);
    return abs(first - second) < difference;
}


BOOST_AUTO_TEST_CASE(test_mains_frequency_calc_55_Hz_without_noise)
{
    gr::top_block_sptr tb = gr::make_top_block("top");
    int samp_rate = 2000000;
    gr::analog::gr_waveform_t waveform = gr::analog::gr_waveform_t::GR_SIN_WAVE;
    double ampl = 325;
    float offset = 0;
    float phase = 0;
    double frequency = 55;
    gr::analog::sig_source<float>::sptr signal_source =
        gr::analog::sig_source<float>::make(
            samp_rate, waveform, frequency, ampl, offset, phase);
    mains_frequency_calc::sptr calc_block =
        mains_frequency_calc::make(samp_rate, -100, 100);
    gr::blocks::vector_sink<float>::sptr vector_sink =
        gr::blocks::vector_sink<float>::make();
    gr::blocks::vector_sink<float>::sptr vector_sink_debug =
        gr::blocks::vector_sink<float>::make();

    signal_source->set_sampling_freq(samp_rate);
    tb->connect(signal_source, 0, calc_block, 0);
    tb->connect(signal_source, 0, vector_sink_debug, 0);
    tb->connect(calc_block, 0, vector_sink, 0);

    tb->start();

    int seconds_to_test = 2;
    boost::this_thread::sleep(boost::posix_time::seconds(seconds_to_test));

    tb->stop();

    std::vector<float> result_data = vector_sink->data();
    std::vector<float> rdebug_data = vector_sink_debug->data();

    // keep last second
    result_data.erase(result_data.begin(), result_data.end() - samp_rate);

    int res_size = result_data.size();
    for (int i = 0; i < res_size; i++) {
        // std::cout<<result_data.at(i);
        if (!isSimilar(result_data.at(i), frequency, 5)) {
            BOOST_CHECK(false);
            break;
        } else {
            BOOST_CHECK(true);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_mains_frequency_calc_55_Hz_with_noise_1_percent)
{
    gr::top_block_sptr tb = gr::make_top_block("top");
    int samp_rate = 2000000;
    gr::analog::gr_waveform_t waveform = gr::analog::gr_waveform_t::GR_SIN_WAVE;
    gr::analog::noise_type_t noise_type = gr::analog::noise_type_t::GR_GAUSSIAN;
    double ampl = 325;
    float offset = 0;
    float phase = 0;
    double frequency = 55;
    gr::analog::sig_source<float>::sptr signal_source =
        gr::analog::sig_source<float>::make(
            samp_rate, waveform, frequency, ampl, offset, phase);
    gr::analog::noise_source<float>::sptr noise_source =
        gr::analog::noise_source<float>::make(noise_type, ampl / 100);
    mains_frequency_calc::sptr calc_block =
        mains_frequency_calc::make(samp_rate, -100, 100);
    gr::blocks::vector_sink<float>::sptr vector_sink =
        gr::blocks::vector_sink<float>::make();
    gr::blocks::add_ff::sptr add_block = gr::blocks::add_ff::make();

    tb->connect(signal_source, 0, add_block, 0);
    tb->connect(noise_source, 0, add_block, 1);
    tb->connect(add_block, 0, calc_block, 0);
    tb->connect(calc_block, 0, vector_sink, 0);

    tb->start();

    int seconds_to_test = 2;
    boost::this_thread::sleep(boost::posix_time::seconds(seconds_to_test));

    tb->stop();

    std::vector<float> result_data = vector_sink->data();

    // keep last second
    result_data.erase(result_data.begin(), result_data.end() - samp_rate);

    int res_size = result_data.size();
    for (int i = 0; i < res_size; i++) {
        // std::cout<<result_data.at(i);
        if (!isSimilar(result_data.at(i), frequency, 4)) {
            BOOST_CHECK(false);
            break;
        } else {
            BOOST_CHECK(true);
        }
    }
}
BOOST_AUTO_TEST_CASE(test_mains_frequency_calc_55_Hz_with_noise_5_percent)
{
    gr::top_block_sptr tb = gr::make_top_block("top");
    int samp_rate = 2000000;
    gr::analog::gr_waveform_t waveform = gr::analog::gr_waveform_t::GR_SIN_WAVE;
    gr::analog::noise_type_t noise_type = gr::analog::noise_type_t::GR_GAUSSIAN;
    double ampl = 325;
    float offset = 0;
    float phase = 0;
    double frequency = 55;
    gr::analog::sig_source<float>::sptr signal_source =
        gr::analog::sig_source<float>::make(
            samp_rate, waveform, frequency, ampl, offset, phase);
    gr::analog::noise_source<float>::sptr noise_source =
        gr::analog::noise_source<float>::make(noise_type, ampl / 20);
    mains_frequency_calc::sptr calc_block =
        mains_frequency_calc::make(samp_rate, -100, 100);
    gr::blocks::vector_sink<float>::sptr vector_sink =
        gr::blocks::vector_sink<float>::make();
    gr::blocks::add_ff::sptr add_block = gr::blocks::add_ff::make();

    tb->connect(signal_source, 0, add_block, 0);
    tb->connect(noise_source, 0, add_block, 1);
    tb->connect(add_block, 0, calc_block, 0);
    tb->connect(calc_block, 0, vector_sink, 0);

    tb->start();

    int seconds_to_test = 2;
    boost::this_thread::sleep(boost::posix_time::seconds(seconds_to_test));

    tb->stop();

    std::vector<float> result_data = vector_sink->data();

    // keep last second
    result_data.erase(result_data.begin(), result_data.end() - samp_rate);

    int res_size = result_data.size();
    for (int i = 0; i < res_size; i++) {
        // std::cout<<result_data.at(i);
        if (!isSimilar(result_data.at(i), frequency, 3)) {
            BOOST_CHECK(false);
            break;
        } else {
            BOOST_CHECK(true);
        }
    }
}
BOOST_AUTO_TEST_SUITE_END();
} // namespace pulsed_power
} /* namespace gr */
