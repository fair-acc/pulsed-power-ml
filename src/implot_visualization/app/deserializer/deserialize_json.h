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

    void      addPoint(double x, double y);
    void      erase();
    DataPoint back();
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

struct AcquisitionSpectra {
    uint64_t           refTrigger_ns = 0;
    double             refTrigger_s  = 0.0;
    std::string        signalName;
    std::vector<float> channelMagnitudeValues;
    std::vector<float> channelFrequencyValues;
};

class Deserializer {
public:
    uint64_t    lastRefTrigger = 0;
    std::string jsonString     = "";

    void        deserializeJson(std::vector<SignalBuffer> &signals);

private:
    void deserializeAcquisition(std::vector<SignalBuffer> &signals);
    void deserializeAcquisitionSpectra(std::vector<SignalBuffer> &signals);
    void addToSignalBuffers(std::vector<SignalBuffer> &signals, const Acquisition &acquisitionData);
    void addToSignalBuffers(std::vector<SignalBuffer> &signals, const AcquisitionSpectra &acquisitionData);
};
