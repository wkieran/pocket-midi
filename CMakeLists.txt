cmake_minimum_required(VERSION 3.13)

include(cmake/pico_sdk_import.cmake)

project(pocket_midi C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# write build/compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/freertos)

# RP2350 Cortex-M33 port, non-TrustZone. Lives in the nested
# Community-Supported-Ports submodule (git submodule update --init --recursive).
add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ FREERTOS_KERNEL)

pico_sdk_init()

add_executable(pocket_midi
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(pocket_midi PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(pocket_midi PRIVATE
    pico_stdlib
    tinyusb_device
    pico_unique_id
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    hardware_i2c
)

pico_add_extra_outputs(pocket_midi)

# -- stdio --------------------------------------------------------------------

# stdout goes over USB CDC via a custom TinyUSB stdio driver in src/.
# Both SDK stdio backends are left off so they don't fight that driver.
pico_enable_stdio_uart(pocket_midi 0)
pico_enable_stdio_usb(pocket_midi 0)
