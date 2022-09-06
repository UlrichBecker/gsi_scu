/*!
 * @file ros_timeout.h
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
#ifndef _ROS_TIMEOUT_H
#define _ROS_TIMEOUT_H

#include <stdbool.h>
#include <FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @defgroup ROS_TIMEOUT Timeout handling, additional module for FreeRTOS
 */

/*!
 * @ingroup ROS_TIMEOUT
 * @brief Timeout object for FreeRTOS
 */
typedef struct
{  /*!
    * @brief Timeout duration in system-ticks.
    */
   TickType_t duration;

   /*!
    * @brief Trigger threshold for timeout.
    */
   TickType_t threshold;

   /*!
    * @brief Overflow flag.
    */
   bool overflow;
} TIMEOUT_T;


/*! ---------------------------------------------------------------------------
 * @ingroup ROS_TIMEOUT
 * @brief Initializes the timeout object and starts it.
 * @param pTimeout Pointer to the timeout object.
 * @param duration Duration of timeout in system-ticks.
 */
void toStart( TIMEOUT_T* pTimeout, const TickType_t duration );

/*! ---------------------------------------------------------------------------
 * @ingroup ROS_TIMEOUT
 * @brief Restarts the timeout duration.
 * @param pTimeout Pointer to the timeout object.
 */
void toRestart( TIMEOUT_T* pTimeout );

/*! ---------------------------------------------------------------------------
 * @ingroup ROS_TIMEOUT
 * @brief Returns "true" when the timeout time has elapsed since "toStart()"
 *        or "toRestart()"
 * @param pTimeout Pointer to the timeout object.
 * @retval true Timeout time elapsed.
 * @retval false No timeout yet.
 */
bool toIsElapsed( TIMEOUT_T* pTimeout );

/*! ---------------------------------------------------------------------------
 * @ingroup ROS_TIMEOUT
 * @brief Returns "true" when the timeout time has elapsed since "toStart()"
 *        or "toRestart()" and restarts the timeout interval.
 * @param pTimeout Pointer to the timeout object.
 * @retval true Timeout time elapsed.
 * @retval false No timeout yet.
 */
bool toInterval( TIMEOUT_T* pTimeout );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _ROS_TIMEOUT_H */
/*================================== EOF ====================================*/
