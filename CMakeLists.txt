# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picOS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

option(USE_8080 "Use ST7789 8080 parallel interface" ON)
# Alternatively, use 4-pin SPI. Not recommended as this is slower

# Add executable. Default name is the project name, version 0.1

add_executable(picOS src/picOS.c)
target_include_directories(picOS PRIVATE src)

target_sources(picOS PRIVATE src/drivers/test/button_driver.c)

if (USE_8080)
    target_sources(picOS PRIVATE src/drivers/st7789_8080_driver.c)
    pico_generate_pio_header(picOS ${CMAKE_CURRENT_LIST_DIR}/src/drivers/st7789_8080.pio)
    target_link_libraries(picOS hardware_pio hardware_dma)

    target_compile_definitions(picOS PRIVATE USE_8080=1)
else()
    target_sources(picOS PRIVATE src/drivers/st7789_spi_driver.c)
    target_link_libraries(picOS hardware_spi)

    target_compile_definitions(picOS PRIVATE USE_8080=0)
endif()

pico_enable_stdio_usb(picOS 1)
pico_enable_stdio_uart(picOS 0)

# pull in common dependencies
target_link_libraries(picOS pico_stdlib)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(picOS pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(picOS)

# add url via pico_set_program_url
