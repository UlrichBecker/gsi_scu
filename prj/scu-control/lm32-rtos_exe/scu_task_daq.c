/*!
 * @file scu_task_daq.c
 * @brief FreeRTOS task for ADDAC-DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_daq.h
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
#include <FreeRTOS.h>
#include <task.h>
#include <daq_main.h>
#include "scu_task_daq.h"

STATIC TaskHandle_t mg_taskDaqHandle = NULL;

/*! ---------------------------------------------------------------------------
 * @brief RTOS- task for ADDAC-DAQs respectively SCU-bus DAQs.
 */
STATIC void taskDaq( void* pTaskData UNUSED )
{
   taskInfoLog();

   while( true )
   {
      addacDaqTask();
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStartDaqIfAnyPresent( void )
{
   if( (mg_taskDaqHandle == NULL) && (daqBusGetFoundDevices( &g_scuDaqAdmin.oDaqDevs ) > 0) )
   {
      queueReset( &g_queueAddacDaq );
      TASK_CREATE_OR_DIE( taskDaq, 512, 1, &mg_taskDaqHandle );
      vTaskDelay( pdMS_TO_TICKS( 1 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStopDaqIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskDaqHandle );
}


/*================================== EOF ====================================*/
