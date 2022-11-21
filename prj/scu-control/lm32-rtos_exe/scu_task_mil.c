/*!
 * @file scu_task_mil.c
 * @brief FreeRTOS task for MIL function generators and MIL DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_mil.h
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
#include "scu_task_mil.h"


STATIC TaskHandle_t mg_taskMilHandle = NULL;

EV_CREATE_STATIC( g_ecaEvent, 16 );

/*! ---------------------------------------------------------------------------
 * @brief RTOS- task for MIL-FGs and MIL-DAQs
 */
STATIC void taskMil( void* pTaskData UNUSED )
{
   taskInfoLog();

   queueResetSave( &g_queueMilFg );
   evDelete( &g_ecaEvent );

   while( true )
   {
      if( evPopSave( &g_ecaEvent ) )
      {
         ecaHandler();
         lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "ECA received\n" ESC_NORMAL );
      }
      milExecuteTasks();
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskStartMilIfAnyPresent( void )
{
   if( (mg_taskMilHandle == NULL) && (milGetNumberOfFg() > 0) )
   {
      TASK_CREATE_OR_DIE( taskMil, 1024, TASK_PRIO_MIL_FG, &mg_taskMilHandle );
      //vTaskDelay( pdMS_TO_TICKS( 1 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskStopMilIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskMilHandle );
}

/*================================== EOF ====================================*/
