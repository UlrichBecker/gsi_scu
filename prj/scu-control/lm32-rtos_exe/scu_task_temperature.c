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
#ifndef __DOCFSM__
 #include <FreeRTOS.h>
 #include <task.h>
 #include <lm32_syslog.h>
 #include <scu_lm32_common.h>
 #include <scu_temperature.h>
 #include "scu_task_temperature.h"
#endif

#define TEMPERATURE_UPDATE_PERIOD 10



STATIC int getDegree( const int v )
{
   return v >> 4;
}

STATIC int getTenthDegree( const int v )
{
   return getDegree((v & 0x0F) * 10);
}


#define FSM_DECLARE_STATE( state, attr... ) state

typedef enum
{
   FSM_DECLARE_STATE( ST_START, color = blue, label = 'Start of temperature\nwatching.' ),
   FSM_DECLARE_STATE( ST_NORMAL, color = green, label = 'Tamperature is whithin\nnormal range.'),
   FSM_DECLARE_STATE( ST_HIGH, color = magenta, label = 'Temperature is out of\nthe normal range!' ),
   FSM_DECLARE_STATE( ST_CRITICAL, color = red, label = 'Temperature is critical!' )
} STATE_T;

typedef struct PACKED
{
   int* pCurrentTemp;
   const char* name;
   STATE_T state;
} TEMP_WATCH_T;



#define TEMP_FORMAT( name )                             \
   getDegree( g_shared.oSaftLib.oTemperatures.name ),   \
   getTenthDegree(g_shared.oSaftLib.oTemperatures.name) \

STATIC void printTemperatures( void ) //TODO
{
   lm32Log( LM32_LOG_INFO, "Board temperature: %d.%d °C,   "
                           "Backplane temperature: %d °C,   "
                           "Extern temperature: %d °C,   ",
                           TEMP_FORMAT( board_temp ),
                           getDegree(g_shared.oSaftLib.oTemperatures.backplane_temp),
                           getDegree(g_shared.oSaftLib.oTemperatures.ext_temp));
}

#define TEMP_HIGH     50
#define TEMP_CRITICAL 75
#define HYSTERESIS     2

/*! ---------------------------------------------------------------------------
 * @brief RTOS-task updates periodically the temperature sensors via one wire
 *        bus
 */
STATIC void taskTempWatch( void* pTaskData UNUSED )
{
   taskInfoLog();

   #define FSM_INIT_FSM( init, attr... )       .state = init
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   TEMP_WATCH_T watchObject[3] =
   {
      {
         .pCurrentTemp = (int*)&g_shared.oSaftLib.oTemperatures.board_temp,
         .name = "board",
         FSM_INIT_FSM( ST_START, color = blue )
      },
      {
         .pCurrentTemp = (int*)&g_shared.oSaftLib.oTemperatures.backplane_temp,
         .name = "backplane",
         FSM_INIT_FSM( ST_START )
      },
      {
         .pCurrentTemp = (int*)&g_shared.oSaftLib.oTemperatures.ext_temp,
         .name = "extern",
         FSM_INIT_FSM( ST_START )
      }
   };
   #pragma GCC diagnostic pop

   #define FSM_TRANSITION( newState, attr... ) \
   {                                           \
      pWatchTemp->state = newState;            \
      break;                                   \
   }

   #define FSM_TRANSITION_SELF( attr... ) break

   TickType_t xLastWakeTime = xTaskGetTickCount();
   while( true )
   {
      updateTemperature();
      printTemperatures(); //TODO
      for( unsigned int i = 0; i < ARRAY_SIZE( watchObject ); i++ )
      {
         TEMP_WATCH_T* pWatchTemp = &watchObject[i];
         const STATE_T lastState = pWatchTemp->state;
         int currentTemperature = getDegree( *pWatchTemp->pCurrentTemp );
         switch( lastState )
         {
            case ST_START:
            {
               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, color = red );
               if( currentTemperature >= TEMP_HIGH )
                  FSM_TRANSITION( ST_HIGH, color = magenta );
               FSM_TRANSITION( ST_NORMAL, color = green );
            }
            case ST_NORMAL:
            {
               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, color = red );
               if( currentTemperature >= TEMP_HIGH )
                  FSM_TRANSITION( ST_HIGH, color = magenta );
               FSM_TRANSITION_SELF( color = green );
            }
            case ST_HIGH:
            {
               if( currentTemperature <= (TEMP_HIGH - HYSTERESIS) )
                  FSM_TRANSITION( ST_NORMAL, color = green );
               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, color = red );
               FSM_TRANSITION_SELF( color = magenta );
            }
            case ST_CRITICAL:
            {
               if( currentTemperature <= (TEMP_HIGH - HYSTERESIS) )
                  FSM_TRANSITION( ST_NORMAL, color = green );
               if( currentTemperature <= (TEMP_CRITICAL - HYSTERESIS) )
                  FSM_TRANSITION( ST_HIGH, color = magenta );
               FSM_TRANSITION_SELF( color = red );
            }  
         }

         if( lastState == pWatchTemp->state )
            continue;

         const unsigned int tenthDegree = getTenthDegree( *pWatchTemp->pCurrentTemp );
         switch( pWatchTemp->state )
         {
            case ST_NORMAL:
            {
               lm32Log( LM32_LOG_INFO,
                        "Temperature of %s is normal: %d.%u °C",
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }
            case ST_HIGH:
            {
               lm32Log( LM32_LOG_WARNING, ESC_WARNING
                        "WARNING: Temperature of %s is high: %d.%u °C" ESC_NORMAL,
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }
            case ST_CRITICAL:
            {
               lm32Log( LM32_LOG_ERROR, ESC_ERROR
                        "ERROR: Temperature of %s is critical: %d.%u °C" ESC_NORMAL,
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }
            default: break;
         }
      } /* for() */
      vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 * TEMPERATURE_UPDATE_PERIOD ));
   } /* while( true ) */
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_temperature.h
 */
void taskStartTemperatureWatcher( void )
{
   TASK_CREATE_OR_DIE( taskTempWatch, 256, TASK_PRIO_TEMPERATURE, NULL );
}

/*================================== EOF ====================================*/
