set(TARGETNAME imgui)

add_library(
    ${TARGETNAME} OBJECT
        imgui_demo.cpp 
        imgui_draw.cpp 
        imgui_impl_opengl3.cpp
        imgui_impl_sdl.cpp 
        imgui_tables.cpp 
        imgui_widgets.cpp 
        imgui.cpp
)

target_include_directories(
    ${TARGETNAME} BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)