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
#include <fair_header.h>
#include <IconsFontAwesome6.h>
#include <plot_tools.h>

#define ELECTRICY_PRICE 30

class AppStateNilm {
public:
    SDL_Window                               *window    = nullptr;
    SDL_GLContext                             GLContext = nullptr;
    std::vector<Subscription<PowerUsage>>     subscriptionsPowerUsage;
    std::vector<Subscription<PowerUsageWeek>> subscriptionsPowerUsageWeek;
    std::vector<Subscription<Acquisition>>    subscriptionsTimeDomain;
    std::vector<Subscription<RealPowerUsage>> subscriptionsRealPowerUsage;
    Plotter::DataInterval                     Interval;

    struct AppFonts {
        ImFont *title;
        ImFont *text;
        ImFont *fontawesome;
    };
    AppStateNilm::AppFonts fonts{};

    AppStateNilm(std::vector<Subscription<PowerUsage>> &_powerUsages,
            std::vector<Subscription<PowerUsageWeek>>  &_subscriptionsPowerUsageWeek,
            std::vector<Subscription<Acquisition>>     &_subscriptionsTimeDomain,
            std::vector<Subscription<RealPowerUsage>>  &_subscriptionRealPowerUsage,
            Plotter::DataInterval                       _Interval) {
        this->subscriptionsPowerUsage     = _powerUsages;
        this->subscriptionsPowerUsageWeek = _subscriptionsPowerUsageWeek;
        this->subscriptionsTimeDomain     = _subscriptionsTimeDomain;
        this->subscriptionsRealPowerUsage = _subscriptionRealPowerUsage;
        this->Interval                    = _Interval;
    }
};

enum ColorTheme { Light,
    Dark };

// Emscripten requires to have full control over the main loop. We're going to
// store our SDL book-keeping variables globally. Having a single function that
// acts as a loop prevents us to store state in the stack of said function. So
// we need some location for this.

static void main_loop(void *);

