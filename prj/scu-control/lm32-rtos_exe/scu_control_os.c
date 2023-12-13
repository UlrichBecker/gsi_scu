/*!
 *  @file scu_control_os.c
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
#ifndef CONFIG_RTOS
 #error "ERROR: Application requires FreeRTOS, macro CONFIG_RTOS has to be defined in makefile!"
#endif
#include <scu_lm32_common.h>
#include <FreeRTOS.h>
#include <lm32signal.h>
#include <task.h>
#include <timers.h>
#include <scu_command_handler.h>
#ifdef CONFIG_USE_TEMPERATURE_WATCHER
 #include <scu_task_temperature.h>
#endif
#ifdef CONFIG_USE_ADDAC_FG_TASK
 #include <scu_task_fg.h>
#else
 #include <scu_fg_handler.h>
#endif
#ifdef CONFIG_MIL_FG
 #include <scu_eca_handler.h>
 #include <scu_task_mil.h>
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <scu_task_daq.h>
#endif
#include <ros_timeout.h>

#if !(defined( CONFIG_SCU3 ) != defined( CONFIG_SCU4 ))
  #error CONFIG_SCU3 or CONFIG_SCU4 has to be defined!
#endif

#if ( configCHECK_FOR_STACK_OVERFLOW != 0 )
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Becomes invoked in the case of a stack overflow.
 * @note Use this function for develop and debug purposes only!
 * @see https://www.freertos.org/Stacks-and-stack-overflow-checking.html
 * @see FreeRTOSConfig.h
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask UNUSED, char* pcTaskName )
{
   criticalSectionEnterBase();
   scuLog( LM32_LOG_ERROR,
           ESC_ERROR "Panic: Stack overflow at task \"%s\"!\n"
                     "+++ LM32 stopped! +++" ESC_NORMAL, pcTaskName );
   /*
    * Remaining in the atomic section until reset. :-(
    */
   while( true );
}
#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW != 0 ) */
#if ( configUSE_MALLOC_FAILED_HOOK != 0 )
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Becomes invoked when a memory allocation was not successful.
 * @note Use this function for develop and debug purposes only!
 * @see https://www.freertos.org/a00016.html
 * @see FreeRTOSConfig.h
 */
void vApplicationMallocFailedHook( void )
{
   die( "Memory allocation failed!" );
}
#endif /* #if ( configUSE_MALLOC_FAILED_HOOK != 0 ) */

/*!----------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @see scu_control.h
 */
void taskInfoLog( void )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Task: \"%s\" started.\n" ESC_NORMAL,
            pcTaskGetName( NULL ) );
}

/*!----------------------------------------------------------------------------
 * @see scu_control.h
 */
void taskDeleteIfRunning( void* pTaskHandle )
{
   if( *(TaskHandle_t*)pTaskHandle != NULL )
   {
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Deleting task: \"%s\".\n" ESC_NORMAL,
               pcTaskGetName( *(TaskHandle_t*)pTaskHandle ) );
      vTaskDelete( *(TaskHandle_t*)pTaskHandle  );
      *(TaskHandle_t*)pTaskHandle = NULL;
   }
}

/*!----------------------------------------------------------------------------
 * @see scu_control.h
 */
void taskDeleteAllRunningFgAndDaq( void )
{
#ifdef CONFIG_USE_ADDAC_FG_TASK
   taskStopFgIfRunning();
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   taskStopDaqIfRunning();
#endif
#ifdef CONFIG_MIL_FG
   taskStopMilIfRunning();
#endif
}

/*!----------------------------------------------------------------------------
 * @see scu_control.h
 */
