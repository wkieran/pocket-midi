#include <FreeRTOS.h>
#include "task.h"
#include "pico/stdlib.h"

void vApplicationMallocFailedHook() { while(1){} }
void vApplicationStackOverflowHook(TaskHandle_t t, char *c) { panic(c); }

int main() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (1) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(500);
    }
}

// #include <FreeRTOS.h>
// #include "task.h"
// #include "tusb.h"
// #include "pico/stdlib.h"
//
// void vApplicationMallocFailedHook() { while(1){} }
// void vApplicationStackOverflowHook(TaskHandle_t task_handle, char * c) { panic(c); }
//
// int main() {
//     tusb_init();
//     vTaskStartScheduler();
//     return 0;
// }
