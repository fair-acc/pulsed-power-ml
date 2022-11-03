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

#include <deserialize_json.h>
#include <emscripten_fetch.h>
#include <plot_tools.h>

// Emscripten requires to have full control over the main loop. We're going to
// store our SDL book-keeping variables globally. Having a single function that
// acts as a loop prevents us to store state in the stack of said function. So
// we need some location for this.
SDL_Window   *g_Window    = NULL;
SDL_GLContext g_GLContext = NULL;

class AppState {
public:
    std::vector<Subscription> _subscriptions;
    Plotter                   _plotter;

    AppState(std::vector<Subscription> &subscriptions) {
        _subscriptions = subscriptions;
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
    g_Window                     = SDL_CreateWindow("Pulsed Power Monitoring", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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

    // Load Fonts
    // io.Fonts->AddFontDefault();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);
#endif

    Subscription              grSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "sinus@4000Hz", "square@4000Hz" });
    Subscription              powerSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "saw@4000Hz" });
    Subscription              frequencySubscription("http://localhost:8080/pulsed_power_freq/AcquisitionSpectra?channelNameFilter=", { "sinus_fft@32000Hz" });
    std::vector<Subscription> subscriptions = { grSubscription, powerSubscription, frequencySubscription };
    AppState                  appState(subscriptions);

    // This function call won't return, and will engage in an infinite loop, processing events from the browser, and dispatching them.
    emscripten_set_main_loop_arg(main_loop, &appState, 25, true);
}

static void main_loop(void *arg) {
    ImGuiIO &io = ImGui::GetIO();

    // Parse arguments from main
    AppState                  *args          = static_cast<AppState *>(arg);
    std::vector<Subscription> &subscriptions = args->_subscriptions;
    Plotter                   &plotter       = args->_plotter;

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool   show_demo_window = false;
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

    // Pulsed Power Monitoring Dashboard
    {
        for (Subscription &sub : subscriptions) {
            sub.fetch();
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_None);
        ImGui::Begin("Pulsed Power Monitoring");

        ImGui::ShowStyleSelector("Colors##Selector");

        static ImPlotSubplotFlags flags     = ImPlotSubplotFlags_NoTitle;
        static int                rows      = 2;
        static int                cols      = 2;
        static float              rratios[] = { 1, 1, 1, 1 };
        static float              cratios[] = { 1, 1, 1, 1 };
        if (ImPlot::BeginSubplots("My Subplots", rows, cols, ImVec2(-1, (window_height * 2 / 3) - 30), flags, rratios, cratios)) {
            // GR Signals Plot
            if (ImPlot::BeginPlot("GR Signals")) {
                plotter.plotGrSignals(subscriptions[0].signals);
                ImPlot::EndPlot();
            }

            // Bandpass Filter Plot
            if (ImPlot::BeginPlot("U/I Bandpass Filter")) {
                plotter.plotBandpassFilter(subscriptions[0].signals);
                ImPlot::EndPlot();
            }

            // Power Plot
            if (ImPlot::BeginPlot("Power")) {
                plotter.plotPower(subscriptions[1].signals);
                ImPlot::EndPlot();
            }

            // Mains Frequency Plot
            if (ImPlot::BeginPlot("Mains Frequency")) {
                plotter.plotMainsFrequency(subscriptions[1].signals);
                ImPlot::EndPlot();
            }
        }
        ImPlot::EndSubplots();

        // Power Spectrum
        if (ImPlot::BeginPlot("Power Spectrum")) {
            plotter.plotPowerSpectrum(subscriptions[2].signals);
            ImPlot::EndPlot();
        }
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
