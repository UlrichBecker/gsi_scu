/*!
 * @file scu_event.h
 * @brief Event handling. Similar like a FiFo but without payload-data.
 * @see scu_event.c
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
#ifndef _SCU_EVENT_H
#define _SCU_EVENT_H

#include <stdbool.h>
#include <stdint.h>
#include <helper_macros.h>

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*!
 * @brief Data-type of event counter.
 */
typedef uint32_t EV_COUNTER_T;

/*!
 * @brief Data type for event object.
 */
typedef struct
{  /*!
    * @brief Current number of events.
    */
   EV_COUNTER_T counter;
   
   /*!
    * @brief Maximum number of events.
    */
   size_t       capacity;
} EVENT_T;
   
/*! ---------------------------------------------------------------------------
 * @brief Initializer of a event-queue
 * @param  maxCapacity Maximum number of events.
 */
#define EV_STATIC_INITIALIZER( maxCapacity )           \
{                                                      \
   .counter = 0,                                       \
   .capacity = maxCapacity                             \
}

/*! ---------------------------------------------------------------------------
 * @brief Creates a event queue object.
 * @param name Name of the event object.
 * @param maxCapacity Maximum number of events.
 */
#define EV_CREATE_STATIC( name, maxCapacity )          \
   EVENT_T name = EV_STATIC_INITIALIZER( maxCapacity )

/*! ---------------------------------------------------------------------------
 * @brief Initializes a event queue object
 * @param pEvent Pointer to event queue object.
 * @param maxCapacity Maximum number of events.
 */
void evInit( EVENT_T* pEvent, const size_t maxCapacity );

/*! ---------------------------------------------------------------------------
 * @brief Deletes all events in the queue.
 * @note This action is necessary by a static queue creation in the case of a
 *       SCU-CPU reset. 
 * @param pEvent Pointer to event queue object.
 */
ALWAYS_INLINE STATIC inline
void evDelete( EVENT_T* pEvent )
{
   pEvent->counter = 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Pushes a event in the queue.
 * @param pEvent Pointer to event queue object.
 * @retval true Action was successful.
 * @retval false Queue already full, no additional event stored.
 */
bool evPush( EVENT_T* pEvent );

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Pushes a event in the queue within a atomic section.
 * @param pEvent Pointer to event queue object.
 * @retval true Action was successful.
 * @retval false Queue already full, no additional event stored.
 */
bool evPushSafe( EVENT_T* pEvent );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Removes a event from the queue if any present.
 * @param pEvent Pointer to event queue object.
 * @retval true At least one event was in queue and has been removed.
 * @retval false No event in the queue.
 */
bool evPop( EVENT_T* pEvent );

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Removes a event from the queue within a atomic section 
 *        if any present.
 * @param pEvent Pointer to event queue object.
 * @retval true At least one event was in queue and has been removed.
 * @retval false No event in the queue.
 */
bool evPopSafe( EVENT_T* pEvent );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of events which are currently in the queue.
 */
ALWAYS_INLINE STATIC inline
EV_COUNTER_T evGetNumberOf( const EVENT_T* pEvent )
{
   return pEvent->counter;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns true if at least one event in the queue.
 * @param pEvent Pointer to event queue object.
 * @retval false No event in the queue.
 * @retval true At least one event in the queue.
 */
ALWAYS_INLINE STATIC inline
bool evIsPresent( const EVENT_T* pEvent )
{
   return evGetNumberOf( pEvent ) != 0;
}

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _SCU_EVENT_H */
/*================================== EOF ====================================*/
