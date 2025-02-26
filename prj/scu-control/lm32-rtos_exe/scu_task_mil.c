/*!
 * @file scu_task_mil.c
 * @brief FreeRTOS task for MIL function generators and MIL DAQs.
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
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
#include <scu_function_generator.h>
#include "scu_task_mil.h"


STATIC TaskHandle_t mg_taskMilHandle = NULL;

EV_CREATE_STATIC( g_ecaEvent, MAX_FG_CHANNELS );

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
bool taskIsMilTaskRunning( void )
{
   return mg_taskMilHandle != NULL;
}

/*! ---------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief RTOS- task for MIL-FGs and MIL-DAQs
 */
STATIC void taskMil( void* pTaskData UNUSED )
{
   taskInfoLog();

   queueResetSafe( &g_queueMilFg );
   evDelete( &g_ecaEvent );

   /*
    * Main loop of MIL-FG handling.
    */
   while( true )
   {
   #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
    #ifdef CONFIG_READ_MIL_TIME_GAP
      /*
       * When the gap reading mode is activated so the maximum sleep time of
       * this task will be at 2 milliseconds.
       */
      #define MIL_TASK_WAITING_TIME pdMS_TO_TICKS( 2 )
    #else
      #define MIL_TASK_WAITING_TIME portMAX_DELAY
    #endif
      /*
       * All MIL-FSMs in state ST_WAIT ?
       */
      if( milAllInWateState() )
      { /*
         * Sleep till wake up by interrupt or waiting time expired.
         */
         xTaskNotifyWait( pdFALSE, 0, NULL, MIL_TASK_WAITING_TIME );
      }
   #endif
      if( evPopSafe( &g_ecaEvent ) )
      {
         ecaHandler();
      }
      milExecuteTasks();
   } /* while() */
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskStartMilIfAnyPresent( void )
{
   if( (mg_taskMilHandle == NULL) && (milGetNumberOfFg() > 0) )
   {
      TASK_CREATE_OR_DIE( taskMil, 1024, TASK_PRIO_MIL_FG, &mg_taskMilHandle );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskStopMilIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskMilHandle );
}

#if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskWakeupMilFromISR( void )
{
   if( mg_taskMilHandle != NULL )
   {
   #ifdef CONFIG_MIL_TASK_SHOULD_RUN_IMMEDIATELY
      /*
       * Task will run immediately after the ISR has returned.
       */
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR( mg_taskMilHandle, &xHigherPriorityTaskWoken );
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
   #else
      /*
       * Task will run by the next regular task-change-tick.
       */
      xTaskNotifyFromISR( mg_taskMilHandle, 0, 0, NULL );
   #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_mil.h
 */
void taskWakeupMil( void )
{
   if( mg_taskMilHandle != NULL )
      xTaskNotify( mg_taskMilHandle, 0, eNoAction );
}
#endif /* if (configUSE_TASK_NOTIFICATIONS == 1) */


/*================================== EOF ====================================*/
