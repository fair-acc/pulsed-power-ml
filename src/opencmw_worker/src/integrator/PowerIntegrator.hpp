#ifndef POWER_INTEGRATOR_H
#define POWER_INTEGRATOR_H

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <time.h>
#include <vector>

#ifndef POWER_USAGE_PATH
#define POWER_USAGE_PATH "src/data/"
#endif

class PowerIntegrator {
private:
    size_t                          _amount_of_devices;
    const std::vector<std::string>  _devices;
    const int                       _save_interval;
    const std::string               _data_path;

    int64_t                         last_timestamp;

    std::vector<int64_t>            _timestamps;
    std::vector<int64_t>            _timestamps_week;

    std::vector<std::vector<float>> _devices_values;
    std::vector<std::vector<float>> _devices_values_last_week;
    std::vector<std::vector<float>> _devices_values_current_month;

    int                             _month[2]; // 0 - month, 1 year

    // current cumulate power usage of devices
    std::vector<float> power_usages_month;
    std::vector<float> power_usages_week;
    std::vector<float> power_usages_day;
    std::vector<float> power_usages_calendar_day;

    bool               init         = false;
    bool               _init_usage  = false;
    bool               _init_week   = false;
    bool               _init_month  = false;
    int                _counter     = 0;

    const float        _unit_faktor = 0.000000000000001f / 3.6f;

    // power values
    std::vector<float> power_values;

    // bool               read_values_from_file(std::string file_name, std::vector<float> &values, int64_t &time);
    bool              read_values_from_file(std::string file_name, int64_t &time);
    void              save_values_to_file(std::string file_name, int64_t time);
    void              save_values_to_file();
    bool              read_values_from_file();
    void              save_month_usage();
    bool              read_month_usage();

    void              check_last_powers_usages();

    const std::string file_month  = "month_usage.txt";
    const std::string file_week   = "week_usage.txt";
    const std::string file_day    = "day_usage.txt";
    const std::string file_values = "time_values.txt";

    bool              check_same_day(int64_t t_0, int64_t t_1);
    bool              check_same_week(int64_t t_0, int64_t t_1);
    bool              check_same_month(int64_t t_0, int64_t t_1);
    bool              check_same_calendar_day(int64_t t_0, int64_t t_1);
    bool              check_month(int64_t timestamp);

    // void              init_month(int64_t timestamp);
    void reset_month(int64_t timestamp);

public:
    void                                  update(int64_t timestamp, std::vector<float> &values);
    const std::vector<float>              get_power_usages_month(); // not implemented
    const std::vector<float>              get_power_usages_week();  // not implemented
    const std::vector<float>              get_power_usages_day();   // not implemented
    const std::vector<float>              get_power_values();
    bool                                  get_init();
    const std::vector<std::vector<float>> get_devices_values();
    const std::vector<std::vector<float>> get_devices_values_last_week();

    float                                 calculate_current_usage(int64_t t_0, float last_value,
                                            int64_t t_1, float current_value);

    PowerIntegrator(std::vector<std::string> &devices, const int save_interval = 100);
    PowerIntegrator(const std::string data_path, const size_t amount_of_devices, const int save_interval = 1000000000);
    ~PowerIntegrator();
};

PowerIntegrator::PowerIntegrator(std::vector<std::string> &devices, const int save_interval)
    : _devices(devices), _save_interval(save_interval), _data_path(POWER_USAGE_PATH), _amount_of_devices(devices.size()) {
    fmt::print("PowerIntegrator constructor\n");
    fmt::print("\tcurrent path {0}\n", std::filesystem::current_path());

    std::vector<float> temp_values;
    int64_t            time;

    // bool               month_bool = read_values_from_file("month_usage.txt", time);
    // fmt::print("\ttime {0}  \n", time);
    // fmt::print("\tvalues {0}  \n", power_usages_month);

    // _amount_of_devices = _devices.size();

    last_timestamp = time;

    bool week_bool = read_values_from_file("week_usage.txt", time);

    bool day_bool  = read_values_from_file("day_usage.txt", time);

    // read month

    bool month_bool = read_month_usage();

    init            = month_bool && week_bool && day_bool;

    fmt::print("\t init {0}  \n", init);

    // save_values_to_file("day_usage.txt",power_usages_month, time);
}

