cmake_minimum_required(VERSION 3.30)
project(cnes C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# CPM
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set(LIBS)

set(CNES_SOURCES
    src/main.c
    src/apu/apu.c
    src/cpu/cpu.c
    src/cpu/addressing_modes.c
    src/cpu/instructions.c
    src/bus.c
    src/cartridge/ines.c
    src/cartridge/mapper/mapper.c
    src/cartridge/mapper/mapper_000.c
    src/cartridge/mapper/mapper_001.c
    src/cartridge/mapper/mapper_002.c
    src/cartridge/cartridge.c
    src/input/input.c
    src/ppu/ppu.c
    src/ppu/rendering.c
    src/frontend/sdl_context.c
)

CPMAddPackage("gh:libsdl-org/SDL#release-3.2.8")
CPMAddPackage("gh:libsdl-org/SDL_ttf#release-3.2.0")

list(APPEND LIBS SDL3::SDL3-shared)
list(APPEND LIBS SDL3_ttf::SDL3_ttf)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${CNES_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

set(SDL3_DLL_PATH "${CMAKE_BINARY_DIR}/_deps/sdl-build/SDL3.dll")
set(SDL3_TTF_DLL_PATH "${CMAKE_BINARY_DIR}/_deps/sdl_ttf-build/SDL3_ttf.dll")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDL3_DLL_PATH} ${CMAKE_BINARY_DIR}/SDL3.dll
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDL3_TTF_DLL_PATH} ${CMAKE_BINARY_DIR}/SDL3_ttf.dll
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/2C02G_wiki.pal ${CMAKE_BINARY_DIR}/2C02G_wiki.pal
)