#pragma once

#include <deserialize_json.h>
#include <vector>

class Plotter {
public:
    static void plotGrSignals(std::vector<ScrollingBuffer> &signals);
    static void plotBandpassFilter(std::vector<ScrollingBuffer> &signals);
    static void plotPower(std::vector<ScrollingBuffer> &signals);
    static void plotMainsFrequency(std::vector<ScrollingBuffer> &signals);
    static void plotPowerSpectrum(std::vector<Buffer> &signals);

private:
    static void plotSignals(std::vector<ScrollingBuffer> &signals);
    static void plotSignals(std::vector<Buffer> &signals);
};