PowerIntegrator::PowerIntegrator(const std::string data_path, const size_t amount_of_devices, const int save_interval)
    : _amount_of_devices(amount_of_devices), _data_path(data_path), _save_interval(save_interval) {
    int64_t                                                  time;

    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    fmt::print("\tcurrent path {0}\n", std::filesystem::current_path());

    //  bool month_bool = read_values_from_file("month_usage.txt", time);
    // fmt::print("\ttime {0}  \n", time);
    // fmt::print("\tvalues {0}  \n", power_usages_month);

    last_timestamp  = time;

    bool week_bool  = read_values_from_file("week_usage.txt", time);

    bool day_bool   = read_values_from_file("day_usage.txt", time);
    bool month_bool = read_month_usage();

    init            = month_bool && week_bool && day_bool;
}

PowerIntegrator::~PowerIntegrator() {
}

void PowerIntegrator::update(int64_t timestamp, std::vector<float> &values) {
    /**
     * check init - if values are initialised
     * - yes
     *    comupute the usage
     *    - usage init
     *      - yes
     *        - update the list with power usage (new 3 new private methods)
     *          - check same day
     *          - check same week
     *          - check same month
     *      - no
     *         initialize lists with usage
     * -no
     *    initialize the list with values
     *
     * check counter - save in file or not
     */

    if (timestamp == 0) return;

    // fmt::print("Ingreator update timestamo {}\n", timestamp);

    if (init) {
        if (last_timestamp == timestamp) return;

        if (!check_month(timestamp)) {
            fmt::print("change date, old date: {}\n", _month);
            fmt::print("old power_values_month, : {}\n", power_usages_month);
            fmt::print("counter: {}\n", _counter);
            reset_month(timestamp);
        }

        for (size_t i = 0; i < _amount_of_devices; i++) {
            auto last_power = _devices_values.at(i).back();
            // auto  lastSegment        = devices_time_powers.at(i).back();

            float last_segment_usage = calculate_current_usage(last_timestamp, last_power,
                    timestamp, values[i]);

            // fmt::print("after calculate current usage\n");

            if (_init_usage) {
                power_usages_day.at(i) += last_segment_usage;
                power_usages_week.at(i) += last_segment_usage;
            } else {
                power_usages_day.push_back(last_segment_usage);
                power_usages_week.push_back(last_segment_usage);
            }

            power_usages_month.at(i) += last_segment_usage;
        }
        _init_usage = true;

        for (size_t i = 0; i < values.size(); i++) {
            _devices_values.at(i).push_back(values[i]);
        }

        while (!check_same_day(_timestamps.front(), timestamp)) {
            // fmt::print("Check day, week, momth not implemented!\n");
            //  check week
            //  Change value
            auto headTimestamp = _timestamps.front();
            _timestamps.erase(_timestamps.begin());
            for (size_t i = 0; i < _amount_of_devices; i++) {
                auto value = _devices_values.at(i).front();
                _devices_values.at(i).erase(_devices_values.at(i).begin());

                if (_init_week) {
                    _devices_values_last_week.at(i).push_back(value);
                } else {
                    std::vector<float> week_values;
                    week_values.push_back(value);
                    _devices_values_last_week.push_back(week_values);
                }

                // update tag
                //  berechnen values
                //  substrhieren values
                float old_usage = calculate_current_usage(headTimestamp, value, _timestamps.front(), _devices_values.at(i).front());
                power_usages_day.at(i) -= old_usage;
            }

            _init_week = true;
            _timestamps_week.push_back(headTimestamp);
        }

        while (_init_week && !check_same_week(_timestamps_week.front(), timestamp)) {
            auto headWeekTimestamp = _timestamps_week.front();
            _timestamps_week.erase(_timestamps_week.begin());
            auto firstValues = _devices_values_last_week.front();
            for (size_t i = 0; i < _amount_of_devices; i++) {
                auto removedValue = _devices_values_last_week.at(i).front();
                _devices_values_last_week.at(i).erase(_devices_values_last_week.at(i).begin());

                auto powerValue = calculate_current_usage(headWeekTimestamp, removedValue,
                        _timestamps_week.front(), _devices_values_last_week.at(i).front());

                power_usages_week.at(i) -= powerValue;
            }
        }

    } else {
        init = true;

        for (size_t i = 0; i < values.size(); i++) {
            std::vector<float> dev_values;
            dev_values.push_back(values[i]);
            _devices_values.push_back(dev_values);
        }

        reset_month(timestamp);
    }

    last_timestamp = timestamp;
    _timestamps.push_back(timestamp);

    // fmt::print("usages day at last timestamp: {},useges: {}\n", last_timestamp, power_usages_day);
    _counter++;

    // fmt::print("counter: {}\n", _counter);

    if (_counter == _save_interval) {
        save_month_usage();
        save_values_to_file();
        _counter = 0;
    }
}

