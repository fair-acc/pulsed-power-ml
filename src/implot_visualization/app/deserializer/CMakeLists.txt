add_library(
    deserializer OBJECT
        "deserialize_json.cpp"
)

target_include_directories(
    deserializer BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    deserializer
    PUBLIC
        imgui
        nlohmann_json
        serialiser
)
