target_sources(index PRIVATE main.cpp)

add_subdirectory(deserializer)
add_subdirectory(fetch)

target_include_directories(${outputFileName}
    PRIVATE
    $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:implot,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:deserializer,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:fetch,INTERFACE_INCLUDE_DIRECTORIES>
)

# link the executable
target_link_libraries(
    ${outputFileName}
    imgui
    implot
    nlohmann_json
    deserializer
    fetch
)