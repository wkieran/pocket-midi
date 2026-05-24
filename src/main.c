#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "pico/stdlib.h"
#include "task.h"
#include "tusb.h"
#include <pico/stdio.h>

void vApplicationMallocFailedHook()
{
    while (1) {
    }
}
void vApplicationStackOverflowHook(TaskHandle_t task_handle, char *c) { panic(c); }

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

void usb_device_task(void *p)
{
    tusb_init();
    while (1) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int main()
{
    stdio_init_all();
    printf("Start LED blink\n");

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, &led_arg, 2, NULL);

    xTaskCreate(usb_device_task, "USB_Task", configMINIMAL_STACK_SIZE * 4, NULL,
                configMAX_PRIORITIES - 1, NULL);

    vTaskStartScheduler();

    return 0;
}
