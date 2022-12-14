
#include <deserialize_json.h>
#include <implot.h>
#include <plot_tools.h>
#include <vector>

void Plotter::plotSignals(std::vector<ScrollingBuffer> &signals) {
    for (int i = 0; i < signals.size(); i++) {
        if (signals[i].data.size() > 0) {
            ImPlot::PlotLine((signals[i].signalName).c_str(),
                    &signals[i].data[0].x,
                    &signals[i].data[0].y,
                    signals[i].data.size(),
                    0,
                    signals[i].offset,
                    2 * sizeof(double));
        }
    }
}

void Plotter::plotSignals(std::vector<Buffer> &signals) {
    for (int i = 0; i < signals.size(); i++) {
        if (signals[i].data.size() > 0) {
            ImPlot::PlotLine((signals[i].signalName).c_str(),
                    &signals[i].data[0].x,
                    &signals[i].data[0].y,
                    signals[i].data.size(),
                    0,
                    0,
                    2 * sizeof(double));
        }
    }
}

void Plotter::plotGrSignals(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("UTC Time", "U", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 1.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    plotSignals(signals);
}

void Plotter::plotBandpassFilter(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (ms)", "U(V)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 1.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupAxis(ImAxis_Y2, "I(A)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetupAxisLimits(ImAxis_Y2, -3, 3);
    std::cout << "Start plotting" << std::endl;
    for (int i = 0; i < signals.size(); i++) {
        if (signals[i].data.size() > 0) {
            if (i == 1) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
            }
            ImPlot::PlotLine((signals[i].signalName).c_str(),
                    &signals[i].data[0].x,
                    &signals[i].data[0].y,
                    signals[i].data.size(),
                    0,
                    signals[i].offset,
                    2 * sizeof(double));
            std::cout << "PlotLine finished" << std::endl;
        }
    }

    // if (signals.size() > 1) {
    //     std::vector<ScrollingBuffer> voltage = { signals[0] };
    //     plotSignals(voltage);
    //     std::vector<ScrollingBuffer> current = { signals[1] };
    //     plotSignals(current);
    // }
}

void Plotter::plotPower(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (s)", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 1.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(deg)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}

void Plotter::plotMainsFrequency(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (s)", "Frequency (Hz)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 10.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    // plotSignals(signals);
}

void Plotter::plotPowerSpectrum(std::vector<Buffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("Frequency (Hz)", "Power Density (dB)", xflags, yflags);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}
