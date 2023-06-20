/*!
 * @file ros_mutex.c
 * @brief Very easy resource-saving mutex for FreeRTOS
 * >>> PvdS <<<
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 10.05.2023
 *
 * @note This module requires FreeRTOS.
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
#include "ros_mutex.h"

/*! ---------------------------------------------------------------------------
 * @see ros_mutex.h
 */
void osMutexInit( OS_MUTEX_T volatile * pThis )
{
   portENTER_CRITICAL();

   pThis->lockedTask = NULL;
   pThis->nestingCount = 0;

   portEXIT_CRITICAL();
}

/*! ---------------------------------------------------------------------------
 * @see ros_mutex.h
 */
bool inline osMutexIsLocked( OS_MUTEX_T volatile * pThis )
{
   return pThis->lockedTask != NULL;
}

/*! ---------------------------------------------------------------------------
 * @see ros_mutex.h
 */
void osMutexLock( OS_MUTEX_T volatile * pThis )
{
   const TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

   portENTER_CRITICAL();

   if( pThis->lockedTask != currentTask )
   {
      while( osMutexIsLocked( pThis ) )
      { /*
         * While the mutex is still locked, so the work of other tasks will do.
         * That works, because the critical section is only for this task valid.
         * The critical section nesting counter for interrupts becomes handled
         * like a CPU-register during task change.
         */
         portYIELD();
      }
   }
   pThis->lockedTask = currentTask;
   pThis->nestingCount++;

   portEXIT_CRITICAL();
}

/*! ---------------------------------------------------------------------------
 * @see ros_mutex.h
 */
bool osMutexTryLock( OS_MUTEX_T volatile * pThis )
{
   if( osMutexIsLocked( pThis ) && (pThis->lockedTask != xTaskGetCurrentTaskHandle()) )
      return false;

   osMutexLock( pThis );
   return true;
}

/*! ---------------------------------------------------------------------------
 * @see ros_mutex.h
 */
void osMutexUnlock( OS_MUTEX_T volatile * pThis )
{
   portENTER_CRITICAL();

   if( pThis->nestingCount != 0 )
   {
      if( pThis->lockedTask == xTaskGetCurrentTaskHandle() )
      {
         pThis->nestingCount--;
      }
   }

   if( pThis->nestingCount == 0 )
   {
      pThis->lockedTask = NULL;
   }

   portEXIT_CRITICAL();
}

/*================================== EOF ====================================*/
