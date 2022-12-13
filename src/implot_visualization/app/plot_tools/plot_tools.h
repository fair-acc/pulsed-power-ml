#pragma once

#include <deserialize_json.h>
#include <implot.h>
#include <vector>

namespace Plotter {

template<typename T>
static void plotSignals(std::vector<T> &signals) {
    for (const auto &signal : signals) {
        if (!signal.data.empty()) {
            int offset = 0;
            if constexpr (requires {signal.offset;}) {
                offset = signal.offset;
            }
            ImPlot::PlotLine((signal.signalName).c_str(),
                             &signal.data[0].x,
                             &signal.data[0].y,
                             signal.data.size(),
                             0,
                             offset,
                             2 * sizeof(double));
        }
    }
}

void plotGrSignals(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("UTC Time", "Value", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 30.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    Plotter::plotSignals(signals);
}

void plotBandpassFilter(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (ms)", "I(A)", xflags, yflags);
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, 60, ImGuiCond_Always);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    // plotSignals(signals);
}

void plotPower(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (s)", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 10.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(deg)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}

void plotMainsFrequency(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (s)", "Frequency (Hz)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 10.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    // plotSignals(signals);
}

void plotPowerSpectrum(std::vector<Buffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("Frequency (Hz)", "Power Density (dB)", xflags, yflags);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}
} // namespace Plotter
