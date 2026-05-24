#include <FreeRTOS.h>
#include "pico/stdlib.h"
#include "task.h"
#include "tusb.h"
#include <pico/stdio.h>

void vApplicationMallocFailedHook() {
    while (1) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t task_handle, char *c) { panic(c); }
struct led_task_arg {
    int gpio;
    int delay;
};

void led_task(void *p) {
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

int main() {
    tusb_init();
    stdio_init_all();
    printf("Start LED blink\n");

    struct led_task_arg arg = {PICO_DEFAULT_LED_PIN, 250};
    xTaskCreate(led_task, "LED_Task", 256, &arg, 1, NULL);

    vTaskStartScheduler();

    return 0;
}
