target_sources(index PRIVATE main.cpp)

add_subdirectory(deserializer)
add_subdirectory(fetch)
add_subdirectory(plot_tools)

# link the executable
target_link_libraries(
    ${outputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
)
