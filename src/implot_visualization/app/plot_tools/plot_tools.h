#pragma once

#include <deserialize_json.h>
#include <vector>

class Plotter {
public:
    // static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    // static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;

    void plotGrSignals(std::vector<SignalBuffer> &signals);
    void plotBandpassFilter(std::vector<SignalBuffer> &signals);
    void plotPower(std::vector<SignalBuffer> &signals);
    void plotMainsFrequency(std::vector<SignalBuffer> &signals);
    void plotPowerSpectrum(std::vector<SignalBuffer> &signals);

private:
    void plotSignals(std::vector<SignalBuffer> &signals);
};