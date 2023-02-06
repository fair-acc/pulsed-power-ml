add_definitions (-fexceptions)
include(Catch)

include_directories(${CMAKE_SOURCE_DIR}/app)

function(add_test_app name sources)
    set (test_SRCS ${sources})
    message("Test sources: ${test_SRCS}")
    add_executable(${name} ${test_SRCS})

    target_link_libraries(${name}
        PUBLIC
            Catch2::Catch2
            deserializer
            imgui
            nlohmann_json
        )
endfunction()

function(add_test_catch2 name sources)
    add_test_app(${name} "${sources};catch_main.cpp")
    catch_discover_tests(${name})
endfunction()

add_test_catch2(deserialize_json_tests deserialize_json_tests.cpp)
