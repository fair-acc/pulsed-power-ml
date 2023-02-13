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

class BufferPower {
public:
    bool                init = false;
    std::vector<double> values;
    double              timestamp;

    void                updateValues(const std::vector<double> &_values);
};

struct StrideArray {
    std::vector<int>    dims;
    std::vector<double> values;
};

template<typename T>
class IAcquisition {
public:
    std::vector<std::string> signalNames;
    std::string              jsonString    = "";
    uint64_t                 lastTimeStamp = 0.0;
    std::vector<T>           buffers;

    IAcquisition();
    IAcquisition(const std::vector<std::string> _signalNames);

    virtual void deserialize() = 0;

protected:
    bool receivedRequestedSignals(std::vector<std::string> receivedSignals);
};

class Acquisition : public IAcquisition<ScrollingBuffer> {
public:
    uint64_t            lastRefTrigger = 0;
    std::vector<double> relativeTimestamps;

    Acquisition();
    Acquisition(const std::vector<std::string> &_signalNames);

    void deserialize();

private:
    uint64_t    refTrigger_ns = 0;
    double      refTrigger_s  = 0.0;
    StrideArray strideArray;

    void        addToBuffers();
};

class AcquisitionSpectra : public IAcquisition<Buffer> {
public:
    uint64_t            lastRefTrigger     = 0;
    std::vector<double> relativeTimestamps = { 0 };

    AcquisitionSpectra();
    AcquisitionSpectra(const std::vector<std::string> &_signalNames);

    void deserialize();

private:
    uint64_t            refTrigger_ns = 0;
    double              refTrigger_s  = 0.0;
    std::vector<double> channelMagnitudeValues;
    std::vector<double> channelFrequencyValues;

    void                addToBuffers();
};

class PowerUsage : public IAcquisition<Buffer> {
public:
    // dummy devices
    std::vector<std::string> devices; // = {"Device 1", "Device 2", "Sys Laptop", "Lamp", "Light", "Fridge","Fan","TV","PC","Radio",  "Others"};
    std::vector<double>      powerUsages;
    std::vector<double>      powerUsagesDay;
    std::vector<double>      powerUsagesWeek;
    std::vector<double>      powerUsagesMonth;
    uint64_t                 lastRefTrigger = 0;
    std::string              jsonString     = "";
    std::vector<Buffer>      buffers;
    bool                     success = false;
    bool                     init    = false;
    double                   deliveryTime;
    int64_t                  timestamp;
    uint64_t                 lastTimeStamp = 0.0;
    std::vector<std::string> signalNames;

    // dummy values
    std::vector<double> lastWeekUsage = { 85.0, 64.0, 56.0, 97.0, 79.0, 71.0, 20.0 }; // other service

    // dummy value
    double powerUsageToday = 0.0; //= 45.9;

    // dummy value
    double kWhUsedDay = 0.0; //   = 1098.99;

    // dummy value
    double kWhUsedMonth = 0.0; // = 1098.99;

    // dummy value
    double costPerMonth = 0.0; //  = 3919.45;

    // dummy value
    double kWhUsedWeek = 0.0; //    = 398.99;

    // dummy value
    double costPerWeek = 0.0; //     = 1238.98;

    PowerUsage();
    PowerUsage(const std::vector<std::string> &_signalNames);
    PowerUsage(int numSignals);

    void   deserialize();
    void   fail();
    double sumOfUsage();

private:
    void setSumOfUsageDay();
    void setSumOfUsageWeek();
    void setSumOfUsageMonth();

    // // TODO - define Buffer if needed
    // private:
    //     void                addToBuffers();
};
