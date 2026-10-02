cmake_minimum_required(VERSION 3.30)
project(cnes_synth C)

# Standalone: reuse cnes's SDL version, but fetch no emulator/SDL_ttf code.
include(FetchContent)
FetchContent_Declare(SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL)

add_executable(synth synth.c)
target_compile_features(synth PRIVATE c_std_99)
target_link_libraries(synth PRIVATE SDL3::SDL3)
