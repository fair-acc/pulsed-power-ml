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
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

using opencmw::majordomo::Broker;
using opencmw::majordomo::BrokerMessage;
using opencmw::majordomo::Command;
using opencmw::majordomo::MdpMessage;
using opencmw::majordomo::MessageFrame;
using opencmw::majordomo::Settings;
using opencmw::majordomo::Worker;

using opencmw::majordomo::DEFAULT_REST_PORT;
using opencmw::majordomo::PLAIN_HTTP;

TEST_CASE("integrator-constructor-not-exists-1", "[constuctor][file does not exist]") {
    size_t                          size = 7;
    PowerIntegrator                 powerIntegrator(size, "../../test/data/test_no_init_file/");

    std::vector<float>              usage_day       = powerIntegrator.get_power_usages_day();
    std::vector<float>              usage_week      = powerIntegrator.get_power_usages_week();
    std::vector<float>              usage_month     = powerIntegrator.get_power_usages_month();

    std::vector<int64_t>            timestamps      = powerIntegrator.get_timestamps();
    std::vector<int64_t>            timestamps_week = powerIntegrator.get_timestamps_week();

    std::vector<std::vector<float>> values_day      = powerIntegrator.get_devices_values();
    std::vector<std::vector<float>> values_week     = powerIntegrator.get_devices_values_last_week();

    REQUIRE(usage_day.size() == size);
    REQUIRE(usage_week.size() == size);
    REQUIRE(usage_month.size() == size);
    REQUIRE(timestamps.size() == 1);
    REQUIRE(timestamps_week.size() == 0);

    for (size_t i = 0; i < size; i++) {
        REQUIRE(usage_month.at(i) == 0);
        REQUIRE(usage_day.at(i) == 0);
        REQUIRE(usage_week.at(i) == 0);
        REQUIRE(values_day.at(i).size() == 1);
        REQUIRE(values_week.at(i).size() == 0);
    }
}

