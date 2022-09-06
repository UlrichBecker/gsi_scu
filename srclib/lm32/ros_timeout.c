/*!
 * @file ros_timeout.c
 * @brief Handling of timeout by concerning of integer-overflow.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 06.09.2022
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
#include "ros_timeout.h"
#include <task.h>

/*!----------------------------------------------------------------------------
 * @see ros_timeout.h
 */
void toStart( TIMEOUT_T* pTimeout, const TickType_t duration )
{
   pTimeout->duration = duration;
   toRestart( pTimeout );
}

/*!----------------------------------------------------------------------------
 * @see ros_timeout.h
 */
void toRestart( TIMEOUT_T* pTimeout )
{
   const TickType_t tick = xTaskGetTickCount();

   pTimeout->threshold = pTimeout->duration + tick;
   pTimeout->overflow = (pTimeout->threshold < tick);
}

/*!----------------------------------------------------------------------------
 * @see ros_timeout.h
 */
bool toIsElapsed( TIMEOUT_T* pTimeout )
{
   const TickType_t tick = xTaskGetTickCount();

   if( pTimeout->overflow )
   {
      if( pTimeout->threshold < tick )
         return false;

      pTimeout->overflow = false;
   }
   return (tick >= pTimeout->threshold);
}

/*!----------------------------------------------------------------------------
 * @see ros_timeout.h
 */
bool toInterval( TIMEOUT_T* pTimeout )
{
   const TickType_t tick = xTaskGetTickCount();

   if( pTimeout->overflow )
   {
      if( pTimeout->threshold < tick )
         return false;

      pTimeout->overflow = false;
   }

   if( tick < pTimeout->threshold )
      return false;

   pTimeout->threshold = pTimeout->duration + tick;
   pTimeout->overflow = (pTimeout->threshold < tick);

   return true;
}

/*================================== EOF ====================================*/
