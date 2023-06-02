#ifndef POWER_INTEGRATOR_H
#define POWER_INTEGRATOR_H

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class PowerIntegrator {
private:
    const double                                       _unitFactor         = 0.000000000000001 / 3.6;
    const std::string                                  _file               = "nilm_usage.txt";
    const std::chrono::hours                           _resetIntervalMonth = std::chrono::hours(24 * 30);
    const std::chrono::hours                           _resetIntervalWeek  = std::chrono::hours(24 * 7);
    const std::chrono::hours                           _resetIntervalDay   = std::chrono::hours(24);

    const size_t                                       _amountOfDevices;
    const std::string                                  _dataPath;
    const std::chrono::minutes                         _saveInterval;
    const std::chrono::seconds                         _updateInterval = std::chrono::seconds(10);

    std::chrono::time_point<std::chrono::system_clock> _lastSave;
    std::chrono::time_point<std::chrono::system_clock> _lastResetDay;
    std::chrono::time_point<std::chrono::system_clock> _lastResetWeek;
    std::chrono::time_point<std::chrono::system_clock> _lastResetMonth;

    int64_t                                            _lastTimestamp = 0;
    std::vector<float>                                 _lastValues;

    // current cumulate power usage of devices
    std::vector<double>                                       _powerUsagesMonth;
    std::vector<double>                                       _powerUsagesWeek;
    std::vector<double>                                       _powerUsagesDay;

    std::mutex                                                _dataMutex;
    std::atomic<bool>                                         _shutdownRequested;
    std::jthread                                              _resetAndUpdateThread;

    static bool                                               createDirectory(const std::string &directoryPath);
    static int64_t                                            getUnixTimestampFromTimepoint(std::chrono::time_point<std::chrono::system_clock> timePoint);
    static std::chrono::time_point<std::chrono::system_clock> getTimePointFromUnixTimestamp(int64_t timestamp);

    bool                                                      saveValuesToFile();
    bool                                                      readValuesFromFile();
    double                                                    calculateUsage(int64_t t_0, float lastValue, int64_t t_1, float currentValue) const;
    void                                                      resetValuesAndRewriteFile();

public:
    void               update(int64_t timestamp, std::vector<float> &values);
    std::vector<float> getPowerUsagesMonth() const;
    std::vector<float> getPowerUsagesWeek() const;
    std::vector<float> getPowerUsagesDay() const;

    explicit PowerIntegrator(size_t amountOfDevices, const std::string &dataPath = "./", std::chrono::minutes saveInterval = std::chrono::minutes(10));
    ~PowerIntegrator();
};

PowerIntegrator::PowerIntegrator(const size_t amountOfDevices, const std::string &dataPath, const std::chrono::minutes saveInterval)
    : _amountOfDevices(amountOfDevices), _dataPath(dataPath), _saveInterval(saveInterval), _shutdownRequested(false) {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    if (!createDirectory(dataPath)) {
        std::cout << "Error creating directory " << dataPath << std::endl;
    }

    // init usage day, week, month
    for (size_t i = 0; i < _amountOfDevices; i++) {
        _powerUsagesDay.push_back(0.0);
        _powerUsagesWeek.push_back(0.0);
        _powerUsagesMonth.push_back(0.0);
        _lastValues.push_back(0.0F);
    }

    if (!readValuesFromFile()) {
        _lastSave       = now;
        _lastResetDay   = now;
        _lastResetWeek  = now;
        _lastResetMonth = now;
        if (!saveValuesToFile()) {
            std::cout << "Error writing file" << std::endl;
        }
    }

    _resetAndUpdateThread = std::jthread([this] {
        while (!_shutdownRequested) {
            resetValuesAndRewriteFile();
            std::this_thread::sleep_for(_updateInterval);
        }
    });
}

PowerIntegrator::~PowerIntegrator() {
    _shutdownRequested = true;
    _resetAndUpdateThread.join();
}

bool PowerIntegrator::createDirectory(const std::string &directoryPath) {
    if (std::filesystem::exists(std::filesystem::status(directoryPath.c_str()))) {
        return std::filesystem::is_directory(std::filesystem::status(directoryPath.c_str()));
    } else {
        return std::filesystem::create_directories(directoryPath.c_str());
    }
}

void PowerIntegrator::update(int64_t timestamp, std::vector<float> &values) {
    if (timestamp == 0 || _lastTimestamp == timestamp || values.size() != _amountOfDevices) {
        return;
    }
    // values set => calculation possible
    if (_lastTimestamp != 0) {
        std::lock_guard<std::mutex> lock(_dataMutex);
        for (size_t i = 0; i < _amountOfDevices; i++) {
            double additionalUsage = calculateUsage(_lastTimestamp, _lastValues[i], timestamp, values[i]);
            _powerUsagesDay[i] += additionalUsage;
            _powerUsagesWeek[i] += additionalUsage;
            _powerUsagesMonth[i] += additionalUsage;
        }
    }
    _lastTimestamp = timestamp;
    for (size_t i = 0; i < _amountOfDevices; i++) {
        _lastValues[i] = values[i];
    }
}

double PowerIntegrator::calculateUsage(int64_t t_0, float lastValue, int64_t t_1, float currentValue) const {
    int64_t deltaTimeInt = t_1 - t_0;
    double  deltaTime    = static_cast<double>(deltaTimeInt);
    double  valueSum     = static_cast<double>(lastValue + currentValue);
    double  result       = deltaTime * (valueSum / 2.0) * _unitFactor;
    return result;
}

