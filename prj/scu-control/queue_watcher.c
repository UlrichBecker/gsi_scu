/*!
 * @file queue_watcher.c
 * @brief Queue watcher for data- and event- queues.
 *
 * Watching a queue for a possible overflow and gives alarm if happened.
 *
 * @date 19.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see queue_watcher.h
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
#include <scu_function_generator.h>
#include <scu_command_handler.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <daq_main.h>
#endif
#ifdef CONFIG_MIL_FG
 #include <scu_mil_fg_handler.h>
 #ifdef CONFIG_RTOS
  #include <scu_task_mil.h>
 #endif
#endif
#ifdef CONFIG_RTOS
 #include <scu_task_fg.h>
#endif
#include "queue_watcher.h"

/*
 * Alarm queue containing the pointer of queues in which has been happened
 * a overflow.
 */
QUEUE_CREATE_STATIC( g_queueAlarm, MAX_FG_CHANNELS, SW_QUEUE_T* );

/*!----------------------------------------------------------------------------
 * @see queue_watcher.h
 */
void queuePollAlarm( void )
{
   void* pOverflowedQueue;

   if( !queuePopSave( &g_queueAlarm, &pOverflowedQueue ) )
      return;

   const char* str = "unknown";
   #define QEUE2STRING( name ) if( &name == pOverflowedQueue ) str = #name

   QEUE2STRING( g_queueSaftCmd );
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   QEUE2STRING( g_queueAddacDaq );
#endif
#ifdef CONFIG_MIL_FG
   QEUE2STRING( g_queueMilFg );
#endif
#ifdef CONFIG_RTOS
   QEUE2STRING( g_queueFg );
#endif
   #undef QEUE2STRING

#if (defined( _CONFIG_MIL_EV_QUEUE ) || defined(CONFIG_RTOS) ) && defined( CONFIG_MIL_FG )
   if( pOverflowedQueue != &g_ecaEvent )
   {
#endif
      scuLog( LM32_LOG_ERROR, ESC_ERROR
              "ERROR: Queue \"%s\" has overflowed! Capacity: %d\n"
              ESC_NORMAL, str, queueGetMaxCapacity( pOverflowedQueue ) );
#if (defined( _CONFIG_MIL_EV_QUEUE ) || defined(CONFIG_RTOS) ) && defined( CONFIG_MIL_FG )
   }
   else
   {
      scuLog( LM32_LOG_ERROR, ESC_ERROR
              "ERROR: ECA-event-queue has overflowed! Capacity: %d\n"
              ESC_NORMAL, g_ecaEvent.capacity );
   }
#endif
}

/*================================== EOF ====================================*/
