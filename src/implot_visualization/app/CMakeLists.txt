target_sources(${ppmOutputFileName} PRIVATE main_ppm.cpp)

add_subdirectory(deserializer)
add_subdirectory(fetch)
add_subdirectory(plot_tools)
add_subdirectory(app_header)

# link the executable
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

# add_subdirectory(deserializer)
# add_subdirectory(fetch)
# add_subdirectory(plot_tools)
# add_subdirectory(app_header)

# link the executable
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
