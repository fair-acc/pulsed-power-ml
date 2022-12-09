include(FetchContent)

FetchContent_Declare(
        opencmw-cpp
        GIT_REPOSITORY https://github.com/fair-acc/opencmw-cpp.git
        # GIT_TAG main
        GIT_TAG b5a1d125783ec8d8da62f7266bc7ebb63cc34ecb # todo: use proper release once available
)

FetchContent_MakeAvailable(opencmw-cpp)

FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)