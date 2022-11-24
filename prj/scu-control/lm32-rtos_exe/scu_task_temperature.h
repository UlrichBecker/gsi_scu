/*!
 * @file scu_task_temperature.h
 * @brief FreeRTOS task for watching some temperature sensors via one wire bus.
 *
 * @see       scu_task_temperature.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.11.2022
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
#ifndef _SCU_TASK_TEMPERATURE_H
#define _SCU_TASK_TEMPERATURE_H
#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @ingroup RTOS_TASK
 * @brief Starts the task for the periodically temperature watching.
 */
void taskStartTemperatureWatcher( void );

/*! ---------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @brief Prints the three last received temperatures in the LM32-log system
 */
void printTemperatures( void );


#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_TASK_TEMPERATURE_H */
/*================================== EOF ====================================*/
