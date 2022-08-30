#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include <cmath>
#include <deserialize_json.h>
#include <emscripten_fetch.h>
#include <implot.h>
#include <iostream>
#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdio.h>
#include <string.h>

ScrollingBuffer buffer;

// Emscripten requires to have full control over the main loop. We're going to
// store our SDL book-keeping variables globally. Having a single function that
// acts as a loop prevents us to store state in the stack of said function. So
// we need some location for this.
SDL_Window   *g_Window    = NULL;
SDL_GLContext g_GLContext = NULL;

static void   main_loop(void *);

int           main(int, char **) {
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
    g_Window                     = SDL_CreateWindow("OpenCMW Sinus Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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

    // This function call won't return, and will engage in an infinite loop, processing events from the browser, and dispatching them.
    emscripten_set_main_loop_arg(main_loop, NULL, 60, true);
}

static void main_loop(void *arg) {
    fetch("http://localhost:8080/timeDomainWorker");

    ImGuiIO &io = ImGui::GetIO();
    IM_UNUSED(arg); // We can pass this argument as the second parameter of emscripten_set_main_loop_arg(), but we don't use that.

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool   show_demo_window = false;
    static ImVec4 clear_color      = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

    // Show counter demo
    {
        ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
        ImGui::Begin("Sinus Demo Window");
        if (ImPlot::BeginPlot("Sinus Sink")) {
            ImPlot::SetupAxes("UTC Time", "Value");
            ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2, 1.2);
            if (buffer.Data.size() > 0) {
                int bufferEnd = buffer.Data.size() - 1;
                std::cout << "Buffer size ImPlot: " << buffer.Data.size() << std::endl;
                // ImPlot::SetupAxisLimits(ImAxis_X1, buffer.Data[bufferEnd].x - std::pow(10, 8), buffer.Data[bufferEnd].x, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_X1, buffer.Data[bufferEnd].x - std::pow(10, 3), buffer.Data[bufferEnd].x, ImGuiCond_Always);
                // ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
                // std::cout << "Reinterpret: " << *reinterpret_cast<const double *>(&buffer.Data[0].x) << std::endl;
                // std::cout << "Static cast: " << *static_cast<double *>(&buffer.Data[0].x) << std::endl;
                // ImPlot::PlotLine("Sinus", &buffer.Data[0].x, &buffer.Data[0].y, buffer.Data.size(), 0, buffer.Offset, 2 * sizeof(uint64_t), 2 * sizeof(float));
                ImPlot::PlotLine("Sinus", &buffer.Data[0].x, &buffer.Data[0].y, buffer.Data.size(), 0, buffer.Offset, 2 * sizeof(float));
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
    }

    // Show demo windows
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
