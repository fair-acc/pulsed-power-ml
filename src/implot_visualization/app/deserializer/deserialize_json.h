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

class Buffer {
public:
    ImVector<DataPoint> data;
    std::string         signalName;

    Buffer(int max_size = 200'000);

    void assign(const std::vector<double> &x, const std::vector<double> &y);
};

class ScrollingBuffer {
public:
    int                 maxSize;
    int                 offset;
    ImVector<DataPoint> data;
    std::string         signalName;

    ScrollingBuffer(int max_size = 200'000);

    void addPoint(double x, double y);
    void erase();
};

struct StrideArray {
    std::vector<int>    dims;
    std::vector<double> values;
};

class Acquisition {
public:
    std::vector<std::string>     signalNames;
    std::string                  jsonString     = "";
    uint64_t                     lastRefTrigger = 0;
    std::vector<ScrollingBuffer> buffers;

    Acquisition();
    Acquisition(int numSignals);

    void deserialize();

private:
    uint64_t            refTrigger_ns = 0;
    double              refTrigger_s  = 0.0;
    std::vector<double> relativeTimestamps;
    StrideArray         strideArray;

    void                addToBuffers();
};

class AcquisitionSpectra {
public:
    std::string         signalName;
    uint64_t            lastRefTrigger = 0;
    std::string         jsonString     = "";
    std::vector<Buffer> buffers;

    AcquisitionSpectra();
    AcquisitionSpectra(int numSignals);

    void deserialize();

private:
    uint64_t            refTrigger_ns = 0;
    double              refTrigger_s  = 0.0;
    std::vector<double> channelMagnitudeValues;
    std::vector<double> channelFrequencyValues;

    void                addToBuffers();
};
