/*!
 * @file scu_control_os.h
 * @brief Main module of SCU control including the main-thread FreeRTOS.
 *
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_control.c
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _SCU_CONTROL_OS_H
#define _SCU_CONTROL_OS_H

#include <scu_lm32_macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @defgroup RTOS_TASK Functions and macros for handling FreeRTOS tasks
 *                     respectively threads.
 */

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Helper macro for creating a task.
 * @param func Name of task function.
 * @param stackSize Additional stack size.
 * @param prio Priority
 * @param handle Pointer to the task handle,
 *               if not used then it has to be NULL.
 * @retval pdPASS Success.
 */
#define TASK_CREATE( func, stackSize, prio, handle )                          \
   xTaskCreate( func, #func,                                                  \
                configMINIMAL_STACK_SIZE + stackSize,                         \
                NULL,                                                         \
                tskIDLE_PRIORITY + prio, handle )

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Helper Macro creates a task or when not successful it stops the CPU
 *        by a error-log message.
 * @param func Name of task function.
 * @param stackSize Additional stack size.
 * @param prio Priority
 * @param handle Pointer to the task handle,
 *               if not used then it has to be NULL.
 */
#define TASK_CREATE_OR_DIE( func, stackSize, prio, handle )                   \
   if( TASK_CREATE( func, stackSize, prio, handle ) != pdPASS )               \
   {                                                                          \
      die( "Can't create task: " #func );                                     \
   }

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @see vPortYieldLm32()
 */
#ifdef CONFIG_TASKS_SHOULD_YIELD
   #define TASK_YIELD() vPortYieldLm32()
#else
   #define TASK_YIELD()
#endif

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Prints the task start information in the logging system.
 */
void taskInfoLog( void );

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Deletes the given task and puts an appropriate message in the
 *        logging system, if the content of the given pointer not NULL.
 * @note The argument is a double pointer, the function will made a pointer
 *       cast into "TaskHandle_t*"! 
 * @param pTaskHandle Pointer to the task handle, after this function call
 *                    the content will be NULL.
 */
void taskDeleteIfRunning( void* pTaskHandle );

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Deletes all possible running tasks of MIL-FG/DAQ, ADDAC-FG and
 *        ADDAC-DAQ.
 */
void taskDeleteAllRunningFgAndDaq( void );

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Starts all RTOS tasks for which the corresponding hardware
 *        was detected. That can be MIL
 */
void taskStartAllIfHwPresent( void );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SCU_CONTROL_OS_H */
/*==================================  EOF ===================================*/
