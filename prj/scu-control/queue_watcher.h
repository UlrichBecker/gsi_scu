/*!
 * @file queue_watcher.h
 * @brief Queue watcher for data- and event- queues.
 *
 * Watching a queue for a possible overflow and gives alarm if happened.
 *
 * @date 19.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see queue_watcher.c
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
#ifndef _QUEUE_WATCHER_H
#define _QUEUE_WATCHER_H

#include <sw_queue.h>
#include <scu_event.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_QUEUE_ALARM

#ifdef CONFIG_QUEUE_ALARM

extern SW_QUEUE_T g_queueAlarm;

#ifdef CONFIG_HANDLE_UNKNOWN_MSI
extern SW_QUEUE_T g_queueUnknownMsi;
#endif

/*! ---------------------------------------------------------------------------
 * @brief Put a message in the given queue object.
 *
 * If the concerned queue is full, then a alarm-item will put in the
 * alarm-queue which becomes evaluated in the function queuePollAlarm().
 *
 * @see queuePollAlarm.
 * @param pThis Pointer to the queue object.
 * @param pItem Pointer to the payload object.
 */
STATIC inline
void queuePushWatched( SW_QUEUE_T* pThis, const void* pItem )
{
   if( queueForcePush( pThis, pItem ) )
      return;
   queuePush( &g_queueAlarm, &pThis );
}

/*! ---------------------------------------------------------------------------
 * @brief Put a event in the given event-queue object.
 *
 * If the concerned queue is full, then a alarm-item will put in the
 * alarm-queue which becomes evaluated in the function queuePollAlarm().
 *
 * @see queuePollAlarm.
 * @param pThis Pointer to the queue object.
 * @param pItem Pointer to the payload object.
 */
STATIC inline
void evPushWatched( EVENT_T* pEvent )
{
   if( evPush( pEvent ) )
      return;
   queuePush( &g_queueAlarm, &pEvent );
}

/*! ----------------------------------------------------------------------------
 * @brief Checks whether a possible overflow has been happen in one or more
 *        of the used message queues.
 */
void queuePollAlarm( void );

#else
#define queuePushWatched queuePush
#define queuePollAlarm()
#endif

#ifdef __cplusplus
}
#endif
#endif /* ifndef _QUEUE_WATCHER_H */
/*================================== EOF ====================================*/
