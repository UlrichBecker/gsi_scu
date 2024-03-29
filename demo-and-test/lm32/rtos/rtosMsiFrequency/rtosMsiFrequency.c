/*!
 * @file rtosMsiFrequency.c
 * @brief Test program for measuring MSI-frequency with FreeRTOS
 *
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 * @copyright 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 03.11.2022
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * This example demonstrates handling of message-signaled interrupts (MSI)
 * caused by ECA channel.\n
 * ECA is capable to send MSIs on certain conditions such as producing actions
 * on reception of timing messages.\n
 *
 * Chose SCU: export SCU_URL=scuxl4711
 * build:     make \n
 * load:      make load \n
 * debug:     eb-console tcp/$SCU_URL
 *
 *  To run example:\n
 *  - set ECA rules for eCPU action channel
 * @code
 *  saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 * @endcode
 *  - debug firmware output
 * @code
 *  eb-console dev/wbm0
 * @endcode
 *  - inject timing message (invoke on the second terminal)
 * @code
 *  saft-ctl tr0 inject 0x1122334455667788 0x8877887766556642 0
 * @endcode
 * - or in a loop:
 * @code
 * while [ true ]; do  saft-ctl tr0 inject 0x1122334455667788 0x8877887766556642 0; done
 * @endcode
 */
#include <eb_console_helper.h>
#include <lm32signal.h>
#include <scu_msi.h>
#include <eca_queue_type.h>
#include <scu_lm32Timer.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ros_timeout.h>
#include <scu_assert.h>

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

/*!
 * @brief ECA actions tagged for this LM32 CPU
 *
 * @code
 * saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 *                                                                 ====
 * @endcode
 */
#define MY_ACT_TAG 0x42

/*!
 * @brief WB address of ECA control register set.
 */
ECA_CONTROL_T* g_pEcaCtl;

/*!
 * @brief WB address of ECA queue
 */
ECA_QUEUE_ITEM_T* g_pEcaQueue;

typedef struct
{
   unsigned int irq;
   unsigned int msi;
} COUNTERS_T;

volatile COUNTERS_T g_counters = {0,0};

#ifdef CONFIG_DEBUG_BY_LOGIK_ANALYSATOR
volatile uint16_t* g_pScuBusBase = NULL;
#endif

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception appeared.
 */
