cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pocket-midi C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/freertos)

add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040 FREERTOS_KERNEL)

pico_sdk_init()

add_executable(pocket_midi
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(pocket_midi PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(pocket_midi PRIVATE
    pico_stdlib
    tinyusb_device
    tinyusb_board
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    hardware_i2c
)

pico_add_extra_outputs(pocket_midi)

# -- stdio: uncomment one block -----------------------------------------------

# USB CDC — stdout via TinyUSB CDC, no adapter needed:
pico_enable_stdio_uart(pocket_midi 0)
pico_enable_stdio_usb(pocket_midi 0)
target_compile_definitions(pocket_midi PRIVATE STDIO_USB_CDC)

# UART — stdout via TX/RX pins, needs a USB-to-serial adapter:
# pico_enable_stdio_uart(pocket_midi 1)
# pico_enable_stdio_usb(pocket_midi 0)
