#include <chrono>
#include <cstdio>
#include <SDL.h>
#include <SDL_opengles2.h>
#include <vector>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <implot.h>

#include <deserialize_json.h>
#include <emscripten_fetch.h>
#include <fair_header.h>
#include <IconsFontAwesome6.h>
#include <plot_tools.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <vector>

using namespace rapidjson;

static void main_loop(void *);
int         main(int argc, char **argv) {
    int                   timeRange           = 300;
    double                sampRate            = 100;
    float                 updateFreq          = 25.0f;

    // sample rate must equal sink sample rate, F12 for constructor, (8080 -> worker, PulsedPowerservice)
    // subscription to retrieve P, Q, S data for all phases
    Subscription<Acquisition> powerSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", 
                            { "P_0", "Q_0", "S_0", "P_1", "Q_1", "S_1", "P_2", "Q_2", "S_2", "P_acc", "Q_acc", "S_acc" }, 
                                                                            sampRate, timeRange * sampRate, updateFreq);

    // Fetch signal data
    powerSubscription.fetch();

    //data -> json string
    //printf("%s \n", vectorPowerSubscription[0].acquisition.buffers.data()->signalName.c_str());
    //Plotter::plotPowerPhaseZero(vectorPowerSubscription[0].acquisition.buffers, Interval);   

}
