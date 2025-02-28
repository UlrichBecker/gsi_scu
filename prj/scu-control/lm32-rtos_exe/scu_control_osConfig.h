/*!
 * @file scu_controlConfig_os.h
 * @brief Configuration file for FreeRTOS- application "scu3_control_os"
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      30.06.2022
 */
#ifndef _SCU_CONTROLCONFIG_H
#define _SCU_CONTROLCONFIG_H


//#define CONFIG_TASKS_SHOULD_YIELD

/*!
 * @brief Shows a software fan on eb-console.
 */
#define CONFIG_STILL_ALIVE_SIGNAL

/*!
 * @brief That is the UART for the function mprintf()
 *        The task will changed within the UARTs polling- loop.
 */
#define CONFIG_TASK_YIELD_WHEN_UART_WAITING

/*
 * Here is a good place to include header files that are required across
 * your application.
 */
#include <scu_control_os.h>

/*
 * Minimal stack size is the stack size of the idle-task in 32-bit words as well.
 */
#define configMINIMAL_STACK_SIZE                128

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 1
#define configTICK_RATE_HZ                      10000 //20000
#define configMAX_PRIORITIES                    4
#define configMAX_TASK_NAME_LEN                 16
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       0
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#ifdef CONFIG_MIL_DAQ_USE_RAM
  #define configTOTAL_HEAP_SIZE                   16240
#else
  #define configTOTAL_HEAP_SIZE                   12240
#endif
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               TASK_PRIO_TEMPERATURE
#define configTIMER_QUEUE_LENGTH                1
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE + 200)

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY         [dependent of processor]
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    [dependent on processor and application]
#define configMAX_API_CALL_INTERRUPT_PRIORITY   [dependent on processor and application]


/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

/* A header file that defines trace macro can be included here. */

#define CONFIG_NO_PRINTF_MUTEX

#define CONFIG_SLEEP_FG_TASK
#define CONFIG_SLEEP_MIL_TASK
//#define CONFIG_SLEEP_DAQ_TASK //TODO Doesn't work properly yet!

#define TASK_PRIO_STD         1

#define TASK_PRIO_MAIN        TASK_PRIO_STD
#define TASK_PRIO_TEMPERATURE TASK_PRIO_STD

#if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_DAQ_TASK )
 #define TASK_PRIO_ADDAC_DAQ   TASK_PRIO_STD + 1
#else
 #define TASK_PRIO_ADDAC_DAQ   TASK_PRIO_STD
#endif
#if TASK_PRIO_ADDAC_DAQ >= configMAX_PRIORITIES
 #error TASK_PRIO_ADDAC_DAQ >= configMAX_PRIORITIES
#endif

#ifdef CONFIG_USE_ADDAC_FG_TASK
 #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_FG_TASK )
  #define CONFIG_ADDAC_FG_TASK_SHOULD_RUN_IMMEDIATELY
  #define TASK_PRIO_ADDAC_FG    TASK_PRIO_STD + 2
 #else
  #define TASK_PRIO_ADDAC_FG    TASK_PRIO_STD
 #endif
 #if TASK_PRIO_ADDAC_FG >= configMAX_PRIORITIES
  #error TASK_PRIO_ADDAC_FG >= configMAX_PRIORITIES
 #endif
#endif

#if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
 #define CONFIG_MIL_TASK_SHOULD_RUN_IMMEDIATELY
 #define TASK_PRIO_MIL_FG      TASK_PRIO_STD + 1
#else
 #define TASK_PRIO_MIL_FG      TASK_PRIO_STD
#endif
#if TASK_PRIO_MIL_FG >= configMAX_PRIORITIES
 #error TASK_PRIO_MIL_FG >= configMAX_PRIORITIES
#endif

#endif /* ifndef _SCU_CONTROLCONFIG_H */
/*================================== EOF ====================================*/
