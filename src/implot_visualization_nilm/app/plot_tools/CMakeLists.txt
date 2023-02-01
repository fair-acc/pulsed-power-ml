add_library(plot_tools INTERFACE "plot_tools.h")
target_include_directories(plot_tools INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(plot_tools INTERFACE deserializer implot)

