/*!
 * @file scu_fg_macros.c
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#include "scu_fg_macros.h"
#include "scu_fg_handler.h"
#include "scu_fg_list.h"
#ifdef CONFIG_MIL_FG
   #include "scu_mil_fg_handler.h"
#endif

#define CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ

extern void*               g_pScub_base;
extern void*               g_pScub_irq_base;

#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
extern void*               g_pScu_mil_base;
extern void*               g_pMil_irq_base;
#endif
#ifdef CONFIG_MIL_FG
typedef enum
{
   MIL_INL = 0x00,
   MIL_DRY = 0x01,
   MIL_DRQ = 0x02
} MIL_T;
#endif /* ifdef CONFIG_MIL_FG */

/*!
 * @brief Memory space of sent function generator data.
 *        Non shared memory part for each function generator channel.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] =
{
   {
    #ifdef CONFIG_USE_FG_MSI_TIMEOUT
      .timeout      = 0L,
    #endif
    #ifdef CONFIG_USE_SENT_COUNTER
      .param_sent   = 0,
    #endif
      .last_c_coeff = 0
   }
};

STATIC_ASSERT( ARRAY_SIZE(g_aFgChannels) == MAX_FG_CHANNELS );

#ifdef CONFIG_USE_FG_MSI_TIMEOUT

#ifndef MSI_TIMEOUT
   #define MSI_TIMEOUT 3
#endif

#define MSI_TIMEOUT_OFFSET (MSI_TIMEOUT * 1000000000ULL)

/*! ---------------------------------------------------------------------------
 */
void wdtReset( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );
   if( fgIsStarted( channel ) )
      ATOMIC_SECTION() g_aFgChannels[channel].timeout = getWrSysTime() + MSI_TIMEOUT_OFFSET;
}

/*! ---------------------------------------------------------------------------
 */
void wdtDisable( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );

   ATOMIC_SECTION() g_aFgChannels[channel].timeout = 0LL;
}

/*! ---------------------------------------------------------------------------
 */
void wdtPoll( void )
{
   const uint64_t currentTime = getWrSysTimeSafe();

   for( unsigned int channel = 0; channel < ARRAY_SIZE( g_aFgChannels ); channel++ )
   {
      criticalSectionEnter();
      volatile const uint64_t timeout = g_aFgChannels[channel].timeout;
      criticalSectionExit();

      if( timeout == 0LL )
      { /*
         * Watchdog for this FG- channel is not active.
         * Go to the next channel.
         */
         continue;
      }

      if( currentTime <= timeout )
      { /*
         * Watchdog is active, but there is no timeout yet.
         * Go to the next channel.
         */
         continue;
      }

      /*
       * A timeout happened!
       * The watchdog of this channel becomes disabled to prevent
       * multiple timeout messages.
       */
      wdtDisable( channel );

      if( !fgIsStarted( channel ) )
      { /*
         * FG- channel is not enabled, so the timeout will ignored.
         * Go to the next channel.
         */
         continue;
      }

      /*
       * The timeout becomes posted only if the
       * concerned function-generator is active.
       */
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
               "ERROR: MSI-timeout on fg-%u-%u channel: %u !\n" ESC_NORMAL,
               getSocket(channel), getDevice(channel), channel );

      /*!
       * @todo Is the channel disabling by MSI- timeout really meaningful?
       *       Because in this case the concerned DAQ- channels becomes
       *       disabled as well.
       */
      makeStop( channel );
      fgDisableChannel( channel );
   }
}
#endif /* ifdef CONFIG_USE_FG_MSI_TIMEOUT */

/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Send a signal back to the Linux-host (SAFTLIB)
 * @param sig Signal
 * @param channel Concerning channel number.
 */
