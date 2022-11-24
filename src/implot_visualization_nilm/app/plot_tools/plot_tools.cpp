
#include <deserialize_json.h>
#include <implot.h>
#include <plot_tools.h>
#include <vector>
#include <implot_internal.h>

//void Plotter::plotBarchart(){};


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

// void Plotter::plotGrSignals(std::vector<ScrollingBuffer> &signals) {
//     static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
//     static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
//     ImPlot::SetupAxes("UTC Time", "Value", xflags, yflags);
//     auto   clock       = std::chrono::system_clock::now();
//     double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
//     ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 10.0, currentTime, ImGuiCond_Always);
//     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
//     ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
//     plotSignals(signals);
// }

// void Plotter::plotBandpassFilter(std::vector<ScrollingBuffer> &signals) {
//     static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
//     static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
//     ImPlot::SetupAxes("time (ms)", "I(A)", xflags, yflags);
//     ImPlot::SetupAxisLimits(ImAxis_X1, 0, 60, ImGuiCond_Always);
//     ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
//     // plotSignals(signals);
// }

void Plotter::plotPower(std::vector<ScrollingBuffer> &signals) {
    static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlot::SetupAxes("time (s)", "P(W), Q(Var), S(VA)", xflags, yflags);
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 60.0, currentTime, ImGuiCond_Always);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y2, "phi(deg)", ImPlotAxisFlags_AuxDefault);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    plotSignals(signals);
}

// void Plotter::plotMainsFrequency(std::vector<ScrollingBuffer> &signals) {
//     static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
//     static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
//     ImPlot::SetupAxes("time (s)", "Frequency (Hz)", xflags, yflags);
//     auto   clock       = std::chrono::system_clock::now();
//     double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
//     ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - 10.0, currentTime, ImGuiCond_Always);
//     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
//     ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
//     // plotSignals(signals);
// }

// void Plotter::plotPowerSpectrum(std::vector<Buffer> &signals) {
//     static ImPlotAxisFlags xflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
//     static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
//     ImPlot::SetupAxes("Frequency (Hz)", "Power Density (dB)", xflags, yflags);
//     ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
//     plotSignals(signals);
// }

void DeviceTable::plotTable(PowerUsage &powerUsage){

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    if (powerUsage.success == false){
        ImGui::TextColored(ImVec4(1,0.5,0,1),"%s","Connection failed\n");
        if(powerUsage.init == true){
            
            char buff[32];
            char buff_time[32];
            
            ImPlot::FormatDate(ImPlotTime::FromDouble(powerUsage.deliveryTime),buff,32,ImPlotDateFmt_DayMoYr,ImPlot::GetStyle().UseISO8601);
            ImPlot::FormatTime(ImPlotTime::FromDouble(powerUsage.deliveryTime),buff_time,32,ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
            ImGui::TextColored(ImVec4(1,0.5,0,1),"Last delivery time %s %s" , buff, buff_time);
        } else {
            ImGui::TextColored(ImVec4(1,0,0,1),"%s","Connection Error\n");
            ImGui::TextColored(ImVec4(1,0,0,1),"%s","Server not available");
        }
    }else{       
        
        ImGui::TextColored(ImVec4(1,1,0,1),"%s","Connection OK\n");
        ImGui::Text(" ");
    }

    size_t rows = (int)powerUsage.devices.size();

    double sum_of_usage = powerUsage.sumOfUsage();


    // nilm Visualisation
    if (ImGui::BeginTable("Devices table", 4, tableFlags, ImVec2(-1,0))){
        ImGui::TableSetupColumn("Device", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("On/Off", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Apparent Power [VA]", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Relative Usage");
        ImGui::TableHeadersRow();
        ImPlot::PushColormap(ImPlotColormap_Cool);
 
        std::string               output_simbol ;  

        for (int row = 0; row < rows; row++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", powerUsage.devices[row].c_str());
           
            if (powerUsage.init){
           
                ImGui::TableSetColumnIndex(1);
                
                if (powerUsage.powerUsages[row] == 0){
                    output_simbol = "X";
                    ImGui::Text("%s", output_simbol.c_str());
                    
                }else{ 
                    output_simbol = "O";
                    ImGui::TextColored(ImVec4(1,0.5,0,1),"%s", output_simbol.c_str());
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 1.00));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);               
                }
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.2f", powerUsage.powerUsages[row] );
                ImGui::TableSetColumnIndex(3);
                double relative = powerUsage.powerUsages[row]/sum_of_usage;
                ImGui::Text("%.2f", relative);
            }
        }
        ImPlot::PopColormap();
        ImGui::EndTable();
    }

}