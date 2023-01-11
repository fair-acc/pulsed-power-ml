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
    // TODO 
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

    ScrollingBuffer(int max_size = 500'000); //200'000

    void addPoint(double x, double y);
    void erase();
};

class BufferPower {
public:
    bool                   init = false;
    std::vector<double>    values;
    double                 timestamp;   

    void updateValues(const std::vector<double> &_values);
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
    bool                         success = false;
    bool                         init = false;
       
    Acquisition();
    Acquisition(int numSignals);

    void deserialize(); 
    void fail(){this->success = false;};

    std::vector<double> relativeTimestamps;
    uint64_t            lastTimeStamp = 0.0;

private:
    uint64_t            refTrigger_ns = 0;
    double              refTrigger_s  = 0.0;
 
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
    void fail(){};

    std::vector<double> relativeTimestamps = { 0 };
    uint64_t            lastTimeStamp      = 0.0;

private:
    uint64_t            refTrigger_ns = 0;
    double              refTrigger_s  = 0.0;
    std::vector<double> channelMagnitudeValues;
    std::vector<double> channelFrequencyValues;

    void                addToBuffers();
};

class PowerUsage {
public:
    //dummy devices
    std::vector<std::string> devices; // = {"Device 1", "Device 2", "Sys Laptop", "Lamp", "Light", "Fridge","Fan","TV","PC","Radio",  "Others"}; 
    std::vector<double> powerUsages;
    std::vector<double> powerUsagesDay;
    std::vector<double> powerUsagesWeek;
    std::vector<double> powerUsagesMonth;
    uint64_t            lastRefTrigger = 0;
    std::string         jsonString = "";
    std::vector<Buffer> buffers;
    bool                success = false;
    bool                init = false; 
    double              deliveryTime;
    int64_t             timestamp;
    uint64_t            lastTimeStamp      = 0.0;


    // dummy values
    std::vector<double> lastWeekUsage   = {85.0, 64.0, 56.0, 97.0, 79.0, 71.0, 20.0}; // other service

     // dummy value
    double              powerUsageToday ; //= 45.9;

    //dummy value
    double              kWhUsedDay   ;//   = 1098.99;

    //dummy value
    double              kWhUsedMonth   ;// = 1098.99;

    //dummy value
    double              costPerMonth  ;//  = 3919.45;

    // dummy value
    double              kWhUsedWeek;//    = 398.99;

    // dummy value 
    double              costPerWeek; //     = 1238.98;
 

    PowerUsage();
    PowerUsage(int numSignals);

    void                deserialize();
    void                fail();
    double              sumOfUsage();

private: 
    void                setSumOfUsageDay();
    void                setSumOfUsageWeek();
    void                setSumOfUsageMonth();

// // TODO - define Buffer if needed
// private:
//     void                addToBuffers();  
};