int         main(int argc, char **argv) {
            // Set color theme via query parameters of url
    Plotter::DataInterval Interval            = Plotter::Short;
    double                sampRate            = 100;
    float                 updateFreq          = 25.0f;
    ColorTheme            ColorTheme          = Light;
    std::string           integrationInterval = "Day";
    for (int i = 0; i < argc; i++) {
                std::string arg = argv[i];
                if (arg.find("interval") != std::string::npos) {
                    if (arg.find("short") != std::string::npos) {
                        Interval            = Plotter::Short;
                        sampRate            = 100;
                        updateFreq          = 25.0f;
                        integrationInterval = "Day";
            } else if (arg.find("mid") != std::string::npos) {
                        Interval            = Plotter::Mid;
                        sampRate            = 1;
                        updateFreq          = 1.0f;
                        integrationInterval = "Week";
            } else if (arg.find("long") != std::string::npos) {
                        Interval            = Plotter::Long;
                        sampRate            = 0.016666668;
                        updateFreq          = 0.1f;
                        integrationInterval = "Month";
            }
        }
                if (arg.find("color") != std::string::npos) {
                    if (arg.find("light") != std::string::npos) {
                        ColorTheme = Light;
            } else if (arg.find("dark") != std::string::npos) {
                        ColorTheme = Dark;
            }
        }
    }

    Subscription<PowerUsage>     nilmSubscription("http://localhost:8081/", { "nilm_predict_values" }, 0, 1, updateFreq);
    Subscription<PowerUsageWeek> nilmWeekSubscription("http://localhost:8081/", { "nilm_week_values" }, 0, 1, updateFreq);
    Subscription<RealPowerUsage> integratedValues("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "P_Int_" + integrationInterval, "S_Int_" + integrationInterval }, 1, 1, updateFreq);
    Subscription<Acquisition>    powerSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "P", "Q", "S", "phi" }, sampRate, 30'000, updateFreq);

    // Subscription<PowerUsage>     nilmSubscription("http://10.0.0.2:8081/", { "nilm_predict_values" }, 0, 1, updateFreq);
    // Subscription<RealPowerUsage> integratedValues("http://10.0.0.2:8080/pulsed_power/Acquisition?channelNameFilter=", { "P_Int_" + integrationInterval, "S_Int_" + integrationInterval }, 1, 1, updateFreq);
    // Subscription<Acquisition>    powerSubscription("http://10.0.0.2:8080/pulsed_power/Acquisition?channelNameFilter=", { "P", "Q", "S", "phi" }, sampRate, 30'000, updateFreq);

    std::vector<Subscription<PowerUsage>>     subscriptionsPowerUsage     = { nilmSubscription };
    std::vector<Subscription<Acquisition>>    subscriptionsTimeDomain     = { powerSubscription };
    std::vector<Subscription<RealPowerUsage>> subscriptionRealPowerUsage  = { integratedValues };
    std::vector<Subscription<PowerUsageWeek>> subscriptionsPowerUsageWeek = { nilmWeekSubscription };
    AppStateNilm                              appState(subscriptionsPowerUsage, subscriptionsPowerUsageWeek, subscriptionsTimeDomain, subscriptionRealPowerUsage, Interval);

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
    auto window_flags  = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    appState.window    = SDL_CreateWindow("Non-Intrusive Load Monitoring", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    appState.GLContext = SDL_GL_CreateContext(appState.window);
    if (!appState.GLContext) {
                fprintf(stderr, "Failed to initialize WebGL context!\n");
                return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    //(void) io;

    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(appState.window, appState.GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Colors
    ImGui::StyleColorsLight();
    if (ColorTheme == Dark) {
                ImGui::StyleColorsDark();
    }

    const auto fontname = "assets/xkcd-script/xkcd-script.ttf"; // engineering font
    // const auto fontname = "assets/liberation_sans/LiberationSans-Regular.ttf"; // final font
    appState.fonts.text  = io.Fonts->AddFontFromFileTTF(fontname, 18.0f);
    appState.fonts.title = io.Fonts->AddFontFromFileTTF(fontname, 32.0f);
    // appState.fonts.mono = io.Fonts->AddFontFromFileTTF("", 16.0f);

    ImVector<ImWchar>        symbols;
    ImFontGlyphRangesBuilder builder;
    builder.AddText(ICON_FA_TRIANGLE_EXCLAMATION);
    builder.AddText(ICON_FA_CIRCLE_QUESTION);
    builder.BuildRanges(&symbols);
    appState.fonts.fontawesome = io.Fonts->AddFontFromFileTTF("assets/fontawesome/fa-solid-900.ttf", 32.0f, nullptr, symbols.Data);
    // appState.fonts.fontawesome = io.Fonts->AddFontFromFileTTF("assets/fontawesome/fa-regular.ttf", 16.0f);

    app_header::load_header_assets();

    // time format
    ImPlot::GetStyle().UseISO8601     = true;
    ImPlot::GetStyle().UseLocalTime   = true;
    ImPlot::GetStyle().Use24HourClock = true;

    // This function call won't return, and will engage in an infinite loop, processing events from the browser, and dispatching them.

    emscripten_set_main_loop_arg(main_loop, &appState, 0, true);

    SDL_GL_SetSwapInterval(1); // Enable vsync
}

static void main_loop(void *arg) {
    ImGuiIO &io = ImGui::GetIO();

    // Parse arguments from main
    AppStateNilm                              *args                         = static_cast<AppStateNilm *>(arg);
    std::vector<Subscription<PowerUsage>>     &subscriptionsPowerUsages     = args->subscriptionsPowerUsage;
    std::vector<Subscription<PowerUsageWeek>> &subscriptionsPowerUsagesWeek = args->subscriptionsPowerUsageWeek;
    std::vector<Subscription<Acquisition>>    &subscriptionsTimeDomain      = args->subscriptionsTimeDomain;
    std::vector<Subscription<RealPowerUsage>> &subscriptionsRealPowerUsages = args->subscriptionsRealPowerUsage;
    Plotter::DataInterval                     &Interval                     = args->Interval;

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool show_demo_window = false;
    // static bool   show_demo_window = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Layout options
    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImVec2               window_center = main_viewport->GetWorkCenter();
    float                window_height = 2 * window_center.y;
    float                window_width  = 2 * window_center.x;

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
        for (Subscription<PowerUsage> &powerUsage : subscriptionsPowerUsages) {
            powerUsage.fetch();
        }
        for (Subscription<PowerUsageWeek> &powerUsageWeek : subscriptionsPowerUsagesWeek) {
            powerUsageWeek.fetch();
        }
        for (Subscription<Acquisition> &subTime : subscriptionsTimeDomain) {
            subTime.fetch();
        }
        for (Subscription<RealPowerUsage> &realPower : subscriptionsRealPowerUsages) {
            realPower.fetch();
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_None);
        ImGui::Begin("Electricity", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

        app_header::draw_header_bar("Non-Intrusive Load Monitoring", args->fonts.title);

        static ImPlotSubplotFlags flags      = ImPlotSubplotFlags_NoTitle;
        static int                rows       = 1;
        static int                cols       = 2;

        static ImGuiTableFlags    tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody;

        if (ImGui::BeginTable("ComboStyle", 2, tableFlags, ImVec2(-1, 0))) {
            ImGui::TableSetupColumn("style", ImGuiTableColumnFlags_WidthFixed, 400.0f); // Default to 100.0f
            ImGui::TableSetupColumn("empty");
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::EndTable();
        }

        if (ImPlot::BeginSubplots("My Subplots", rows, cols, ImVec2(-1, 400), flags)) {
            // Power plot
            if (ImPlot::BeginPlot("Power")) {
                Plotter::plotPower(subscriptionsTimeDomain[0].acquisition.buffers, Interval);
                ImPlot::EndPlot();
            }

            // Barchart for usage of the last 7 days
            std::vector<double> weekValues = subscriptionsPowerUsagesWeek[0].acquisition.getValues();
            std::reverse(weekValues.begin(), weekValues.end());
            Plotter::plotBarchart(weekValues);

            ImPlot::EndSubplots();
        }

        ImGui::Spacing();

        RealPowerUsage realPowerUsage   = subscriptionsRealPowerUsages[0].acquisition;
        double         integratedValue  = realPowerUsage.realPowerUsages[1];
        PowerUsage     powerUsageValues = subscriptionsPowerUsages[0].acquisition;

        if (realPowerUsage.init) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 125, 0, 255));

            if (Interval == Plotter::Long) {
                ImGui::Text("EURO %.2f / %.2f kWh used current month\n",
                        ELECTRICY_PRICE * integratedValue, integratedValue);
            } else if (Interval == Plotter::Mid) {
                ImGui::Text("EURO %.2f / %.2f kWh used current week\n",
                        ELECTRICY_PRICE * integratedValue, integratedValue);
            } else {
                ImGui::Text("EURO %.2f / %.2f kWh used today\n",
                        ELECTRICY_PRICE * integratedValue, integratedValue);
            }

            ImGui::PopStyleColor();

            if (realPowerUsage.success) {
                ImGui::Text(" ");
            } else {
                char buff[32];
                char buff_time[32];

                ImPlot::FormatDate(ImPlotTime::FromDouble(realPowerUsage.deliveryTime), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
                ImPlot::FormatTime(ImPlotTime::FromDouble(realPowerUsage.deliveryTime), buff_time, 32, ImPlotTimeFmt_HrMinSMs, ImPlot::GetStyle().Use24HourClock);
                ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "Server not available - Last delivery time %s %s", buff, buff_time);
            }
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", "Connection Error\n");
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", "Server not available");
        }

        Plotter::plotTable(powerUsageValues, Interval, integratedValue);

        ImGui::End();
    }

    // Show ImGui and ImPlot demo windows
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
        ImPlot::ShowDemoWindow();
    }

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(args->window, args->GLContext);
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(args->window);
}
