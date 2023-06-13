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

    ScrollingBuffer(int max_size = 50'000);

    void addPoint(double x, double y);
    void erase();
};

class DummyBuffer {
public:
    DummyBuffer(size_t _size) {}
};

class StrideArray {
public:
    std::vector<int>    dims;
    std::vector<double> values;

    StrideArray();

    ~StrideArray(){};

    void cut(const std::vector<int> newDims);
};

struct ConvertPair {
    std::vector<double> relativeTimestamps;
    uint64_t            referenceTimestamps;
    StrideArray         strideArray;
} typedef Convert;

template<typename T>
class IAcquisition {
public:
    std::vector<std::string> signalNames;
    std::string              jsonString    = "";
    uint64_t                 lastTimeStamp = 0.0;
    std::vector<T>           buffers;
    bool                     success = false;

    IAcquisition();
    IAcquisition(const std::vector<std::string> _signalNames, const int _bufferSize = 50'000);

    virtual ~IAcquisition()    = default;

    virtual void deserialize() = 0;

protected:
    bool receivedRequestedSignals(std::vector<std::string> receivedSignals);
};

class Acquisition : public IAcquisition<ScrollingBuffer> {
public:
    Acquisition();
    Acquisition(const std::vector<std::string> &_signalNames, const int _bufferSize = 50'000);

    ~Acquisition(){};

    void deserialize();

private:
    uint64_t lastRefTrigger = 0;

    void     addToBuffers(const StrideArray &strideArray, const std::vector<double> &relativeTimestamp, double refTrigger_ns);
    bool     receivedVoltageCurrentData(std::vector<std::string> receivedSignals);
};

class AcquisitionSpectra : public IAcquisition<Buffer> {
public:
    AcquisitionSpectra();
    AcquisitionSpectra(const std::vector<std::string> &_signalNames, const int _bufferSize = 50'000);

    ~AcquisitionSpectra(){};

    void deserialize();

private:
    void addToBuffers(const std::vector<double> &channelMagnitudeValues, const std::vector<double> &channelFrequencyValues);
};

class PowerUsage : public IAcquisition<Buffer> {
public:
    std::vector<std::string> devices;
    std::vector<double>      powerUsages;
    std::vector<double>      powerUsagesDay;
    std::vector<double>      powerUsagesWeek;
    std::vector<double>      powerUsagesMonth;
    bool                     init = false;
    double                   deliveryTime;
    int64_t                  timestamp;

    double                   kWhUsedDay   = 0.0;
    double                   kWhUsedMonth = 0.0;
    double                   kWhUsedWeek  = 0.0;

    PowerUsage();
    PowerUsage(const std::vector<std::string> &_signalNames, const int _bufferSize = 50'000);

    ~PowerUsage(){};

    void   deserialize();
    void   fail();
    double sumOfUsage();

private:
    void setSumOfUsageDay();
    void setSumOfUsageWeek();
    void setSumOfUsageMonth();
};

class RealPowerUsage : public IAcquisition<DummyBuffer> {
public:
    double              deliveryTime;
    bool                init = false;

    std::vector<double> realPowerUsages;

    RealPowerUsage();
    RealPowerUsage(const std::vector<std::string> &_signalNames, const int _bufferSize = 0);

    ~RealPowerUsage(){};

    void deserialize();
    void fail();
    void addPowerUsage(const StrideArray &strideArray);
};

class PowerUsageWeek : public IAcquisition<DummyBuffer> {
public:
    PowerUsageWeek() = default;
    PowerUsageWeek(const std::vector<std::string> &_signalNames, const int _bufferSize = 0);

    ~PowerUsageWeek() = default;

    void                deserialize();
    std::vector<double> getValues();

private:
    std::vector<double> values;
    std::int64_t        timestamp;
};
