#pragma once

#include <deserialize_json.h>
#include <vector>

class Plotter {
public:
//    void plotGrSignals(std::vector<ScrollingBuffer> &signals);
//     void plotBandpassFilter(std::vector<ScrollingBuffer> &signals);
    void plotPower(std::vector<ScrollingBuffer> &signals);
    void plotBarchart(PowerUsage &powerUsage);
    // void plotMainsFrequency(std::vector<ScrollingBuffer> &signals);
    // void plotPowerSpectrum(std::vector<Buffer> &signals);

private:
    void plotSignals(std::vector<ScrollingBuffer> &signals);
    void plotSignals(std::vector<Buffer> &signals);
};

class DeviceTable {
public:
    void plotTable(PowerUsage &powerUsage, int m_d_w=0);
};