/*!
 * @file scu_command_handler.c
 * @brief  Module for receiving of commands from SAFT-LIB
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */
#include <scu_fg_macros.h>
#include <scu_fg_list.h>
#ifdef CONFIG_MIL_FG
  #include <scu_mil_fg_handler.h>
#endif
#include <scu_command_handler.h>
#ifdef CONFIG_RTOS
  #include <FreeRTOS.h>
  #include <task.h>
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #ifdef CONFIG_RTOS
  #include <scu_task_daq.h>
 #else
  #include <daq_main.h>
 #endif
 #include <daq_command_interface_uc.h>
#endif
#if defined( CONFIG_RTOS ) && defined( CONFIG_USE_TEMPERATURE_WATCHER )
 #include <scu_task_temperature.h>
#endif


extern FG_CHANNEL_T   g_aFgChannels[MAX_FG_CHANNELS];
extern void*          g_pScub_base;
#ifdef CONFIG_MIL_FG
extern void*          g_pScu_mil_base;
#endif
#ifdef DEBUG_SAFTLIB
  #warning "DEBUG_SAFTLIB is defined! This could lead to timing problems!"
#endif

#ifdef CONFIG_COUNT_MSI_PER_IRQ
unsigned int g_msiCnt = 0;
#endif

//#define CONFIG_DEBUG_SWI

/*
 * Creating a message queue for by the interrupt received messages from SAFT-LIB
 */
QUEUE_CREATE_STATIC( g_queueSaftCmd, MAX_FG_CHANNELS, SAFT_CMD_T );

#ifdef CONFIG_DEBUG_SWI
#warning Function printSwIrqCode() is activated! In this mode the software will not work!
/*! ---------------------------------------------------------------------------
 * @brief For debug purposes only!
 */
STATIC inline
void printSwIrqCode( const unsigned int code, const unsigned int value )
{
   mprintf( ESC_DEBUG "SW-IRQ: %s\tValue: %2d" ESC_NORMAL "\n",
            fgCommand2String( code ), value );
}
#else
#define printSwIrqCode( code, value )
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Handles so called software interrupts (SWI) coming from SAFTLIB.
 */
