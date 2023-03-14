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

class AppState {
public:
    SDL_Window                                   *window    = nullptr;
    SDL_GLContext                                 GLContext = nullptr;
    std::vector<Subscription<Acquisition>>        subscriptionsTimeDomain;
    std::vector<Subscription<AcquisitionSpectra>> subscriptionsFrequency;
    std::vector<Subscription<RealPowerUsage>>     subscriptionRealPowerUsage;
    Plotter::DataInterval                         Interval;
    struct AppFonts {
        ImFont *title;
        ImFont *text;
        ImFont *fontawesome;
    };
    AppState::AppFonts fonts{};

    AppState(std::vector<Subscription<Acquisition>>       &_subscriptionsTimeDomain,
            std::vector<Subscription<AcquisitionSpectra>> &_subscriptionsFrequency,
            std::vector<Subscription<RealPowerUsage>>     &_subscriptionRealPowerUsage,
            Plotter::DataInterval                          _Interval) {
        this->subscriptionsTimeDomain    = _subscriptionsTimeDomain;
        this->subscriptionsFrequency     = _subscriptionsFrequency;
        this->subscriptionRealPowerUsage = _subscriptionRealPowerUsage;
        this->Interval                   = _Interval;
    }
};

enum ColorTheme { Light,
    Dark };

static void main_loop(void *);

