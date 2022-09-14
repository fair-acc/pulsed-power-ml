#pragma once

#include <imgui.h>
#include <iostream>
#include <shared_mutex>
#include <vector>

class DataPoint {
public:
    double x;
    double y;

    constexpr DataPoint();
    constexpr DataPoint(double _x, double _y);
};

class SignalBuffer {
public:
    int                 maxSize;
    int                 offset;
    ImVector<DataPoint> data;
    std::string         signalName;

    SignalBuffer(int max_size = 200'000);

    void addPoint(double x, double y);
    void erase();
};

struct StrideArray {
    std::vector<int>    dims;
    std::vector<double> values;
};

struct Acquisition {
    uint64_t                 refTrigger_ns = 0;
    double                   refTrigger_s  = 0.0;
    std::vector<std::string> signalNames;
    std::vector<double>      relativeTimestamps;
    StrideArray              strideArray;
};

class Deserializer {
public:
    /**
     * @brief Deserializes json string
     *
     * @param jsonString string of type
     * "Acquisition": {"value": key, "timestamp": key}
     */
    void deserializeJson(const std::string &jsonString, std::vector<SignalBuffer> &signals);

private:
    void deserializeAcquisition(const std::string &jsonString, std::vector<SignalBuffer> &signals);
    void deserializeCounter(const std::string &jsonString, std::vector<SignalBuffer> &signals);
    void addToSignalBuffers(std::vector<SignalBuffer> &signals, const Acquisition &acquisitionData);
};
