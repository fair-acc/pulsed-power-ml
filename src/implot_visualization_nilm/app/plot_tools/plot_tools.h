#pragma once

#include <deserialize_json.h>
#include <vector>
#include <implot.h>
#include <IconsFontAwesome6.h>


namespace Plotter{

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

void plotPower(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("UTC Time", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 300.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(deg)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}

void plotBarchart(PowerUsage &powerUsage){
    if(ImPlot::BeginPlot("Usage over Last 7 Days (kWh)")){

        // Todo - dates
        //auto   clock       = std::chrono::system_clock::now();
        //double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
        
        static const char*  labels[]    = {"1","2","3","4","5","6","Today"};
        static double       kWh[7];
        static double       kWhToday[7] = {0.0, 0.0,0.0, 0.0, 0.0, 0.0, powerUsage.lastWeekUsage.back()};
        static const double positions[] = {0,1,2,3,4,5,6};
        bool clamp = false;

        std::copy(powerUsage.lastWeekUsage.begin(),  powerUsage.lastWeekUsage.end()-1, kWh);
        kWh[6] = 0;

        double max_element = *std::max_element(powerUsage.lastWeekUsage.begin(), powerUsage.lastWeekUsage.end());

        //ImPlot::SetupLegend(ImPlotLocation_North | ImPlotLocation_West, ImPlotLegendFlags_Outside);


        ImPlot::SetupAxesLimits(-0.5, 6.5, 0, max_element+20, ImGuiCond_Always);
        ImPlot::SetupAxes("Day","kWh");
        ImPlot::SetupAxisTicks(ImAxis_X1,positions, 7, labels);
        ImPlot::PlotBars("Usage over Last 6 Days (kWh)", kWh, 7, 0.7);
        ImPlot::PlotBars("Usage Today (kWh)", kWhToday, 7, 0.7);

        for(int i=0;i<7;i++){
            if(i!=6){
                ImPlot::Annotation(positions[i], kWh[i], ImVec4(0,0,0,0),ImVec2(0,-5),clamp, "%.2f", kWh[i] );
            }else{
                ImPlot::Annotation(positions[i], kWhToday[i], ImVec4(0,0,0,0),ImVec2(0,-5),clamp, "%.2f", kWhToday[i] );
            }
        }

        ImPlot::EndPlot();
    }
}


void plotNestTable(PowerUsage &powerUsage, int offset, int len, int m_d_w){

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    //  off 
    const ImVec4& disable_col = style.Colors[ImGuiCol_TextDisabled];

    //  on
    const ImVec4& able_col = style.Colors[ImGuiCol_PlotHistogram];

    //const ImVec4& able_col = ImVec4(0.9, 0.7, 0, 1);

    double sum_of_usage;

    std::string timePeriod;

    switch (m_d_w)
    {
    case 0:
        sum_of_usage   = powerUsage.kWhUsedMonth;
        timePeriod = "Relative Usage current month";
        break;
    case 1:
        sum_of_usage   = powerUsage.kWhUsedWeek;
        timePeriod = "Relative Usage current week";
        break;
    case 2:
        sum_of_usage   = powerUsage.kWhUsedDay;
        timePeriod = "Relative Usage current day";
        break;
    default:
        break;
    }

    if (ImGui::BeginTable("Devices table", 4, tableFlags, ImVec2(-1,0))){
            ImGui::TableSetupColumn("Device", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("On/Off", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Apparent Power [VA]", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn(timePeriod.c_str());
            ImGui::TableHeadersRow();
    
            std::string               output_simbol ;  
            for (int row = offset; row < offset+len; row++){
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", powerUsage.devices[row].c_str());

                if (powerUsage.init && row <powerUsage.devices.size()){
                ImGui::TableSetColumnIndex(1);
                ImVec2 pos = window->DC.CursorPos;
                
                    // columns only for initialized values
                    if (powerUsage.powerUsages[row] == 0){
                            
                            draw_list->AddCircleFilled(pos+ ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, 
                                        style.FramePadding.y + draw_list->_Data->FontSize * 0.35f), 
                                draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(disable_col), 8);
                            
                    }else{ 

                            draw_list->AddCircleFilled(pos+ ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, 
                                        style.FramePadding.y + draw_list->_Data->FontSize * 0.35f), 
                                draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(able_col), 8);             
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f", powerUsage.powerUsages[row] );
                    ImGui::TableSetColumnIndex(3);
                    double relative = 0;
                    switch (m_d_w)
                    {
                    case 0:
                        if(row < powerUsage.powerUsagesMonth.size()){
                            relative = powerUsage.powerUsagesMonth[row]/sum_of_usage;
                            ImGui::Text("%3.2f %%", relative * 100);
                        }
                        break;
                    case 1:
                        if(row < powerUsage.powerUsagesWeek.size()){
                            relative = powerUsage.powerUsagesWeek[row]/sum_of_usage;
                            ImGui::Text("%3.2f %%", relative * 100);
                        }
                        break;
                    case 2:
                        if(row < powerUsage.powerUsagesDay.size()){
                            relative = powerUsage.powerUsagesDay[row]/sum_of_usage;
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


void plotTable(PowerUsage &powerUsage, int m_d_w){

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    //  off 
    const ImVec4& disable_col = style.Colors[ImGuiCol_TextDisabled];

    //  on
    const ImVec4& able_col = style.Colors[ImGuiCol_PlotHistogram];

    
    size_t rows = (int)powerUsage.devices.size();

    int rest = rows % 2;
    int len  = rows / 2;

    if (ImGui::BeginTable("table_nested", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |ImGuiTableFlags_NoBordersInBody)){
   
        ImGui::TableNextColumn();

        plotNestTable(powerUsage, 0, len + rest, m_d_w);

        ImGui::TableNextColumn();

        plotNestTable(powerUsage, len+rest, len, m_d_w);

        ImGui::EndTable();
    }

    if (powerUsage.success == false){
        ImGui::TextColored(ImVec4(1,0.5,0,1),"%s","Connection failed\n");
        if(powerUsage.init == true){
            
            char buff[32];
            char buff_time[32];
            
            ImPlot::FormatDate(ImPlotTime::FromDouble(powerUsage.deliveryTime),buff,32,ImPlotDateFmt_DayMoYr,ImPlot::GetStyle().UseISO8601);
            ImPlot::FormatTime(ImPlotTime::FromDouble(powerUsage.deliveryTime),buff_time,32,ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
            ImGui::TextColored(ImVec4(1,0.5,0,1),"Last delivery time %s %s" , buff, buff_time);
        } else {
            ImGui::TextColored(ImVec4(1,0,0,1),"%s","Server not available");
        }
    }else{             
        ImGui::TextColored(able_col,"%s","Connected\n");
        ImGui::Text(" ");
    }


}

} // namespace Plotter







// class Plotter {
// public:
// //    void plotGrSignals(std::vector<ScrollingBuffer> &signals);
// //     void plotBandpassFilter(std::vector<ScrollingBuffer> &signals);
//     void plotPower(std::vector<ScrollingBuffer> &signals, bool success = true);
//     void plotBarchart(PowerUsage &powerUsage);
//     // void plotMainsFrequency(std::vector<ScrollingBuffer> &signals);
//     // void plotPowerSpectrum(std::vector<Buffer> &signals);

// private:
//     void plotSignals(std::vector<ScrollingBuffer> &signals);
//     void plotSignals(std::vector<Buffer> &signals);
// };

// class DeviceTable {
// public:
//     void plotTable(PowerUsage &powerUsage, int m_d_w=0);
// private:
//     void plotNestTable(PowerUsage &powerUsage, int offset, int len, int m_d_w=0);
// };