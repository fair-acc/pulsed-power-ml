set(TARGETNAME deserializer)

add_library(
    ${TARGETNAME} OBJECT 
        "deserialize_json.cpp"
)

target_include_directories(
    ${TARGETNAME} BEFORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
        $<TARGET_PROPERTY:nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    ${TARGETNAME}
    PRIVATE
        imgui
        nlohmann_json
)