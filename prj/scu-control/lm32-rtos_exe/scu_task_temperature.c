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
#define _CONFIG_TEMPERATURE_WATCH_GRADIENT

#ifndef __DOCFSM__
 #include <FreeRTOS.h>
 #include <task.h>
 #include <stdlib.h>
 #include <lm32_syslog.h>
 #include <scu_lm32_common.h>
 #include <scu_temperature.h>
 #include "scu_task_temperature.h"
#endif

//#define CONFIG_DEBUG_TEMPERATURE_WATCHER

#define TEMPERATURE_UPDATE_PERIOD 10

#ifdef CONFIG_DEBUG_TEMPERATURE_WATCHER
 #define TEMP_HIGH     30
 #define TEMP_CRITICAL 40
#else
 #define TEMP_HIGH     50
 #define TEMP_CRITICAL 75
#endif

#define HYSTERESIS     2
#define MAX_TEMP_GRADIENT 10
#define MAX_TEMPERATURE 1000

#if ( TEMP_HIGH >= TEMP_CRITICAL )
 #error "Error: TEMP_HIGH has to be smaller than TEMP_CRITICAL!"
#endif

/*!
 * @ingroup TEMPERATURE
 * @todo Introducing a additional warning for low temperature.
 */

/*! --------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @brief Calculates the temperature in degree without decimal places.
 */
STATIC int getDegree( const uint32_t v )
{
   return v >> 4;
}

/*! --------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @brief Calculates the first decimal place of the temperature in degree.
 */
STATIC unsigned int getTenthDegree( const uint32_t v )
{
   return getDegree((v & 0x0F) * 10);
}

/*! --------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_DECLARE_STATE( state, attr... ) state

/*! --------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @brief Definition of the states of the finite state machine (FSM)
 */
typedef enum
{
   FSM_DECLARE_STATE( ST_START, color = blue, label = 'Start of temperature\nwatching.' ),
   FSM_DECLARE_STATE( ST_NORMAL, color = green, label = 'Tamperature is whithin\nnormal range.'),
   FSM_DECLARE_STATE( ST_HIGH, color = magenta, label = 'Temperature is out of\nthe normal range!' ),
   FSM_DECLARE_STATE( ST_CRITICAL, color = red, label = 'Temperature is critical!' )
} STATE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @brief Definition of the temperature watch object. 
 */
typedef struct
{  /*!
    * @brief Points to the received value of the concerning temperature sensor.
    */
   uint32_t* pCurrentTemp;

#ifdef _CONFIG_TEMPERATURE_WATCH_GRADIENT
   /*!
    * @brief Value of the last measured temperature for watching of the
    *        temperature gradient.
    */
   int       lastTemperature;

   /*!
    * @brief Flag prevents an multiple warning log message.
    */
   bool      wasGradientError;
#endif
   /*!
    * @brief Sensor error detected.
    *        Prevents multiple error log messages.
    */
   bool      wasError;

   /*!
    * @brief Name of the temperature sensor.
    */
   const char* name;

   /*!
    * @brief Keeps the current state of the FSM.
    */
   STATE_T state;
} TEMP_WATCH_T;

#define BOARD_TEMP      g_shared.oSaftLib.oTemperatures.board_temp
#define BACKPLANE_TEMP  g_shared.oSaftLib.oTemperatures.backplane_temp
#define EXTERN_TEMP     g_shared.oSaftLib.oTemperatures.ext_temp

#define TEMP_FORMAT( name )    \
   getDegree( name ),          \
   getTenthDegree( name )      \

/*! ---------------------------------------------------------------------------
 * @see scu_task_temperature.h
 */
