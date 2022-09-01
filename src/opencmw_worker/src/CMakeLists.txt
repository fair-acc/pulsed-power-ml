cmrc_add_resource_library(
    REST_SERVER_ASSETS ALIAS assets::rest NAMESPACE assets
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/error.mustache
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/main.css
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/english.png
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/german.png
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/logo_b.png
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/logo_b.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/logo_w.png
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/img/logo_w.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/mustache/default.mustache
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/mustache/ServicesList.mustache
    )

message("COPY ${CMAKE_SOURCE_DIR}/demo_sslcert/demo_private.key DESTINATION ${CMAKE_CURRENT_BINARY_DIR}" )
file(COPY "${CMAKE_SOURCE_DIR}/demo_sslcert/demo_private.key" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(COPY "${CMAKE_SOURCE_DIR}/demo_sslcert/demo_public.crt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

add_executable(PulsedPowerService PulsedPowerService.cpp)
target_link_libraries(PulsedPowerService PRIVATE 
    majordomo
    disruptor 
    project_options 
    project_warnings 
    assets::rest
    gnuradio::gnuradio-analog
    gnuradio::gnuradio-blocks
    gnuradio-pulsed_power)
