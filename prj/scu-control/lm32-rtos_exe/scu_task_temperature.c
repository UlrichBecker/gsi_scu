/*!
 * @file scu_task_temperature.c
 * @brief FreeRTOS task for watching some temperature sensors via one wire bus.
 *
 * @see       scu_task_temperature.h
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
#include <FreeRTOS.h>
#include <task.h>
#include <lm32_syslog.h>
#include <scu_lm32_common.h>
#include <scu_temperature.h>
#include "scu_task_temperature.h"

#define TEMPERATURE_UPDATE_PERIOD 10

STATIC int getTenthDegree( const int v )
{
   return ((v & 0x0F) * 10) / 16;
}

STATIC int getDegree( const int v )
{
   return v >> 4;
}

STATIC void printTemperatures( void ) //TODO
{
   lm32Log( LM32_LOG_INFO, "Board temperature: %d.%d °C,   "
                           "Backplane temperature: %d °C,   "
                           "Extern temperature: %d °C,   ",
                           getDegree(g_shared.oSaftLib.oTemperatures.board_temp),
                           getTenthDegree(g_shared.oSaftLib.oTemperatures.board_temp),
                           getDegree(g_shared.oSaftLib.oTemperatures.backplane_temp),
                           getDegree(g_shared.oSaftLib.oTemperatures.ext_temp));
}

/*! ---------------------------------------------------------------------------
 * @brief RTOS-task updates periodically the temperature sensors via one wire
 *        bus
 */
STATIC void taskTempWatch( void* pTaskData UNUSED )
{
   taskInfoLog();

   TickType_t xLastWakeTime = xTaskGetTickCount();
   while( true )
   {
      updateTemperature();
      printTemperatures(); //TODO
      vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 * TEMPERATURE_UPDATE_PERIOD ));
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_temperature.h
 */
void taskStartTemperatureWatcher( void )
{
   TASK_CREATE_OR_DIE( taskTempWatch, 256, TASK_PRIO_TEMPERATURE, NULL );
}

/*================================== EOF ====================================*/
