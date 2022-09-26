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
#include <scu_lm32_common.h>
#include <FreeRTOS.h>
#include <lm32signal.h>
#include <task.h>
#include <timers.h>
#include <scu_command_handler.h>

#include <scu_task_fg.h>
#ifdef CONFIG_MIL_FG
 #include <scu_eca_handler.h>
 #include <scu_task_mil.h>
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <scu_task_daq.h>
#endif
#include <ros_timeout.h>

extern volatile uint32_t __atomic_section_nesting_count;

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
   scuLog( LM32_LOG_ERROR,
           ESC_ERROR "Panic: Stack overflow at task \"%s\"!\n"
                     "+++ LM32 stopped! +++" ESC_NORMAL, pcTaskName );
   irqDisable();
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
 * @see scu_control.h
 */
void taskInfoLog( void )
{
   lm32Log( LM32_LOG_INFO, ESC_BOLD "Task: \"%s\" started.\n" ESC_NORMAL,
            pcTaskGetName( NULL ) );
}

/*!----------------------------------------------------------------------------
 * @see scu_control.h
 */
void taskDeleteIfRunning( void* pTaskHandle )
{
   if( *(TaskHandle_t*)pTaskHandle != NULL )
   {
      lm32Log( LM32_LOG_INFO, ESC_BOLD "Deleting task: \"%s\".\n" ESC_NORMAL,
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
   taskStopFgIfRunning();
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
   taskStartFgIfAnyPresent();
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

   while( (queueScuBusIrq.pendingIrqs = scuBusGetAndResetIterruptPendingFlags((void*)g_pScub_base, slot )) != 0)
   {
      if( (queueScuBusIrq.pendingIrqs & (FG1_IRQ | FG2_IRQ)) != 0 )
      {
         queuePushWatched( &g_queueFg, &queueScuBusIrq );
      }
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
      }
   #endif

   #ifdef CONFIG_SCU_DAQ_INTEGRATION
      if( (queueScuBusIrq.pendingIrqs & ((1 << DAQ_IRQ_DAQ_FIFO_FULL) | (1 << DAQ_IRQ_HIRES_FINISHED))) != 0 )
      {
         queuePushWatched( &g_queueAddacDaq, &queueScuBusIrq );
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
               break;
            }
         #endif

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
     #ifdef CONFIG_MIL_FG
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
            break;
         }
     #endif /* ifdef CONFIG_MIL_FG */
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

#if (configUSE_TIMERS > 0)
void onTimer( TimerHandle_t xTimer UNUSED )
{
   static unsigned int count = 0;
  // ATOMIC_SECTION()
   //   lm32Log( LM32_LOG_DEBUG, "%s", __func__ );
   //ATOMIC_SECTION()
   //mprintf(  "%s: %u", __func__, count );
   //ATOMIC_SECTION()
   count++;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Main task
 */
STATIC void taskMain( void* pTaskData UNUSED )
{
   taskInfoLog();

   tellMailboxSlot();


   //vTaskDelay( pdMS_TO_TICKS( 1500 ) );

   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      die( "No SYS_CON found!" );
   scuLog( LM32_LOG_INFO, "SYS_CON found on addr: 0x%p\n", BASE_SYSCON );

   printCpuId();

   mmuAllocateDaqBuffer();

   scuLog( LM32_LOG_INFO, "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
   scuLog( LM32_LOG_INFO, "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser );
   scuLog( LM32_LOG_INFO, "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );

#ifdef CONFIG_MIL_FG
   scuLog( LM32_LOG_INFO, "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
   initEcaQueue();
#endif

   initAndScan();

   initInterrupt();

#if (configUSE_TIMERS > 0)
   TimerHandle_t th = xTimerCreate( "Timer", pdMS_TO_TICKS( 1000 ), pdTRUE, NULL, onTimer );
   xTimerStart( th, 0 );
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

   while( true )
   {
   #ifdef CONFIG_STILL_ALIVE_SIGNAL
      /*
       * Showing a animated software fan as still alive signal
       * in the LM32- console.
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
#ifdef CONFIG_USE_LM32LOG
  lm32Log( LM32_LOG_INFO, text,
                          __reset_count,
                          pCpuMsiBox,
                          pMyMsi,
                          sizeof( SCU_SHARED_DATA_T )
          );
#endif
#endif
   scuLog( LM32_LOG_DEBUG, ESC_DEBUG "Addr nesting count: 0x%p\n" ESC_NORMAL,
           &__atomic_section_nesting_count );

   initializeGlobalPointers();

  /*!
   * Will need by usleep_init()
   */
   timer_init(1);
   usleep_init();

   /*
    * Creating the main task. 
    */
   TASK_CREATE_OR_DIE( taskMain, 1024, 1, NULL );

   scuLog( LM32_LOG_DEBUG, ESC_DEBUG "Starting sheduler...\n" ESC_NORMAL );
   /*
    * Starting the main- and idle- task.
    * If successful, this function will not left.
    */
   vTaskStartScheduler();

   die( "This point shall never be reached!" );
}

/*================================== EOF ====================================*/