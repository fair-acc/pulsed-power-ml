#pragma once

#include <deserialize_json.h>
#include <fmt/core.h>
#include <IconsFontAwesome6.h>
#include <implot.h>
#include <vector>

#include "implot_internal.h"

namespace Plotter {

enum DataInterval { Short,
    Mid,
    Long };

/**
 * Adds an overlay to the current plot and shows an icon and a message.
 * @param text the message text shown after the icon
 * @param fontawesome the icon font to use
 * @param icon the icon to print
 * @param icon_color the color to draw the icon with
 * @param relPos optional relative position inside the plot bounds
 */
void addPlotNotice(
        const std::string_view text,
        ImFont                *fontawesome,
        const std::string_view icon       = ICON_FA_TRIANGLE_EXCLAMATION,
        const ImVec4           icon_color = { 1.0, 0, 0, 1.0 }, // red
        const ImVec2           relPos     = { 0.15, 0.05 }) {
    const auto plot_size = ImPlot::GetPlotSize();
    const auto plot_pos  = ImPlot::GetPlotPos();
    const auto pos       = ImVec2(plot_pos.x + plot_size.x * relPos.x, plot_pos.y + plot_size.y * relPos.y);
    ImGui::SetNextWindowPos(pos);
    ImGui::PushFont(fontawesome);
    const auto iconSize = ImGui::CalcTextSize(icon.data(), icon.data() + icon.size());
    ImGui::PopFont();
    const auto   textSize = ImGui::CalcTextSize(text.data(), text.data() + text.size());
    ImVec2      &spacing  = ImGui::GetStyle().ItemSpacing;
    const ImVec2 overlaySize{ iconSize.x + textSize.x + 2 * spacing.x, std::max(iconSize.y, textSize.y) + 2 * spacing.y };
    if (ImGui::BeginChild(text.data(), overlaySize, false,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground)) {
        ImGui::PushFont(fontawesome);
        ImGui::TextColored(icon_color, icon.data(), icon.data() + icon.size());
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + iconSize.y - textSize.y); // bottom align
        ImGui::Text("%.*s", static_cast<int>(text.size()), text.data());
    }
    ImGui::EndChild();
}

bool violatesLimitingCurve(Buffer &limitingCurve, std::vector<Buffer> &buffers) {
    bool violates = false;
    for (Buffer buffer : buffers) {
        for (int idxBuffer = 0; idxBuffer < buffer.data.size(); idxBuffer++) {
            for (int idxLimit = 0; idxLimit < limitingCurve.data.size() - 1; idxLimit++) {
                if (buffer.data[idxBuffer].x >= limitingCurve.data[idxLimit].x && buffer.data[idxBuffer].x < limitingCurve.data[idxLimit + 1].x) {
                    // linear interpolation
                    DataPoint limitLeft  = limitingCurve.data[idxLimit];
                    DataPoint limitRight = limitingCurve.data[idxLimit + 1];
                    double    slope      = (limitRight.y - limitLeft.y) / (limitRight.x - limitLeft.x);
                    double    yshift     = limitLeft.y;
                    double    limitY     = slope * (buffer.data[idxBuffer].x - limitLeft.x) + yshift;
                    if (limitY < buffer.data[idxBuffer].y) {
                        return violates = true;
                    }
                }
            }
        }
    }
    return violates;
}

