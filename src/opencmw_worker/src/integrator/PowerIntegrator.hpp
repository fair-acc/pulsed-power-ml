#ifndef POWER_INTEGRATOR_H
#define POWER_INTEGRATOR_H

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>

class PowerIntegrator {
private:
    size_t                          _amount_of_devices;
    const std::vector<std::string>  _devices;
    const std::string               _data_path;
    const int                       _save_interval;

    int64_t                         last_timestamp;
    int64_t                         start_timestamp;

    std::vector<int64_t>            _timestamps;
    std::vector<int64_t>            _timestamps_week;

    std::vector<std::vector<float>> _devices_values;
    std::vector<std::vector<float>> _devices_values_last_week;

    int                             _month[2]; // 0 - month, 1 year

    // current cumulate power usage of devices
    std::vector<float> power_usages_month;
    std::vector<float> power_usages_week;
    std::vector<float> power_usages_day;

    bool               init         = false;
    bool               _init_usage  = false;
    bool               _init_day    = false;
    bool               _init_week   = false;
    bool               _init_month  = false;
    int                _counter     = 0;

    const float        _unit_faktor = 0.000000000000001f / 3.6f;

    void               save_values_to_file();
    bool               read_values_from_file();
    void               save_month_usage();
    bool               read_month_usage();

    const std::string  file_month  = "month_usage.txt";
    const std::string  file_values = "time_values.txt";

    bool               check_same_day(int64_t t_0, int64_t t_1);
    bool               check_same_week(int64_t t_0, int64_t t_1);
    bool               check_same_month(int64_t t_0, int64_t t_1);
    bool               check_same_calendar_day(int64_t t_0, int64_t t_1);
    bool               check_month(int64_t timestamp);

    void               reset_month(int64_t timestamp);

public:
    void                                  update(int64_t timestamp, std::vector<float> &values);
    const std::vector<float>              get_power_usages_month();
    const std::vector<float>              get_power_usages_week();
    const std::vector<float>              get_power_usages_day();
    bool                                  get_init();
    const std::vector<std::vector<float>> get_devices_values();
    const std::vector<std::vector<float>> get_devices_values_last_week();

    const std::vector<int64_t>            get_timestamps();
    const std::vector<int64_t>            get_timestamps_week();

    float                                 calculate_usage(int64_t t_0, float last_value,
                                            int64_t t_1, float current_value);

    PowerIntegrator(std::vector<std::string> &devices, const std::string data_path = "./", const int save_interval = 100);
    PowerIntegrator(const size_t amount_of_devices, const std::string data_path = "./", const int save_interval = 100);
    ~PowerIntegrator();
};

PowerIntegrator::PowerIntegrator(std::vector<std::string> &devices, const std::string data_path, const int save_interval)
    : PowerIntegrator(devices.size(), data_path, save_interval) {}

PowerIntegrator::PowerIntegrator(const size_t amount_of_devices, const std::string data_path, const int save_interval)
    : _amount_of_devices(amount_of_devices), _data_path(data_path), _save_interval(save_interval) {
    start_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    fmt::print("\tcurrent path {0}\n", std::filesystem::current_path());

    // init usage day, week, month
    for (size_t i = 0; i < _amount_of_devices; i++) {
        power_usages_day.push_back(0.0f);
        power_usages_week.push_back(0.0f);
        power_usages_month.push_back(0.0f);
    }

    // initialization of vectors with values
    for (size_t i = 0; i < _amount_of_devices; i++) {
        std::vector<float> values;
        _devices_values.push_back(values);
        std::vector<float> values_week;
        _devices_values_last_week.push_back(values_week);
    }

    fmt::print("Read month values\n");
    read_month_usage();
    fmt::print("Read week and day values\n");
    read_values_from_file();

    if (!_init_month) {
        reset_month(start_timestamp);
        _init_month = true;
    }

    if (!_init_day) {
        // add to time stamp to week and update value
        if (_init_week) {
            auto last_week_timestamp = _timestamps_week.back();
            for (size_t i = 0; i < _amount_of_devices; i++) {
                auto  last_week_value = _devices_values_last_week.at(i).back();

                float value           = calculate_usage(last_week_timestamp, last_week_value,
                                  last_week_timestamp + 100, 0.0f);
                _devices_values_last_week.at(i).push_back(0.0f);
                power_usages_week.at(i) += value;
            }
            _timestamps_week.push_back(last_week_timestamp + 100);
        }

        _timestamps.push_back(start_timestamp);
        for (size_t i = 0; i < _amount_of_devices; i++) {
            _devices_values.at(i).push_back(0.0f);
        }
        last_timestamp = start_timestamp;
    }
    init = _init_day;

    fmt::print("Size of timestamps: {}\n", _timestamps.size());
    fmt::print("Size of values: {}\n", _devices_values[0].size());

    fmt::print("---usages-------Usage in month: {}\n", power_usages_month);
    fmt::print("---usages-------Usage in week: {}\n", power_usages_week);
    fmt::print("---usages-------Usage in day: {}\n", power_usages_day);
}

PowerIntegrator::~PowerIntegrator() {
}

