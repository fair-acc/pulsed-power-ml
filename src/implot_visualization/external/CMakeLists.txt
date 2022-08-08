add_subdirectory(imgui)
add_subdirectory(implot)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         69d744867f8847c91a126fa25e9a6a3d67b3be41 #v3.11.1
)
FetchContent_MakeAvailable(nlohmann_json)