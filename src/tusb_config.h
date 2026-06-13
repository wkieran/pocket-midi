#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

/* --- Controller --- */

/* Tells TinyUSB which USB hardware to use. RP2040 value covers RP2350 too —
   pico-sdk maps this to the correct peripheral regardless of chip revision. */
#define CFG_TUSB_MCU                            OPT_MCU_RP2040

/* --- Device mode --- */

/* Enable USB device mode (we are the peripheral, host is the DAW/PC). */
#define CFG_TUD_ENABLED                         1

/* --- Class drivers --- */

/* One MIDI interface. Exposes a single bidirectional MIDI stream over USB. */
#define CFG_TUD_MIDI                            1

/* MIDI RX/TX FIFO depth in bytes. 64 bytes = one full USB full-speed packet.
   Increase if you're sending bursts of events faster than the host polls. */
#define CFG_TUD_MIDI_RX_BUFSIZE                 64
#define CFG_TUD_MIDI_TX_BUFSIZE                 64

#ifdef STDIO_USB_CDC
#define CFG_TUD_CDC                             1
#define CFG_TUD_CDC_RX_BUFSIZE                  64
#define CFG_TUD_CDC_TX_BUFSIZE                  64
#else
#define CFG_TUD_CDC                             0
#endif

#define CFG_TUD_MSC                             0
#define CFG_TUD_HID                             0
#define CFG_TUD_VENDOR                          0

/* --- OS / task integration --- */

/* CFG_TUSB_OS is intentionally not defined here — pico-sdk sets it to
   OPT_OS_FREERTOS via compiler flags when FreeRTOS is detected. Defining it
   here too causes a redefinition warning and risks diverging from what the
   SDK expects. */

/* Depth of TinyUSB's internal event queue. 16 events is sufficient for MIDI. */
#define CFG_TUD_TASK_QUEUE_SZ                   16

/* --- Debug --- */

/* 0 = off. Set to 1 or 2 for USB stack debug output over UART during bringup. */
#define CFG_TUSB_DEBUG                          0

/* USB port 0 in device mode — we are the peripheral. */
#define CFG_TUSB_RHPORT0_MODE    OPT_MODE_DEVICE

#include "FreeRTOSConfig.h"

#endif /* TUSB_CONFIG_H */
