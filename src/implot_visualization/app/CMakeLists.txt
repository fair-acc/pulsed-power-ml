target_sources(index PRIVATE main.cpp)

add_subdirectory(deserializer)
add_subdirectory(fetch)

# link the executable
target_link_libraries(
    ${outputFileName}
    PRIVATE
        implot
        deserializer
        fetch
)
