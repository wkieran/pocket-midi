#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "class/midi/midi_device.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "task.h"
#include "tusb.h"
#include <pico/stdio.h>
#include "notes.h"

// MIDI Device Defines
#define BPM 120
#define SUBDIVISION 1 // 1: quarter notes, 2: eigth notes, 4: sixteenth notes
#define NOTE_MS (60000 / BPM / SUBDIVISION)

void vApplicationMallocFailedHook()
{
    while (1) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t task_handle, char *c) { panic(c); }

// BLINK LED Task
struct led_task_arg
{
    int gpio;
    int delay;
};

struct led_task_arg led_arg = {PICO_DEFAULT_LED_PIN, 250};

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

// USB Device Enumeration Task
void usb_device_task(void *p)
{
    tusb_init();
    while (1) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// MIDI Note Sender Task

const uint8_t note_sequence[] = {D5,  Fs5, A5,  D6,  Fs6, A6,  D7,  Fs7, A3,  Cs4, Fs4, A4, Cs5,
                                 Fs5, A5,  Cs6, E6,  Gs6, Cs7, E7,  Cs7, Gs6, E6,  Cs6, A5, Fs5,
                                 D5,  A4,  Fs4, D4,  A3,  D4,  Fs4, A4,  D5,  Fs5, A5,  D6, Fs6,
                                 A6,  Cs7, Fs7, Cs7, A6,  Fs6, Cs6, A5,  Fs5, Cs5, Gs4, E4, Cs4,
                                 Gs3, Cs4, E4,  Gs4, D5,  Fs5, A5,  D6,  Fs6, A6,  D7,  Fs7};

void midi_task(void *p)
{
    const uint8_t cable_num = 0; // MIDI jack associated with USB endpoint
    const uint8_t channel   = 0; // 0 for channel 1

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

int main()
{
    stdio_init_all();
    printf("Start LED blink\n");

    xTaskCreate(led_task, "led blinky", configMINIMAL_STACK_SIZE, &led_arg, 1, NULL);
    xTaskCreate(usb_device_task, "usb device", configMINIMAL_STACK_SIZE * 4, NULL,
                configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(midi_task, "midi song", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                NULL);

    vTaskStartScheduler();

    return 0;
}
