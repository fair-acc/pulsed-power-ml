set(TARGETNAME implot)

add_library(
    ${TARGETNAME} OBJECT
        implot_demo.cpp 
        implot_items.cpp 
        implot.cpp 
)

target_include_directories(
    ${TARGETNAME} BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>
)