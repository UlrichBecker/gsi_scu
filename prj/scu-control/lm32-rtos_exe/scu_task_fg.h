/*!
 * @file scu_task_fg.h
 * @brief FreeRTOS task for ADDAC function generators
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_fg.c
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
#ifndef _SCU_TASK_FG_H
#define _SCU_TASK_FG_H

#ifndef CONFIG_USE_ADDAC_FG_TASK
  #error CONFIG_USE_ADDAC_FG_TASK respectively USE_ADDAC_FG_TASK has to be defined in makefile!
#endif

#include <scu_fg_handler.h>
#include <sw_queue.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SW_QUEUE_T g_queueFg;

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Starts the RTOS-ADDAC FG task for SCU-bus FGs if at least
 *        one FG present or - if not successful - it stops the CPU with a final
 *        error- log message.
 */
void taskStartFgIfAnyPresent( void );

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Stops the possible running ADDAC-FG- task if running, else this
 *        function is without effect.
 */
void taskStopFgIfRunning( void );

#if (configUSE_TASK_NOTIFICATIONS == 1)
/*! ---------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @ingroup INTERRUPT
 * @brief Wakes up the possible sleeping ADDAC function generator task from
 *        the interrupt routine.
 */
void taskWakeupFgFromISR( void );
#endif

#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_TASK_FG_H */
/*================================== EOF ====================================*/
