#include <chrono>
#include <iostream>
#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include <implot.h>
#include "implot_internal.h"

#include <deserialize_json.h>
#include <emscripten_fetch.h>
// TODO - barchart
#include <plot_tools.h>

#define ELECTRICY_PRICE 30

// Emscripten requires to have full control over the main loop. We're going to
// store our SDL book-keeping variables globally. Having a single function that
// acts as a loop prevents us to store state in the stack of said function. So
// we need some location for this.
SDL_Window   *g_Window    = NULL;
SDL_GLContext g_GLContext = NULL;

class AppState {
public:
    std::vector<Subscription<PowerUsage>>         subscritpionsPowerUsage;
    // TODO barchart
    Plotter                                       plotter;
    DeviceTable                                   deviceTable;
    double                                        lastFrequencyFetchTime = 0.0;
    std::vector<Subscription<Acquisition>>        subscriptionsTimeDomain;

    AppState(std::vector<Subscription<PowerUsage>> &_powerUsages, std::vector<Subscription<Acquisition>> &_subscriptionsTimeDomain) {
        this->subscritpionsPowerUsage = _powerUsages;
        this->subscriptionsTimeDomain = _subscriptionsTimeDomain;

        auto   clock                  = std::chrono::system_clock::now();
        double currentTime            = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
        this->lastFrequencyFetchTime  = currentTime;
    }
};

static void main_loop(void *);

int         main(int, char **) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // For the browser using Emscripten, we are going to use WebGL1 with GL ES2.
    // It is very likely the generated file won't work in many browsers.
    // Firefox is the only sure bet, but I have successfully run this code on
    // Chrome for Android for example.
    const char *glsl_version = "#version 100";
    // const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_Window                     = SDL_CreateWindow("Nilm Power Monitoring", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    g_GLContext                  = SDL_GL_CreateContext(g_Window);
    if (!g_GLContext) {
        fprintf(stderr, "Failed to initialize WebGL context!\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // time format
    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().Use24HourClock = true;


    // Load Fonts
     //io.Fonts->AddFontDefault();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
   // io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);
#endif

    ImGui::StyleColorsLight();

    Subscription<PowerUsage>                      nilmSubscription("http://localhost:8080/", {"nilm_values"});
    Subscription<Acquisition>                     powerSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "saw@4000Hz" });
    std::vector<Subscription<PowerUsage>>         subscritpionsPowerUsage = {nilmSubscription};
    std::vector<Subscription<Acquisition>>        subscriptionsTimeDomain = { powerSubscription };
    AppState                                      appState(subscritpionsPowerUsage, subscriptionsTimeDomain);

    // This function call won't return, and will engage in an infinite loop, processing events from the browser, and dispatching them.
    emscripten_set_main_loop_arg(main_loop, &appState, 25, true);
}

