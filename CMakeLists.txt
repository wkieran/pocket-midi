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
)

pico_add_extra_outputs(pocket_midi)
