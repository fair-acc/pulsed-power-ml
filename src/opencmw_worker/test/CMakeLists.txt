add_definitions (-fexceptions)

include(Catch)

message("COPY ${CMAKE_SOURCE_DIR}/demo_sslcert/demo_private.key DESTINATION ${CMAKE_CURRENT_BINARY_DIR}" )
file(COPY "${CMAKE_SOURCE_DIR}/demo_sslcert/demo_private.key" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(COPY "${CMAKE_SOURCE_DIR}/demo_sslcert/demo_public.crt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

include_directories(${CMAKE_SOURCE_DIR}/src)

function(opencmw_add_test_app name sources)
    set (test_SRCS ${sources})
    message("Test sources: ${test_SRCS}")
    add_executable(${name} ${test_SRCS})

    target_link_libraries(${name}
        PUBLIC
            opencmw_project_warnings
            opencmw_project_options
            serialiser
            disruptor
            majordomo
            Catch2::Catch2
            gnuradio::gnuradio-analog
            gnuradio::gnuradio-blocks
            gnuradio::gnuradio-pulsed_power
        )
endfunction()

function(opencmw_add_test_catch2 name sources)
    opencmw_add_test_app(${name} "${sources};catch_main.cpp")
    catch_discover_tests(${name})
endfunction()

#opencmw_add_test_catch2(time_domain_worker_rest_tests time_domain_worker_rest_tests.cpp)
opencmw_add_test_catch2(integrator integrator_tests.cpp)