void PowerIntegrator::check_last_powers_usages() {
}

float PowerIntegrator::calculate_current_usage(int64_t t_0, float last_value, int64_t t_1, float current_value) {
    int64_t delta_time_int = t_1 - t_0;
    float   delta_time     = static_cast<float>(delta_time_int);
    float   delta_values   = current_value - last_value;

    if (last_value == current_value) {
        return current_value * delta_time * _unit_faktor;
    } else if (last_value < current_value) {
        float temp = last_value * delta_time * _unit_faktor;
        return temp + 0.5f * delta_values * delta_time * _unit_faktor;
    } else {
        float temp = current_value * delta_time;
        return temp + 0.5f * (-delta_values) * delta_time;
    }
}

bool PowerIntegrator::read_values_from_file(std::string file_name, int64_t &time) {
    std::string file_path = _data_path + file_name;
    // fmt::print("read_values_from_file, data_path: {0}\n", file_path);

    std::ifstream values_file(file_path.c_str(), std::ios_base::in);

    if (values_file.good()) {
        //  fmt::print("\tFile {0}  exists\n", file_name.c_str());
        // int64_t time;
        float value;
        values_file >> time;
        while (!values_file.eof()) {
            values_file >> value;
        }
        values_file.close();

        // to implement
        return false;
    } else {
        //  fmt::print("\tFile {0} does not exists\n", file_name.c_str());
        return false;
    }
    return false;
}

bool PowerIntegrator read_values_from_file() {
}

bool PowerIntegrator::read_month_usage() {
    std::string   file_path = _data_path + file_month;
    std::ifstream values_file(file_path.c_str(), std::ios_base::in);

    if (values_file.good()) {
        int   month;
        int   year;
        float value;

        int   counter = 1;

        while (!values_file.eof()) {
            if (counter == 1) {
                values_file >> _month[0];
            } else if (counter == 2) {
                values_file >> _month[1];
            } else {
                values_file >> value;
                power_usages_month.push_back(value);
                fmt::print("value: {}\n", value);
            }

            counter++;
        }

        values_file.close();
        return true;
    } else {
        return false;
    }
    return false;
}

void PowerIntegrator::save_values_to_file(std::string file_name, int64_t time) {
    std::string   file_path = _data_path + file_name;
    std::ofstream output_file;

    output_file.open(file_path);
    output_file << last_timestamp;

    // To implement
    // for (auto value : values) {
    //     output_file << "\n";
    //     output_file << value;
    // }

    output_file.close();
}

void PowerIntegrator::save_values_to_file() {
    std::string   file_path = _data_path + file_values;

    std::ofstream output_file;

    output_file.open(file_path);
    output_file << last_timestamp << std::endl;

    // optimalization possibility - table with iterators

    // save values from last week
    for (size_t j = 0; j < _timestamps_week.size(); j++) {
        output_file << _timestamps_week.at(j) << " ";
        for (size_t i = 0; i < _amount_of_devices; i++) {
            output_file << _devices_values_last_week.at(i).at(j) << " ";
        }
        output_file << std::endl;
    }

    // save values from last 24 hours
    for (size_t j = 0; j < _timestamps.size(); j++) {
        output_file << _timestamps.at(j) << " ";
        for (size_t i = 0; i < _amount_of_devices; i++) {
            output_file << _devices_values.at(i).at(j) << " ";
        }
        output_file << std::endl;
    }

    output_file.close();
}

