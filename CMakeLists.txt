cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pocket_midi C CXX ASM)

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

# -- stdio --------------------------------------------------------------------

# stdout goes over USB CDC via the custom TinyUSB stdio driver in src/main.c.
# Both SDK stdio backends are left off so they don't fight that driver.
pico_enable_stdio_uart(pocket_midi 0)
pico_enable_stdio_usb(pocket_midi 0)
