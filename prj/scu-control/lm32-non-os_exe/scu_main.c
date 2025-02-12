/*!
 *  @file scu_main.c
 *  @brief Main module of SCU function generators in LM32.
 *
 *  @date 10.07.2019
 *
 *  @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
 *  @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin Stefan Rauch
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
#define _CONFIG_MIL_EV_QUEUE

#ifdef CONFIG_RTOS
 #error ERROR: This application is not for FreeRTOS!
#endif
#include "scu_main.h"
#include <sys_exception.h>
#include "scu_command_handler.h"
#include <scu_fg_list.h>
#include "scu_fg_macros.h"
#ifdef CONFIG_MIL_FG
 #ifdef CONFIG_MIL_IN_TIMER_INTERRUPT
  #include "scu_lm32Timer.h"
 #endif
 #ifdef _CONFIG_MIL_EV_QUEUE
  #include <scu_event.h>
 #endif
 #include "scu_eca_handler.h"
 #include "scu_mil_fg_handler.h"
#endif
#include "scu_fg_handler.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include "daq_main.h"
#endif

#define CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES

#if defined( CONFIG_MIL_IN_TIMER_INTERRUPT ) && defined( CONFIG_MIL_FG ) && defined( CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES )
#warning CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES is defined and aktive for debug purposes only!
#endif

#if !(defined( CONFIG_SCU3 ) != defined( CONFIG_SCU4 ))
  #error CONFIG_SCU3 or CONFIG_SCU4 has to be defined!
#endif

#ifndef USRCPUCLK
   #define CPU_FREQUENCY 125000000
#else
   #define CPU_FREQUENCY (USRCPUCLK * 1000)
#endif

#ifdef CONFIG_DBG_MEASURE_IRQ_TIME
TIME_MEASUREMENT_T g_irqTimeMeasurement = TIME_MEASUREMENT_INITIALIZER;
#endif

//  #define _CONFIG_NO_INTERRUPT

#ifdef CONFIG_MIL_IN_TIMER_INTERRUPT
/*!
 * @brief Module global flag becones "true" when the mil-handling runs
 *        in timer-interrupt context.
 */
bool g_milUseTimerinterrupt = false;
#endif
#ifdef CONFIG_COUNT_MSI_PER_IRQ
extern unsigned int g_msiCnt;
#endif

#ifdef CONFIG_MIL_FG
#ifdef _CONFIG_MIL_EV_QUEUE
EV_CREATE_STATIC( g_ecaEvent, 16 );
#endif
#endif /* CONFIG_MIL_FG */

unsigned char* BASE_SYSCON;

/*===========================================================================*/
/*! ---------------------------------------------------------------------------
 * @brief delay in multiples of one millisecond
 *  uses the system timer
 *  @param ms delay value in milliseconds
 */