void PowerIntegrator::update(int64_t timestamp, std::vector<float> &values) {
    // fmt::print("Integrator: Update\n");
    if (timestamp == 0) return;

    if (last_timestamp == timestamp) return;

    if (!check_month(timestamp)) {
        fmt::print("change date, old date: {}\n", _month);
        fmt::print("old power_values_month, : {}\n", power_usages_month);
        fmt::print("counter: {}\n", _counter);
        reset_month(timestamp);
    }

    for (size_t i = 0; i < _amount_of_devices; i++) {
        auto  last_power         = _devices_values.at(i).back();

        float last_segment_usage = calculate_usage(last_timestamp, last_power,
                timestamp, values[i]);

        power_usages_day.at(i) += last_segment_usage;
        power_usages_week.at(i) += last_segment_usage;
        power_usages_month.at(i) += last_segment_usage;
    }

    for (size_t i = 0; i < values.size(); i++) {
        _devices_values.at(i).push_back(values[i]);
    }

    size_t timestamps_size = _timestamps.size();
    while (timestamps_size > 0 && !check_same_day(_timestamps.front(), timestamp)) {
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

            float old_usage = calculate_usage(headTimestamp, value, _timestamps.front(), _devices_values.at(i).front());
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

            auto powerValue = calculate_usage(headWeekTimestamp, removedValue,
                    _timestamps_week.front(), _devices_values_last_week.at(i).front());

            power_usages_week.at(i) -= powerValue;
        }
    }

    last_timestamp = timestamp;
    _timestamps.push_back(timestamp);

    _counter++;

    if (_counter == _save_interval) {
        fmt::print("Save data form Integrator\n");
        fmt::print("Save month data\n");
        save_month_usage();
        fmt::print("Save week and day data\n");
        save_values_to_file();
        _counter = 0;
        fmt::print("---usages-------Usage in month: {}\n", power_usages_month);
        fmt::print("---usages-------Usage in week: {}\n", power_usages_week);
        fmt::print("---usages-------Usage in day: {}\n", power_usages_day);
    }
}

float PowerIntegrator::calculate_usage(int64_t t_0, float last_value, int64_t t_1, float current_value) {
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
        return temp - 0.5f * delta_values * delta_time * _unit_faktor;
    }
}

bool PowerIntegrator::read_values_from_file() {
    std::string file_path = _data_path + file_values;
    // fmt::print("read_values_from_file, data_path: {0}\n", file_path);

    std::ifstream      values_file(file_path.c_str(), std::ios_base::in);

    std::vector<float> previous_values;
    int64_t            previous_timestamp;

    if (values_file.good()) {
        int64_t     time_saved;
        std::string line;

        // read save time
        if (std::getline(values_file, line)) {
            if (line.length() > 0) {
                std::istringstream iss(line);
                iss >> time_saved;
            } else {
                values_file.close();
                return false;
            }
        } else {
            values_file.close();
            return false;
        }

        // the data are to old
        if (!check_same_week(time_saved, start_timestamp)) {
            values_file.close();
            return false;
        }

        while (std::getline(values_file, line)) {
            if (line.length() > 0) {
                int64_t            timestamp;
                std::istringstream iss(line);
                iss >> timestamp;
                // same week
                if (check_same_week(timestamp, start_timestamp)) {
                    if (check_same_day(timestamp, start_timestamp)) {
                        _timestamps.push_back(timestamp);
                        for (size_t i = 0; i < _amount_of_devices; i++) {
                            float value;
                            iss >> value;
                            _devices_values.at(i).push_back(value);

                            if (previous_values.size() == _amount_of_devices) {
                                float usage = calculate_usage(previous_timestamp, previous_values.at(i),
                                        timestamp, value);
                                power_usages_day.at(i) += usage;
                                power_usages_week.at(i) += usage;
                                previous_values.at(i) = value;
                            } else {
                                previous_values.push_back(value);
                            }
                        }
                        previous_timestamp = timestamp;
                        _init_day          = true;
                        // _init_week         = true;
                        last_timestamp = timestamp;
                    } else {
                        _timestamps_week.push_back(timestamp);
                        for (size_t i = 0; i < _amount_of_devices; i++) {
                            float value;
                            iss >> value;
                            _devices_values_last_week.at(i).push_back(value);

                            if (previous_values.size() == _amount_of_devices) {
                                float usage = calculate_usage(previous_timestamp, previous_values.at(i),
                                        timestamp, value);
                                power_usages_week.at(i) += usage;
                                previous_values.at(i) = value;
                            } else {
                                previous_values.push_back(value);
                            }
                        }
                        previous_timestamp = timestamp;
                        _init_week         = true;
                    }
                }
            }
        }

        values_file.close();
        return true;
    } else {
        return false;
    }
    return false;
}

bool PowerIntegrator::read_month_usage() {
    std::string   file_path = _data_path + file_month;
    std::ifstream month_file(file_path.c_str(), std::ios_base::in);

    if (month_file.good()) {
        std::string line;
        if (std::getline(month_file, line)) {
            if (line.length() > 0) {
                std::istringstream iss(line);
                iss >> _month[0] >> _month[1];
            } else {
                month_file.close();
                return false;
            }

        } else {
            month_file.close();
            return false;
        }

        float  value;

        size_t index = 0;
        while (std::getline(month_file, line) && index < _amount_of_devices) {
            if (line.length() > 0) {
                std::istringstream iss(line);
                iss >> value;
                power_usages_month.at(index) = value;
                index++;
            }
        }

        month_file.close();
        _init_month = true;
        return true;
    } else {
        return false;
    }
    return false;
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

const std::vector<int64_t> PowerIntegrator::get_timestamps() {
    return _timestamps;
}
const std::vector<int64_t> PowerIntegrator::get_timestamps_week() {
    return _timestamps_week;
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