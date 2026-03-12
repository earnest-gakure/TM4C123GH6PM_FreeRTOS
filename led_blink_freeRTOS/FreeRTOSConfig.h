#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
#define configPRIO_BITS    3

/* Scheduler */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configCPU_CLOCK_HZ                      ( 16000000UL )
#define configTICK_RATE_HZ                      ( 1000 )
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( 128 )
#define configTOTAL_HEAP_SIZE                   ( 16 * 1024 )
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* Disable all hooks for now */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configCHECK_FOR_STACK_OVERFLOW          0

/* Disable timers — removes timer task from startup */
#define configUSE_TIMERS                        0
#define configUSE_MUTEXES                       0
#define configUSE_SEMAPHORES                    0

/* Cortex-M4 interrupt priorities */
#define configKERNEL_INTERRUPT_PRIORITY         ( 7 << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << (8 - configPRIO_BITS) )

/* API includes */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetSchedulerState          1

#endif /* FREERTOS_CONFIG_H */
