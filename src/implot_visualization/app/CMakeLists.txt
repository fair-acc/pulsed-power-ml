target_sources(${ppemOutputFileName} PRIVATE main_ppem.cpp)
target_link_libraries(
    ${ppemOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

target_sources(${nilmOutputFileName} PRIVATE main_nilm.cpp)
target_link_libraries(
    ${nilmOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

# voltage dashboard (U_0, U_1, U_2)
target_sources(${voltageOutputFileName} PRIVATE main_voltage.cpp)
target_link_libraries(
    ${voltageOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)


# current dashboard (I_0, I_1, I_2)
target_sources(${currentOutputFileName} PRIVATE main_current.cpp)
target_link_libraries(
    ${currentOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)
# U0 I0 dashboard
target_sources(${u0i0OutputFileName} PRIVATE main_u0i0.cpp)
target_link_libraries(
    ${u0i0OutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

# power dashboard
target_sources(${powerOutputFileName} PRIVATE main_power.cpp)
target_link_libraries(
    ${powerOutputFileName}
    PRIVATE
        implot
        deserializer
        fetch
        plot_tools
        app_header
        stb
        iconfont
)

add_subdirectory(deserializer)
add_subdirectory(fetch)
add_subdirectory(plot_tools)
add_subdirectory(app_header)
