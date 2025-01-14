/*!
 * @file scu_event.c
 * @brief Event handling. Similar like a FiFo but without payload-data.
 * @see scu_event.h
 * @date 16.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * 
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
#include <scu_event.h>
#if defined(__lm32__) || defined(__DOXYGEN__)
  #include <lm32Interrupts.h>
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_event.h
 */
void evInit( EVENT_T* pEvent, const size_t maxCapacity )
{
   pEvent->counter = 0;
   pEvent->capacity = maxCapacity;
}

/*! ---------------------------------------------------------------------------
 * @see scu_event.h
 */
bool evPush( EVENT_T* pEvent )
{
   if( pEvent->counter >= pEvent->capacity )
      return false;

   pEvent->counter++;
   return true;
}

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see scu_event.h
 */
bool evPushSafe( EVENT_T* pEvent )
{
   criticalSectionEnter();
   const bool ret = evPush( pEvent );
   criticalSectionExit();
   return ret;
}
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_event.h
 */
bool evPop( EVENT_T* pEvent )
{
   if( !evIsPresent( pEvent ) )
      return false;

   pEvent->counter--;
   return true;
}

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see scu_event.h
 */
bool evPopSafe( EVENT_T* pEvent )
{
   criticalSectionEnter();
   const bool ret = evPop( pEvent );
   criticalSectionExit();
   return ret;
}
#endif
/*================================== EOF ====================================*/
