set(TARGETNAME fetch)

add_library(
    ${TARGETNAME} OBJECT 
        "emscripten_fetch.cpp"
)

target_include_directories(
    ${TARGETNAME} BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        $<TARGET_PROPERTY:deserializer,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    ${TARGETNAME}
    PRIVATE
        deserializer
)