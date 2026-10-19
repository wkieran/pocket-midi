#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "class/midi/midi_device.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "tusb.h"
#include <hardware/gpio.h>
#include <pico/stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "notes.h"
#include "hardware/i2c.h"
#include "pico/stdio/driver.h"

// -- config ------------------------------------------------------------------

#define BPM 120
#define SUBDIVISION 1 // 1: quarter notes, 2: eighth notes, 4: sixteenth notes
#define NOTE_MS (60000 / BPM / SUBDIVISION)

// 100 kHz standard mode. Currently running on the RP2350's weak internal
// pull-ups only.
#define I2C_BAUD_RATE 100000
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// mcp23017 constants
#define MCP23017_ADDR 0x20
#define GPPUB 0x0D
#define GPIOB 0x13
#define IODIRB 0x01

// -- hooks -------------------------------------------------------------------

void vApplicationMallocFailedHook()
{
    while (1) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t task_handle, char *c) { panic(c); }

// -- usb cdc stdio driver ----------------------------------------------------

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

void format_bits(uint8_t n, char *buffer)
{
    for (int i = 7; i >= 0; i--) {
        // Store '1' if bit is true, otherwise store '0'
        buffer[7 - i] = ((n >> i) & 1) ? '1' : '0';
    }
    buffer[8] = '\0'; // Null-terminate the string
}

bool mcp_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};

    int err = i2c_write_timeout_us(i2c0, MCP23017_ADDR, buf, 2, false, 10000);
    if (err < 0) {
        printf("mcp_write_reg: i2c write error %d on %d\r\n", err, reg);
        return false;
    }

    return true;
}

bool mcp_read_reg(uint8_t reg, uint8_t *rxdata)
{
    int err = i2c_write_timeout_us(i2c0, MCP23017_ADDR, &reg, 1, true, 10000);
    if (err < 0) {
        printf("mcp_read_reg: i2c write error %d on %d\r\n", err, reg);
        return false;
    }

    err = i2c_read_timeout_us(i2c0, MCP23017_ADDR, rxdata, 1, false, 10000);
    if (err < 0) {
        printf("mcp_read_reg: i2c read error %d on %d\r\n", err, reg);
        return false;
    }

    return true;
}

void input_task(void *p)
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

    if (!mcp_write_reg(GPPUB, 0xFF)) {
        printf("error setting pull-ups in GPIOB\r\n");
    }
    if (!mcp_write_reg(IODIRB, 0xFF)) {
        printf("error setting direction to input in GPIOB\r\n");
    }

    uint8_t raw;
    mcp_read_reg(GPPUB, &raw);
    if (raw != 0xFF) {
        printf("error: pull-up resistors not correctly set. Error setting GPPUB register\r\n");
    }

    printf("        B7 B6 B5 B4 B3 B2 B1 B0\r\n");
    bool first  = true;
    bool bus_ok = true;
    uint8_t prev;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(20));

        uint8_t raw;
        if (!mcp_read_reg(GPIOB, &raw)) {
            if (bus_ok) {
                printf("BUS ERROR\r\n");
                bus_ok = false;
            }
            continue;
        }

        if (!bus_ok) {
            printf("bus recovered\r\n");
            bus_ok = true;
        }

        if (first || (raw != prev)) {
            char bits[9];
            format_bits(raw, bits);
            TickType_t now = xTaskGetTickCount();
            printf("%u  %02x  %s\r\n", now, raw, bits);
            first = false;
        }

        prev = raw;
    }
}

// -- main --------------------------------------------------------------------

int main()
{
    stdio_init_all();

    stdio_set_driver_enabled(&cdc_stdio, true);

    xTaskCreate(usb_device_task, "usb device", configMINIMAL_STACK_SIZE * 4, NULL,
                configMAX_PRIORITIES - 1, NULL);
    // xTaskCreate(midi_task, "midi song", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
    // NULL);
    xTaskCreate(input_task, "input task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3,
                NULL);

    vTaskStartScheduler();

    return 0;
}
