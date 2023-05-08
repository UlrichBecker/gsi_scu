/*!
 * @file   circular_index.c
 * @brief  Administration of memory read- and write indexes for circular
 *         buffers resp. ring buffers and FiFos.
 *
 * @note Suitable for LM32 and Linux.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      17.08.2020
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

#include <circular_index.h>

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
void ramRingReset( RAM_RING_INDEXES_T* pThis )
{
   pThis->start = 0;
   pThis->end   = 0;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetSize( const RAM_RING_INDEXES_T* pThis )
{  /*
    * Is ring-buffer full?
    */
   if( pThis->end == pThis->capacity )
      return pThis->capacity;

   /*
    * Valid payload range within the allocated memory or empty?
    */
   if( pThis->end >= pThis->start )
      return pThis->end - pThis->start;

   /*
    * In this case the valid payload is fragmented in the upper and lower
    * part of the allocated memory.
    */
   return (pThis->capacity - pThis->start) + pThis->end;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetRemainingCapacity( const RAM_RING_INDEXES_T* pThis )
{
   return pThis->capacity - ramRingGetSize( pThis );
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetReadIndex( const RAM_RING_INDEXES_T* pThis )
{
   return pThis->start + pThis->offset;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetWriteIndex( const RAM_RING_INDEXES_T* pThis )
{ /*
   * Is the buffer full?
   */
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * But in this case the read index has the correct value.
      */
      return ramRingGetReadIndex( pThis );
   }
   return pThis->end + pThis->offset;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetRemainingCapacity( pThis ) >= toAdd );
   RAM_ASSERT( pThis->end < pThis->capacity );

   if( toAdd == 0 )
      return;

   pThis->end = (pThis->end + toAdd) % pThis->capacity;

   /*
    * Has the buffer become full?
    */
   if( pThis->end == pThis->start )
   { /*
      * To distinguish between buffer empty and full,
      * in the case of full the write index will set to a value out of
      * valid range. Here: the maximum capacity.
      */
      pThis->end = pThis->capacity;
   }
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetSize( pThis ) >= toAdd );

   if( toAdd == 0 )
      return;

   /*
    * Was ring-buffer full?
    */
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * Here the buffer is still full and write index will set back to a valid
      * value.
      */
      pThis->end = pThis->start;
   }
   pThis->start = (pThis->start + toAdd) % pThis->capacity;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetUpperReadSize( const RAM_RING_INDEXES_T* pThis )
{
   RAM_ASSERT( pThis->capacity > 0 );
   return pThis->capacity - pThis->start;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetUpperWriteSize( const RAM_RING_INDEXES_T* pThis )
{
   RAM_ASSERT( pThis->capacity > 0 );
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * But in this case the read index has the correct value.
      */
      return ramRingGetUpperReadSize( pThis );
   }
   return pThis->capacity - pThis->end;
}

#ifdef CONFIG_CIRCULAR_DEBUG
/*! ---------------------------------------------------------------------------
 * @brief Prints the values of the members of RAM_RING_INDEXES_T
 */
#ifndef __lm32__
  #include  <stdio.h>
  #define mprintf printf
#endif
void ramRingDbgPrintIndexes( const RAM_RING_INDEXES_T* pThis,
                                                             const char* txt )
{
   if( txt != NULL )
     mprintf( "DBG: %s\n", txt );
   mprintf( "  DBG: offset:   %d\n"
            "  DBG: capacity: %d\n"
            "  DBG: start:    %d\n"
            "  DBG: end:      %d\n"
            "  DBG: used:     %d\n"
            "  DBG: free      %d\n\n",
            pThis->offset,
            pThis->capacity,
            pThis->start,
            pThis->end,
            ramRingGetSize( pThis ),
            ramRingGetRemainingCapacity( pThis )
          );
}
#endif

/*================================== EOF ====================================*/
