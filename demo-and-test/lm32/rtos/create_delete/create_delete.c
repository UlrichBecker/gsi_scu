/*!
 * @file   reate_delete.c
 * @brief  FreeRTOS queue test on LM32 in SCU
 *
 * Sending of message queues from normal tasks and form a interrupt context.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      25.08.2022
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
#include <eb_console_helper.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ros_timeout.h>
#include <lm32signal.h>

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

/*!----------------------------------------------------------------------------
 * @brief Helper macro for creating a task.
 * @param func Name of task function.
 * @param stackSize Additional stack size.
 * @param prio Priority
 * @param handle Pointer to the task handle,
 *               if not used then it has to be NULL.
 * @retval pdPASS Success.
 */
#define TASK_CREATE( func, stackSize, prio, handle )                          \
   xTaskCreate( func, #func,                                                  \
                configMINIMAL_STACK_SIZE + stackSize,                         \
                NULL,                                                         \
                tskIDLE_PRIORITY + prio, handle )

/*!----------------------------------------------------------------------------
 * @brief Helper Macro creates a task or when not successful it stops the CPU
 *        by a error-log message.
 * @param func Name of task function.
 * @param stackSize Additional stack size.
 * @param prio Priority
 * @param handle Pointer to the task handle,
 *               if not used then it has to be NULL.
 */
#define TASK_CREATE_OR_DIE( func, stackSize, prio, handle )                   \
   if( TASK_CREATE( func, stackSize, prio, handle ) != pdPASS )               \
   {                                                                          \
      mprintf( ESC_ERROR "Can't create task: " #func ESC_NORMAL );            \
      while( true );                                                          \
   }

STATIC TaskHandle_t mg_childTaskandle = NULL;

/*!----------------------------------------------------------------------------
 */
void taskInfoLog( void )
{
   mprintf( ESC_BOLD "Task: \"%s\" started.\n" ESC_NORMAL,
            pcTaskGetName( NULL ) );
}

/*!----------------------------------------------------------------------------
 */
void taskDeleteIfRunning( void* pTaskHandle )
{
   if( *(TaskHandle_t*)pTaskHandle != NULL )
   {
      ATOMIC_SECTION() mprintf( ESC_BOLD "Deleting task: \"%s\".\n" ESC_NORMAL,
               pcTaskGetName( *(TaskHandle_t*)pTaskHandle ) );
      const unsigned int prevTaskNum = uxTaskGetNumberOfTasks();
      vTaskDelete( *(TaskHandle_t*)pTaskHandle  );
      *(TaskHandle_t*)pTaskHandle = NULL;
      while( prevTaskNum == uxTaskGetNumberOfTasks() )
      {
         mprintf( "*" );
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception appeared.
 */
void _onException( const uint32_t sig )
{
   irqDisable();
   char* str;
   #define _CASE_SIGNAL( S ) case S: str = #S; break;
   switch( sig )
   {
      _CASE_SIGNAL( SIGINT )
      _CASE_SIGNAL( SIGTRAP )
      _CASE_SIGNAL( SIGFPE )
      _CASE_SIGNAL( SIGSEGV )
      default: str = "unknown"; break;
   }
   mprintf( ESC_ERROR "%s( %d ): %s\n" ESC_NORMAL, __func__, sig, str );
   while( true );
}

STATIC void childTask( void* pvParameters UNUSED )
{
   taskInfoLog();
#if 1
   unsigned int i = 0;
   const char fan[] = { '|', '/', '-', '\\' };
   TIMEOUT_T fanInterval;
   toStart( &fanInterval, pdMS_TO_TICKS( 250 ) );
   while( true )
   {
      if( toInterval( &fanInterval ) )
      {
         mprintf( ESC_BOLD "\r%c" ESC_NORMAL, fan[i++] );
         i %= ARRAY_SIZE( fan );
      }
   }
#else
   while( true )
   {
      vPortYield();
   }
#endif
}

STATIC void parrentTask( void* pvParameters UNUSED )
{
   taskInfoLog();

   unsigned int i = 0;
   
   TIMEOUT_T interval;
   toStart( &interval, pdMS_TO_TICKS( 2000 ) );
   while( true )
   {
      if( toInterval( &interval ))
      {
         mprintf( "Tick: %u, tasks: %u\n", ++i, uxTaskGetNumberOfTasks() );
         if( mg_childTaskandle == NULL )
         {
            TASK_CREATE_OR_DIE( childTask, 0, 1, &mg_childTaskandle )
         }
         else
         {
            taskDeleteIfRunning( &mg_childTaskandle );
         }
      }
   }
}

void main( void )
{
    mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR
            "FreeRTOS create and delete of sub-tasks test\n"
            "Compiler:  " COMPILER_VERSION_STRING "\n"
            "Tick-rate: %d Hz\n", configTICK_RATE_HZ );

   TASK_CREATE_OR_DIE( parrentTask, 1024, 1, NULL );
   vTaskStartScheduler();
   irqDisable();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n" ESC_NORMAL );
   while( true );
}
/*================================== EOF =====================================*/
