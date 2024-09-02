cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico_w)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)
project(soil-sensor C CXX ASM)
pico_sdk_init()

add_executable(soil-sensor
    main.c
)

target_include_directories(soil-sensor PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(soil-sensor 
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_smtp
    pico_lwip_mbedtls
    pico_mbedtls
    pico_unique_id
    hardware_rtc)
pico_add_extra_outputs(soil-sensor)