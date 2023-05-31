#include <catch2/catch.hpp>
#include <cstdint>
#include <fmt/format.h>
#include <refl.hpp>

#include "integrator/PowerIntegrator.hpp"
#include <filesystem>
#include <thread>

TEST_CASE("integrator-constructor-file-not-exists", "[constuctor][file does not exist]") {
    size_t             size     = 7;
    std::string        datapath = "../../test/data/test_save_data/";

    PowerIntegrator    powerIntegrator(size, datapath.c_str());

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == size);
    REQUIRE(usage_week.size() == size);
    REQUIRE(usage_month.size() == size);

    for (size_t i = 0; i < size; i++) {
        REQUIRE(usage_month.at(i) < 1e-6f);
        REQUIRE(usage_day.at(i) < 1e-6f);
        REQUIRE(usage_week.at(i) < 1e-6f);
    }

    std::string file = datapath + "nilm_usage.txt";
    REQUIRE(std::filesystem::exists(file));
    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-update-test-add-values", "[update][inital values does not  exists]") {
    size_t             size     = 7;
    std::string        datapath = "../../test/data/test_save_data/";

    PowerIntegrator    powerIntegrator(size, datapath.c_str());
    std::vector<float> values      = { 100.0, 400.9, 8300.6, 300.5, 1200.7, 400.3, 2700.0 };
    int64_t            nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    powerIntegrator.update(nanoseconds, values);

    nanoseconds += 1000000000; // 1sec
    powerIntegrator.update(nanoseconds, values);

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == size);
    REQUIRE(usage_week.size() == size);
    REQUIRE(usage_month.size() == size);

    REQUIRE(usage_day.at(0) - 2.78e-5f < 1e-6f);
    REQUIRE(usage_day.at(1) - 11.14e-5f < 1e-6f);
    REQUIRE(usage_day.at(2) - 230.57e-5f < 1e-6f);
    REQUIRE(usage_day.at(3) - 8.35e-5f < 1e-6f);
    REQUIRE(usage_day.at(4) - 33.35e-5f < 1e-6f);
    REQUIRE(usage_day.at(5) - 11.12e-5f < 1e-6f);
    REQUIRE(usage_day.at(6) - 75e-5f < 1e-6f);

    REQUIRE(usage_week.at(0) - 2.78e-5f < 1e-6f);
    REQUIRE(usage_week.at(1) - 11.14e-5f < 1e-6f);
    REQUIRE(usage_week.at(2) - 230.57e-5f < 1e-6f);
    REQUIRE(usage_week.at(3) - 8.35e-5f < 1e-6f);
    REQUIRE(usage_week.at(4) - 33.35e-5f < 1e-6f);
    REQUIRE(usage_week.at(5) - 11.12e-5f < 1e-6f);
    REQUIRE(usage_week.at(6) - 75e-5f < 1e-6f);

    REQUIRE(usage_month.at(0) - 2.78e-5f < 1e-6f);
    REQUIRE(usage_month.at(1) - 11.14e-5f < 1e-6f);
    REQUIRE(usage_month.at(2) - 230.57e-5f < 1e-6f);
    REQUIRE(usage_month.at(3) - 8.35e-5f < 1e-6f);
    REQUIRE(usage_month.at(4) - 33.35e-5f < 1e-6f);
    REQUIRE(usage_month.at(5) - 11.12e-5f < 1e-6f);
    REQUIRE(usage_month.at(6) - 75e-5f < 1e-6f);

    std::string file = datapath + "nilm_usage.txt";
    REQUIRE(std::filesystem::exists(file));
    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-read-data") {
    std::string   datapath = "../../test/data/test_save_data/";
    std::string   file     = datapath + "nilm_usage.txt";
    int64_t       now      = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream testfile;
    testfile.open(file, std::ofstream::out | std::ofstream::trunc);
    testfile << now << '\n';
    testfile << now << '\n';
    testfile << now << '\n';
    testfile << now << '\n';
    testfile << 1.1 << '\n';
    testfile << 2.2 << '\n';
    testfile << 3.3 << '\n';
    testfile << 4.4 << '\n';
    testfile << 5.5 << '\n';
    testfile << 6.6 << '\n';
    testfile << 7.7 << '\n';
    testfile << 8.8 << '\n';
    testfile << 9.9 << '\n';
    testfile << 10.10 << '\n';
    testfile << 11.11 << '\n';
    testfile << 12.12 << '\n';
    testfile.close();
    PowerIntegrator    powerIntegrator(4, datapath);

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == 4);
    REQUIRE(usage_week.size() == 4);
    REQUIRE(usage_month.size() == 4);

    REQUIRE(usage_day.at(0) - 1.1f < 1e-6f);
    REQUIRE(usage_day.at(1) - 2.2f < 1e-6f);
    REQUIRE(usage_day.at(2) - 3.3f < 1e-6f);
    REQUIRE(usage_day.at(3) - 4.4f < 1e-6f);
    REQUIRE(usage_week.at(0) - 5.5f < 1e-6f);
    REQUIRE(usage_week.at(1) - 6.6f < 1e-6f);
    REQUIRE(usage_week.at(2) - 7.7f < 1e-6f);
    REQUIRE(usage_week.at(3) - 8.8f < 1e-6f);
    REQUIRE(usage_month.at(0) - 9.9f < 1e-6f);
    REQUIRE(usage_month.at(1) - 10.1f < 1e-6f);
    REQUIRE(usage_month.at(2) - 11.11f < 1e-6f);
    REQUIRE(usage_month.at(3) - 12.12f < 1e-6f);

    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-read-data-reset-day") {
    std::string   datapath = "../../test/data/test_save_data/";
    std::string   file     = datapath + "nilm_usage.txt";
    int64_t       now_25h  = std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() - std::chrono::hours(25)).time_since_epoch()).count();
    std::ofstream testfile;
    testfile.open(file, std::ofstream::out | std::ofstream::trunc);
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << 1.1 << '\n';
    testfile << 2.2 << '\n';
    testfile << 3.3 << '\n';
    testfile << 4.4 << '\n';
    testfile << 5.5 << '\n';
    testfile << 6.6 << '\n';
    testfile << 7.7 << '\n';
    testfile << 8.8 << '\n';
    testfile << 9.9 << '\n';
    testfile << 10.10 << '\n';
    testfile << 11.11 << '\n';
    testfile << 12.12 << '\n';
    testfile.close();
    PowerIntegrator powerIntegrator(4, datapath);
    std::this_thread::sleep_for(std::chrono::seconds(10)); // time to check for reset

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == 4);
    REQUIRE(usage_week.size() == 4);
    REQUIRE(usage_month.size() == 4);

    REQUIRE(usage_day.at(0) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(1) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(2) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(3) - 0.0f < 1e-6f);
    REQUIRE(usage_week.at(0) - 5.5f < 1e-6f);
    REQUIRE(usage_week.at(1) - 6.6f < 1e-6f);
    REQUIRE(usage_week.at(2) - 7.7f < 1e-6f);
    REQUIRE(usage_week.at(3) - 8.8f < 1e-6f);
    REQUIRE(usage_month.at(0) - 9.9f < 1e-6f);
    REQUIRE(usage_month.at(1) - 10.1f < 1e-6f);
    REQUIRE(usage_month.at(2) - 11.11f < 1e-6f);
    REQUIRE(usage_month.at(3) - 12.12f < 1e-6f);

    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-read-data-reset-week") {
    std::string   datapath = "../../test/data/test_save_data/";
    std::string   file     = datapath + "nilm_usage.txt";
    int64_t       now_25h  = std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() - std::chrono::hours(25 * 7)).time_since_epoch()).count();
    std::ofstream testfile;
    testfile.open(file, std::ofstream::out | std::ofstream::trunc);
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << 1.1 << '\n';
    testfile << 2.2 << '\n';
    testfile << 3.3 << '\n';
    testfile << 4.4 << '\n';
    testfile << 5.5 << '\n';
    testfile << 6.6 << '\n';
    testfile << 7.7 << '\n';
    testfile << 8.8 << '\n';
    testfile << 9.9 << '\n';
    testfile << 10.10 << '\n';
    testfile << 11.11 << '\n';
    testfile << 12.12 << '\n';
    testfile.close();
    PowerIntegrator powerIntegrator(4, datapath);
    std::this_thread::sleep_for(std::chrono::seconds(10)); // time to check for reset

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == 4);
    REQUIRE(usage_week.size() == 4);
    REQUIRE(usage_month.size() == 4);

    REQUIRE(usage_day.at(0) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(1) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(2) - 0.0f < 1e-6f);
    REQUIRE(usage_day.at(3) - 0.0f < 1e-6f);
    REQUIRE(usage_week.at(0) - 0.0f < 1e-6f);
    REQUIRE(usage_week.at(1) - 0.0f < 1e-6f);
    REQUIRE(usage_week.at(2) - 0.0f < 1e-6f);
    REQUIRE(usage_week.at(3) - 0.0f < 1e-6f);
    REQUIRE(usage_month.at(0) - 9.9f < 1e-6f);
    REQUIRE(usage_month.at(1) - 10.1f < 1e-6f);
    REQUIRE(usage_month.at(2) - 11.11f < 1e-6f);
    REQUIRE(usage_month.at(3) - 12.12f < 1e-6f);

    // TODO file einlesen?

    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-read-data-reset-month") {
    std::string   datapath = "../../test/data/test_save_data/";
    std::string   file     = datapath + "nilm_usage.txt";
    int64_t       now_25h  = std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() - std::chrono::hours(25 * 30)).time_since_epoch()).count();
    std::ofstream testfile;
    testfile.open(file, std::ofstream::out | std::ofstream::trunc);
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << now_25h << '\n';
    testfile << 1.1 << '\n';
    testfile << 2.2 << '\n';
    testfile << 3.3 << '\n';
    testfile << 4.4 << '\n';
    testfile << 5.5 << '\n';
    testfile << 6.6 << '\n';
    testfile << 7.7 << '\n';
    testfile << 8.8 << '\n';
    testfile << 9.9 << '\n';
    testfile << 10.10 << '\n';
    testfile << 11.11 << '\n';
    testfile << 12.12 << '\n';
    testfile.close();
    PowerIntegrator powerIntegrator(4, datapath);
    std::this_thread::sleep_for(std::chrono::seconds(10)); // time to check for reset

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == 4);
    REQUIRE(usage_week.size() == 4);
    REQUIRE(usage_month.size() == 4);

    for (size_t i = 0; i < 4; i++) {
        REQUIRE(usage_month.at(i) < 1e-6f);
        REQUIRE(usage_day.at(i) < 1e-6f);
        REQUIRE(usage_week.at(i) < 1e-6f);
    }

    std::remove(file.c_str()); // clean for next test
}

