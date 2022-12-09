#pragma once

#include <imgui.h>
#include <iostream>
#include <shared_mutex>
#include <vector>
#include <span>

class DataPoint {
public:
    double x;
    double y;

    constexpr DataPoint();
    constexpr DataPoint(double _x, double _y);
};

class Buffer {
public:
    ImVector<DataPoint> data;
    std::string         signalName;

    Buffer(int max_size = 200'000);

    void assign(const std::vector<double> &x, const std::vector<double> &y);
};

class ScrollingBuffer {
public:
    std::size_t         maxSize;
    std::size_t         offset;
    std::vector<double> x_data;
    std::vector<double> y_data;
    std::string         signalName;

    ScrollingBuffer(int max_size = 200'000);

    void addPoint(double x, double y);
    void addPoints(std::span<double> x, std::span<double> y);
    void erase();
};

class Acquisition {
public:
    std::vector<std::string>     signalNames;
    std::string                  jsonString     = "";
    uint64_t                     lastRefTrigger = 0;
    std::vector<ScrollingBuffer> buffers;

    Acquisition();
    Acquisition(int numSignals);

    void                deserialize();

    uint64_t            lastTimeStamp = 0.0;

private:
    uint64_t    refTrigger_ns = 0;

    void        addToBuffers(std::vector<int>, std::vector<double>, std::vector<double>);
};

class AcquisitionSpectra {
public:
    std::string         signalName;
    uint64_t            lastRefTrigger = 0;
    std::string         jsonString     = "";
    std::vector<Buffer> buffers;

    AcquisitionSpectra();
    AcquisitionSpectra(int numSignals);

    void                deserialize();

    std::vector<double> relativeTimestamps = { 0 };
    uint64_t            lastTimeStamp      = 0.0;

private:
    uint64_t            refTrigger_ns = 0;

    void                addToBuffers(std::vector<double> channelMagnitudeValues, std::vector<double> channelFrequencyValues);
};