static void main_loop(void *arg) {
    ImGuiIO &io = ImGui::GetIO();

    // Parse arguments from main
    AppState                                      *args                     = static_cast<AppState *>(arg);
    std::vector<Subscription<PowerUsage>>         &subscriptionsPowerUsages = args->subscritpionsPowerUsage;
    std::vector<Subscription<Acquisition>>        &subscriptionsTimeDomain  = args->subscriptionsTimeDomain;
    // TODO
    Plotter                                       &plotter                  = args->plotter;
    DeviceTable                                   &deviceTable              = args->deviceTable;
    double                                        &lastFrequencyFetchTime   = args->lastFrequencyFetchTime;
   

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    //static bool   show_demo_window = false;
    static bool   show_demo_window = true;
    static ImVec4 clear_color      = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Layout options
    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImVec2               window_center = main_viewport->GetWorkCenter();
    int                  window_height = 2 * window_center.y;
    int                  window_width  = 2 * window_center.x;

    // Poll and handle events (inputs, window resize, etc.)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        // Capture events here, based on io.WantCaptureMouse and io.WantCaptureKeyboard
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Nilm Power Monitoring Dashboard
    {
        auto   clock       = std::chrono::system_clock::now();
        double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;

        for (Subscription<PowerUsage> &powerUsage : subscriptionsPowerUsages){
            if(currentTime -powerUsage.lastFetchtime >= 2.0 || !powerUsage.acquisition.init){
                powerUsage.fetch();
                powerUsage.lastFetchtime = currentTime;
            }
        }
    
        for (Subscription<Acquisition> &subTime : subscriptionsTimeDomain) {
            if(currentTime -subTime.lastFetchtime >= 0.1){
                subTime.fetch();
                subTime.lastFetchtime = currentTime;
            }
        }
        
        // ImDrawList* draw_list = ImGui::GetWindowDrawList();
        // ImGuiWindow* window = ImGui::GetCurrentWindow();
       

        // ImVec4* colors = ImGui::GetStyle().Colors;
        
        // const ImVec4& disable_col = Colors[ImGuiCol_TextDisabled];

        PowerUsage powerUsageValues = subscriptionsPowerUsages[0].acquisition;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_None);
        ImGui::Begin("Eletricity");

       // ImGui::ShowFontSelector("Font");

        static ImPlotSubplotFlags flags     = ImPlotSubplotFlags_NoTitle;
        static int                rows      = 1;
        static int                cols      = 2;
        static float              rratios[] = { 1, 1, 1, 1 };
        static float              cratios[] = { 1, 1, 1, 1 };

        std::string               output_simbol;    

        // ImFont* font_current = ImGui::GetFont();
        // for(int n=0; n<io.Fonts->Fonts.Size; n++){
        //     ImFont* font = io.Fonts->Fonts[n];
        //     ImGui::PushID((void*)font);
        //     ImGui::Text("Font test %d",n);
        //     ImGui::PopID();
        // }

        deviceTable.drawHeader(currentTime);

        ImGui::ShowStyleSelector("Colors##Selector");

        const char* items[] = {"month", "week", "day"};
        static int item_current = 0;

        // subplots
        if (ImPlot::BeginSubplots("My Subplots",rows,cols, ImVec2(-1,400), flags)){ 

            if(ImPlot::BeginPlot("Power")){
                // power      
               // plotter.plotPower(subscriptionsTimeDomain[0].acquisition.buffers, subscriptionsTimeDomain[0].acquisition.success);
                plotter.plotPower(subscriptionsTimeDomain[0].acquisition.buffers);// subscriptionsTimeDomain[0].acquisition.success);
                ImPlot::EndPlot();
            }
       
            plotter.plotBarchart(powerUsageValues);
       
            ImPlot::EndSubplots();
        }

        // TODO -add wrapping
        ImGui::Combo("month/week/day", &item_current, items, IM_ARRAYSIZE(items));

        ImGui::Spacing();

        if (powerUsageValues.init){

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 125, 0, 255));

            if (item_current == 0){
                ImGui::Text("EURO %.2f / %.2f kWh used current month\n", 
                    ELECTRICY_PRICE *  powerUsageValues.kWhUsedMonth, powerUsageValues.kWhUsedMonth);
            }else if (item_current == 1) {
                ImGui::Text("EURO %.2f / %.2f kWh used current week\n", 
                    ELECTRICY_PRICE * powerUsageValues.kWhUsedWeek, powerUsageValues.kWhUsedWeek);
            } else {
                ImGui::Text("EURO %.2f / %.2f kWh used today\n", 
                    ELECTRICY_PRICE * powerUsageValues.kWhUsedDay, powerUsageValues.kWhUsedDay);
            }

            ImGui::Text(" ");
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(ImVec4(1,0,0,1),"%s","Connection Error\n");
            ImGui::TextColored(ImVec4(1,0,0,1),"%s","Server not available");
        }
        
        deviceTable.plotTable(powerUsageValues, item_current);

        ImGui::End();
    }

    // Show ImGui and ImPlot demo windows
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
        ImPlot::ShowDemoWindow();
    }

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(g_Window, g_GLContext);
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_Window);
}
