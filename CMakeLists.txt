cmake_minimum_required(VERSION 3.10)

include(pico_sdk_import.cmake)


project(AS3935 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(AS3935 
    main.c
    ${SOURCES}
)

target_include_directories(AS3935 PRIVATE include)

target_link_libraries(AS3935 pico_stdlib hardware_spi)

