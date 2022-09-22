include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        cpp-httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.11.2 # latest v0.11.2
)
FetchContent_MakeAvailable(cpp-httplib)

FetchContent_Declare(
        opencmw-cpp
        GIT_REPOSITORY https://github.com/fair-acc/opencmw-cpp.git
        GIT_TAG main # todo: use proper release once available
)

FetchContent_MakeAvailable(opencmw-cpp)

FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)