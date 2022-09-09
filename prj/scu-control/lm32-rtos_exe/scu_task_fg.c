/*!
 * @file scu_task_fg.c
 * @brief FreeRTOS task for ADDAC function generators
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_fg.h
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
#include "scu_task_fg.h"

STATIC TaskHandle_t mg_taskFgHandle = NULL;

QUEUE_CREATE_STATIC( g_queueFg, MAX_FG_CHANNELS, SCU_BUS_IRQ_QUEUE_T );

/*!----------------------------------------------------------------------------
 * @brief Task for handling SCU-bus respectively ADDAC function generators.
 */
STATIC void taskFg( void* pTaskData UNUSED )
{
   taskInfoLog();

   queueResetSave( &g_queueFg );

   while( true )
   {
      SCU_BUS_IRQ_QUEUE_T queueFgItem;

      if( queuePopSave( &g_queueFg, &queueFgItem ) )
      {
         if( (queueFgItem.pendingIrqs & FG1_IRQ) != 0 )
            handleAdacFg( queueFgItem.slot, FG1_BASE );

         if( (queueFgItem.pendingIrqs & FG2_IRQ) != 0 )
            handleAdacFg( queueFgItem.slot, FG2_BASE );
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_fg.h
 */
void taskStartFgIfAnyPresent( void )
{
   if( (mg_taskFgHandle == NULL) && (addacGetNumberOfFg() > 0) )
   {
      TASK_CREATE_OR_DIE( taskFg, 512, 1, &mg_taskFgHandle );
      vTaskDelay( pdMS_TO_TICKS( 1 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_fg.h
 */
void taskStopFgIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskFgHandle );
}


/*================================== EOF ====================================*/
