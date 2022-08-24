/*!
 * @file scu_task_mil.h
 * @brief FreeRTOS task for MIL function generators and MIL DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_mil.c
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
#ifndef _SCU_TASK_MIL_H
#define _SCU_TASK_MIL_H

#ifndef CONFIG_MIL_FG
#error Compilerswitch CONFIG_MIL_FG is not defined!
#endif

#include <scu_eca_handler.h>
#include <scu_mil_fg_handler.h>

#ifdef __cplusplus
extern "C" {
#endif

extern EVENT_T g_ecaEvent;

/*!----------------------------------------------------------------------------
 * @brief Starts the RTOS- MIL task for MIL-FGs and MIL-DAQs,
 *        or - if not successful - it stops the CPU with a final
 *        error- log message.
 */
void taskStartMilIfAnyPresent( void );

/*!----------------------------------------------------------------------------
 * @brief Stops the possible running MIL- task if running, else this function
 *        is without effect.
 */
void taskStopMilIfRunning( void );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_TASK_MIL_H */
/*================================== EOF ====================================*/