TEST_CASE("integrator-calculate-same-value-2", "[calculate][same values") {
    size_t          size = 7;
    PowerIntegrator powerIntegrator(size, "../../test/data/test_no_init_file/");

    double          power_usage = powerIntegrator.calculate_usage(1673858501452341457, 50, 1673858501460000000, 50);
    fmt::print("integrator-calculate-same-value power\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-calculate-old-greater-value-3", "[calculate][old greater values") {
    size_t          size = 7;
    PowerIntegrator powerIntegrator(size, "../../test/data/test_no_init_file/");
    double          power_usage = powerIntegrator.calculate_usage(1673858501452341457, 50,
                     1673858501460000000, 60);
    fmt::print("integrator-calculate-old-greater-value\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-calculate-old-less-value-4", "[calculate][less values") {
    size_t          size = 7;
    PowerIntegrator powerIntegrator(size, "../../test/data/test_no_init_file/");
    double          power_usage = powerIntegrator.calculate_usage(673858501452341457, 50,
                     1673858501460000000, 10);
    fmt::print("integrator-calculate-old-less-value\n", power_usage);
    REQUIRE(power_usage > 0);
}

TEST_CASE("integrator-update-test-add-values-5", "[update][inital values does not  exists]") {
    size_t             size = 7;
    PowerIntegrator    powerIntegrator(size, "../../test/data/test_update_values/");
    std::vector<float> values      = { 1.2, 0.4, 8.3, 30.0, 1.2, 0.4, 27.0 };
    int64_t            nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    powerIntegrator.update(nanoseconds, values);

    fmt::print("Size: {}\n", powerIntegrator.get_devices_values().size());
    fmt::print("Amount of measerement: {}\n", powerIntegrator.get_devices_values().at(0).size());

    nanoseconds += 1000000;
    powerIntegrator.update(nanoseconds, values);

    std::vector<float>              usage_day       = powerIntegrator.get_power_usages_day();
    std::vector<float>              usage_week      = powerIntegrator.get_power_usages_week();
    std::vector<float>              usage_month     = powerIntegrator.get_power_usages_month();

    std::vector<int64_t>            timestamps      = powerIntegrator.get_timestamps();
    std::vector<int64_t>            timestamps_week = powerIntegrator.get_timestamps_week();

    std::vector<std::vector<float>> values_day      = powerIntegrator.get_devices_values();
    std::vector<std::vector<float>> values_week     = powerIntegrator.get_devices_values_last_week();

    REQUIRE(usage_day.size() == size);
    REQUIRE(usage_week.size() == size);
    REQUIRE(usage_month.size() == size);
    REQUIRE(timestamps.size() == 3);
    REQUIRE(timestamps_week.size() == 0);

    for (size_t i = 0; i < size; i++) {
        REQUIRE(usage_month.at(i) > 0);
        REQUIRE(usage_day.at(i) > 0);
        REQUIRE(usage_week.at(i) > 0);
        REQUIRE(values_day.at(i).size() == timestamps.size());
        REQUIRE(values_week.at(i).size() == timestamps_week.size());
    }
}

TEST_CASE("integrator-save-data-6") {
    std::string        datapath = "../../test/data/test_save_data/";
    PowerIntegrator    powerIntegrator(11, datapath, 20);

    int64_t            time_point = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    std::vector<float> values     = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(time_point, values);

    int64_t time_interval = 500000000;
    for (size_t i = 0; i < 3000; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }

    // check files existence

    std::string month_file  = datapath + "month_usage.txt";
    std::string values_file = datapath + "time_values.txt";
    REQUIRE(std::filesystem::exists(month_file));
    std::remove(month_file.c_str()); // clean for next test
    REQUIRE(std::filesystem::exists(values_file));
    std::remove(values_file.c_str()); // clean for next test
}

//   std::this_thread::sleep_for(willSleepFor);

TEST_CASE("integrator-save_read-data-7") {
    std::string        datapath = "../../test/data/test_save_read_data/";
    PowerIntegrator    powerIntegrator(11, datapath);
    int64_t            time_point = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    std::vector<float> values     = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(time_point, values);

    for (size_t i = 0; i < 200; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        time_point = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        powerIntegrator.update(time_point, values);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(6000));

    PowerIntegrator      powerIntegratorReader(11, datapath, 20);

    std::vector<int64_t> timestamps = powerIntegrator.get_timestamps();
    // std::vector<int64_t> timestamps_week = powerIntegrator.get_timestamps_week();

    std::vector<int64_t> timestamps_reader = powerIntegrator.get_timestamps();
    // std::vector<int64_t> timestamps_week_reader = powerIntegrator.get_timestamps_week();

    fmt::print("timestamps saved first: {}\n", timestamps.front());
    fmt::print("timestamps readed first: {}\n", timestamps_reader.front());

    fmt::print("timestamps size saved: {}\n", timestamps.size());
    fmt::print("timestamps size readed : {}\n", timestamps_reader.size());

    std::string month_file  = datapath + "month_usage.txt";
    std::string values_file = datapath + "time_values.txt";
    REQUIRE(std::filesystem::exists(month_file));
    std::remove(month_file.c_str()); // clean for next test
    REQUIRE(std::filesystem::exists(values_file));
    std::remove(values_file.c_str()); // clean for next test
}

//  stress test
/*
TEST_CASE("update-values-n-times") {
     // change if needed
    size_t             n        = 300000;
    std::string        datapath = "../../test/data/test_n_times";
    PowerIntegrator    powerIntegrator(11, datapath, 1000);

    int64_t            time_point = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    std::vector<float> values     = { 8.3, 1.2, 0.4, 0.0, 1.2, 0.4, 8.3, 1.2, 0.0, 1.2, 0.4 };
    powerIntegrator.update(time_point, values);
    // change if needed
    int64_t time_interval = 500000;

    for (size_t i = 0; i < n; i++) {
        auto value = values.front();
        values.erase(values.begin());
        values.push_back(value);
        time_point += time_interval;
        powerIntegrator.update(time_point, values);
    }
}*/
