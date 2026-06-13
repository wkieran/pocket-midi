#include <FreeRTOS.h>
#include <string.h>
#include "pico/unique_id.h"
#include "tusb.h"
#include "class/audio/audio.h"
#include "class/midi/midi.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

#ifdef STDIO_USB_CDC
#define USB_DEVICE_CLASS    0xEF  // Miscellaneous — required when using IAD (CDC)
#define USB_DEVICE_SUBCLASS 0x02
#define USB_DEVICE_PROTOCOL 0x01
#else
#define USB_DEVICE_CLASS    0x00
#define USB_DEVICE_SUBCLASS 0x00
#define USB_DEVICE_PROTOCOL 0x00
#endif

static tusb_desc_device_t const desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = 0x0200,
    .bDeviceClass    = USB_DEVICE_CLASS,
    .bDeviceSubClass = USB_DEVICE_SUBCLASS,
    .bDeviceProtocol = USB_DEVICE_PROTOCOL,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor  = 0xcafe,
    .idProduct = 0x4062,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct      = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_device; }

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#ifdef STDIO_USB_CDC

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_MIDI,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define EPNUM_CDC_NOTIF  0x81
#define EPNUM_CDC_OUT    0x02
#define EPNUM_CDC_IN     0x82
#define EPNUM_MIDI_OUT   0x03
#define EPNUM_MIDI_IN    0x83

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)

static uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

#else

enum {
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define EPNUM_MIDI_OUT 0x01
#define EPNUM_MIDI_IN  0x81

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

static uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

#endif

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

enum {
    STRID_LANGID       = 0,
    STRID_MANUFACTURER = 1,
    STRID_PRODUCT      = 2,
    STRID_SERIAL       = 3,
#ifdef STDIO_USB_CDC
    STRID_CDC          = 4,
#endif
};

static char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04},
    "kieran",
    "pocket-midi",
    NULL,
#ifdef STDIO_USB_CDC
    "pocket-midi cdc",
#endif
};

static uint16_t _desc_str[32 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t chr_count;

    switch (index) {
    case STRID_LANGID:
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
        break;

    case STRID_SERIAL: {
        pico_unique_board_id_t pico_id;
        pico_get_unique_board_id(&pico_id);
        chr_count = 16;
        for (int i = 0; i < 8; i++) {
            uint8_t byte             = pico_id.id[i];
            _desc_str[1 + i * 2]     = "0123456789ABCDEF"[(byte >> 4) & 0xF];
            _desc_str[1 + i * 2 + 1] = "0123456789ABCDEF"[byte & 0xF];
        }
        break;
    }

    default: {
        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
            return NULL;
        }

        const char *str        = string_desc_arr[index];
        chr_count              = strlen(str);
        const size_t max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
        if (chr_count > max_count) {
            chr_count = max_count;
        }

        for (size_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
        break;
    }
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
