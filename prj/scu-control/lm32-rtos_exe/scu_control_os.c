/*!
 *  @file scu_control_os.c
 *  @brief Main module of SCU control including the main-thread FreeRTOS.
 *
 *  @bug The timer-interrupt doesn't work, when the application starts at first,
 *       but it works when the classical non-OS version "scu_control.bin"
 *       was loaded before.
 *
 *  @date 18.08.2022
 *
 *  @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
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

#ifdef CONFIG_COUNT_MSI_PER_IRQ
extern unsigned int g_msiCnt;
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
                 #ifdef CONFIG_STOP_ON_LM32_EXCEPTION
                     "+++ LM32 stopped! +++"
                 #else
                     "Restarting application!"
                 #endif
                     ESC_NORMAL, pcTaskName );
#ifdef CONFIG_STOP_ON_LM32_EXCEPTION
   /*
    * Remaining in the atomic section until reset. :-(
    */
   while( true );
#else
   LM32_RESTART_APP();
#endif
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
      /*!
       * @todo CAUTION: By this configuration sometimes a race condition will hurt when
       *       "saft-fg-ctl" becomes terminated. The main thread seems to be blocked and the
       *       application restarts.
       *       The more FGs are running in parallel the more often this effect happens! :-(
       */
      #warning This configuration is not stable yet.
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
            * In this way the MIL handler function knows the message comes
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
 * @brief Interrupt callback function for each Message Signaled Interrupt (MSI)
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
#ifdef CONFIG_COUNT_MSI_PER_IRQ
   unsigned int msiCnt = 0;
#endif
   MSI_ITEM_T m;

   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
   #ifdef CONFIG_COUNT_MSI_PER_IRQ
      msiCnt++;
   #endif
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
              // mprintf( "*\n" );
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
         #if defined( CONFIG_HANDLE_UNKNOWN_MSI ) && defined( CONFIG_QUEUE_ALARM  )
            queuePushWatched( &g_queueUnknownMsi, &m );
         #endif
            break;
         }
      }
   }
#ifdef CONFIG_COUNT_MSI_PER_IRQ
   if( g_msiCnt < msiCnt )
      g_msiCnt = msiCnt;
#endif
}

#if (configUSE_TICK_HOOK == 1 )
/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by each timer interrupt.
 */
#warning vApplicationTickHook() becomes implemented!
void vApplicationTickHook( void )
{
   if( irqIsSpecificEnabled( ECA_INTERRUPT_NUMBER ) )
      onScuMSInterrupt( ECA_INTERRUPT_NUMBER, NULL );
}
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Installs the interrupt callback function and clears the interrupt
 *        message buffer.
 * @see onScuMSInterrupt
 */
ONE_TIME_CALL void initInterrupt( void )
{
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   queueReset( &g_queueAddacDaq );
#endif
#ifdef CONFIG_MIL_FG
   queueReset( &g_queueMilFg );
   evDelete( &g_ecaEvent );
#endif
#ifdef CONFIG_QUEUE_ALARM
   queueReset( &g_queueAlarm );
 #ifdef CONFIG_HANDLE_UNKNOWN_MSI
   queueReset( &g_queueUnknownMsi );
 #endif
#endif
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

   /*!
    * @todo Check whether this delay is really necessary.
    */
 //  vTaskDelay( pdMS_TO_TICKS( 1500 ) );

   printCpuId();

   mmuAllocateDaqBuffer();

   scuLog( LM32_LOG_INFO, "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
   scuLog( LM32_LOG_INFO, "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser );
   scuLog( LM32_LOG_INFO, "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
   scuLog( LM32_LOG_INFO, "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
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
   #ifdef CONFIG_QUEUE_ALARM
      queuePollAlarm();
   #endif
   #ifdef CONFIG_USE_FG_MSI_TIMEOUT
      wdtPoll();
   #endif
      commandHandler();
   #if defined( CONFIG_MIL_FG ) && defined( CONFIG_HANDLE_UNUSED_ECAS )
      if( !taskIsMilTaskRunning() )
      {
         if( evPopSafe( &g_ecaEvent ) )
         {
            ecaTestTagAndPop( g_eca.pQueue, g_eca.tag );
            lm32Log( LM32_LOG_WARNING, ESC_WARNING
                     "Unused ECA with tag: 0x%04X received!"
                     ESC_NORMAL,
                     g_eca.pQueue->tag );
         }
      }
   #endif
      TASK_YIELD();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Function seems to be necessary, otherwise the timerinterrupt doesn't
 *        work if this application started at first, immediately after power-on.
 * @todo Why? Power-on interrupt?
 */
void cleanEcaQueue( void )
{
   const uint64_t future = getWrSysTime() + 1500000000L;
   while( getWrSysTime() < future )
   {
      NOP();
   }

   unsigned int i = 0;
   MSI_ITEM_T m;
   while( irqMsiCopyObjectAndRemoveIfActive( &m, ECA_INTERRUPT_NUMBER ) )
   {
      i++;
   }
   scuLog( LM32_LOG_DEBUG, ESC_DEBUG "%u MSIs removed.\n" ESC_NORMAL, i );
}

/*! ---------------------------------------------------------------------------
 * @brief Main-function: establishing the memory management unit (MMU) and
 *        LM32-logging system;
 *        creates the main task and start it.
 */
void main( void )
{
   irqClearEntryTab();
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
  /*
   * Initializing of the LM32-log-system for the Linux log-daemon "lm32logd" with a buffer
   * of a maximum of 1000 log-items in DDR3-RAM for SCU3 respectively SRAM for SCU4.
   */
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
#ifdef CONFIG_MIL_FG
   initEcaQueue();
#endif
   cleanEcaQueue();
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
