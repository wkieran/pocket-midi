# pocket-midi

A pocket-sized midi controller based on the RP2350 MCU, and using the Seeed Studio XIAO RP2350. Built with FreeRTOS, tiny-usb, and pico-sdk. I'm also using a Pico 2 as a development board, it shares the same MCU.

## dependencies
- `arm-none-eabi-gcc`
- `cmake`
- `picotool`
- pico-sdk with `PICO_SDK_PATH` set

## how to build
```
git submodule update --init
cmake -B build -DPICO_BOARD=seeed_xiao_rp2350
cmake --build build
```

## how to flash
hold BOOTSEL, plug into your computer, mount the `RP2350` volume, and copy the `.uf2` file from `build/` onto the drive.

## project status
first phase - build tooling, FreeRTOS boots, USB descriptions
