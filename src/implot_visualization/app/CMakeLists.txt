target_sources(${ppmOutputFileName} PRIVATE main_ppm.cpp)
target_link_libraries(
    ${ppmOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

target_sources(${nilmOutputFileName} PRIVATE main_nilm.cpp)
target_link_libraries(
    ${nilmOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

add_subdirectory(deserializer)
add_subdirectory(fetch)
add_subdirectory(plot_tools)
add_subdirectory(app_header)
