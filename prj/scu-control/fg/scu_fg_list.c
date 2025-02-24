/*!
 * @file scu_fg_list.c
 * @brief Module for scanning the SCU for function generators and initializing
 *        the function generator list in the shared memory.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 21.10.2019
 * Renamed from scu_function_generator.c 21.10.2019
 */
#if !defined(__lm32__) && !defined(__DOXYGEN__) && !defined(__DOCFSM__) && !defined(__CPPCHECK__)
  #error This module is for the target LM32 only!
#endif

#include <scu_fg_handler.h>
#include <scu_lm32_common.h>
#include <eb_console_helper.h>
#include <scu_fg_handler.h>
#include <string.h>
#ifdef CONFIG_MIL_FG
 #include <scu_mil.h>
 #include <scu_mil_fg_handler.h>
#endif
#include <sdb_lm32.h>
#include <scu_fg_macros.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #ifdef CONFIG_RTOS
  #include <scu_task_daq.h>
 #else
  #include <daq_main.h>
 #endif
#endif
#include "scu_fg_list.h"

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
void printFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.oSaftLib.oFg.aMacros ); i++ )
   {
      FG_MACRO_T* pFgMacro = &g_shared.oSaftLib.oFg.aMacros[i];

      /*
       * Is the end of list been reached?
       */
      if( pFgMacro->outputBits == 0 )
         break;

      scuLog( LM32_LOG_INFO, ESC_FG_CYAN ESC_BOLD
              "%2u: fg-%u-%d\tver: %u output-bits: %u\n" ESC_NORMAL,
              i+1,
              pFgMacro->socket,
              pFgMacro->device,
              pFgMacro->version,
              pFgMacro->outputBits );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
void print_regs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ); i++ )
   {
      FG_CHANNEL_REG_T* pFgReg = &g_shared.oSaftLib.oFg.aRegs[i];
      mprintf("Registers of channel %d:\n", i );
      mprintf("\twr_ptr:\t%d\n",       pFgReg->wr_ptr );
      mprintf("\trd_ptr:\t%d\n",       pFgReg->rd_ptr );
      mprintf("\tmbx_slot:\t0x%x\n",   pFgReg->mbx_slot );
      mprintf("\tmacro_number:\t%d\n", pFgReg->macro_number );
      mprintf("\tramp_count:\t%d\n",   pFgReg->ramp_count );
      mprintf("\ttag:\t%d\n",          pFgReg->tag );
      mprintf("\tstate:\t%d\n\n",      pFgReg->state );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_list.h
 */
inline void fgListReset( FG_MACRO_T* pFgList )
{
   memset( pFgList, 0, sizeof( FG_MACRO_T ) * MAX_FG_MACROS );
}

/*! ---------------------------------------------------------------------------
 */
STATIC void fgListInitItem( FG_MACRO_T* pMacro,
                         const uint8_t outputBits,
                         const uint8_t version,
                         const uint8_t device, /* mil extension */
                         const uint8_t socket )
{
   pMacro->outputBits  = outputBits;
   pMacro->version     = version;
   pMacro->device      = device;
   pMacro->socket      = socket;
}

/*! ---------------------------------------------------------------------------
 */
int fgListAdd( const uint8_t socked,
               const uint8_t dev,
               const uint16_t cid_sys,
               const uint16_t cid_group,
               const uint8_t fg_ver,
               FG_MACRO_T* pFgList )
{
   int count = 0;

   /*
    * Climbing to the first free list item.
    */
   while( (count < MAX_FG_MACROS) && (pFgList[count].outputBits != 0) )
      count++;

   if( !(cid_sys == SYS_CSCO || cid_sys == SYS_PBRF || cid_sys == SYS_LOEP) )
      return count;

   switch( cid_group )
   {
      case GRP_ADDAC1: FALL_THROUGH
      case GRP_ADDAC2: FALL_THROUGH
      case GRP_DIOB:
      {  /* two FG */
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 16, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 16, fg_ver, 1, socked );
         /* ACU/MFU */
         break;
      }
      case GRP_MFU: /* two FGs */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 20, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 20, fg_ver, 1, socked );
         /* FIB */
         break;
      }
      case GRP_FIB_DDS: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 32, fg_ver, 0, socked );
         /* IFA8 */
         break;
      }
      case GRP_IFA8: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 16, fg_ver, dev, socked );
         break;
      }
   }
   return count; //return number of found fgs
}

/*! ---------------------------------------------------------------------------
 */

#ifndef CONFIG_SCU_DAQ_INTEGRATION
STATIC inline
#endif
void addAddacToFgList( const void* pScuBusBase,
                       const unsigned int slot,
                       FG_MACRO_T* pFGlist )
{
   FG_ASSERT( pFGlist != NULL );
   fgListAdd( slot,
              0,
              SYS_CSCO,
              GRP_ADDAC2,
              getFgFirmwareVersion( pScuBusBase, slot ),
              pFGlist );
}

