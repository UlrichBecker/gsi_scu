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

#include <scu_command_handler.h>

#include <scu_task_fg.h>
#ifdef CONFIG_MIL_FG
 #include <scu_eca_handler.h>
 #include <scu_task_mil.h>
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <scu_task_daq.h>
#endif

#define MAIN_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )

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
void taskDelete( void* pTaskHandle )
{
   if( *(TaskHandle_t*)pTaskHandle  != NULL )
   {
      lm32Log( LM32_LOG_INFO, ESC_BOLD "Deleting task: \"%s\".\n" ESC_NORMAL,
               pcTaskGetName( *(TaskHandle_t*)pTaskHandle ) );
      vTaskDelete( *(TaskHandle_t*)pTaskHandle  );
      *(TaskHandle_t*)pTaskHandle = NULL;
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Handling of all SCU-bus MSI events.
 */
ONE_TIME_CALL void onScuBusEvent( const unsigned int slot )
{
   uint16_t pendingIrqs;

   while( (pendingIrqs = scuBusGetAndResetIterruptPendingFlags((void*)g_pScub_base, slot )) != 0)
   {
      if( (pendingIrqs & (FG1_IRQ | FG2_IRQ)) != 0 )
      {
         const FG_QUEUE_T queueFgItem =
         {
            .slot     = slot,
            .msiFlags = pendingIrqs
         };
         queuePushWatched( &g_queueFg, &queueFgItem );
      }
      //TODO
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
         //TODO
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
   //TODO

   initCommandHandler();
   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );
   scuLog( LM32_LOG_INFO, "IRQ table configured: 0b%b\n", irqGetMaskRegister() );
}

/*! ---------------------------------------------------------------------------
 * @brief Main task
 */
STATIC void taskMain( void* pTaskData UNUSED )
{
   taskInfoLog();

   tellMailboxSlot();
   printCpuId();

   mmuAllocateDaqBuffer();

 //  scuLog( LM32_LOG_INFO, "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
 //  scuLog( LM32_LOG_INFO, "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser );
   scuLog( LM32_LOG_INFO, "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );

#ifdef CONFIG_MIL_FG
   scuLog( LM32_LOG_INFO, "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
   initEcaQueue();
#endif


   initInterrupt();

   //taskStartMil(); //!!

   scuLog( LM32_LOG_INFO, ESC_FG_GREEN ESC_BOLD
           "\n *** Initialization done, %u tasks running, going in main loop of task \"%s\" ***\n\n"
           ESC_NORMAL, uxTaskGetNumberOfTasks(), pcTaskGetName( NULL ) );

#ifdef CONFIG_STILL_ALIVE_SIGNAL
   unsigned int i = 0;
   const char fan[] = { '|', '/', '-', '\\' };
   TickType_t fanTime = 0;
#endif

   while( true )
   {
   #ifdef CONFIG_STILL_ALIVE_SIGNAL
      /*
       * Showing a animated software fan as still alive signal
       * in the LM32- console.
       */
      const TickType_t tick = xTaskGetTickCount();
      if( fanTime <= tick )
      {
         fanTime = tick + pdMS_TO_TICKS( 250 );
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
#ifdef CONFIG_USE_MMU
  lm32Log( LM32_LOG_INFO, text,
                          __reset_count,
                          pCpuMsiBox,
                          pMyMsi,
                          sizeof( SCU_SHARED_DATA_T )
          );
#endif
#endif
   initializeGlobalPointers();

   TASK_CREATE_OR_DIE( taskMain, 1024, 1, NULL );

   vTaskStartScheduler();
}

/*================================== EOF ====================================*/
