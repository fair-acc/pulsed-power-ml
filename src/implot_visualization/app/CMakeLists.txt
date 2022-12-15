target_sources(index PRIVATE main.cpp)

add_subdirectory(deserializer)
add_subdirectory(fetch)
add_subdirectory(plot_tools)
add_subdirectory(app_header)

# link the executable
target_link_libraries(
    ${outputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)
