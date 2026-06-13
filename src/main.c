#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "class/midi/midi_device.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "task.h"
#include "tusb.h"
#include <pico/stdio.h>
#include "notes.h"
#include "hardware/i2c.h"
#include "pico/stdio/driver.h"

// -- config ------------------------------------------------------------------

#define BPM 120
#define SUBDIVISION 1 // 1: quarter notes, 2: eighth notes, 4: sixteenth notes
#define NOTE_MS (60000 / BPM / SUBDIVISION)

#define I2C_BAUD_RATE 100000
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// -- hooks -------------------------------------------------------------------

void vApplicationMallocFailedHook()
{
    while (1) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t task_handle, char *c) { panic(c); }

// -- led task ----------------------------------------------------------------

struct led_task_arg
{
    int gpio;
    int delay;
};

struct led_task_arg led_arg = {PICO_DEFAULT_LED_PIN, 125};

void led_task(void *p)
{
    struct led_task_arg *a = (struct led_task_arg *)p;

    gpio_init(a->gpio);
    gpio_set_dir(a->gpio, GPIO_OUT);
    while (1) {
        gpio_put(a->gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(a->delay));
        gpio_put(a->gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(a->delay));
    }
}

// -- usb cdc stdio driver ----------------------------------------------------

#ifdef STDIO_USB_CDC

static void cdc_out_chars(const char *buf, int length)
{
    if (tud_cdc_connected()) {
        tud_cdc_write(buf, (uint32_t)length);
    }
}

static void cdc_out_flush(void)
{
    if (tud_cdc_connected()) {
        tud_cdc_write_flush();
    }
}

static stdio_driver_t cdc_stdio = {
    .out_chars = cdc_out_chars,
    .out_flush = cdc_out_flush,
};

#endif

// -- usb device task ---------------------------------------------------------

void usb_device_task(void *p)
{
    tusb_init();
    while (1) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// -- midi task ---------------------------------------------------------------

const uint8_t note_sequence[] = {D5,  Fs5, A5,  D6,  Fs6, A6,  D7,  Fs7, A3,  Cs4, Fs4, A4, Cs5,
                                 Fs5, A5,  Cs6, E6,  Gs6, Cs7, E7,  Cs7, Gs6, E6,  Cs6, A5, Fs5,
                                 D5,  A4,  Fs4, D4,  A3,  D4,  Fs4, A4,  D5,  Fs5, A5,  D6, Fs6,
                                 A6,  Cs7, Fs7, Cs7, A6,  Fs6, Cs6, A5,  Fs5, Cs5, Gs4, E4, Cs4,
                                 Gs3, Cs4, E4,  Gs4, D5,  Fs5, A5,  D6,  Fs6, A6,  D7,  Fs7};

void midi_task(void *p)
{
    const uint8_t cable_num = 0;
    const uint8_t channel   = 0;

    uint32_t note_pos = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(NOTE_MS));

        int previous = (int)(note_pos - 1);

        if (previous < 0) {
            previous = sizeof(note_sequence) - 1;
        }
        uint8_t note_off[3] = {0x80 | channel, note_sequence[previous], 0};
        tud_midi_stream_write(cable_num, note_off, 3);

        uint8_t note_on[3] = {0x90 | channel, note_sequence[note_pos], 127};
        tud_midi_stream_write(cable_num, note_on, 3);

        note_pos++;

        if (note_pos >= sizeof(note_sequence)) {
            note_pos = 0;
        }

        while (tud_midi_available()) {
            uint8_t packet[4];
            tud_midi_packet_read(packet);
        }
    }
}

// -- mcp23017 i2c task -------------------------------------------------------

bool reserved_addr(uint8_t addr) { return (addr & 0x78) == 0 || (addr & 0x78) == 0x78; }

void mcp23017_task(void *p)
{
    i2c_init(i2c0, I2C_BAUD_RATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    while (!tud_cdc_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    printf("CDC connected\r\n");

    while (1) {
        // temp snippet to test i2c address gathering
        for (int addr = 0; addr < (1 << 7); ++addr) {
            if (addr % 16 == 0) {
                printf("%02x ", addr);
            }

            // Perform a 1-byte dummy read from the probe address. If a slave
            // acknowledges this address, the function returns the number of bytes
            // transferred. If the address byte is ignored, the function returns
            // -1.

            // Skip over any reserved addresses.
            int ret;
            uint8_t rxdata;
            if (reserved_addr(addr))
                ret = PICO_ERROR_GENERIC;
            else
                ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

            printf(ret < 0 ? "." : "@");
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
        printf("Done.\n");
        // printf("testing!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// -- main --------------------------------------------------------------------

int main()
{
    stdio_init_all();

#ifdef STDIO_USB_CDC
    stdio_set_driver_enabled(&cdc_stdio, true);
#endif

    // xTaskCreate(led_task, "led blinky", configMINIMAL_STACK_SIZE, &led_arg, 1, NULL);
    xTaskCreate(usb_device_task, "usb device", configMINIMAL_STACK_SIZE * 4, NULL,
                configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(midi_task, "midi song", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                NULL);
    xTaskCreate(mcp23017_task, "mcp23017 task", configMINIMAL_STACK_SIZE, NULL,
                configMAX_PRIORITIES - 3, NULL);

    vTaskStartScheduler();

    return 0;
}