void printTemperatures( void )
{
   criticalSectionEnter();
   volatile const uint32_t boardTemp     = BOARD_TEMP;
   volatile const uint32_t backplaneTemp = BACKPLANE_TEMP;
   volatile const uint32_t externTemp    = EXTERN_TEMP;
   criticalSectionExit();

   lm32Log( LM32_LOG_INFO, "Board temperature:     %d.%u °C", TEMP_FORMAT( boardTemp ) );
   lm32Log( LM32_LOG_INFO, "Backplane temperature: %d.%u °C", TEMP_FORMAT( backplaneTemp ) );
   lm32Log( LM32_LOG_INFO, "Extern temperature:    %d.%u °C", TEMP_FORMAT( externTemp ) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @ingroup RTOS_TASK
 * @brief RTOS-task updates periodically the temperature sensors via one-wire-
 *        bus and sends log messages to the LM32- logging system if one
 *        or more temperatures exceeds a critical threshold.  
 */
STATIC void taskTempWatch( void* pTaskData UNUSED )
{
   taskInfoLog();

   /*!
    * @see https://github.com/UlrichBecker/DocFsm
    */
   #define FSM_INIT_FSM( init, attr... )  .state = init

   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   TEMP_WATCH_T watchObject[3] =
   {
      {
         .pCurrentTemp = &BOARD_TEMP,
         .name = "board",
         .wasError = false,
         FSM_INIT_FSM( ST_START, color = blue )
      },
      {
         .pCurrentTemp = &BACKPLANE_TEMP,
         .name = "backplane",
         .wasError = false,
         .state = ST_START
      },
      {
         .pCurrentTemp = &EXTERN_TEMP,
         .name = "extern",
         .wasError = false,
         .state = ST_START
      }
   };
   #pragma GCC diagnostic pop

   /*!
    * https://github.com/UlrichBecker/DocFsm
    */
   #define FSM_TRANSITION( newState, attr... ) \
   {                                           \
      pWatchTemp->state = newState;            \
      break;                                   \
   }

   /*!
    * https://github.com/UlrichBecker/DocFsm
    */
   #define FSM_TRANSITION_SELF( attr... ) break

   TickType_t xLastWakeTime = xTaskGetTickCount();
   /*
    *     *** Task main- loop ***
    */
   while( true )
   {
      updateTemperature();
   #ifdef CONFIG_DEBUG_TEMPERATURE_WATCHER
      #warning Temperature task will compiled in debug mode!
      printTemperatures();
   #endif
      /*
       * For each temperature sensor...
       */
      for( unsigned int i = 0; i < ARRAY_SIZE( watchObject ); i++ )
      {
         TEMP_WATCH_T* const pWatchTemp = &watchObject[i];
         const int currentTemperature = getDegree( *pWatchTemp->pCurrentTemp );
         const STATE_T lastState = pWatchTemp->state;

      #ifdef _CONFIG_TEMPERATURE_WATCH_GRADIENT
         /*
          * Every now and then a invalid value will read from the temperature sensor.
          * Therefore a check of the temperature gradient will made here.
          */
         const int gradient = currentTemperature - pWatchTemp->lastTemperature;
         pWatchTemp->lastTemperature = currentTemperature;
         if( lastState != ST_START )
         {
            if( abs( gradient ) >= MAX_TEMP_GRADIENT )
            { /*
               * Impossible temperature gradient, perhaps a measurement error.
               * Jump to the next temperature sensor.
               */
               if( !pWatchTemp->wasGradientError )
               {
                  pWatchTemp->wasGradientError = true;
                  lm32Log( LM32_LOG_WARNING, ESC_WARNING
                           "WARNING: Impossible temperature gradient (%d°C/"
                           TO_STRING(TEMPERATURE_UPDATE_PERIOD) "sec) from sensor: \"%s\"!"
                           ESC_NORMAL, gradient, pWatchTemp->name );
               }
               continue;
            }
         }
         pWatchTemp->wasGradientError = false;
      #endif

         if( currentTemperature > MAX_TEMPERATURE )
         { /*
            * Impossible temperature received, perhaps the sensor is demaged or not present.
            * Jump to the next temperature sensor.
            */
            if( !pWatchTemp->wasError )
            {
               pWatchTemp->wasError = true;
               lm32Log( LM32_LOG_ERROR, ESC_ERROR
                        "ERROR: Temperature sensor \"%s\" failed!"
                        ESC_NORMAL, pWatchTemp->name  );
            }
            continue;
         }
         pWatchTemp->wasError = false;

         /*
          * Executing the FSM-do function
          */
         switch( lastState )
         {
            case ST_START:
            {
               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, fontcolor = red, color = red, label = 'T >= TC' );

               if( currentTemperature >= TEMP_HIGH )
                  FSM_TRANSITION( ST_HIGH, fontcolor = magenta, color = magenta, label = 'T >= TH' );

               FSM_TRANSITION( ST_NORMAL, color = green );
            }

            case ST_NORMAL:
            {
               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, fontcolor = red, color = red, label = 'T >= TC' );

               if( currentTemperature >= TEMP_HIGH )
                  FSM_TRANSITION( ST_HIGH, fontcolor = magenta, color = magenta, label = 'T >= TH' );

               FSM_TRANSITION_SELF( color = green );
            }

            case ST_HIGH:
            {
               if( currentTemperature <= (TEMP_HIGH - HYSTERESIS) )
                  FSM_TRANSITION( ST_NORMAL, fontcolor = green, color = green, label = 'T <= (TH - H)' );

               if( currentTemperature >= TEMP_CRITICAL )
                  FSM_TRANSITION( ST_CRITICAL, fontcolor = red, color = red, label = 'T >= TC' );

               FSM_TRANSITION_SELF( color = magenta );
            }

            case ST_CRITICAL:
            {
               if( currentTemperature <= (TEMP_HIGH - HYSTERESIS) )
                  FSM_TRANSITION( ST_NORMAL, fontcolor = green, color = green, label = 'T <= (TH - H)' );

               if( currentTemperature <= (TEMP_CRITICAL - HYSTERESIS) )
                  FSM_TRANSITION( ST_HIGH, fontcolor = magenta, color = magenta, label = 'T <= (TC - H)' );

               FSM_TRANSITION_SELF( color = red );
            }
         }

         /*
          * Has state changed?
          */
         if( lastState == pWatchTemp->state )
         { /*
            * No!
            */
            continue;
         }

         /*
          * State has changed executing the FSM-entry-function.
          */
         const unsigned int tenthDegree = getTenthDegree( *pWatchTemp->pCurrentTemp );
         switch( pWatchTemp->state )
         {
            case ST_NORMAL:
            {
               lm32Log( LM32_LOG_INFO,
                        "Temperature of \"%s\" is normal: %d.%u °C",
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }

            case ST_HIGH:
            {
               lm32Log( LM32_LOG_WARNING, ESC_WARNING
                        "WARNING: Temperature of \"%s\" is high: %d.%u °C" ESC_NORMAL,
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }

            case ST_CRITICAL:
            {
               lm32Log( LM32_LOG_ERROR, ESC_ERROR
                        "ERROR: Temperature of \"%s\" is critical: %d.%u °C" ESC_NORMAL,
                        pWatchTemp->name, 
                        currentTemperature, tenthDegree );
               break;
            }

            default: break;
         }
      } /* for() */

      /*
       * Task will sleep for the in TEMPERATURE_UPDATE_PERIOD defined seconds.
       */
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
