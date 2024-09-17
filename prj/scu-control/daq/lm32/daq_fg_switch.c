/*!
 *  @file daq_fg_switch.c
 *  @brief Module for switching both DAQ channels for set- and actual values,
 *         belonging to the concerned function generator, on or off.
 *
 *  @date 22.11.2023
 *  @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#include <daq.h>
#include <daq_fg_allocator.h>
#include <daq_main.h>
#include <scu_fg_macros.h>
#include <lm32Interrupts.h>
#include <lm32_syslog.h>
#include <eb_console_helper.h>
#include "daq_fg_switch.h"

//#define CONFIG_NO_DAQ_AUTOSWITCHING

extern DAQ_ADMIN_T g_scuDaqAdmin;

#ifdef CONFIG_NO_DAQ_AUTOSWITCHING
  #warning DAQ-autoswitching will not implemented!
#endif

/*! ---------------------------------------------------------------------------
 * @see daq_main.h
 */
void daqEnableFgFeedback( const unsigned int slot, const unsigned int fgNum,
                          const uint32_t tag )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %d, %d, 0x%04X )\n" ESC_NORMAL, __func__, slot, fgNum, tag );

#ifndef CONFIG_NO_DAQ_AUTOSWITCHING
   DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceBySlotNumber( &g_scuDaqAdmin.oDaqDevs, slot );

 #ifdef CONFIG_NON_DAQ_FG_SUPPORT
   if( (pDaqDevice == NULL) || (pDaqDevice->type == UNKNOWN))
      return;
 #endif
   DAQ_ASSERT( pDaqDevice != NULL );

   const unsigned int setChannelNumber = daqGetSetDaqNumberOfFg( fgNum, pDaqDevice->type );
   const unsigned int actChannelNumber = daqGetActualDaqNumberOfFg( fgNum, pDaqDevice->type );
   DAQ_CANNEL_T* pSetChannel = daqDeviceGetChannelObject( pDaqDevice, setChannelNumber );
   DAQ_CANNEL_T* pActChannel = daqDeviceGetChannelObject( pDaqDevice, actChannelNumber );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Enable DAQ-channels of FG %u: set %u and act %u\n" ESC_NORMAL,
            fgNum, setChannelNumber, actChannelNumber );

   ATOMIC_SECTION()
   {
   #ifdef CONFIG_DAQ_SW_SEQUENCE
      pSetChannel->sequenceContinuous = 0;
      pActChannel->sequenceContinuous = 0;
   #endif
#if 1
      if( daqDeviceGetTimeStampTag( pDaqDevice ) != tag )
      {
         daqDeviceSetTimeStampCounterEcaTag( pDaqDevice, tag );
         daqDevicePresetTimeStampCounter( pDaqDevice, DAQ_DEFAULT_SYNC_TIMEOFFSET );
      }
#endif
      daqChannelSetTriggerCondition( pSetChannel, tag );
      daqChannelSetTriggerCondition( pActChannel, tag );
      daqChannelSetTriggerDelay( pSetChannel, 10000 );
      daqChannelSetTriggerDelay( pActChannel, 10000 );
      daqChannelSample1msOn( pSetChannel );
      daqChannelSample1msOn( pActChannel );
   }
#else
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "CAUTION: DAQ-channels will not enabled!" ESC_NORMAL );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see daq_main.h
 */
void daqDisableFgFeedback( const unsigned int slot, const unsigned int fgNum )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %d, %d )\n" ESC_NORMAL, __func__, slot, fgNum );
#ifndef CONFIG_NO_DAQ_AUTOSWITCHING
   DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceBySlotNumber( &g_scuDaqAdmin.oDaqDevs, slot );

 #ifdef CONFIG_NON_DAQ_FG_SUPPORT
   if( (pDaqDevice == NULL) || (pDaqDevice->type == UNKNOWN))
      return;
 #endif
   DAQ_ASSERT( pDaqDevice != NULL );

   const unsigned int setChannelNumber = daqGetSetDaqNumberOfFg( fgNum, pDaqDevice->type );
   const unsigned int actChannelNumber = daqGetActualDaqNumberOfFg( fgNum, pDaqDevice->type );
   DAQ_CANNEL_T* pSetChannel = daqDeviceGetChannelObject( pDaqDevice, setChannelNumber );
   DAQ_CANNEL_T* pActChannel = daqDeviceGetChannelObject( pDaqDevice, actChannelNumber );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "Disable DAQ-channels of FG %u: set %u and act %u\n" ESC_NORMAL,
            fgNum, setChannelNumber, actChannelNumber );

   ATOMIC_SECTION()
   {
      daqChannelSample1msOff( pSetChannel );
      daqChannelSample1msOff( pActChannel );
   }
#else
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "CAUTION: DAQ-channels will not disabled!" ESC_NORMAL );
#endif
}

/*================================== EOF ====================================*/

