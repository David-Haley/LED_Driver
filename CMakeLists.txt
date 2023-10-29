cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project (addressable_led C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_library (addressable_led STATIC addressable_led.cpp ws2812.pio)

# generate the header file
pico_generate_pio_header (addressable_led
                          /home/david/Christmas/LED_Driver/ws2812.pio)

target_link_libraries(addressable_led
                      pico_stdlib
                      hardware_pio
                      hardware_dma)
