cmake_minimum_required(VERSION 3.13)

message(STATUS "Pico SDK Path: ${PICO_SDK_PATH}")

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(cmake_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(soil-sensor
	soil-sensor.c
)

pico_enable_stdio_uart(soil-sensor 1)

pico_add_extra_outputs(soil-sensor)

target_link_libraries(soil-sensor 
	pico_stdlib
	hardware_adc
	pico_cyw43_arch_none
)
