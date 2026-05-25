#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "class/midi/midi_device.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "task.h"
#include "tusb.h"
#include <pico/stdio.h>

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
const uint8_t note_sequence[] = {
    74, 78, 81, 86, 90, 93, 98, 102, 57, 61, 66, 69, 73, 78, 81, 85, 88, 92, 97, 100, 97, 92,
    88, 85, 81, 78, 74, 69, 66, 62,  57, 62, 66, 69, 74, 78, 81, 86, 90, 93, 97, 102, 97, 93,
    90, 85, 81, 78, 73, 68, 64, 61,  56, 61, 64, 68, 74, 78, 81, 86, 90, 93, 98, 102};

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
