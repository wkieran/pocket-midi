#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* --- Scheduler --- */

/* Preemption: higher-priority tasks always preempt lower ones immediately. */
#define configUSE_PREEMPTION                    1

/* No low-power tickless idle — keeps timing simple during development. */
#define configUSE_TICKLESS_IDLE                 0

/* 1ms tick = 1000Hz. Gives 1ms resolution for vTaskDelay() and timeouts. */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )

/* 5 priority levels: 0 (lowest/idle) through 4 (highest). Matches our 4-task architecture. */
#define configMAX_PRIORITIES                    5

/* Minimum stack for idle task, in words (4 bytes each). 512 words = 2KB. */
#define configMINIMAL_STACK_SIZE                ( ( configSTACK_DEPTH_TYPE ) 512 )

/* Use 32-bit tick counter — overflows after ~49 days at 1kHz. */
#define configUSE_16_BIT_TICKS                  0

/* Idle task yields immediately if any task at priority 0 is ready. */
#define configIDLE_SHOULD_YIELD                 1

/* Single core to start. Add second core (SMP) after Phase 1 is stable. */
#define configNUMBER_OF_CORES                   1

/* No idle/tick hooks needed yet. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

/* --- Memory --- */

#define configSTACK_DEPTH_TYPE                  uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* Dynamic allocation only — no static buffers needed at this stage. */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1

/* 64KB heap. TinyUSB takes some RAM too; tune up if malloc hooks fire. */
#define configTOTAL_HEAP_SIZE                   ( 64 * 1024 )
#define configAPPLICATION_ALLOCATED_HEAP        0

/* --- Development safety --- */

/* Mode 2: checks stack watermark on every context switch. Catches overflows early. */
#define configCHECK_FOR_STACK_OVERFLOW          2

/* Calls vApplicationMallocFailedHook() on heap exhaustion instead of returning NULL silently. */
#define configUSE_MALLOC_FAILED_HOOK            1

#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Trap assertion failures during development. */
#include <assert.h>
#define configASSERT( x )                       assert( x )

/* --- Synchronization --- */

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_TIME_SLICING                  1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/* --- Stats (off for now, enable if profiling tasks) --- */

#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* --- Software timers --- */

/* Required by xEventGroupSetBitsFromISR(), which the RP2040 FreeRTOS port uses
   for pico-sdk sync interop even in single-core mode. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            512

/* --- Co-routines (deprecated, off) --- */

#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* --- pico-sdk interop --- */

/* Allows FreeRTOS and pico-sdk sync primitives (mutex, sem) to interoperate. */
#define configSUPPORT_PICO_SYNC_INTEROP         1

/* Allows FreeRTOS delays and pico-sdk time functions to share the same tick source. */
#define configSUPPORT_PICO_TIME_INTEROP         1

/* --- RP2350 (Cortex-M33) specific --- */

#if PICO_RP2350
/* No MPU or TrustZone — running non-secure only, keeps things simple. */
#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
/* M33 has hardware FPU — enable so float ops don't trap. */
#define configENABLE_FPU                        1
/* Highest interrupt priority that can safely call FreeRTOS ISR-safe APIs. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16
#endif

/* --- Optional API inclusion --- */

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xQueueGetMutexHolder            1
#define INCLUDE_xTimerPendFunctionCall          1

#endif /* FREERTOS_CONFIG_H */
