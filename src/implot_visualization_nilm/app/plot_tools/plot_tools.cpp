
#include <deserialize_json.h>
#include <implot.h>
#include <plot_tools.h>
#include <vector>
#include <implot_internal.h>
#include <algorithm>


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
            //ImPlot::TagY(signals[0].data.back().y, ImVec4(0,1,1,1), "Current Power: \n%.2f",  signals[0].data.back().y);
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

void Plotter::plotPower(std::vector<ScrollingBuffer> &signals, bool success) {
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
    if(!success){
        ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1,0.5,0,1));
        ImPlot::PlotText("Connection failed", currentTime-30, 2.0f);
        ImPlot::PopStyleColor();
    }
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

void Plotter::plotBarchart(PowerUsage &powerUsage){
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

void DeviceTable::plotTable(PowerUsage &powerUsage, int m_d_w){

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    //  off 
    const ImVec4& disable_col = style.Colors[ImGuiCol_TextDisabled];

    //  on
    const ImVec4& able_col = style.Colors[ImGuiCol_PlotHistogram];

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

}


void DeviceTable::plotNestTable(PowerUsage &powerUsage, int offset, int len, int m_d_w){

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

                if (powerUsage.init){
                ImGui::TableSetColumnIndex(1);
                ImVec2 pos = window->DC.CursorPos;
                
                    // columns only for initialized values
                    if (powerUsage.powerUsages[row] == 0){
                            
                            draw_list->AddCircleFilled(pos+ ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, 
                                        style.FramePadding.y + draw_list->_Data->FontSize * 0.25f), 
                                draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(disable_col), 8);
                            
                    }else{ 

                            draw_list->AddCircleFilled(pos+ ImVec2(style.FramePadding.x + draw_list->_Data->FontSize * 1.0f, 
                                        style.FramePadding.y + draw_list->_Data->FontSize * 0.25f), 
                                draw_list->_Data->FontSize * 0.50f, ImGui::GetColorU32(able_col), 8);             
                    }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f", powerUsage.powerUsages[row] );
                    ImGui::TableSetColumnIndex(3);
                    double relative = 0;
                    switch (m_d_w)
                    {
                    case 0:
                        relative = powerUsage.powerUsagesMonth[row]/sum_of_usage;
                        break;
                    case 1:
                        relative = powerUsage.powerUsagesWeek[row]/sum_of_usage;
                        break;
                    case 2:
                        relative = powerUsage.powerUsagesDay[row]/sum_of_usage;
                        break;
                    default:
                        break;
                    }

                    ImGui::Text("%.2f", relative);
                }
            }

        ImGui::EndTable();
    }

}

void DeviceTable::drawHeader(double currentTime){

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |ImGuiTableFlags_NoBordersInBody;

    // https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples image logo
            // std::string  filename = "picture/77612335.png";

            // static ImVec2 bmin(0,0);
            // static ImVec2 bmax(1,1);
            // static ImVec2 uv0(0,0);
            // static ImVec2 uv1(1,1);
            // static ImVec4 tint(1,1,1,1);

            // bool ret = LoadTextureFromFile("../../MyImage01.jpg", &my_image_texture, &my_image_width, &my_image_height);

            // if (ImPlot::BeginPlot("##image")) {
            //     ImPlot::PlotImage("Logo",ImGui::GetIO().Fonts->TexID, bmin, bmax, uv0, uv1, tint);
            //     ImPlot::EndPlot();
            // }


    if(ImGui::BeginTable("Header", 3 , tableFlags, ImVec2(-1,0)) ){

        ImGui::TableSetupColumn("logo", ImGuiTableColumnFlags_WidthFixed, 400.0f); // Default to 100.0f
        // ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed,500.f); 
        ImGui::TableSetupColumn("name"); 
        ImGui::TableSetupColumn("time", ImGuiTableColumnFlags_WidthFixed,300.0f);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Non Intrusive Load Monitoring @ FAIR"); ImGui::SameLine();

        char buff[32];
        char buff_time[32];
        
        ImGui::TableSetColumnIndex(2);

        ImGui::BeginGroup();
        ImPlot::FormatDate(ImPlotTime::FromDouble(currentTime),buff,32,ImPlotDateFmt_DayMoYr,ImPlot::GetStyle().UseISO8601);
        ImPlot::FormatTime(ImPlotTime::FromDouble(currentTime),buff_time,32,ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
        ImGui::TextColored(ImVec4(1,0.5,0,1)," %s %s (local)" , buff, buff_time);

        ImPlot::GetStyle().UseLocalTime = false;
        ImPlot::FormatTime(ImPlotTime::FromDouble(currentTime),buff_time,32,ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
        ImGui::TextColored(ImVec4(1,0.5,0,1),"            %s (UTC)  " , buff_time);
        ImPlot::GetStyle().UseLocalTime = true;
        ImGui::EndGroup();   

        ImGui::EndTable();
    
    } 

}