/*!
 * @file scu_task_daq.h
 * @brief FreeRTOS task for ADDAC-DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_daq.c
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
#ifndef _SCU_TASK_DAQ_H
#define _SCU_TASK_DAQ_H

#ifndef CONFIG_SCU_DAQ_INTEGRATION
#error "Compilerswitch CONFIG_SCU_DAQ_INTEGRATION is not defined!"
#endif

#include <daq_main.h> 

#ifdef __cplusplus
extern "C" {
#endif

/*!----------------------------------------------------------------------------
 * @brief Starts the ADDAC_DAQ task for SCU-bus DAQs if not already running or
 *        at least one DAQ present or - if not successful - it stops the CPU
 *        with a final error- log message.
 */
void taskStartDaqIfAnyPresent( void );

/*!----------------------------------------------------------------------------
 * @brief Stops the possible running ADDAC-DAQ- task if running, else this
 *        function is without effect.
 */
void taskStopDaqIfRunning( void );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_TASK_DAQ_H */
/*================================== EOF ====================================*/
