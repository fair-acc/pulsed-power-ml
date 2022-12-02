#pragma once

#include <deserialize_json.h>
#include <vector>

class Plotter {
public:
    void plotGrSignals(std::vector<ScrollingBuffer> &signals);
    void plotBandpassFilter(std::vector<ScrollingBuffer> &signals);
    void plotPower(std::vector<ScrollingBuffer> &signals);
    void plotMainsFrequency(std::vector<ScrollingBuffer> &signals);
    void plotPowerSpectrum(std::vector<Buffer> &signals);

private:
    void plotSignals(std::vector<ScrollingBuffer> &signals);
    void plotSignals(std::vector<Buffer> &signals);
};