void _onException( const uint32_t sig )
{
   ATOMIC_SECTION()
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
      mprintf( ESC_ERROR "%s( %d ): %s\n" ESC_NORMAL, __func__, sig, str );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Clear pending valid actions for LM32
 */
STATIC inline void clearActions( void )
{
   uint32_t valCnt = ecaControlGetAndResetLM32ValidCount( g_pEcaCtl );
   if( valCnt != 0 )
   {
      mprintf( "pending actions: %d\n", valCnt );
      valCnt = ecaClearQueue( g_pEcaQueue, valCnt ); // pop pending actions
      mprintf( "cleared actions: %d\n", valCnt );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Configure ECA to send MSI to embedded soft-core LM32.
 *
 * - ECA action channel for LM32 is selected and MSI target address of LM32 is
 *   set in the ECA MSI target register
 *
 */
STATIC inline void configureEcaMsiForLM32( void )
{
   clearActions();   // clean ECA queue and channel from pending actions
   ecaControlSetMsiLM32TargetAddress( g_pEcaCtl, (void*)g_pMyMsi, true );
   mprintf( "MSI path (ECA -> LM32)           : enabled\n"
           "\tECA channel = %d\n\tdestination = 0x%08X\n",
            ECA_SELECT_LM32_CHANNEL, (uint32_t)g_pMyMsi);
}

/*! ---------------------------------------------------------------------------
 * @brief Interrupt function: Handle MSIs sent by ECA
 *
 * If interrupt was caused by a valid action, then MSI has value of (4<<16|num).\n
 * Both ECA action channel and ECA queue connected to that channel must be handled:\n
 * - read and clear the valid counter value of ECA action channel for LM32 and,\n
 * - pop pending actions from ECA queue connected to this action channel
 */
STATIC void onIrqEcaEvent( const unsigned int intNum,
                           const void* pContext )
{
   MSI_ITEM_T m;

   g_counters.irq++;

   //irqMsiCopyObjectAndRemove( &m, intNum );
   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
      if( (m.msg & ECA_VALID_ACTION) == 0 )
         continue;
      const unsigned int valCnt = ecaControlGetAndResetLM32ValidCount( g_pEcaCtl );
      if( valCnt == 0 )
         continue;
      for( unsigned int i = 0; i < valCnt; i++ )
      {
         if( !ecaIsValid( g_pEcaQueue ) )
            continue;
         ecaPop( g_pEcaQueue );
         g_counters.msi++;
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @brief The task main function!
 */
STATIC void vTaskEcaMain( void* pvParameters UNUSED )
{
   mprintf( ESC_BOLD ESC_FG_CYAN "Function \"%s()\" of task \"%s\" started\n"
            ESC_NORMAL,
            __func__, pcTaskGetName( NULL ) );

#ifdef CONFIG_DEBUG_BY_LOGIK_ANALYSATOR
   g_pScuBusBase = (uint16_t*)find_device_adr(GSI, SCU_BUS_MASTER);
   SCU_ASSERT( g_pScuBusBase != (uint16_t*)ERROR_NOT_FOUND );
   mprintf( "SCU base address: 0x%p\n", g_pScuBusBase );
   *g_pScuBusBase = 0;
#endif

   mprintf("MSI destination addr for LM32    : 0x%p\n", g_pMyMsi );

   g_pEcaCtl = ecaControlGetRegisters();
   if( g_pEcaCtl == NULL )
   {
      mprintf( ESC_ERROR "Could not find the ECA channel control. Exit!\n"
               ESC_NORMAL );
      vTaskEndScheduler();
   }
   mprintf( "ECA channel control              @ 0x%p\n", g_pEcaCtl );

   g_pEcaQueue = ecaGetLM32Queue();
   if( g_pEcaQueue == NULL )
   {
      mprintf( ESC_ERROR "Could not find the ECA queue connected"
                         " to eCPU action channel. Exit!\n" ESC_NORMAL );
      vTaskEndScheduler();
   }
   mprintf( "ECA queue to LM32 action channel @ 0x%08p\n", g_pEcaQueue );

 
   configureEcaMsiForLM32();

   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onIrqEcaEvent );
   mprintf( "Installing of ECA interrupt is done.\n" );

   unsigned int i = 0;
   const char fan[] = { '|', '/', '-', '\\' };
   TIMEOUT_T fanInterval;
   toStart( &fanInterval, pdMS_TO_TICKS( 250 ) );

   mprintf( ESC_BOLD "Entering task main loop and waiting for MSI ...\n" ESC_NORMAL );
   while( true )
   {
      if( !toInterval( &fanInterval ) )
         continue;

      mprintf( ESC_BOLD ESC_XY( "1", "16" ) "%c" ESC_NORMAL, fan[i++] );
      i %= ARRAY_SIZE( fan );
      if( i != 1 )
         continue;

      COUNTERS_T localCounter;

      ATOMIC_SECTION()
      {
         localCounter = g_counters;
         g_counters.irq = 0;
         g_counters.msi = 0;
      }

      mprintf( ESC_CURSOR_OFF
               ESC_XY( "3", "16" ) "IRQ: %4u Hz" 
               ESC_XY( "3", "17" ) "MSI: %4u Hz",
               localCounter.irq, localCounter.msi );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   static const char* taskName = "ECA-Frequ";
   BaseType_t status;
   mprintf( "Creating task \"%s\"\n", taskName );
   status = xTaskCreate( vTaskEcaMain,
                         taskName,
                         configMINIMAL_STACK_SIZE + 256,
                         NULL,
                         tskIDLE_PRIORITY + 1,
                         NULL
                       );
   if( status != pdPASS )
      return status;


   vTaskStartScheduler();

   return pdPASS;
}

/*! ---------------------------------------------------------------------------
 */
void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS MSI-Frequency test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n" );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}
/*================================== EOF ====================================*/
