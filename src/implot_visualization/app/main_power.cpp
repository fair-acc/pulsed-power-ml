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

//todo: needed?
class AppState {
public:
    SDL_Window                                   *window    = nullptr;
    SDL_GLContext                                 GLContext = nullptr;
    std::vector<Subscription<Acquisition>>        vectorPowerSubscription;

    Plotter::DataInterval                         Interval;
    struct AppFonts {
        ImFont *title;
        ImFont *text;
        ImFont *fontawesome;
    };
    AppState::AppFonts fonts{};

    AppState(std::vector<Subscription<Acquisition>>       &_vectorPowerSubscription,
                Plotter::DataInterval                          _Interval) {
        this->vectorPowerSubscription    = _vectorPowerSubscription;
        this->Interval                   = _Interval;
    }
};

enum ColorTheme { Light,
    Dark };

static void main_loop(void *);
int         main(int argc, char **argv) {
    // Read query parameters
    Plotter::DataInterval Interval            = Plotter::Short;
    int                   timeRange           = 300;
    double                sampRate            = 100;
    float                 updateFreq          = 25.0f;
    ColorTheme            ColorTheme          = Light;


    // Setup subscriptions
    // sample rate must equal sink sample rate, F12 for constructor, (8080 -> worker, PulsedPowerservice)
    // subscription to retrieve P, Q, S data for all phases
    Subscription<Acquisition> powerSubscription("http://localhost:8080/pulsed_power/Acquisition?channelNameFilter=", 
                            { "P_0", "Q_0", "S_0", "P_1", "Q_1", "S_1", "P_2", "Q_2", "S_2", "P_acc", "Q_acc", "S_acc" }, 
                                                                            sampRate, timeRange * sampRate, updateFreq);
//    Subscription(const std::string &_url, const std::vector<std::string> &_requestedSignals, const double _sampRate, const int _bufferSize, const float _updateFrequency);
   
    std::vector<Subscription<Acquisition>>        vectorPowerSubscription    = { powerSubscription };
    AppState                                      appState(vectorPowerSubscription, Interval);

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

    ImPlot::GetStyle().UseISO8601     = true;
    ImPlot::GetStyle().UseLocalTime   = true;
    ImPlot::GetStyle().Use24HourClock = true;

    emscripten_set_main_loop_arg(main_loop, &appState, 0, true);

    SDL_GL_SetSwapInterval(1); // Enable vsync
}

static void main_loop(void *arg) {
    ImGuiIO &io = ImGui::GetIO();

    // Parse arguments from main
    auto                                          *args                        = static_cast<AppState *>(arg);
    std::vector<Subscription<Acquisition>>        &vectorPowerSubscription     = args->vectorPowerSubscription;
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
        for (Subscription<Acquisition> &subPower : vectorPowerSubscription) {
            subPower.fetch();
        }


        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_None);
        ImGui::Begin("Pulsed Power Monitoring", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        app_header::draw_header_bar("PowerMonitoring", args->fonts.title);

        static ImPlotSubplotFlags flags     = ImPlotSubplotFlags_NoTitle;
        static int                rows      = 1;
        static int                cols      = 2;
        static float              rratios[] = { 1, 1, 1, 1 };
        static float              cratios[] = { 1, 1, 1, 1 };

        
        // plot P_0, Q_0, S_0
        if (ImPlot::BeginPlot("P, Q, S Phase 0")) {
            if (vectorPowerSubscription.size() >= 1) {
                printf("%s \n", vectorPowerSubscription[0].acquisition.buffers.data()->signalName.c_str());
                Plotter::plotPowerPhaseZero(vectorPowerSubscription[0].acquisition.buffers, Interval);
            }
            ImPlot::EndPlot();
        }
        // plot P_1, Q_1, S_1
        if (ImPlot::BeginPlot("P, Q, S Phase 1")) {
            if (vectorPowerSubscription.size() >= 1) {
                Plotter::plotPowerPhaseOne(vectorPowerSubscription[0].acquisition.buffers, Interval);
            }
            ImPlot::EndPlot();
        }
        // plot P_2, Q_2, S_2
        if (ImPlot::BeginPlot("P, Q, S Phase 2")) {
            if (vectorPowerSubscription.size() >= 1) {
                Plotter::plotPowerPhaseTwo(vectorPowerSubscription[0].acquisition.buffers, Interval);
            }
            ImPlot::EndPlot();
        }
        //plot P_acc, Q_acc, S_acc
        if (ImPlot::BeginPlot("P, Q, S accumulated")) {
            if (vectorPowerSubscription.size() >= 1) {
                Plotter::plotPowerAcc(vectorPowerSubscription[0].acquisition.buffers, Interval);
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