#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>
#include <majordomo/Settings.hpp>
#include <majordomo/Worker.hpp>

#include <IoSerialiserJson.hpp>
#include <MIME.hpp>
#include <opencmw.hpp>

#include <catch2/catch.hpp>
#include <cstdint>
#include <fmt/format.h>
#include <refl.hpp>

#include <exception>

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/top_block.h>

#include "helpers.hpp"
#include "integrator/PowerIntegrator.hpp"

using opencmw::majordomo::Broker;
using opencmw::majordomo::BrokerMessage;
using opencmw::majordomo::Command;
using opencmw::majordomo::MdpMessage;
using opencmw::majordomo::MessageFrame;
using opencmw::majordomo::Settings;
using opencmw::majordomo::Worker;

using opencmw::majordomo::DEFAULT_REST_PORT;
using opencmw::majordomo::PLAIN_HTTP;

// 2023-01-19 - new Implementation
// TEST_CASE("integrator-constuctor-exist", "[constuctor][file exist]") {
//     PowerIntegrator powerIntegrator("../../test/data/", 3);
//     fmt::print("\tcurrent path {0}\n", std::filesystem::current_path());
//     REQUIRE(powerIntegrator.get_init() == true);
//     REQUIRE(powerIntegrator.get_power_usages_month().size() == 3);
//     REQUIRE(powerIntegrator.get_power_usages_week().size() == 3);
//     REQUIRE(powerIntegrator.get_power_usages_day().size() == 3);
//     // check values
//     // check time
// }

TEST_CASE("integrator-constructor-not-exists", "[constuctor][file does not exist]") {
    PowerIntegrator powerIntegrator("./", 3);
    REQUIRE(powerIntegrator.get_init() == false);
}

TEST_CASE("integrator-calculate-same-value", "[calculate][same values") {
    PowerIntegrator powerIntegrator("../../test/data/", 3);

    double          power_usage = powerIntegrator.calculate_current_usage(1673858501452341457, 50, 1673858501460000000, 50);
    fmt::print("integrator-calculate-same-value power\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-calculate-old-greater-value", "[calculate][old greater values") {
    PowerIntegrator powerIntegrator("../../test/data/", 3);
    double          power_usage = powerIntegrator.calculate_current_usage(673858501452341457, 50,
                     1673858501460000000, 60);
    fmt::print("integrator-calculate-old-greater-value\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-calculate-old-less-value", "[calculate][less values") {
    PowerIntegrator powerIntegrator("../../test/data/", 3);
    double          power_usage = powerIntegrator.calculate_current_usage(673858501452341457, 50,
                     1673858501460000000, 10);
    fmt::print("integrator-calculate-old-less-value\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-update-test-add-values", "[update][inital values exists]") {
    PowerIntegrator    powerIntegrator("./", 3);
    std::vector<float> values = { 1.2, 0.4, 8.3 };
    powerIntegrator.update(12342144, values);
    fmt::print("Size: {}\n", powerIntegrator.get_devices_values().size());
    // no power yet
    REQUIRE(powerIntegrator.get_devices_values().at(0).size() == 1);
}
/*
TEST_CASE("update-values-3_cases-times") {
    PowerIntegrator    powerIntegrator("./", 11);
    std::vector<float> values = { 1.2, 0.4, 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0 };
    powerIntegrator.update(1674044238450644654, values);

    auto power_values_day = powerIntegrator.get_power_usages_day();
    auto device_values    = powerIntegrator.get_devices_values();

    REQUIRE(power_values_day.size() == 0); // not initated
    REQUIRE(device_values.at(0).size() == 1);
    fmt::print("length: {}\n", power_values_day.size());

    values = { 0.4, 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2 };
    powerIntegrator.update(1674044238450647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 2);

    values = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(1674044238459647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 3);
}
*/
/*
TEST_CASE("update-values-200000-times") {
    PowerIntegrator    powerIntegrator("./", 11);
    std::vector<float> values = { 1.2, 0.4, 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0 };
    powerIntegrator.update(1674044238450644654, values);

    auto power_values_day = powerIntegrator.get_power_usages_day();
    auto device_values    = powerIntegrator.get_devices_values();

    REQUIRE(power_values_day.size() == 0); // not initated
    REQUIRE(device_values.at(0).size() == 1);
    fmt::print("length: {}\n", power_values_day.size());

    values = { 0.4, 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2 };
    powerIntegrator.update(1674044238450647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 2);

    values = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(1674044238459647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 3);

    int64_t time_interval = 500000000;
    int64_t time_point    = 1674044238469647654;
    for (size_t i = 0; i < 200000; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }
}
*/
/*
TEST_CASE("update-values-300000-times") {
    PowerIntegrator    powerIntegrator("./", 11);
    std::vector<float> values = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    powerIntegrator.update(1674044238450644654, values);

    auto power_values_day = powerIntegrator.get_power_usages_day();
    auto device_values    = powerIntegrator.get_devices_values();

    REQUIRE(power_values_day.size() == 0); // not initated
    REQUIRE(device_values.at(0).size() == 1);
    fmt::print("length: {}\n", power_values_day.size());

    powerIntegrator.update(1674044238450647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 2);

    powerIntegrator.update(1674044238459647654, values);

    power_values_day = powerIntegrator.get_power_usages_day();
    device_values    = powerIntegrator.get_devices_values();
    REQUIRE(power_values_day.size() == 11);
    REQUIRE(device_values.at(0).size() == 3);

    int64_t time_interval = 500000000;
    int64_t time_point    = 1674044238469647654;
    for (size_t i = 0; i < 300000; i++) {
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }

    fmt::print("day usage size: {}", powerIntegrator.get_devices_values().at(0).size());
    fmt::print("week usage size: {}", powerIntegrator.get_devices_values_last_week().at(0).size());
}
*/
/*
TEST_CASE("integrator-change-month-times") {
    PowerIntegrator    powerIntegrator("./", 11);
    std::vector<float> values = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(1643607514469647654, values);

    int64_t time_interval = 500000000;
    int64_t time_point    = 1643607514469647654;
    for (size_t i = 0; i < 200000; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }

    fmt::print("power sage month: {}\n", powerIntegrator.get_power_usages_month());
}
*/
TEST_CASE("integrator-save-data") {
    PowerIntegrator powerIntegrator("../../test/data/", 11, 20);
    // PowerIntegrator    powerIntegrator("data/", 11, 20);
    std::vector<float> values = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(1643607514469647654, values);

    int64_t time_interval = 500000000;
    int64_t time_point    = 1643607514469647654;
    for (size_t i = 0; i < 3000; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }
}

// TEST_CASE("integrator-read_month") {
//     std::string path = "../../test/data/month_usage.txt";
//     read_month_usage(path);
// }

/*
TEST_CASE("integrator", "[update][inital values exists]") {
}


TEST_CASE("integrator", "[update][inital does not exists values exists]") {
}

TEST_CASE("integrator", "[write_to_disk][inital does not exists values exists]") {
}

TEST_CASE("integrator", "[calculate_current_usage][same values]") {
}

TEST_CASE("integrator", "[calculate_current_usage][fist less]") {
}

TEST_CASE("integrator", "[calculate_current_usage][fist greater]") {
}

TEST_CASE("integrator", "[update][new day]") {
}

TEST_CASE("integrator", "[update][new week]") {
}

TEST_CASE("Integrator", "[update][new month]") {
} */
