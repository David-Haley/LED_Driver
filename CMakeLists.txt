cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project (addressable_led C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(example_8_x_150)

add_library (addressable_led STATIC addressable_led.cpp ws2812.pio)

# generate the header file
pico_generate_pio_header (addressable_led
                          ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
                          OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

target_sources(example_8_x_150 PRIVATE example_8_x_150.cpp addressable_led.hpp)

target_link_libraries(addressable_led
                      pico_stdlib
                      hardware_pio
                      hardware_dma)

target_link_libraries(example_8_x_150 pico_stdlib hardware_pio addressable_led)

# enable usb output, disable uart output
    pico_enable_stdio_usb(example_8_x_150 0)
# enable serial output, disable uart output (UART0 TX, pin 1)
    pico_enable_stdio_uart(example_8_x_150 1)

pico_add_extra_outputs(example_8_x_150)