double setTimeInterval(DataInterval Interval) {
    double dt = 0;
    switch (Interval) {
    case Short:
        dt = 300.0; // 5 minutes
        break;
    case Mid:
        dt = 3'600.0; // 1 hour
        break;
    case Long:
        dt = 86'400.0; // 24 hours
        break;
    }
    return dt;
}

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
std::string to_si_prefix(T value_base, std::string_view unit = "s", std::size_t significant_digits = 0) {
    static constexpr std::array si_prefixes{ 'q', 'r', 'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R', 'Q' };
    static constexpr double     base     = 1000.0;
    long double                 value    = value_base;

    std::size_t                 exponent = 10;
    if (value == 0) {
        return fmt::format("{:.{}f}{}{}{}", value, significant_digits, unit.empty() ? "" : " ", si_prefixes[exponent], unit);
    }
    while (value >= base && exponent < si_prefixes.size()) {
        value /= base;
        ++exponent;
    }
    while (value < 1.0 && exponent > 0) {
        value *= base;
        --exponent;
    }

    return fmt::format("{:.{}f}{}{}{}", value, significant_digits, unit.empty() ? "" : " ", si_prefixes[exponent], unit);
}

template<typename T>
static void plotSignals(std::vector<T> &signals) {
    for (const auto &signal : signals) {
        if (!signal.data.empty()) {
            int offset = 0;
            if constexpr (requires { signal.offset; }) {
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

void plotSignals(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags   xflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotAxisFlags   yflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotLineFlags   lineFlag    = ImPlotLineFlags_None;
    static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
    static ImPlotLegendFlags legendFlags = 0;
    ImU32                    blue        = 4289753676;
    ImU32                    red         = 4283584196;
    ImPlot::SetupAxes("", "U(V)", xflags, yflags);
    ImPlot::SetupAxis(ImAxis_Y2, "I(A)", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupLegend(legendLoc, legendFlags);

    // color axis
    ImPlotPlot &plot    = *GImPlot->CurrentPlot;

    ImPlotAxis &axis_Y2 = plot.Axes[ImAxis_Y2];
    axis_Y2.ColorTxt    = red;
    axis_Y2.ColorTick   = red;

    ImPlotAxis &axis_Y1 = plot.Axes[ImAxis_Y1];
    axis_Y1.ColorTxt    = blue;
    axis_Y1.ColorTick   = blue;

    for (const auto &signal : signals) {
        if (!signal.data.empty()) {
            int offset = 0;
            if constexpr (requires { signal.offset; }) {
                offset = signal.offset;
            }
            if (signal.signalName.find("I") != std::string::npos) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
            }
            if (signal.signalName.find("bpf") != std::string::npos) {
                lineFlag = ImPlotLineFlags_Segments;
            } else {
                lineFlag = ImPlotLineFlags_None;
            }
            ImPlot::PlotLine((signal.signalName).c_str(),
                    &signal.data[0].x,
                    &signal.data[0].y,
                    signal.data.size(),
                    lineFlag,
                    offset,
                    2 * sizeof(double));
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        }
    }
}

void plotPower(std::vector<ScrollingBuffer> &signals, DataInterval Interval = Short) {
    static ImPlotAxisFlags   xflags      = ImPlotAxisFlags_None;
    static ImPlotAxisFlags   yflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
    static ImPlotLegendFlags legendFlags = 0;
    ImPlot::SetupAxes("", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    double dt          = setTimeInterval(Interval);
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - dt, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(rad)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupLegend(legendLoc, legendFlags);
    for (const auto &signal : signals) {
        if (!signal.data.empty()) {
            int offset = 0;
            if constexpr (requires { signal.offset; }) {
                offset = signal.offset;
            }
            if (signal.signalName.find("phi") != std::string::npos) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
            }
            ImPlot::PlotLine((signal.signalName).c_str(),
                    &signal.data[0].x,
                    &signal.data[0].y,
                    signal.data.size(),
                    0,
                    offset,
                    2 * sizeof(double));

            // Add tags with signal value
            DataPoint   lastPoint = signal.data.back();
            ImVec4      col       = ImPlot::GetLastItemColor();
            std::string tagValue  = to_si_prefix(lastPoint.y, " ", 2);
            ImPlot::TagY(lastPoint.y, col, "%s", tagValue.c_str());
        }
    }
}

void plotBarchart(std::vector<double> &day_values) {
    if (ImPlot::BeginPlot("Usage Over Last 7 Days (kWh)")) {
        static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
        static ImPlotLegendFlags legendFlags = 0;
        ImPlot::SetupLegend(legendLoc, legendFlags);

        static const char *labels[]     = { "", "", "", "", "", "", "Today" };
        static double      kWh[7]       = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        double             kWhToday[7]  = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        kWhToday[6]                     = day_values.back();

        static const double positions[] = { 0, 1, 2, 3, 4, 5, 6 };
        bool                clamp       = false;

        std::copy(day_values.begin(), day_values.end() - 1, kWh);
        kWh[6]             = 0;

        double max_element = *std::max_element(day_values.begin(), day_values.end());

        double plot_buffer = max_element * 0.1 + 5;

        ImPlot::SetupAxesLimits(-0.5, 6.5, 0, max_element + plot_buffer, ImGuiCond_Always);
        ImPlot::SetupAxes("Day", "kWh");
        ImPlot::SetupAxisTicks(ImAxis_X1, positions, 7, labels);
        ImPlot::PlotBars("Usage Over Last 7 Days (kWh)", kWh, 7, 0.7);
        ImPlot::PlotBars("Usage Today (kWh)", kWhToday, 7, 0.7);

        for (int i = 0; i < 7; i++) {
            if (i != 6) {
                ImPlot::Annotation(positions[i], kWh[i], ImVec4(0, 0, 0, 0), ImVec2(0, -5), clamp, "%.2f", kWh[i]);
            } else {
                ImPlot::Annotation(positions[i], kWhToday[i], ImVec4(0, 0, 0, 0), ImVec2(0, -5), clamp, "%.2f", kWhToday[i]);
            }
        }

        ImPlot::EndPlot();
    }
}

void plotNestTable(
        PowerUsage  &powerUsage,
        int          offset,
        int          len,
        DataInterval m_d_w,
        double       integratedValue = 0) {
    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImDrawList            *draw_list  = ImGui::GetWindowDrawList();
    ImGuiWindow           *window     = ImGui::GetCurrentWindow();

    ImGuiContext          &g          = *GImGui;
    const ImGuiStyle      &style      = g.Style;

    //  off
    const ImVec4 &disable_col = style.Colors[ImGuiCol_TextDisabled];

    //  on
    const ImVec4 &able_col = style.Colors[ImGuiCol_PlotHistogram];

    std::string   timePeriod;

    switch (m_d_w) {
    case Long:
        timePeriod = "Relative Usage Current Month";
        break;
    case Mid:
        timePeriod = "Relative Usage Current Week";
        break;
    case Short:
        timePeriod = "Relative Usage Current Day";
        break;
    default:
        break;
    }

    if (ImGui::BeginTable("Devices Table", 4, tableFlags, ImVec2(-1, 0))) {
        ImGui::TableSetupColumn("Device", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("On/Off", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Apparent Power [VA]", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(timePeriod.c_str());
        ImGui::TableHeadersRow();

        std::string output_simbol;
        for (int row = offset; row < offset + len; row++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            if (powerUsage.init && row < powerUsage.devices.size()) {
                ImGui::Text("%s", powerUsage.devices[row].c_str());
                ImGui::TableSetColumnIndex(1);
                ImVec2 pos = window->DC.CursorPos;

                // columns only for initialized values
                if (powerUsage.powerUsages[row] == 0) {
                    draw_list->AddCircleFilled(pos + ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, style.FramePadding.y + draw_list->_Data->FontSize * 0.35f),
                            draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(disable_col), 8);

                } else {
                    draw_list->AddCircleFilled(pos + ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, style.FramePadding.y + draw_list->_Data->FontSize * 0.35f),
                            draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(able_col), 8);
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.2f", powerUsage.powerUsages[row]);
                ImGui::TableSetColumnIndex(3);
                double relative = 0.0;
                switch (m_d_w) {
                case 0:
                    if (row < powerUsage.powerUsagesMonth.size()) {
                        if (integratedValue > 0) {
                            relative = powerUsage.powerUsagesMonth[row] / integratedValue;
                        }
                        ImGui::Text("%3.2f %%", relative * 100);
                    }
                    break;
                case 1:
                    if (row < powerUsage.powerUsagesWeek.size()) {
                        if (integratedValue > 0) {
                            relative = powerUsage.powerUsagesWeek[row] / integratedValue;
                        }
                        ImGui::Text("%3.2f %%", relative * 100);
                    }
                    break;
                case 2:
                    if (row < powerUsage.powerUsagesDay.size()) {
                        if (integratedValue > 0) {
                            relative = powerUsage.powerUsagesDay[row] / integratedValue;
                        }
                        ImGui::Text("%3.2f %%", relative * 100);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        ImGui::EndTable();
    }
}

void plotTable(PowerUsage &powerUsage, DataInterval m_d_w, double integratedValue = 0) {
    ImGuiContext     &g     = *GImGui;
    const ImGuiStyle &style = g.Style;

    // on
    const ImVec4 &able_col = style.Colors[ImGuiCol_PlotHistogram];
    int           rest     = 0;
    int           len      = 0;

    if (powerUsage.init) {
        size_t rows = (int) powerUsage.devices.size();
        rest        = rows % 2;
        len         = rows / 2;
    }

    if (ImGui::BeginTable("table_nested", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableNextColumn();
        plotNestTable(powerUsage, 0, len + rest, m_d_w, integratedValue);
        ImGui::TableNextColumn();
        plotNestTable(powerUsage, len + rest, len, m_d_w, integratedValue);
        ImGui::EndTable();
    }

    if (powerUsage.success == false) {
        ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "%s", "Connection failed\n");
        if (powerUsage.init == true) {
            char buff[32];
            char buff_time[32];

            ImPlot::FormatDate(ImPlotTime::FromDouble(powerUsage.deliveryTime), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
            ImPlot::FormatTime(ImPlotTime::FromDouble(powerUsage.deliveryTime), buff_time, 32, ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
            ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "Last delivery time %s %s", buff, buff_time);
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", "Server not available");
        }
    } else {
        ImGui::TextColored(able_col, "%s", "Connected\n");
        ImGui::Text(" ");
    }
}

void plotStatistics(std::vector<ScrollingBuffer> &signals, DataInterval Interval = Short) {
    static ImPlotAxisFlags   xflags      = ImPlotAxisFlags_None;
    static ImPlotAxisFlags   yflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
    static ImPlotLegendFlags legendFlags = 0;
    ImPlot::SetupAxes("", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    double dt          = setTimeInterval(Interval);
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - dt, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(rad)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupLegend(legendLoc, legendFlags);

    for (auto signal : signals) {
        if (signal.data.empty()) {
            return;
        }
    }

    static float alpha = 0.25f;
    ImGui::DragFloat("Alpha", &alpha, 0.01f, 0, 1);
    if (signals.size() == 12) {
        int offset = 0;
        if constexpr (requires { signals[0].offset; }) {
            offset = signals[0].offset;
        }

        // Plot min and max as shaded
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, alpha);
        for (int i = 0; i < 12; i += 3) {
            if (signals[i].signalName.find("phi") != std::string::npos) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
            }
            ImPlot::PlotShaded((signals[i].signalName).c_str(),
                    &signals[i].data[0].x,
                    &signals[i + 2].data[0].y,
                    &signals[i + 1].data[0].y,
                    signals[i].data.size(),
                    0,
                    offset,
                    2 * sizeof(double));
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        }
        ImPlot::PopStyleVar();

        // Plot mean as line
        for (int i = 0; i < 12; i += 3) {
            if (signals[i].signalName.find("phi") != std::string::npos) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
            }
            ImPlot::PlotLine((signals[i].signalName).c_str(),
                    &signals[i].data[0].x,
                    &signals[i].data[0].y,
                    signals[i].data.size(),
                    0,
                    offset,
                    2 * sizeof(double));

            // Add tags with signal value
            ImVec4      col       = ImPlot::GetLastItemColor();
            DataPoint   lastPoint = signals[i].data.back();
            std::string tagValue  = to_si_prefix(lastPoint.y, " ", 2);
            ImPlot::TagY(lastPoint.y, col, "%s", tagValue.c_str());
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        }
    }
}

void plotMainsFrequency(std::vector<ScrollingBuffer> &signals, DataInterval Interval = Short) {
    static ImPlotAxisFlags   xflags      = ImPlotAxisFlags_None;
    static ImPlotAxisFlags   yflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
    static ImPlotLegendFlags legendFlags = 0;
    ImU32                    blue        = 4289753676;
    ImPlot::SetupAxes("", "mains frequency (Hz)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    double dt          = setTimeInterval(Interval);
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - dt, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupLegend(legendLoc, legendFlags);

    // color axis
    ImPlotPlot &plot    = *GImPlot->CurrentPlot;

    ImPlotAxis &axis_Y1 = plot.Axes[ImAxis_Y1];
    axis_Y1.ColorTxt    = blue;
    axis_Y1.ColorTick   = blue;

    for (const auto &signal : signals) {
        if (!signal.data.empty()) {
            int offset = 0;
            if constexpr (requires { signal.offset; }) {
                offset = signal.offset;
            }
            ImPlot::PlotLine((signal.signalName).c_str(),
                    &signal.data[0].x,
                    &signal.data[0].y,
                    signal.data.size(),
                    0,
                    offset,
                    2 * sizeof(double));

            // Add tags with signal value
            DataPoint   lastPoint = signal.data.back();
            ImVec4      col       = ImPlot::GetLastItemColor();
            std::string tagValue  = to_si_prefix(lastPoint.y, " ", 3);
            ImPlot::TagY(lastPoint.y, col, "%s", tagValue.c_str());
        }
    }
}

void plotPowerSpectrum(std::vector<Buffer> &signals, std::vector<Buffer> &limitingCurve, ImFont *fontawesome) {
    bool                     violation   = false;
    static ImPlotAxisFlags   xflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotAxisFlags   yflags      = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    static ImPlotLocation    legendLoc   = ImPlotLocation_NorthEast;
    static ImPlotLegendFlags legendFlags = 0;
    ImPlot::SetupAxes("Frequency (Hz)", "Power Density (W)", xflags, yflags);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::SetupLegend(legendLoc, legendFlags);
    plotSignals(signals);
    plotSignals(limitingCurve);
    if (limitingCurve.size() == 1) {
        violation = violatesLimitingCurve(limitingCurve[0], signals);
    }
    if (violation) {
        addPlotNotice("warning: pulsed power limits exceeded!", fontawesome, ICON_FA_TRIANGLE_EXCLAMATION, { 1.0, 0, 0, 1.0 }, { 0.15, 0.05 });
        // addPlotNotice("warning: pulsed power limits exceeded!", fontawesome, ICON_FA_TRIANGLE_EXCLAMATION, { 1, 0.5, 0, 1.0 }, { 0.15, 0.25 });
        // addPlotNotice("info: pulsed power limits exceeded!", fontawesome, ICON_FA_CIRCLE_QUESTION, { 0, 0, 1.0, 1.0 }, { 0.15, 0.45 });
    }
}

void plotUsageTable(std::vector<double> powerUsages) {
    ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthFixed, 200.0f);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    ImGui::TableSetupColumn("Usage", ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableHeadersRow();
    if (powerUsages.size() >= 2) {
        ImGui::TableNextRow();
        // Active power P
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Active Power");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("P");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%2f kWh", powerUsages[0]);
        ImGui::TableNextRow();
        // Apparent power S
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Apparent Power");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("S");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%2f kVAh", powerUsages[1]);
    }
}

} // namespace Plotter
