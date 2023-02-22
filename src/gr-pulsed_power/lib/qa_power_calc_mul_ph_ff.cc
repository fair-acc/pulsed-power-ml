#include <gnuradio/attributes.h>
#include <gnuradio/pulsed_power/power_calc_mul_ph_ff.h>
#include <boost/test/unit_test.hpp>

namespace gr {
namespace pulsed_power {

BOOST_AUTO_TEST_SUITE(power_calc_mul_ph_ff_testing);

bool isSimilar(float first, float second, float decimals)
{
    float difference = 1 / (decimals * 10);
    return abs(first - second) < difference;
}


BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_active_and_reactive_power_isnan)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float out_active_float[1] = { 0 };
    float* out_active = out_active_float;
    float out_reactive_float[1] = { 0 };
    float* out_reactive = out_reactive_float;
    float float_voltage[1] = { 1 };
    float* voltage = float_voltage;
    float float_current[1] = { 1 };
    float* current = float_current;
    float float_phi_out[1] = { 0 };
    float* phi_out = float_phi_out;
    int noutput_items = 1;
    calc_block->calc_active_power(out_active, voltage, current, phi_out, noutput_items);
    calc_block->calc_reactive_power(
        out_reactive, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(out_active[0] == (float)(1 * 1 * cos(0)));
    BOOST_CHECK(out_reactive[0] == (float)(1 * 1 * sin(0)));


    // TODO: what to expect from NaN?
    // right now we expect NaN

    /**
     * checking for one input being NaN
     */
    voltage[0] = 1;
    current[0] = 1;
    voltage[0] = nanf("");
    calc_block->calc_active_power(out_active, voltage, current, phi_out, noutput_items);
    calc_block->calc_reactive_power(
        out_reactive, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(std::isnan(out_active[0]));
    BOOST_CHECK(std::isnan(out_reactive[0]));

    voltage[0] = 1;
    current[0] = nanf("");
    phi_out[0] = 0;
    calc_block->calc_active_power(out_active, voltage, current, phi_out, noutput_items);
    calc_block->calc_reactive_power(
        out_reactive, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(std::isnan(out_active[0]));
    BOOST_CHECK(std::isnan(out_reactive[0]));

    voltage[0] = 1;
    current[0] = 1;
    phi_out[0] = nanf("");
    calc_block->calc_active_power(out_active, voltage, current, phi_out, noutput_items);
    calc_block->calc_reactive_power(
        out_reactive, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(std::isnan(out_active[0]));
    BOOST_CHECK(std::isnan(out_reactive[0]));

    /**
     * checking for all inputs being NaN
     */
    voltage[0] = nanf("");
    current[0] = nanf("");
    phi_out[0] = nanf("");
    calc_block->calc_active_power(out_active, voltage, current, phi_out, noutput_items);
    calc_block->calc_reactive_power(
        out_reactive, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(std::isnan(out_active[0]));
    BOOST_CHECK(std::isnan(out_reactive[0]));
}
BOOST_AUTO_TEST_CASE(
    test_power_calc_mul_ph_ff_Calc_active_reactive_and_apparent_power_basic_values)
{
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(0.0001);
    float out_float[1] = { 0 };
    float* out = out_float;
    float float_voltage[1] = { 1 };
    float* voltage = float_voltage;
    float float_current[1] = { 1 };
    float* current = float_current;
    float float_phi_out[1] = { 0 };
    float* phi_out = float_phi_out;
    int noutput_items = 1;
    calc_block->calc_active_power(out, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(out[0] == (float)(1 * 1 * cos(0)));
    calc_block->calc_reactive_power(out, voltage, current, phi_out, noutput_items);
    BOOST_CHECK(out[0] == (float)(1 * 1 * sin(0)));
    calc_block->calc_apparent_power(out, voltage, current, noutput_items);
    BOOST_CHECK(out[0] == (float)(1 * 1));
}

BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_apparent_power)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float out_float[1] = { 0 };
    float* out_apparent = out_float;
    float float_voltage[1] = { 2 };
    float* voltage = float_voltage;
    float float_current[1] = { 2 };
    float* current = float_current;
    int noutput_items = 1;
    calc_block->calc_apparent_power(out_apparent, voltage, current, noutput_items);
    BOOST_CHECK(out_apparent[0] == 4.f);

    voltage[0] = nanf("");
    current[0] = 2;
    calc_block->calc_apparent_power(out_apparent, voltage, current, noutput_items);
    BOOST_CHECK(std::isnan(out_apparent[0]));

    voltage[0] = 2;
    current[0] = nanf("");
    calc_block->calc_apparent_power(out_apparent, voltage, current, noutput_items);
    BOOST_CHECK(std::isnan(out_apparent[0]));

    voltage[0] = nanf("");
    current[0] = nanf("");
    calc_block->calc_apparent_power(out_apparent, voltage, current, noutput_items);
    BOOST_CHECK(std::isnan(out_apparent[0]));
}

BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_rms)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float out_u_float[1] = { 0 };
    float* out_rms_u = out_u_float;
    float out_i_float[1] = { 0 };
    float* out_rms_i = out_i_float;
    float float_value[1] = { 0 };
    float* value = float_value;
    int noutput_items = 1;

    // test 0
    calc_block->calc_rms_u(out_rms_u, value, noutput_items);
    calc_block->calc_rms_i(out_rms_i, value, noutput_items);
    BOOST_CHECK(out_rms_u[0] == 0.f);
    BOOST_CHECK(out_rms_i[0] == 0.f);
    calc_block->calc_rms_u(out_rms_u, value, noutput_items);
    calc_block->calc_rms_i(out_rms_i, value, noutput_items);
    BOOST_CHECK(out_rms_u[0] == 0.f);
    BOOST_CHECK(out_rms_i[0] == 0.f);

    // test 1 --theoretically: multiply peak value with 1/sqrt2
    float out_float[1] = { 0 };
    float* out = out_float;
    float input_float[1] = { 1 };
    float* input = input_float;

    noutput_items = 1;
    calc_block->calc_rms_u(out, input, noutput_items);
    BOOST_CHECK(isSimilar(out[0], (float)(sqrt(0.0001 * 2)), 10));
    calc_block->calc_rms_i(out, input, noutput_items);
    BOOST_CHECK(isSimilar(out[0], (float)(sqrt(0.0001 * 2)), 10));
    // todo: check if values approach as expected

    // test 100
    float curValue = 100;
    value[0] = curValue;
    calc_block->calc_rms_u(out_rms_u, value, noutput_items);
    calc_block->calc_rms_i(out_rms_i, value, noutput_items);
    BOOST_CHECK(isSimilar(
        out_rms_u[0], sqrt(((1.f - alpha) * 0 + alpha * curValue * curValue)), 10));
    BOOST_CHECK(isSimilar(
        out_rms_i[0], sqrt(((1.f - alpha) * 0 + alpha * curValue * curValue)), 10));

    float avg_rms_u = ((1.f - alpha) * 0 + alpha * curValue * curValue);
    float avg_rms_i = ((1.f - alpha) * 0 + alpha * curValue * curValue);
    for (int i = 0; i < 100; i++) {
        value[0] = curValue;
        calc_block->calc_rms_u(out_rms_u, value, noutput_items);
        calc_block->calc_rms_i(out_rms_i, value, noutput_items);
        BOOST_CHECK(
            isSimilar(out_rms_u[0],
                      sqrt(((1.f - alpha) * avg_rms_u + alpha * curValue * curValue)),
                      7));
        BOOST_CHECK(
            isSimilar(out_rms_i[0],
                      sqrt(((1.f - alpha) * avg_rms_i + alpha * curValue * curValue)),
                      7));
        avg_rms_u = ((1 - alpha) * avg_rms_u + alpha * curValue * curValue);
        avg_rms_i = ((1 - alpha) * avg_rms_i + alpha * curValue * curValue);
    }
}
BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float out_phi_float[1] = { 0 };
    float* out_phi = out_phi_float;
    float delta_phi_float[1] = { 0 };
    float* delta_phi = delta_phi_float;
    int noutput_items = 1;

    // test 0
    calc_block->calc_phi_phase_correction(out_phi, delta_phi, noutput_items);
    BOOST_CHECK(out_phi[0] == 0.f);
    calc_block->calc_phi_phase_correction(out_phi, delta_phi, noutput_items);
    BOOST_CHECK(out_phi[0] == 0.f);

    // test input out of bounds
    delta_phi_float[0] = 2 * M_1_PI;
    calc_block->calc_phi_phase_correction(out_phi, delta_phi, noutput_items);
    BOOST_CHECK(isSimilar(out_phi[0], 0.f, 10));

    // test multiple values as input
    float out_phi_float_mulval[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    float* out_phi_mulval = out_phi_float_mulval;
    float delta_phi_float_mulval[10] = {
        0.2f, 0.2f, 0.2f, 0.2f, 0.2f,
        0.2f, 0.2f, 0.2f, 0.2f, -2.9415926535897932384626433832795f
    };
    float* delta_phi_mulval = delta_phi_float_mulval;
    noutput_items = 10;
    calc_block->calc_phi_phase_correction(
        out_phi_mulval, delta_phi_mulval, noutput_items);
    for (int i = 0; i < noutput_items; i++) {
        // BOOST_CHECK(isSimilar(out_phi_mulval[i], 0.2f, 2));
    }
}
BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_acc_val_active_power)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float p_out_1_f[1] = { 3 };
    float* p_out_1 = p_out_1_f;
    float p_out_2_f[1] = { 3 };
    float* p_out_2 = p_out_2_f;
    float p_out_3_f[1] = { 3 };
    float* p_out_3 = p_out_3_f;
    float p_out_acc_f[1] = { 0 };
    float* p_out_acc = p_out_acc_f;
    calc_block->calc_acc_val_active_power(p_out_acc, p_out_1, p_out_2, p_out_3, 1);
    BOOST_CHECK(p_out_acc[0] == 9.0f);
}
BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_acc_val_reactive_power)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float p_out_1_f[1] = { 3 };
    float* p_out_1 = p_out_1_f;
    float p_out_2_f[1] = { 3 };
    float* p_out_2 = p_out_2_f;
    float p_out_3_f[1] = { 3 };
    float* p_out_3 = p_out_3_f;
    float p_out_acc_f[1] = { 0 };
    float* p_out_acc = p_out_acc_f;
    calc_block->calc_acc_val_reactive_power(p_out_acc, p_out_1, p_out_2, p_out_3, 1);
    BOOST_CHECK(p_out_acc[0] == 9.0f);
}
BOOST_AUTO_TEST_CASE(test_power_calc_mul_ph_ff_Calc_acc_val_apparent_power)
{
    float alpha = 0.0001;
    auto calc_block = gr::pulsed_power::power_calc_mul_ph_ff::make(alpha);
    float p_acc_f[1] = { 1 };
    float* p_acc = p_acc_f;
    float q_acc_f[1] = { 1 };
    float* q_acc = q_acc_f;
    float s_out_acc_f[1] = { 0 };
    float* s_out_acc = s_out_acc_f;
    calc_block->calc_acc_val_apparent_power(s_out_acc, p_acc, q_acc, 1);
    BOOST_CHECK(isSimilar(s_out_acc[0], sqrt(2), 10));
}
BOOST_AUTO_TEST_SUITE_END();
} /* namespace pulsed_power */
} /* namespace gr */