inline void sendSignal( const SIGNAL_T sig, const unsigned int channel )
{
   STATIC_ASSERT( sizeof( g_pCpuMsiBox[0] ) == sizeof( uint32_t ) );
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) );

   ATOMIC_SECTION()
      MSI_BOX_SLOT_ACCESS( g_shared.oSaftLib.oFg.aRegs[channel].mbx_slot, signal ) = sig;

#ifdef CONFIG_DEBUG_FG_SIGNAL
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG
                            "Signal: %s, channel: %d sent\n" ESC_NORMAL,
            signal2String( sig ), channel );
#endif
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void sendSignalArmed( const unsigned int channel )
{
   g_shared.oSaftLib.oFg.aRegs[channel].state = STATE_ARMED;
   sendSignal( IRQ_DAT_ARMED, channel );
}

/*! ---------------------------------------------------------------------------
 * @brief enables MSI generation for the specified channel.
 *
 * Messages from the SCU bus are send to the MSI queue of this CPU with the
 * offset 0x0. \n
 * Messages from the MIL extension are send to the MSI queue of this CPU with
 * the offset 0x20. \n
 * A hardware macro is used, which generates MSIs from legacy interrupts.
 *
 * @todo Replace this awful naked index-numbers by well documented
 *       and meaningful constants!
 *
 * @param socket number of the socket between 0 and MAX_FG_CHANNELS-1
 * @see fgDisableInterrupt
 */
STATIC inline
void enableMeassageSignaledInterrupts( const unsigned int socket )
{
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) || isMilScuBusFg( socket ) )
   {
#endif
      FG_ASSERT( getFgSlotNumber( socket ) >= SCUBUS_START_SLOT );
      const uint16_t slot = getFgSlotNumber( socket ) - SCUBUS_START_SLOT;
      ATOMIC_SECTION()
      {
         scuBusSetSlaveValue16( scuBusGetSysAddr( g_pScub_base ), GLOBAL_IRQ_ENA, 0x20 );
         SET_REG32( g_pScub_irq_base, MSI_CHANNEL_SELECT, slot );
         SET_REG32( g_pScub_irq_base, MSI_SOCKET_NUMBER, slot );
         SET_REG32( g_pScub_irq_base, MSI_DEST_ADDR, (uint32_t)&((MSI_LIST_T*)g_pMyMsi)[0] );
         SET_REG32( g_pScub_irq_base, MSI_ENABLE, 1 << slot );
      }
#ifdef CONFIG_MIL_FG
      return;
   }
#ifdef CONFIG_MIL_PIGGY
   if( !isMilExtentionFg( socket ) )
      return;

   ATOMIC_SECTION()
   {
      SET_REG32( g_pMil_irq_base, MSI_CHANNEL_SELECT, MIL_DRQ );
      SET_REG32( g_pMil_irq_base, MSI_SOCKET_NUMBER,  MIL_DRQ );
      SET_REG32( g_pMil_irq_base, MSI_DEST_ADDR, (uint32_t)&((MSI_LIST_T*)g_pMyMsi)[2] );
      SET_REG32( g_pMil_irq_base, MSI_ENABLE, 1 << MIL_DRQ );
   }
#endif /* ifdef CONFIG_MIL_PIGGY */
#endif /* ifdef CONFIG_MIL_FG */
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgEnableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %u ): fg-%u-%u\n" ESC_NORMAL,
            __func__, channel, socket, dev );

   enableMeassageSignaledInterrupts( socket );

#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtReset( channel );
#endif

   FG_REGISTER_T* pAddagFgRegs = NULL;

#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
      STATIC_ASSERT( sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) == sizeof( uint32_t ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
      /*
       * Note: In the case of ADDAC/ACU-FGs the socket-number is equal
       *       to the slot number.
       */
      pAddagFgRegs = addacFgPrepare( g_pScub_base,
                                     socket, dev,
                                     g_shared.oSaftLib.oFg.aRegs[channel].tag );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      if( milHandleClearHandlerState( g_pScub_base, g_pScu_mil_base, socket ) )
         return;

      milFgPrepare( g_pScub_base, g_pScu_mil_base, socket, dev );
   }