ONE_TIME_CALL void saftLibCommandHandler( void )
{
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
   if( !isMilFsmInST_WAIT() )
   { /*
      * Wait a round...
      */
      return;
   }
#endif
#ifdef _CONFIG_MEASURE_COMMAND_HANDLER
   #warning _CONFIG_MEASURE_COMMAND_HANDLER is activated!
   TIME_MEASUREMENT_T tm = TIME_MEASUREMENT_INITIALIZER;
#endif

   SAFT_CMD_T cmd;
  /*
   * Is a message from SATF-LIB for FG present?
   */
   if( !queuePopSave( &g_queueSaftCmd, &cmd ) )
   { /*
      * No, leave this function.
      */
      return;
   }

   /*
    * Signal busy to saftlib.
    */
   g_shared.oSaftLib.oFg.busy = 1;

   const unsigned int code  = GET_UPPER_HALF( cmd );
   const unsigned int value = GET_LOWER_HALF( cmd );

  /*
   * When debug mode active only.
   */
   printSwIrqCode( code, value );
   lm32Log( LM32_LOG_CMD, "MSI command: %s( %u )\n", fgCommand2String( code ), value );

  /*
   * Verifying the command parameter for all commands with a
   * array index as parameter.
   */
   switch( code )
   {
      case FG_OP_RESET_CHANNEL:       FALL_THROUGH
      case FG_OP_ENABLE_CHANNEL:      FALL_THROUGH
      case FG_OP_DISABLE_CHANNEL:     FALL_THROUGH
      case FG_OP_CLEAR_HANDLER_STATE:
      {
         if( value < ARRAY_SIZE( g_aFgChannels ) )
            break;

        /*
         * In the case of a detected parameter error this function
         * becomes terminated.
         */
         lm32Log( LM32_LOG_ERROR, "Value %d out of range!\n", value );

        /*
         * signal done to saftlib
         */
         g_shared.oSaftLib.oFg.busy = 0;
         return;
      }
      default: break;
   }

   /*
    * Executing the SAFT-LIB command if known.
    */
   switch( code )
   {
      case FG_OP_RESET_CHANNEL:
      {
         fgResetAndInit( g_shared.oSaftLib.oFg.aRegs,
                         value,
                         g_shared.oSaftLib.oFg.aMacros,
                         (void*)g_pScub_base
                        #if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
                         ,g_pScu_mil_base
                        #endif
                       );
       #ifdef CONFIG_USE_SENT_COUNTER
         g_aFgChannels[value].param_sent = 0;
       #endif
         break;
      }

      case FG_OP_MIL_GAP_INTERVAL:
      {
      #ifdef _CONFIG_VARIABLE_MIL_GAP_READING
         g_gapReadingTime = value;
      #else
         lm32Log( LM32_LOG_ERROR, "No variable MIL gap reading support!" );
      #endif
         break;
      }

      case FG_OP_ENABLE_CHANNEL:
      { /*
         * Start of a function generator.
         */
        // scuBusEnableMeassageSignaledInterrupts( value ); //duration: 0.03 ms
         fgEnableChannel( value ); //duration: 0.12 ms
         break;
      }

      case FG_OP_DISABLE_CHANNEL:
      { /*
         * Stop of a function generator.
         */
         fgDisableChannel( value );
      #ifdef DEBUG_SAFTLIB
         mprintf( "-%d ", value );
      #endif
         break;
      }

      case FG_OP_RESCAN:
      { /*
         * Rescaning of all function generators.
         */
      #ifdef CONFIG_RTOS
       // vTaskSuspendAll();
         taskDeleteAllRunningFgAndDaq();
      #endif

         scanFgs();

      #ifdef CONFIG_RTOS
       // xTaskResumeAll();
         taskStartAllIfHwPresent();
      #endif
         break;
      }

      case FG_OP_CLEAR_HANDLER_STATE:
      {
       #ifdef CONFIG_MIL_FG
         fgMilClearHandlerState( value );
       #else
         lm32Log( LM32_LOG_ERROR, ESC_ERROR "No MIL support!" ESC_NORMAL );
       #endif
         break;
      }

      case FG_OP_PRINT_HISTORY:
      {
      #ifdef CONFIG_DBG_MEASURE_IRQ_TIME
         mprintf( "\n\nLast IRQ time: ");
         timeMeasurePrintMillisecondsSafe( &g_irqTimeMeasurement );
         mprintf( " ms\n\n" );
      #endif
      #if defined( CONFIG_RTOS ) && defined( CONFIG_USE_TEMPERATURE_WATCHER )
         printTemperatures();
      #else
         lm32Log( LM32_LOG_WARNING, "No history support!" );
      #endif
      #ifdef CONFIG_COUNT_MSI_PER_IRQ
         unsigned int msiCnt;
         ATOMIC_SECTION()
         {
            msiCnt = g_msiCnt;
            g_msiCnt = 0;
         }
         lm32Log( LM32_LOG_INFO, "Maximun MSIs per IRQ: %u", msiCnt );
      #endif
         break;
      }

      default:
      {
         lm32Log( LM32_LOG_ERROR, ESC_ERROR
                  "Error: Unknown MSI-command! op-code: 0x04%X, value: 0x%04X"
                  ESC_NORMAL, code, value );
         break;
      }
   }

   /*
    * signal done to saftlib
    */
   g_shared.oSaftLib.oFg.busy = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Software irq handler
 *
 * dispatch the calls from linux to the helper functions
 * called via scheduler in main loop
 * @see schedule
 */
void commandHandler( void )
{
   saftLibCommandHandler();

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   /*!
    * Executing a possible ADDAC-DAQ command if requested...
    */
   executeIfRequested( &g_scuDaqAdmin );
#endif
}

/*================================== EOF ====================================*/
