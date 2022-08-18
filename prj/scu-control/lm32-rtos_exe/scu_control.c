/*!
 *  @file scu_control.c
 *  @brief Main module of SCU control including the main-thread FreeRTOS.
 *
 *  @date 18.08.2022
 *
 *  @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#include <scu_lm32_common.h>
#include <FreeRTOS.h>
#include <lm32signal.h>
#include <task.h>

#define MAIN_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception has
 *        been appeared.
 */
void _onException( const uint32_t sig )
{
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
   scuLog( LM32_LOG_ERROR, ESC_ERROR "Exception occurred: %d -> %s\n"
                                  #ifdef CONFIG_STOP_ON_LM32_EXCEPTION
                                     "System stopped!\n"
                                  #endif
                           ESC_NORMAL, sig, str );
#ifdef CONFIG_STOP_ON_LM32_EXCEPTION
   irqDisable();
   while( true );
#endif
}

STATIC void vMainTask( void* pTaskData )
{
   scuLog( LM32_LOG_INFO, "Task: \"%s\" started.\n", pcTaskGetName( NULL ) );

   while( true )
   {
   }
}

void main( void )
{
   const char* text;
   text = ESC_BOLD "\nStart of \"" TO_STRING(TARGET_NAME) "\"" ESC_NORMAL
           ", Version: " TO_STRING(SW_VERSION) "\n"
           "Compiler: "COMPILER_VERSION_STRING" Std: " TO_STRING(__STDC_VERSION__) "\n"
           "Git revision: "TO_STRING(GIT_REVISION)"\n"
           "Resets: %u\n"
           "Found MsgBox at 0x%p. MSI Path is 0x%p\n"
           "Shared memory size: %u bytes\n"
       #if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
            ESC_WARNING
            "CAUTION! Time gap reading for MIL FGs implemented!\n"
            ESC_NORMAL
       #endif
            ;
   mprintf( text,
             __reset_count,
             pCpuMsiBox,
             pMyMsi,
             sizeof( SCU_SHARED_DATA_T )
          );
#ifdef CONFIG_USE_MMU
  MMU_STATUS_T status;
#endif
#if defined( CONFIG_USE_MMU ) && !defined( CONFIG_USE_LM32LOG )
  MMU_OBJ_T mmu;
  status = mmuInit( &mmu );
#endif
#ifdef CONFIG_USE_LM32LOG
  status = lm32LogInit( 1000 );
#endif
#ifdef CONFIG_USE_MMU
  mprintf( "\nMMU- status: %s\n", mmuStatus2String( status ) );
  if( !mmuIsOkay( status ) )
  {
     mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
     while( true );
  }
#ifdef CONFIG_USE_MMU
  lm32Log( LM32_LOG_INFO, text,
                          __reset_count,
                          pCpuMsiBox,
                          pMyMsi,
                          sizeof( SCU_SHARED_DATA_T )
          );
#endif
#endif

   BaseType_t xReturned;

   xReturned = xTaskCreate(
                vMainTask,                    /* Function that implements the task. */
                "main",              /* Text name for the task. */
                configMINIMAL_STACK_SIZE, /* Stack size in words, not bytes. */
                NULL,                     /* Parameter passed into the task. */
                MAIN_TASK_PRIORITY,       /* Priority at which the task is created. */
                NULL                      /* Used to pass out the created task's handle. */
              );
   if( xReturned != pdPASS )
   {
      die( "Error by creating main task!" );
   }

   vTaskStartScheduler();
}

/*================================== EOF ====================================*/