#ifdef CONFIG_DIOB_WITH_DAQ
/*! ---------------------------------------------------------------------------
 */
void addDiobToFgList( const void* pScuBusBase,
                      const unsigned int slot,
                      FG_MACRO_T* pFGlist )
{
   FG_ASSERT( pFGlist != NULL );
   fgListAdd( slot,
              0,
              SYS_CSCO,
              GRP_DIOB,
              getFgFirmwareVersion( pScuBusBase, slot ),
              pFGlist );
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus for all kinds of function generators.
 */
ONE_TIME_CALL
void scanScuBusFgs( FG_MACRO_T* pFgList )
{
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   resetAllActiveBySaftlib();
   scuDaqInitialize( &g_scuDaqAdmin, pFgList );
#else
   scanScuBusFgsDirect( g_pScub_base, pFgList );
#endif
#if defined( CONFIG_MIL_FG ) || defined( CONFIG_NON_DAQ_FG_SUPPORT )
 #ifdef CONFIG_SCU_DAQ_INTEGRATION
   if( daqBusIsAcuDeviceOnly( &g_scuDaqAdmin.oDaqDevs ) )
   { /*
      * When a ACU device has been recognized, it's not allow to made
      * further scans on the SCU bus!
      */
      return;
   }
 #endif
 #ifdef CONFIG_NON_DAQ_FG_SUPPORT
   scanScuBusFgsWithoutDaq( g_pScub_base, pFgList );
 #endif
 #ifdef CONFIG_MIL_FG
   scanScuBusFgsViaMil( pFgList );
 #endif
#endif
}

void fgListResetAllFoundFg( void )
{
   for( unsigned int channel = 0; channel < ARRAY_SIZE(g_shared.oSaftLib.oFg.aRegs); channel++ )
   {
      fgResetAndInit( channel );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_function_generator.h
 */
void fgListFindAll( FG_MACRO_T* pFgList, uint64_t* pExtId )
{
   fgListReset( pFgList );
   scanScuBusFgs( pFgList );
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
   scanExtMilFgs( pFgList, pExtId );
#endif
  // fgListResetAllFoundFg();
}


/*! ---------------------------------------------------------------------------
 * @see scu_fg_list.h
 */
void fgResetAndInit( const unsigned int channel )
{
   if( channel >= ARRAY_SIZE(g_shared.oSaftLib.oFg.aRegs) )
   {
      // lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "function: %s(), line: %u" ESC_NORMAL, __func__, __LINE__ );
      return;
   }
   g_shared.oSaftLib.oFg.aRegs[channel].wr_ptr = 0;
   g_shared.oSaftLib.oFg.aRegs[channel].rd_ptr = 0;
   g_shared.oSaftLib.oFg.aRegs[channel].state = STATE_STOPPED;
   g_shared.oSaftLib.oFg.aRegs[channel].ramp_count = 0;

   const int32_t macroNumber = g_shared.oSaftLib.oFg.aRegs[channel].macro_number;

   /*
    *  Is a macro assigned to that channel by SAFTLIB?
    *  FunctionGeneratorImpl::acquireChannel
    */
   if( macroNumber < 0 )
   {
     // lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "function: %s(), line: %u" ESC_NORMAL, __func__, __LINE__ );
      return; /* No */
   }
   const unsigned int socket = g_shared.oSaftLib.oFg.aMacros[macroNumber].socket;
   const unsigned int dev    = g_shared.oSaftLib.oFg.aMacros[macroNumber].device;

#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
      getFgRegisterPtr( g_pScub_base, socket, dev )->cntrl_reg.i16 = FG_RESET;
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Reset ADDAC fg-%u-%u" ESC_NORMAL, socket, dev );
#ifdef CONFIG_MIL_FG
      return;
   }

#ifdef CONFIG_MIL_PIGGY
   if( isMilScuBusFg( socket ) )
   {
#endif
      const unsigned int slot = getFgSlotNumber( socket );
      scub_reset_mil( g_pScub_base, slot );
      scub_write_mil( g_pScub_base, slot, FG_RESET, FC_CNTRL_WR | dev );
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Reset MIL-SIO fg-%u-%u" ESC_NORMAL, socket, dev );
      return;
#ifdef CONFIG_MIL_PIGGY
   }

   /*
    * MIL- extension (PIGGY)
    */
   FG_ASSERT( isMilExtentionFg( socket ) );
   reset_mil( g_pScu_mil_base );
   write_mil( g_pScu_mil_base, FG_RESET, FC_CNTRL_WR | dev );
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Reset MIL-PIGGY fg-%u-%u" ESC_NORMAL, socket, dev );
#endif /* ifdef CONFIG_MIL_PIGGY */
#endif /* ifdef CONFIG_MIL_FG */
}

/*================================== EOF ====================================*/