OPTIMIZE( "-O2"  )
void msDelayBig( const uint64_t ms )
{
   const uint64_t later = getWrSysTime() + ms * 1000000ULL / 8;
   while( getWrSysTime() < later )
   {
      NOP();
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Handling of all SCU-bus MSI events.
 */
ONE_TIME_CALL void onScuBusEvent( const unsigned int slot )
{
   uint16_t pendingIrqs;

   TRACE_MIL_DRQ( "2\n" );
#ifdef CONFIG_SCUBUS_INT_RESET_AFTER
   #warning SCU-bus IRQ flags becomes not immediately reseted.
   if( (pendingIrqs = scuBusGetInterruptPendingFlags( g_pScub_base, slot )) != 0)
#else
   while( (pendingIrqs = scuBusGetAndResetIterruptPendingFlags( g_pScub_base, slot )) != 0)
#endif
   {
      if( (pendingIrqs & FG1_IRQ) != 0 )
      {
         handleAdacFg( slot, FG1_BASE );
      }

      if( (pendingIrqs & FG2_IRQ) != 0 )
      {
         handleAdacFg( slot, FG2_BASE );
      }
   #ifdef CONFIG_MIL_FG
      if( (pendingIrqs & DREQ ) != 0 )
      { /*
         * MIL-SIO function generator recognized. 
         */
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
         TRACE_MIL_DRQ( "3\n" );
         queuePushWatched( &g_queueMilFg, &milMsg );
      }
   #endif
   #ifdef CONFIG_SCU_DAQ_INTEGRATION
      if( (pendingIrqs & ((1 << DAQ_IRQ_DAQ_FIFO_FULL) | (1 << DAQ_IRQ_HIRES_FINISHED))) != 0 )
      {
         const SCU_BUS_IRQ_QUEUE_T queueScuBusIrq =
         {
            .slot        = slot,
            .pendingIrqs = pendingIrqs
         };
         queuePushWatched( &g_queueAddacDaq, &queueScuBusIrq );
      }
   #endif
   #ifdef CONFIG_SCUBUS_INT_RESET_AFTER
      scuBusResetInterruptPendingFlags( g_pScub_base, slot, pendingIrqs );
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
#ifdef CONFIG_COUNT_MSI_PER_IRQ
   unsigned int msiCnt = 0;
#endif
   MSI_ITEM_T m;

   TRACE_MIL_DRQ( "0\n" );
   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
   #ifdef CONFIG_COUNT_MSI_PER_IRQ
      msiCnt++;
   #endif
      TRACE_MIL_DRQ( "1\n" );
      switch( GET_LOWER_HALF( m.adr )  )
      {
         case ADDR_SCUBUS:
         {
         #if defined( CONFIG_MIL_FG ) && defined( _CONFIG_ECA_BY_MSI )
            STATIC_ASSERT( ADDR_SCUBUS == 0 );
            if( (m.msg & ECA_VALID_ACTION) != 0 )
            { /*
               * ECA event received
               */
               TRACE_MIL_DRQ( "*\n" );
             #ifdef _CONFIG_MIL_EV_QUEUE
               evPushWatched( &g_ecaEvent );
             #else
               ecaHandler();
             #endif
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
            TRACE_MIL_DRQ("4\n");
            queuePushWatched( &g_queueMilFg, &milMsg );
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

#if defined( CONFIG_MIL_IN_TIMER_INTERRUPT ) && defined( CONFIG_MIL_FG )
/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Interrupt callback function for timer-tick it drives the
 *        MIL-FG handler
 */
STATIC void onScuTimerInterrupt( const unsigned int intNum,
                                 const void* pContext UNUSED )
{
   //MSI_ITEM_T m;
   //irqMsiCopyObjectAndRemove( &m, intNum );
   /*
    * CAUTION!
    * On SCU the belonging MSI table entry has to be read in any cases
    * doesn't matter what interrupt it treats.
    */
   irqMsiCleanQueue( intNum );
#ifndef CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES
   milExecuteTasks();
#endif
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
   initCommandHandler();
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   queueReset( &g_queueAddacDaq );
#endif
#ifdef CONFIG_MIL_FG
   queueReset( &g_queueMilFg );
 #ifdef _CONFIG_MIL_EV_QUEUE
   evDelete( &g_ecaEvent );
 #endif
#endif
#ifdef CONFIG_QUEUE_ALARM
   queueReset( &g_queueAlarm );
 #ifdef CONFIG_HANDLE_UNKNOWN_MSI
   queueReset( &g_queueUnknownMsi );
 #endif
#endif
#ifndef _CONFIG_NO_INTERRUPT
   IRQ_ASSERT( irqGetAtomicNestingCount() == 1 );
   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );

 #if defined( CONFIG_MIL_IN_TIMER_INTERRUPT ) && defined( CONFIG_MIL_FG )
 #warning "MIL in interrupt context not fully tested yet!!!"
   STATIC_ASSERT( MAX_LM32_INTERRUPTS == 2 );
   g_milUseTimerinterrupt = false;
   /*
    * Is at least one MIL function generator present?
    */
   #ifndef CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES
   if( milGetNumberOfFg() > 0 )
   {
   #endif
      /*
      * Trying to use the timer interrupt for MIL-handling.
      */
      SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
      if( (unsigned int)pTimer == ERROR_NOT_FOUND )
      {
         scuLog( LM32_LOG_WARNING, ESC_WARNING
                 "WARNING: No LM32-timer-macro for MIL-FGs found,"
                 " polling will used for them!"
                 ESC_NORMAL );
      }
      else
      { /*
         * Frequency of timer-interrupt: CPU_FREQUENCY / f_interrupt
         */
         lm32TimerSetPeriod( pTimer, CPU_FREQUENCY / 10000 );
         lm32TimerEnable( pTimer );
         irqRegisterISR( TIMER_IRQ, NULL, onScuTimerInterrupt );
         g_milUseTimerinterrupt = true;

      }
   #ifndef CONFIG_ENABLE_TIMER_INTERRUPT_IN_ANY_CASES
   }
   #endif
 #else
   STATIC_ASSERT( MAX_LM32_INTERRUPTS == 1 );
 #endif
   scuLog( LM32_LOG_INFO, "IRQ table configured: 0b%b\n", irqGetMaskRegister() );
   irqEnable();
   IRQ_ASSERT( irqGetAtomicNestingCount() == 0 );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Scheduler for all SCU-tasks defined in g_aTasks. \n
 *        Performing of a cooperative multitasking.
 * @see TASK_T
 * @see g_aTasks
 * @see milDeviceHandler
 * @see ecaHandler
 * @see commandHandler
 * @see addacDaqTask
 */
ONE_TIME_CALL void schedule( void )
{
#ifdef _CONFIG_NO_INTERRUPT
   #warning "Testversion with no interrupts!!!"
   onScuMSInterrupt( ECA_INTERRUPT_NUMBER, NULL );
#endif

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   addacDaqTask();
#endif
#ifdef CONFIG_MIL_FG
 #ifdef CONFIG_MIL_IN_TIMER_INTERRUPT
   if( !g_milUseTimerinterrupt )
 #endif
      milExecuteTasks();
 #ifndef _CONFIG_ECA_BY_MSI
   ecaHandler();
 #else
  #ifdef _CONFIG_MIL_EV_QUEUE
   if( evPopSafe( &g_ecaEvent ) )
   {
      ecaHandler();
      //lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "ECA received\n" ESC_NORMAL );
   }
  #endif
 #endif
#endif /* ifdef CONFIG_MIL_FG */
   commandHandler();
}

/*================================ MAIN =====================================*/
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

#ifdef CONFIG_MIL_FG
   milInitTasks();
   dbgPrintMilTaskData();
#endif

   initializeGlobalPointers();
   tellMailboxSlot();

  /*
   * Wait for wr deamon to read sdbfs
   */
   msDelayBig( 1500 );

   BASE_SYSCON = (unsigned char *)find_device_adr( CERN, WR_SYS_CON );
   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      die( "No SYS_CON found!" );
   scuLog( LM32_LOG_INFO, "SYS_CON found on addr: 0x%p\n", BASE_SYSCON );
  /*!
   * Will need by usleep_init()
   */
   timer_init(1);
   usleep_init();

   printCpuId();

   scuLog( LM32_LOG_INFO, "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
   scuLog( LM32_LOG_INFO, "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser );
#ifdef CONFIG_MIL_PIGGY
   scuLog( LM32_LOG_INFO, "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );
#endif
#ifdef CONFIG_MIL_FG
 #ifdef CONFIG_MIL_PIGGY
   scuLog( LM32_LOG_INFO, "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
 #endif
   initEcaQueue();
#endif

   /*
    * Scanning and initializing all FG's and DAQ's
    */
   initAndScan();
   //print_regs();

   mmuAllocateDaqBuffer();

#if defined( CONFIG_MIL_FG )
   scuLog( LM32_LOG_INFO, "Found MIL function generators: %d\n", milGetNumberOfFg() );
#endif
   initInterrupt();

   scuLog( LM32_LOG_INFO, ESC_FG_GREEN ESC_BOLD
           "\n *** Initialization done, going in endless loop... ***\n\n"
           ESC_NORMAL );

   while( true )
   {
      if( _endram != STACK_MAGIC )
         die( "Stack overflow!" );
      schedule();
      queuePollAlarm();
   #ifdef CONFIG_USE_FG_MSI_TIMEOUT
      wdtPoll();
   #endif
   }
}

/*================================== EOF ====================================*/
