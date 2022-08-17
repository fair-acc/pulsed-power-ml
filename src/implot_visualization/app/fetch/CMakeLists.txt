add_library(
    fetch OBJECT
        "emscripten_fetch.cpp"
)

target_include_directories(
    fetch BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    fetch
    PUBLIC
        deserializer
)