#endif

   FG_PARAM_SET_T pset;
  /*
   * Fetch first parameter set from buffer
   */
   if( cbReadSave( &g_shared.oSaftLib.oFg.aChannelBuffers[0], &g_shared.oSaftLib.oFg.aRegs[0], channel, &pset ) != 0 )
   {
   #ifdef CONFIG_MIL_FG
      if( pAddagFgRegs != NULL )
      {
   #endif
         addacFgStart( pAddagFgRegs, &pset, channel );
   #ifdef CONFIG_MIL_FG
      }
      else
      {
         milFgStart( g_pScub_base,
                     g_pScu_mil_base,
                     &pset,
                     socket, dev, channel );
      }
   #endif /* CONFIG_MIL_FG */
   #ifdef CONFIG_USE_SENT_COUNTER
      g_aFgChannels[channel].param_sent++;
   #endif
   } /* if( cbRead( ... ) != 0 ) */

   sendSignalArmed( channel );
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgDisableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) );


   FG_CHANNEL_REG_T* pFgRegs = &g_shared.oSaftLib.oFg.aRegs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %u ): fg-%u-%u\n" ESC_NORMAL,
            __func__, channel, socket, dev );

#ifdef CONFIG_MIL_FG
   int status;
   if( isAddacFg( socket ) )
   {
#endif
      /*
       * Note: In the case if ADDAC/ACU- function generator the slot number
       *       is equal to the socket number.
       */
      addacFgDisable( g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      status = milFgDisable( g_pScub_base,
                             g_pScu_mil_base,
                             socket, dev );
      if( status != OKAY )
         return;

   }
#endif /* CONFIG_MIL_FG */

   if( pFgRegs->state == STATE_ACTIVE )
   { /*
      *  hw is running
      */
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG
               "Flush circular buffer of fg-%u-%u channel: %u\n" ESC_NORMAL,
               socket, dev, channel );
      flushCircularBuffer( pFgRegs );
   }
 //  else
 //  {
      pFgRegs->state = STATE_STOPPED;
      sendSignal( IRQ_DAT_DISARMED, channel );
 //  }
#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtDisable( channel );
#endif

}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see enableMeassageSignaledInterrupts
 */
STATIC inline void fgDisableInterrupt( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   //mprintf("IRQs for slave %d disabled.\n", socket);
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
     /*
      * In the case of ADDAC/ACU-FGs the socket-number is equal to the
      * slot number, so it's not necessary to extract the slot number here.
      */
      addacFgDisableIrq( g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
      return;
   }

   milFgDisableIrq( g_pScub_base, g_pScu_mil_base, socket, dev );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
void makeStop( const unsigned int channel )
{

   fgDisableInterrupt( channel );
   if( cbisEmpty( &g_shared.oSaftLib.oFg.aRegs[0], channel ) )
   {
      sendSignal( IRQ_DAT_STOP_EMPTY,  channel );
   }
   else
   {
      sendSignal( IRQ_DAT_STOP_NOT_EMPTY,  channel );
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
               "ERROR: fg-%u-%u, channel: %u has stopped!" ESC_NORMAL,
               getSocket( channel ),
               getDevice( channel ),
               channel
             );
   }
   g_shared.oSaftLib.oFg.aRegs[channel].state = STATE_STOPPED;

#if 0
#ifndef CONFIG_LOG_ALL_SIGNALS
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG
            "Signal: %s, fg-%u-%u, channel: %u\n" ESC_NORMAL,
            signal2String( signal ),
            getSocket( channel ),
            getDevice( channel ),
            channel );
#endif
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCountSave( &g_shared.oSaftLib.oFg.aRegs[0], channel ) == FG_REFILL_THRESHOLD )
   {
     // mprintf( "*" ); //!!
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

/*================================== EOF ====================================*/
