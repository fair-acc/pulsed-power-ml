target_sources(index PRIVATE main.cpp)


# TO DO 
target_include_directories(${outputFileName}
    PRIVATE
    $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:implot,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>
)

# link everything together
target_link_libraries(
    ${outputFileName}
    imgui
    implot
    nlohmann_json
)