void taskStartAllIfHwPresent( void )
{
#ifdef CONFIG_USE_ADDAC_FG_TASK
   taskStartFgIfAnyPresent();
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   taskStartDaqIfAnyPresent();
#endif
#ifdef CONFIG_MIL_FG
   taskStartMilIfAnyPresent();
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Handling of all SCU-bus MSI events.
 */
ONE_TIME_CALL void onScuBusEvent( const unsigned int slot )
{  /*!
    * @brief Contains the slot-number and IRQ-flags for ADDAC-FGs and ADDAC-DAQs
    */
   SCU_BUS_IRQ_QUEUE_T queueScuBusIrq;

   queueScuBusIrq.slot = slot;

   while( (queueScuBusIrq.pendingIrqs = scuBusGetAndResetIterruptPendingFlags( g_pScub_base, slot )) != 0)
   {
   #ifdef CONFIG_USE_ADDAC_FG_TASK
      /*
       * ADDAC function generators running in a separate RTOS-task.
       */
      if( (queueScuBusIrq.pendingIrqs & (FG1_IRQ | FG2_IRQ)) != 0 )
      {
         queuePushWatched( &g_queueFg, &queueScuBusIrq );
      #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_FG_TASK )
         taskWakeupFgFromISR();
      #endif
      }
   #else
      /*
       * ADDAC function generators running in this interrupt context.
       */
      if( (queueScuBusIrq.pendingIrqs & FG1_IRQ) != 0 )
      {
         handleAdacFg( slot, FG1_BASE );
      }

      if( (queueScuBusIrq.pendingIrqs & FG2_IRQ) != 0 )
      {
         handleAdacFg( slot, FG2_BASE );
      }
   #endif

   #ifdef CONFIG_MIL_FG
      if( (queueScuBusIrq.pendingIrqs & DREQ ) != 0 )
      { /*
         * MIL-SIO function generator recognized. 
         */
         TRACE_MIL_DRQ( "3\n" );
         const MIL_QEUE_T milMsg =
         { /*
            * The slot number is in any cases not zero.
            * In this way the MIL handler function knows it comes
            * from a SCU-bus SIO slave.
            */
            .slot = slot,
            .time = getWrSysTime()
         };

        /*!
         * @see milTask
         */
         queuePushWatched( &g_queueMilFg, &milMsg );
      #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
         taskWakeupMilFromISR();
      #endif
      }
   #endif /* ifdef CONFIG_MIL_FG */

   #ifdef CONFIG_SCU_DAQ_INTEGRATION
      if( (queueScuBusIrq.pendingIrqs & ((1 << DAQ_IRQ_DAQ_FIFO_FULL) | (1 << DAQ_IRQ_HIRES_FINISHED))) != 0 )
      {
         queuePushWatched( &g_queueAddacDaq, &queueScuBusIrq );
      #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_DAQ_TASK )
         taskWakeupDaqFromISR();
      #endif
      }
   #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Interrupt callback function for each Message Signaled Interrupt
 * @param intNum Interrupt number.
 * @param pContext Value of second parameter from irqRegisterISR not used
 *                 in this case.
 * @see initInterrupt
 * @see irqRegisterISR
 * @see irq_pop_msi
 * @see dispatch
 */
STATIC void onScuMSInterrupt( const unsigned int intNum,
                              const void* pContext UNUSED )
{
   MSI_ITEM_T m;

   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
      switch( GET_LOWER_HALF( m.adr )  )
      {
         case ADDR_SCUBUS:
         {
         #ifdef CONFIG_MIL_FG
            STATIC_ASSERT( ADDR_SCUBUS == 0 );
            if( (m.msg & ECA_VALID_ACTION) != 0 )
            { /*
               * ECA event received
               */
               evPushWatched( &g_ecaEvent );
             #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
               taskWakeupMilFromISR();
             #endif
               break;
            }
         #endif /* ifdef CONFIG_MIL_FG */

           /*
            * Message from SCU- bus.
            */
            onScuBusEvent( GET_LOWER_HALF( m.msg ) + SCUBUS_START_SLOT );
            break;
         }
         case ADDR_SWI:
         { /*
            * Command message from SAFT-lib
            */

            STATIC_ASSERT( sizeof( m.msg ) == sizeof( SAFT_CMD_T ) );
            queuePushWatched( &g_queueSaftCmd, &m.msg );
            break;
         }
     #if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
         case ADDR_DEVBUS:
         { /*
            * Message from MIL- extention bus respectively device-bus.
            * MIL-Piggy
            */
            const MIL_QEUE_T milMsg =
            { /*
               * In the case of MIL-PIGGY the slot-number has to be zero.
               * In this way the MIL handler function becomes to know that.
               */
               .slot = 0,
               .time = getWrSysTime()
            };

           /*!
            * @see milDeviceHandler
            */
            queuePushWatched( &g_queueMilFg, &milMsg );
         #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_MIL_TASK )
            taskWakeupMilFromISR();
         #endif
            break;
         }
     #endif /* if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY ) */
         default:
         {
            FG_ASSERT( false );
            break;
         }
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Installs the interrupt callback function and clears the interrupt
 *        message buffer.
 * @see onScuMSInterrupt
 */
ONE_TIME_CALL void initInterrupt( void )
{
   initCommandHandler();
   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );
   scuLog( LM32_LOG_INFO, "IRQ table configured: 0b%b\n", irqGetMaskRegister() );
}

/*! ---------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief Main task controls all other tasks and polls the command-module,
 *        the MSI-timeout watchdog and the queue-overflow watchdog,
 *        rotates a software fan as still alive signal.
 * 
 * @note All other tasks becomes started or stopped by this task,
 *       except the idle-task of the FreeRTOS kernel.
 */
STATIC void taskMain( void* pTaskData UNUSED )
{
   taskInfoLog();

   tellMailboxSlot();

   vTaskDelay( pdMS_TO_TICKS( 1500 ) );

   printCpuId();

   mmuAllocateDaqBuffer();

   scuLog( LM32_LOG_INFO, "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
   scuLog( LM32_LOG_INFO, "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser );
   scuLog( LM32_LOG_INFO, "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );

#ifdef CONFIG_MIL_FG
 #ifdef CONFIG_MIL_PIGGY
   scuLog( LM32_LOG_INFO, "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
 #endif
   initEcaQueue();
#endif

   initAndScan();

   initInterrupt();
#ifdef CONFIG_USE_TEMPERATURE_WATCHER
   taskStartTemperatureWatcher();
#endif
   taskStartAllIfHwPresent();

   scuLog( LM32_LOG_INFO, ESC_FG_GREEN ESC_BOLD
           "\n *** Initialization done, %u tasks running, entering main-loop of task \"%s\" ***\n\n"
           ESC_NORMAL, uxTaskGetNumberOfTasks(), pcTaskGetName( NULL ) );

#ifdef CONFIG_STILL_ALIVE_SIGNAL
   unsigned int i = 0;
   static const char fan[] = { '|', '/', '-', '\\' };
   TIMEOUT_T fanInterval;
   toStart( &fanInterval, pdMS_TO_TICKS( 250 ) );
#endif

   /*
    *      *** Main-loop ***
    */
   while( true )
   {
   #ifdef CONFIG_STILL_ALIVE_SIGNAL
      /*
       * Showing a animated software fan as still alive signal
       * on the LM32- console.
       */
      if( toInterval( &fanInterval ) )
      {
         mprintf( ESC_BOLD "\r%c" ESC_NORMAL, fan[i++] );
         i %= ARRAY_SIZE( fan );
      }
   #endif
      queuePollAlarm();
   #ifdef CONFIG_USE_FG_MSI_TIMEOUT
      wdtPoll();
   #endif
      commandHandler();
      TASK_YIELD();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Main-function: establishing the memory management unit (MMU) and
 *        LM32-logging system;
 *        creates the main task and start it.
 */
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
             g_pCpuMsiBox,
             g_pMyMsi,
             sizeof( SCU_SHARED_DATA_T )
          );
  MMU_STATUS_T status;
  status = lm32LogInit( 1000 );
  mprintf( "\nMMU- status: %s\n", mmuStatus2String( status ) );
  if( !mmuIsOkay( status ) )
  {
     mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
     while( true );
  }
  mprintf( "Maximum log extra parameter: " TO_STRING(LM32_LOG_NUM_OF_PARAM) "\n" );
  lm32Log( LM32_LOG_INFO, text,
                          __reset_count,
                          g_pCpuMsiBox,
                          g_pMyMsi,
                          sizeof( SCU_SHARED_DATA_T )
          );

   scuLog( LM32_LOG_DEBUG, ESC_DEBUG "Addr nesting count: 0x%p\n" ESC_NORMAL,
           irqGetNestingCountPointer() );

   initializeGlobalPointers();

   /*
    * Creating the main task.
    */
   TASK_CREATE_OR_DIE( taskMain, 1024, TASK_PRIO_MAIN, NULL );

   scuLog( LM32_LOG_DEBUG, ESC_DEBUG "Starting sheduler...\n" ESC_NORMAL );
   /*
    * Starting the main- and idle- task.
    * If successful, this function will not left.
    */
   vTaskStartScheduler();

   die( "This point shall never be reached!" );
}

/*================================== EOF ====================================*/