TEST_CASE("integrator-update-save-values", "[update][inital values does not  exists]") {
    size_t        size     = 4;
    std::string   datapath = "../../test/data/test_save_data/";
    std::string file = datapath + "nilm_usage.txt";

    int64_t       now_9min = std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() - std::chrono::seconds(590)).time_since_epoch()).count();
    std::ofstream testfile;
    testfile.open(file, std::ofstream::out | std::ofstream::trunc);
    testfile << now_9min << '\n';
    testfile << now_9min << '\n';
    testfile << now_9min << '\n';
    testfile << now_9min << '\n';
    testfile << 1.1 << '\n';
    testfile << 2.2 << '\n';
    testfile << 3.3 << '\n';
    testfile << 4.4 << '\n';
    testfile << 5.5 << '\n';
    testfile << 6.6 << '\n';
    testfile << 7.7 << '\n';
    testfile << 8.8 << '\n';
    testfile << 9.9 << '\n';
    testfile << 10.10 << '\n';
    testfile << 11.11 << '\n';
    testfile << 12.12 << '\n';
    testfile.close();

    PowerIntegrator    powerIntegrator(size, datapath.c_str());
    std::vector<float> values      = { 2700.0, 400.9, 8300.6, 300.5 };
    int64_t            nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    powerIntegrator.update(nanoseconds, values);

    nanoseconds += 1000000000; // 1sec
    powerIntegrator.update(nanoseconds, values);

    std::vector<float> usage_day   = powerIntegrator.getPowerUsagesDay();
    std::vector<float> usage_week  = powerIntegrator.getPowerUsagesWeek();
    std::vector<float> usage_month = powerIntegrator.getPowerUsagesMonth();

    REQUIRE(usage_day.size() == size);
    REQUIRE(usage_week.size() == size);
    REQUIRE(usage_month.size() == size);

    REQUIRE(usage_day.at(0) - (1.1f + 75e-5f) < 1e-6f);
    REQUIRE(usage_day.at(1) - (2.2f + 11.14e-5f) < 1e-6f);
    REQUIRE(usage_day.at(2) - (3.3f + 230.57e-5f) < 1e-6f);
    REQUIRE(usage_day.at(3) - (4.4f + 8.35e-5f) < 1e-6f);

    REQUIRE(usage_week.at(0) - (5.5f + 75e-5f) < 1e-6f);
    REQUIRE(usage_week.at(1) - (6.6f + 11.14e-5f) < 1e-6f);
    REQUIRE(usage_week.at(2) - (7.7f + 230.57e-5f) < 1e-6f);
    REQUIRE(usage_week.at(3) - (8.8f + 8.35e-5f) < 1e-6f);

    REQUIRE(usage_month.at(0) - (9.9f + 75e-5f) < 1e-6f);
    REQUIRE(usage_month.at(1) - (10.1f + 11.14e-5f) < 1e-6f);
    REQUIRE(usage_month.at(2) - (11.11f + 230.57e-5f) < 1e-6f);
    REQUIRE(usage_month.at(3) - (12.12f + 8.35e-5f) < 1e-6f);

    std::this_thread::sleep_for(std::chrono::seconds(15)); // wait until file is written
    REQUIRE(std::filesystem::exists(file));

    int64_t            lastResetDayTimestamp   = 0;
    int64_t            lastResetWeekTimestamp  = 0;
    int64_t            lastResetMonthTimestamp = 0;
    int64_t            lastSaveTimestamp       = 0;
    std::vector<float> read_usage_day;
    std::vector<float> read_usage_week;
    std::vector<float> read_usage_month;
    std::ifstream      readfile;
    readfile.open(file, std::ifstream::in);
    readfile >> lastResetDayTimestamp;
    readfile >> lastResetWeekTimestamp;
    readfile >> lastResetMonthTimestamp;
    readfile >> lastSaveTimestamp;

    float value = 0.0;
    for (size_t i = 0; i < size; i++) {
        readfile >> value;
        read_usage_day.push_back(value);
    }
    for (size_t i = 0; i < size; i++) {
        readfile >> value;
        read_usage_week.push_back(value);
    }
    for (size_t i = 0; i < size; i++) {
        readfile >> value;
        read_usage_month.push_back(value);
    }
    readfile.close();

    REQUIRE(lastResetDayTimestamp == now_9min);
    REQUIRE(lastResetWeekTimestamp == now_9min);
    REQUIRE(lastResetMonthTimestamp == now_9min);
    REQUIRE(lastSaveTimestamp - 5000000000 < 11000000000);

    REQUIRE(read_usage_day.at(0) - (1.1f + 75e-5f) < 1e-6f);
    REQUIRE(read_usage_day.at(1) - (2.2f + 11.14e-5f) < 1e-6f);
    REQUIRE(read_usage_day.at(2) - (3.3f + 230.57e-5f) < 1e-6f);
    REQUIRE(read_usage_day.at(3) - (4.4f + 8.35e-5f) < 1e-6f);

    REQUIRE(read_usage_week.at(0) - (5.5f + 75e-5f) < 1e-6f);
    REQUIRE(read_usage_week.at(1) - (6.6f + 11.14e-5f) < 1e-6f);
    REQUIRE(read_usage_week.at(2) - (7.7f + 230.57e-5f) < 1e-6f);
    REQUIRE(read_usage_week.at(3) - (8.8f + 8.35e-5f) < 1e-6f);

    REQUIRE(read_usage_month.at(0) - (9.9f + 75e-5f) < 1e-6f);
    REQUIRE(read_usage_month.at(1) - (10.1f + 11.14e-5f) < 1e-6f);
    REQUIRE(read_usage_month.at(2) - (11.11f + 230.57e-5f) < 1e-6f);
    REQUIRE(read_usage_month.at(3) - (12.12f + 8.35e-5f) < 1e-6f);

    std::remove(file.c_str()); // clean for next test
}
