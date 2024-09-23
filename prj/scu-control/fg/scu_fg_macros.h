/*!
 * @file scu_fg_macros.h
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_FG_MACROS_H
#define _SCU_FG_MACROS_H

#include <scu_lm32_macros.h>
#include <scu_bus.h>
#include <scu_lm32_common.h>
#include <scu_mailbox.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * @brief Control register of function generator.
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @see FG_MASK_T
 */
typedef struct HW_IMAGE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__) || defined(CONFIG_IS_IN_GITHUB_ACTION)
   /*!
    * @brief  Add frequency select: (wo) bit [15:13]
    */
   unsigned int frequency_select: 3;

   /*!
    * @brief step value M (wo) bit [12:10]
    */
   unsigned int step:             3;

   /*!
    * @brief virtual function generator number (wr) bit [9:4]
    */
   unsigned int number:           6;

   /*!
    * @brief Data request bit [3]
    * @note Only in
    * @see FG_DREQ
    * @see https://github.com/GSI-CS-CO/bel_projects/blob/proposed_master/modules/function_generators/fg_quad/wb_fg_quad.vhd
    */
   const bool dataRequest:    1;

   /*!
    * @brief Indicator if function generator running. (ro) bit [2]
    * @see FG_RUNNING
    */
   const bool isRunning:      1;

   /*!
    * @brief Enable function generator (rw) bit [1]
    * @see FG_ENABLED
    */
   bool enable:               1;

   /*!
    * @brief Reset, 1 -> active (rw) bit [0]
    * @see FG_RESET
    */
   bool reset:                1;
#else
   #error Big endian is requested for this bit- field structure!
#endif
} FG_CTRL_RG_T_BV;

STATIC_ASSERT( sizeof(FG_CTRL_RG_T_BV) == sizeof(uint16_t) );
STATIC_ASSERT( (int)true == 1 );

/*!
 * @brief Access wrapper avoiding suspicious cast operations.
 */
typedef union HW_IMAGE
{
   /*!
    * @brief Segmented access by bit vector.
    */
   FG_CTRL_RG_T_BV bv;

   /*!
    * @brief Total access by integer.
    */
   uint16_t        i16;
} FG_CTRL_RG_T;

STATIC_ASSERT( sizeof(FG_CTRL_RG_T) == sizeof(uint16_t));

/*! ---------------------------------------------------------------------------
 * @brief Sets the "active by SAFTLIB- flag" of the concerned FG- channel.
 * @see resetActiveBySaftLib
 * @see fgIsActiveBySaftLib
 */
void setActiveBySaftLib( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Resets the "active by SAFTLIB- flag" of the concerned FG- channel.
 * @see setActiveBySaftLib
 * @see fgIsActiveBySaftLib
 */
void resetActiveBySaftLib( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Resets all "active by SAFTLIB- flags"
 * @see setActiveBySaftLib
 */
void resetAllActiveBySaftlib( void );

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" when the concerning functiongenerator was switched
 *        on by SAFT-LIB ane returns "false" once it was switched off by
 *        SAFT-LIB.
 * @see setActiveBySaftLib
 * @see resetActiveBySaftLib
 */
bool fgIsActiveBySaftLib( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" when log-messages for enabeling function-generators
 *        are requested.
 * @see setFgLoggingEnnable
 */
bool isFgEnableLoggingActive( void );

/*! ---------------------------------------------------------------------------
 * @brief Enables or disables the log messages for function-generator enabling.
 * @see isFgEnableLoggingActive
 * @param enable If true then the log-messages are enabled.
 */
void setFgLoggingEnnable( bool enable );

/*! ---------------------------------------------------------------------------
 * @brief Returns the control register format for step, frequency select
 *        and channel number.
 * @param pPset Pointer to the polynomial data set.
 * @param channel Channel number of the concerned function generator.
 * @return Value for the function generators control register.
 */
STATIC inline
uint16_t getFgControlRegValue( const FG_PARAM_SET_T* pPset,
                                             const unsigned int channel )
{
   return ((pPset->control.i32 & (PSET_STEP | PSET_FREQU)) << 10) |
          (channel << 4);
}


/*! ---------------------------------------------------------------------------
 * @brief Returns the the or-link of the shift-a and shift-b register value.
 */
STATIC inline
uint16_t getFgShiftRegValue( const FG_PARAM_SET_T* pPset )
{
   return (pPset->control.i32 & (PSET_SHIFT_A | PSET_SHIFT_B)) >> 6;
}

/*! ---------------------------------------------------------------------------
 * @brief Data type for remembering the last data sent to a function generator.
 */
typedef struct
{
#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   uint64_t timeout;      /*!<@brief MSI timeout value. */
#endif
#ifdef CONFIG_USE_SENT_COUNTER
   uint32_t param_sent;   /*!<@brief Sent counter */
#endif
   int32_t  last_c_coeff; /*!<@brief Value of last C-coefficient of polynomial */
} FG_CHANNEL_T;

extern FG_CHANNEL_T g_aFgChannels[];

#ifdef CONFIG_USE_FG_MSI_TIMEOUT
/*! ---------------------------------------------------------------------------
 * @brief Restarts respectively resets the watchdog timer of a given channel.
 */
void wdtReset( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Disables the watchdog timer of the given channel.
 */
void wdtDisable( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Polls all activated watchdog timers and gives a error-message
 *        on LM32 syslog if en timeout was happened.
 */
void wdtPoll( void );
#endif /* ifdef CONFIG_USE_FG_MSI_TIMEOUT */

/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Send a signal back to the Linux-host (SAFTLIB)
 * @param sig Signal
 * @param channel Concerning channel number.
 */
void sendSignal( const SIGNAL_T sig, const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 */
STATIC inline bool fgIsArmed( const unsigned int channel )
{
   return g_shared.oSaftLib.oFg.aRegs[channel].state == STATE_ARMED;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
void makeStop( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 */
STATIC inline bool fgIsStopped( const unsigned int channel )
{
   return g_shared.oSaftLib.oFg.aRegs[channel].state == STATE_STOPPED;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 *
 * @note Function generator has received the tag or brc message.
 *
 * @see handleMacros
 */
STATIC inline void makeStart( const unsigned int channel )
{
   g_shared.oSaftLib.oFg.aRegs[channel].state = STATE_ACTIVE;
   sendSignal( IRQ_DAT_START, channel );
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" if the function generator of this channel is active.
 */
STATIC inline bool fgIsStarted( const unsigned int channel )
{
   return g_shared.oSaftLib.oFg.aRegs[channel].state == STATE_ACTIVE;
}

/*! ---------------------------------------------------------------------------
 * @brief configures each function generator channel.
 *
 *  checks first, if the drq line is inactive, if not the line is cleared
 *  then activate irqs and send the first tuple of data to the function generator
 *  @param channel number of the specified function generator channel from
 *         0 to MAX_FG_CHANNELS-1
 */
void fgEnableChannel( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief disable function generator channel
 * @param channel number of the function generator channel from 0 to MAX_FG_CHANNELS-1
 */
void fgDisableChannel( const unsigned int channel );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_FG_MACROS_H */
/*================================== EOF ====================================*/
