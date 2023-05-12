include(FetchContent)

FetchContent_Declare(
        opencmw-cpp
        GIT_REPOSITORY https://github.com/fair-acc/opencmw-cpp.git
        GIT_TAG 2d503a65410c09bb30b77dcbedc262455456ac4b #main
)

# FetchContent_Declare(
#         cppflow
#         GIT_REPOSITORY https://github.com/serizba/cppflow.git
#         GIT_TAG v2.0.0 
# )

FetchContent_MakeAvailable(opencmw-cpp)
#FetchContent_MakeAvailable(cppflow)

FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