int         main(int argc, char **argv) {
    // Read query parameters
    Plotter::DataInterval Interval   = Plotter::Short;
    std::string           sampRate   = "100Hz";
    float                 updateFreq = 25.0f;
    ColorTheme            ColorTheme = Light;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("interval") != std::string::npos) {
            if (arg.find("short") != std::string::npos) {
                Interval   = Plotter::Short;
                sampRate   = "100Hz";
                updateFreq = 25.0f;
            } else if (arg.find("mid") != std::string::npos) {
                Interval   = Plotter::Mid;
                sampRate   = "1Hz";
                updateFreq = 1.0f;
            } else if (arg.find("long") != std::string::npos) {
                Interval   = Plotter::Long;
                sampRate   = "0.016666668Hz";
                updateFreq = 0.1f;
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

    // Setup subscriptions
    Subscription<Acquisition>        signalSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "U@10000Hz", "I@10000Hz", "U_bpf@10000Hz", "I_bpf@10000Hz" }, 25.0f);
    Subscription<Acquisition>        powerStatsSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "P_mean@" + sampRate, "P_min@" + sampRate, "P_max@" + sampRate, "Q_mean@" + sampRate, "Q_min@" + sampRate, "Q_max@" + sampRate, "S_mean@" + sampRate, "S_min@" + sampRate, "S_max@" + sampRate, "phi_mean@" + sampRate, "phi_min@" + sampRate, "phi_max@" + sampRate }, updateFreq);
    Subscription<Acquisition>        mainsFreqSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "mains_freq@" + sampRate }, updateFreq);
    Subscription<AcquisitionSpectra> frequencySubscription("http://localhost:8080/pulsed_power_freq/AcquisitionSpectra?channelNameFilter=", { "sinus_fft@50Hz" }, 1.0f);
    Subscription<AcquisitionSpectra> limitingCurveSubscription("http://localhost:8080/", { "limiting_curve" }, 1.0f);
    Subscription<RealPowerUsage>     integratedValues("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", { "P_Int@100Hz", "S_Int@100Hz" }, updateFreq);

    // Subscription<Acquisition>                     signalSubscription("http://10.0.0.2:8080/pulsed_power/Acquisition?channelNameFilter=", { "U@1000Hz", "I@1000Hz", "U_bpf@1000Hz", "I_bpf@1000Hz" }, 25.0f);
    // Subscription<Acquisition>                     powerStatsSubscription("http://10.0.0.2:8080/pulsed_power/Acquisition?channelNameFilter=", { "P_mean@" + sampRate, "P_min@" + sampRate, "P_max@" + sampRate, "Q_mean@" + sampRate, "Q_min@" + sampRate, "Q_max@" + sampRate, "S_mean@" + sampRate, "S_min@" + sampRate, "S_max@" + sampRate, "phi_mean@" + sampRate, "phi_min@" + sampRate, "phi_max@" + sampRate }, updateFreq);
    // Subscription<Acquisition>                     mainsFreqSubscription("http://10.0.0.2:8080/pulsed_power/Acquisition?channelNameFilter=", { "mains_freq@" + sampRate }, updateFreq);
    // Subscription<AcquisitionSpectra>              frequencySubscription("http://10.0.0.2:8080/pulsed_power_freq/AcquisitionSpectra?channelNameFilter=", { "sinus_fft@50Hz" }, 1.0f);
    // Subscription<AcquisitionSpectra>              limitingCurveSubscription("http://10.0.0.2:8080/", { "limiting_curve" }, 1.0f);

    std::vector<Subscription<Acquisition>>        subscriptionsTimeDomain    = { signalSubscription, powerStatsSubscription, mainsFreqSubscription };
    std::vector<Subscription<AcquisitionSpectra>> subscriptionsFrequency     = { frequencySubscription, limitingCurveSubscription };
    std::vector<Subscription<RealPowerUsage>>     subscriptionRealPowerUsage = { integratedValues };
    AppState                                      appState(subscriptionsTimeDomain, subscriptionsFrequency, subscriptionRealPowerUsage, Interval);

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
    appState.window    = SDL_CreateWindow("Pulsed Power Monitoring", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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

    // Setup Colors
    ImGui::StyleColorsLight();
    if (ColorTheme == Dark) {
        ImGui::StyleColorsDark();
    }

    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(appState.window, appState.GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

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

    emscripten_set_main_loop_arg(main_loop, &appState, 0, true);

    SDL_GL_SetSwapInterval(1); // Enable vsync
}

static void main_loop(void *arg) {
    ImGuiIO &io = ImGui::GetIO();

    // Parse arguments from main
    auto                                          *args                        = static_cast<AppState *>(arg);
    std::vector<Subscription<Acquisition>>        &subscriptionsTimeDomain     = args->subscriptionsTimeDomain;
    std::vector<Subscription<AcquisitionSpectra>> &subscriptionsFrequency      = args->subscriptionsFrequency;
    std::vector<Subscription<RealPowerUsage>>     &subscriptionRealPowerUsages = args->subscriptionRealPowerUsage;
    Plotter::DataInterval                         &Interval                    = args->Interval;

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool   show_demo_window = false;
    static ImVec4 clear_color      = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

    // Pulsed Power Monitoring Dashboard
    {
        // Fetch signal data
        for (Subscription<Acquisition> &subTime : subscriptionsTimeDomain) {
            subTime.fetch();
        }
        for (Subscription<AcquisitionSpectra> &subFreq : subscriptionsFrequency) {
            subFreq.fetch();
        }
        for (Subscription<RealPowerUsage> &subInt : subscriptionRealPowerUsages) {
            subInt.fetch();
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_None);
        ImGui::Begin("Pulsed Power Monitoring", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        app_header::draw_header_bar("PulsedPowerMonitoring", args->fonts.title);

        static ImPlotSubplotFlags flags     = ImPlotSubplotFlags_NoTitle;
        static int                rows      = 1;
        static int                cols      = 2;
        static float              rratios[] = { 1, 1, 1, 1 };
        static float              cratios[] = { 1, 1, 1, 1 };
        if (ImPlot::BeginSubplots("My Subplots", rows, cols, ImVec2(-1, (window_height * 1 / 3) - 30), flags, rratios, cratios)) {
            // Raw Signals Plot
            static ImPlotColormap Test = -1;
            if (Test == -1) {
                static const ImVec4 blue         = ImVec4(0.298, 0.447, 0.690, 1);
                static const ImVec4 red          = ImVec4(0.769, 0.306, 0.322, 1);
                static const ImVec4 Test_Data[4] = { blue, red, blue, red };
                Test                             = ImPlot::AddColormap("Test", Test_Data, 4);
            }
            ImPlot::PushColormap(Test);
            if (ImPlot::BeginPlot("", ImVec2(), ImPlotFlags_NoLegend)) {
                if (subscriptionsTimeDomain.size() >= 3) {
                    Plotter::plotSignals(subscriptionsTimeDomain[0].acquisition.buffers);
                }
                ImPlot::EndPlot();
                ImPlot::PopColormap();
            }

            // Mains Frequency Plot
            if (ImPlot::BeginPlot("", ImVec2(), ImPlotFlags_NoLegend)) {
                if (subscriptionsTimeDomain.size() >= 3) {
                    Plotter::plotMainsFrequency(subscriptionsTimeDomain[2].acquisition.buffers, Interval);
                }
                ImPlot::EndPlot();
            }

            ImPlot::EndSubplots();
        }
        if (ImPlot::BeginSubplots("My Subplots2", 1, 1, ImVec2(-1, (window_height * 1 / 3) - 30), flags, rratios, cratios)) {
            // Power Statistics
            if (ImPlot::BeginPlot("")) {
                if (subscriptionsTimeDomain.size() > 1) {
                    Plotter::plotStatistics(subscriptionsTimeDomain[1].acquisition.buffers, Interval);
                }
                ImPlot::EndPlot();
            }
            ImPlot::EndSubplots();
        }

        // Integrated Values
        if (ImGui::BeginTable("Integrated Values", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders)) {
            if (subscriptionRealPowerUsages.size() >= 1) {
                Plotter::plotUsageTable(subscriptionRealPowerUsages[0].acquisition.realPowerUsages);
            }
            ImGui::EndTable();
        }

        // Power Spectrum
        if (ImPlot::BeginPlot("Power Spectrum")) {
            if (subscriptionsFrequency.size() >= 2) {
                Plotter::plotPowerSpectrum(subscriptionsFrequency[0].acquisition.buffers, subscriptionsFrequency[1].acquisition.buffers, args->fonts.fontawesome);
            }
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
    SDL_GL_MakeCurrent(args->window, args->GLContext);
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(args->window);
}
