/*!
 * @file sw_queue.c
 * @brief Simple software queue respectively software fifo for small devices.
 * @see sw_queue.h
 * @date 30.04.2021
 * @copyright (C) 2021 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <string.h>
#include <sw_queue.h>
#if defined(__lm32__) || defined(__DOXYGEN__)
  #include <lm32Interrupts.h>
#endif

#if defined(__lm32__) || defined(__DOXYGEN__)
STATIC inline ALWAYS_INLINE void _queueCriticalSectionEnter( void )
{
   criticalSectionEnter();
}

STATIC inline ALWAYS_INLINE void _queueCriticalSectionExit( void )
{
   criticalSectionExit();
}
#else
  #define _queueCriticalSectionEnter()
  #define _queueCriticalSectionExit()
  #warning Using dummys for _queueCriticalSectionEnter() and _queueCriticalSectionExit()
#endif

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
void queueCreateOffset( SW_QUEUE_T* pThis,
                        uint8_t* pBuffer,
                        const unsigned int offset,
                        const size_t itemSize, 
                        const unsigned int capacity )
{
   pThis->pBuffer = pBuffer;
   pThis->indexes.offset = offset;
   pThis->itemSize = itemSize;
   pThis->indexes.capacity = capacity;
   queueReset( pThis );
}

/*! ---------------------------------------------------------------------------
 * @brief Returns true when the queue is empty within a atomic section.
 * @param pThis Pointer to the concerned queue object.
 * @retval true Queue is empty.
 * @retval false Queue is not empty.
 */
inline bool queueIsEmptySafe( SW_QUEUE_T* pThis )
{
   _queueCriticalSectionEnter();
   const bool ret = queueIsEmpty( pThis );
   _queueCriticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePush( SW_QUEUE_T* pThis, const void* pItem )
{
   if( queueIsFull( pThis ) )
      return false;

   memcpy( &pThis->pBuffer[ramRingGetWriteIndex(&pThis->indexes) * pThis->itemSize],
           pItem, pThis->itemSize );

   ramRingIncWriteIndex( &pThis->indexes );

   return true;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePushSafe( SW_QUEUE_T* pThis, const void* pItem )
{
   _queueCriticalSectionEnter();
   const bool ret = queuePush( pThis, pItem );
   _queueCriticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queueForcePush( SW_QUEUE_T* pThis, const void* pItem )
{
   const bool isFull = queueIsFull( pThis );
   if( isFull )
   { /*
      * The queue is full, so the oldest item has to be removed by
      * incrementing the read-index.
      */
      ramRingIncReadIndex( &pThis->indexes );
   }

   memcpy( &pThis->pBuffer[ramRingGetWriteIndex(&pThis->indexes) * pThis->itemSize],
           pItem, pThis->itemSize );

   ramRingIncWriteIndex( &pThis->indexes );

   return !isFull;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queueForcePushSafe( SW_QUEUE_T* pThis, const void* pItem )
{
   _queueCriticalSectionEnter();
   const bool ret = queueForcePush( pThis, pItem );
   _queueCriticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePop( SW_QUEUE_T* pThis, void* pItem )
{
   if( queueIsEmpty( pThis ) )
      return false;

   memcpy( pItem, 
           &pThis->pBuffer[ramRingGetReadIndex(&pThis->indexes) * pThis->itemSize],
           pThis->itemSize );

   ramRingIncReadIndex( &pThis->indexes );

   return true;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePeek( SW_QUEUE_T* pThis, void* pItem )
{
   if( queueIsEmpty( pThis ) )
      return false;

   memcpy( pItem,
           &pThis->pBuffer[ramRingGetReadIndex(&pThis->indexes) * pThis->itemSize],
           pThis->itemSize );

   return true;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePeekSafe( SW_QUEUE_T* pThis, void* pItem )
{
   _queueCriticalSectionEnter();
   const bool ret = queuePeek( pThis, pItem );
   _queueCriticalSectionExit();
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queueSkip( SW_QUEUE_T* pThis )
{
   if( queueIsEmpty( pThis ) )
      return false;

   ramRingIncReadIndex( &pThis->indexes );

   return true;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queueSkipSafe( SW_QUEUE_T* pThis )
{
   _queueCriticalSectionEnter();
   const bool ret = queueSkip( pThis );
   _queueCriticalSectionExit();
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
void queueResetSafe( SW_QUEUE_T* pThis )
{
   _queueCriticalSectionEnter();
   queueReset( pThis );
   _queueCriticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePopSafe( SW_QUEUE_T* pThis, void* pItem )
{
   _queueCriticalSectionEnter();
   const bool ret = queuePop( pThis, pItem );
   _queueCriticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
unsigned int queueGetSizeSafe( SW_QUEUE_T* pThis )
{
   _queueCriticalSectionEnter();
   const unsigned int ret = queueGetSize( pThis );
   _queueCriticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
unsigned int queueGetRemainingCapacitySafe( SW_QUEUE_T* pThis )
{
   _queueCriticalSectionEnter();
   const unsigned int ret = queueGetRemainingCapacity( pThis );
   _queueCriticalSectionExit();

   return ret;
}

/*================================== EOF ====================================*/