bool PowerIntegrator::readValuesFromFile() {
    int64_t           lastResetDayTimestamp   = 0;
    int64_t           lastResetWeekTimestamp  = 0;
    int64_t           lastResetMonthTimestamp = 0;
    int64_t           lastSaveTimestamp       = 0;
    const std::string filePath                = _dataPath + _file;
    try {
        std::ifstream readStream;
        readStream.open(filePath, std::ifstream::in);
        if (!readStream.is_open()) {
            return false;
        }
        readStream >> lastResetDayTimestamp;
        _lastResetDay = getTimePointFromUnixTimestamp(lastResetDayTimestamp);
        readStream >> lastResetWeekTimestamp;
        _lastResetWeek = getTimePointFromUnixTimestamp(lastResetWeekTimestamp);
        readStream >> lastResetMonthTimestamp;
        _lastResetMonth = getTimePointFromUnixTimestamp(lastResetMonthTimestamp);
        readStream >> lastSaveTimestamp;
        _lastSave    = getTimePointFromUnixTimestamp(lastSaveTimestamp);

        double value = 0.0;
        for (size_t i = 0; i < _amountOfDevices; i++) {
            readStream >> value;
            _powerUsagesDay[i] = value;
        }
        for (size_t i = 0; i < _amountOfDevices; i++) {
            readStream >> value;
            _powerUsagesWeek[i] = value;
        }
        for (size_t i = 0; i < _amountOfDevices; i++) {
            readStream >> value;
            _powerUsagesMonth[i] = value;
        }
        readStream.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool PowerIntegrator::saveValuesToFile() {
    const std::string filePath = _dataPath + _file;
    try {
        std::ofstream outputFile;
        outputFile.open(filePath, std::ofstream::out | std::ofstream::trunc);
        if (!outputFile.is_open()) {
            return false;
        }
        outputFile << getUnixTimestampFromTimepoint(_lastResetDay) << '\n';
        outputFile << getUnixTimestampFromTimepoint(_lastResetWeek) << '\n';
        outputFile << getUnixTimestampFromTimepoint(_lastResetMonth) << '\n';
        outputFile << getUnixTimestampFromTimepoint(_lastSave) << '\n';
        for (double value : _powerUsagesDay) {
            outputFile << std::setprecision(16) << value << '\n';
        }
        for (double value : _powerUsagesWeek) {
            outputFile << std::setprecision(16) << value << '\n';
        }
        for (double value : _powerUsagesMonth) {
            outputFile << std::setprecision(16) << value << '\n';
        }
        outputFile.close();
        return true;
    } catch (...) {
        return false;
    }
}

int64_t PowerIntegrator::getUnixTimestampFromTimepoint(std::chrono::time_point<std::chrono::system_clock> timePoint) {
    auto timePointSeconds = std::chrono::time_point_cast<std::chrono::seconds>(timePoint);
    return timePointSeconds.time_since_epoch().count();
}

std::chrono::time_point<std::chrono::system_clock> PowerIntegrator::getTimePointFromUnixTimestamp(int64_t timestamp) {
    std::chrono::seconds                               seconds(timestamp);
    std::chrono::time_point<std::chrono::system_clock> timePoint(
            seconds);
    return timePoint;
}

std::vector<float> PowerIntegrator::getPowerUsagesMonth() const {
    std::vector<float> powerUsagesFloat;
    for (double value : _powerUsagesMonth) {
        powerUsagesFloat.push_back(static_cast<float>(value));
    }
    return powerUsagesFloat;
}

std::vector<float> PowerIntegrator::getPowerUsagesWeek() const {
    std::vector<float> powerUsagesFloat;
    for (double value : _powerUsagesWeek) {
        powerUsagesFloat.push_back(static_cast<float>(value));
    }
    return powerUsagesFloat;
}

std::vector<float> PowerIntegrator::getPowerUsagesDay() const {
    std::vector<float> powerUsagesFloat;
    for (double value : _powerUsagesDay) {
        powerUsagesFloat.push_back(static_cast<float>(value));
    }
    return powerUsagesFloat;
}

void PowerIntegrator::resetValuesAndRewriteFile() {
    bool                                               rewriteFile = false;
    std::lock_guard<std::mutex>                        lock(_dataMutex);
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    if (now - _lastResetMonth > _resetIntervalMonth) {
        _lastResetMonth = now;
        for (size_t i = 0; i < _amountOfDevices; i++) {
            _powerUsagesMonth[i] = 0.0;
        }
        rewriteFile = true;
    }
    if (now - _lastResetWeek > _resetIntervalWeek) {
        _lastResetWeek = now;
        for (size_t i = 0; i < _amountOfDevices; i++) {
            _powerUsagesWeek[i] = 0.0;
        }
        rewriteFile = true;
    }
    if (now - _lastResetDay > _resetIntervalDay) {
        _lastResetDay = now;
        for (size_t i = 0; i < _amountOfDevices; i++) {
            _powerUsagesDay[i] = 0.0;
        }
        rewriteFile = true;
    }
    if (now - _lastSave > _saveInterval) {
        rewriteFile = true;
    }
    if (rewriteFile) {
        _lastSave = now;
        if (!saveValuesToFile()) {
            std::cout << "Error updating save file" << std::endl;
        }
    }
}

#endif /* POWER_INTEGRATOR_H */
