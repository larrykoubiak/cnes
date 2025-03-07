cmake_minimum_required(VERSION 3.10)
project(cnes C)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_DEBUGGER "gdb")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(cnes
    main.c
    cpu.c
    bus.c
    ines.c
    mapper.c
    cartridge.c
    ppu.c
)
