/*!
 * @file ros_mutex.h
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
#ifndef _ROS_MUTEX_H
#define _ROS_MUTEX_H

#ifndef CONFIG_RTOS
#error Macro CONFIG_RTOS is not defined in Makefile.
#endif

#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ingroup ATOMIC
 * @brief Motex-object.
 */
typedef struct
{ /*!
   * @brief Handle of the task which has the mutex currently locked or NULL.
   */
   TaskHandle_t lockedTask;

  /*!
   * @brief Mutex nesting counter
   */
   unsigned int nestingCount;
} OS_MUTEX_T;


/*! ------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Initializes the mutex.
 * @param pThis Pointer to the mutex-object.
 */
void osMutexInit( OS_MUTEX_T volatile * pThis );

/*! ------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Waits until the mutex is free and lock it if free.
 *        counterpart of osMutexUnlock.
 *
 * Nesting calls in the same task are possible.
 *
 * @note When this mutex is already locked by a other task then other tasks
 *       will performed until this mutex becomes free.
 * @param pThis Pointer to the mutex-object.
 */
void osMutexLock( OS_MUTEX_T volatile * pThis );

/*! ------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Tries to lock the mutex. That is a non blocking function.
 * @param pThis Pointer to the mutex-object.
 * @retval true Locking was successful.
 * @retval false Mutex has already been locked by a other task.
 */
bool osMutexTryLock( OS_MUTEX_T volatile * pThis );

/*! ------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Unlock the mutex. Counterpart of osMutexLock
 * @param pThis Pointer to the mutex-object.
 */
void osMutexUnlock( OS_MUTEX_T volatile * pThis );

/*! ------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Returns true when the mutex is locked.
 * @param pThis Pointer to the mutex-object.
 */
bool osMutexIsLocked( OS_MUTEX_T volatile * pThis );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _ROS_MUTEX_H */
/*================================== EOF ====================================*/