void PowerIntegrator::save_month_usage() {
    std::string   file_path = _data_path + file_month;

    std::ofstream output_file;
    output_file.open(file_path);
    output_file << _month[0] << " " << _month[1] << std::endl;
    for (auto v : power_usages_month) {
        output_file << v << std::endl;
    }
    output_file.close();
}

bool PowerIntegrator::get_init() {
    return init;
}

const std::vector<float> PowerIntegrator::get_power_usages_month() {
    return power_usages_month;
}

const std::vector<float> PowerIntegrator::get_power_usages_week() {
    return power_usages_week;
}

const std::vector<float> PowerIntegrator::get_power_usages_day() {
    return power_usages_day;
}

const std::vector<std::vector<float>> PowerIntegrator::get_devices_values() {
    return _devices_values;
}

const std::vector<std::vector<float>> PowerIntegrator::get_devices_values_last_week() {
    return _devices_values_last_week;
}

bool PowerIntegrator::check_same_day(int64_t t_0, int64_t t_1) {
    int64_t temp = t_1 - 86400000000000;
    return temp <= t_0;
}
bool PowerIntegrator::check_same_week(int64_t t_0, int64_t t_1) {
    int64_t temp = t_1 - 604800000000000;
    return temp <= t_0;
}

bool PowerIntegrator::check_same_calendar_day(int64_t t_0, int64_t t_1) {
    std::chrono::system_clock::time_point t_point_0 = std::chrono::system_clock::time_point(std::chrono::nanoseconds(t_0));

    std::time_t                           time_0    = std::chrono::system_clock::to_time_t(t_point_0);

    std::tm                              *tm_0      = std::localtime(&time_0);

    std::chrono::system_clock::time_point t_point_1 = std::chrono::system_clock::time_point(std::chrono::nanoseconds(t_1));

    std::time_t                           time_1    = std::chrono::system_clock::to_time_t(t_point_1);

    std::tm                              *tm_1      = std::localtime(&time_1);

    std::cout << "Day: " << tm_1->tm_mday << std::endl;
    std::cout << "Month: " << tm_1->tm_mon + 1 << std::endl;
    std::cout << "Year: " << tm_1->tm_year + 1900 << std::endl;
    std::cout << "Hour: " << tm_1->tm_hour << std::endl;
    std::cout << "Minutes: " << tm_1->tm_min << std::endl;
    std::cout << "Sec: " << tm_1->tm_sec << std::endl;

    return tm_0->tm_mday == tm_1->tm_mday;
}

bool PowerIntegrator::check_same_month(int64_t t_0, int64_t t_1) {
    std::chrono::system_clock::time_point t_point_0 = std::chrono::system_clock::time_point(std::chrono::nanoseconds(t_0));

    std::time_t                           time_0    = std::chrono::system_clock::to_time_t(t_point_0);

    std::tm                              *tm_0      = std::localtime(&time_0);

    std::chrono::system_clock::time_point t_point_1 = std::chrono::system_clock::time_point(std::chrono::nanoseconds(t_1));

    std::time_t                           time_1    = std::chrono::system_clock::to_time_t(t_point_1);

    std::tm                              *tm_1      = std::localtime(&time_1);

    return tm_0->tm_mon == tm_1->tm_mon;
}

void PowerIntegrator::reset_month(int64_t timestamp) {
    std::chrono::system_clock::time_point t_point = std::chrono::system_clock::time_point(std::chrono::nanoseconds(timestamp));

    std::time_t                           time    = std::chrono::system_clock::to_time_t(t_point);

    std::tm                              *tm      = std::localtime(&time);

    _month[0]                                     = tm->tm_mon + 1;
    _month[1]                                     = tm->tm_year + 1900;

    power_usages_month.clear();
    for (size_t i = 0; i < _amount_of_devices; i++) {
        power_usages_month.push_back(0.0f);
    }
    _init_month = true;
}

bool PowerIntegrator::check_month(int64_t timestamp) {
    std::chrono::system_clock::time_point t_point = std::chrono::system_clock::time_point(std::chrono::nanoseconds(timestamp));

    std::time_t                           time    = std::chrono::system_clock::to_time_t(t_point);

    std::tm                              *tm      = std::localtime(&time);

    return (tm->tm_mon + 1 == _month[0]) && (tm->tm_year + 1900 == _month[1]);
}

#endif /* POWER_INTEGRATOR_H */