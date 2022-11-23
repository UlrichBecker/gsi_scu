/*!
 * @file scu_task_daq.c
 * @brief FreeRTOS task for ADDAC-DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_daq.h
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

///#define _CONFIG_DAQ_MAIN

#include <FreeRTOS.h>
#include <task.h>
#ifdef _CONFIG_DAQ_MAIN
//#include <daq_main.h>
#endif
#include <daq.h>
#include <scu_lm32_common.h>
#include <daq_command_interface_uc.h>
#include <scu_logutil.h>
#include "scu_task_daq.h"

STATIC TaskHandle_t mg_taskDaqHandle = NULL;

#ifndef _CONFIG_DAQ_MAIN
extern volatile uint16_t* g_pScub_base;
DAQ_ADMIN_T g_scuDaqAdmin;


QUEUE_CREATE_STATIC( g_queueAddacDaq, 2 * MAX_FG_CHANNELS, SCU_BUS_IRQ_QUEUE_T );

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
int daqScanScuBus( DAQ_BUS_T* pDaqDevices, FG_MACRO_T* pFgList )
{
   const int ret = daqBusFindAndInitializeAll( pDaqDevices, (void*)g_pScub_base, pFgList );
   if( ret < 0 )
   {
      scuLog( LM32_LOG_ERROR, ESC_ERROR "Error in finding ADDAC-DAQs!\n" ESC_NORMAL );
      return DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND;
   }
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Switceing the DAQ-channels for set- and actual- values on or off
 *        if DAQ present.
 */
STATIC void daqSwitchFeedback( const unsigned int slot, const unsigned int fgNum,
                               const DAQ_FEEDBACK_ACTION_T what )
{
   criticalSectionEnter();
   DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceBySlotNumber( &g_scuDaqAdmin.oDaqDevs, slot );
   criticalSectionExit();

#ifdef CONFIG_NON_DAQ_FG_SUPPORT
   if( (pDaqDevice == NULL) || (pDaqDevice->type == UNKNOWN))
   {
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "No DAQ devices on slave %u\n" ESC_NORMAL, slot ); 
      return;
   }
#else
   DAQ_ASSERT( pDaqDevice != NULL );
#endif
   daqDevicePutFeedbackSwitchCommand( pDaqDevice, what, fgNum );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void daqEnableFgFeedback( const unsigned int slot, const unsigned int fgNum )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %d, %d )\n" ESC_NORMAL, __func__, slot, fgNum );
   daqSwitchFeedback( slot, fgNum, FB_ON );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void daqDisableFgFeedback( const unsigned int slot, const unsigned int fgNum )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %d, %d )\n" ESC_NORMAL, __func__, slot, fgNum );
   daqSwitchFeedback( slot, fgNum, FB_OFF );
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void handleContinuousMode( DAQ_CANNEL_T* pChannel )
{
   if( !daqChannelTestAndClearDaqIntPending( pChannel ) )
      return;
   if( daqChannelGetDaqFifoWords( pChannel ) == 0 )
   {
      return;
   }

#ifdef CONFIG_DAQ_SW_SEQUENCE
   pChannel->sequenceContinuous++;
#endif
   ramPushDaqDataBlock( &g_scuDaqAdmin.oRam, pChannel, true );
   daqChannelDecrementBlockCounter( pChannel );
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void handleHiresMode( DAQ_CANNEL_T* pChannel )
{
   if( !daqChannelTestAndClearHiResIntPending( pChannel ) )
      return;

   daqChannelDisableHighResolution( pChannel );
#ifdef CONFIG_DAQ_SW_SEQUENCE
   pChannel->sequencePmHires++;
#endif
   ramPushDaqDataBlock( &g_scuDaqAdmin.oRam, pChannel, false );
}

#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline void daqDeviceScanPostMortem( DAQ_DEVICE_T* pDaqDevice )
{
   const unsigned int maxChannels = daqDeviceGetMaxChannels( pDaqDevice );
   for( unsigned int channelNumber = 0; channelNumber < maxChannels; channelNumber++ )
   {
      DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( pDaqDevice, channelNumber );
      if( !pChannel->properties.postMortemEvent )
        continue;
      if( !daqChannelIsPmHiResFiFoFull( pChannel ) )
        continue;

      pChannel->properties.postMortemEvent = false;
      daqChannelDisablePostMortem( pChannel ); //!!
    #ifdef CONFIG_DAQ_SW_SEQUENCE
      pChannel->sequencePmHires++;
    #endif
      ramPushDaqDataBlock( &g_scuDaqAdmin.oRam, pChannel, false );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void daqScanForCommands( void )
{
   const unsigned int maxDevices = daqBusGetFoundDevices( &g_scuDaqAdmin.oDaqDevs );
   for( unsigned int devNr = 0; devNr < maxDevices; devNr++ )
   {
      DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceObject( &g_scuDaqAdmin.oDaqDevs, devNr );
      daqDeviceScanPostMortem( pDaqDevice );
      daqDeviceDoFeedbackSwitchOnOffFSM( pDaqDevice );
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief RTOS- task for ADDAC-DAQs respectively SCU-bus DAQs.
 */
OPTIMIZE( "-O1"  ) //TODO Necessary if LTO activated I don't know why yet!
STATIC void taskDaq( void* pTaskData UNUSED )
{
   taskInfoLog();
   queueResetSave( &g_queueAddacDaq );

   while( true )
   {
#ifdef _CONFIG_DAQ_MAIN
      addacDaqTask();
#else
   #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
      /*
       * Removing old data which has been possibly read and evaluated by the
       * Linux client
       * NOTE: This has to be made in any cases here independently whether one or more
       *       MIL FG are active or not.
       *       Because only in this way it becomes possible to continuing the
       *       handshake transfer at reading the possible remaining data from
       *       the DDR3 memory by the Linux client.
       * See daq_base_interface.cpp  function: DaqBaseInterface::getNumberOfNewData
       * See daq_base_interface.cpp  function: DaqBaseInterface::sendWasRead
       * See daq_administration.cpp  function: DaqAdministration::distributeData
       */
      ramRingSharedSynchonizeReadIndex( &GET_SHARED().ringAdmin );
   #endif
      daqScanForCommands();

      SCU_BUS_IRQ_QUEUE_T queueScuBusIrq;
      if( !queuePopSave( &g_queueAddacDaq, &queueScuBusIrq ) )
         continue;

      DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceBySlotNumber( &g_scuDaqAdmin.oDaqDevs, queueScuBusIrq.slot );
      const unsigned int maxChannels = daqDeviceGetMaxChannels( pDaqDevice );
      for( unsigned int channelNumber = 0; channelNumber < maxChannels; channelNumber++ )
      {
         DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( pDaqDevice, channelNumber );

         if(( queueScuBusIrq.pendingIrqs & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0 )
         {
            handleContinuousMode( pChannel );
         }

         if(( queueScuBusIrq.pendingIrqs & (1 << DAQ_IRQ_HIRES_FINISHED)) != 0 )
         {
            handleHiresMode( pChannel );
         }
      }
#endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStartDaqIfAnyPresent( void )
{
   if( (mg_taskDaqHandle == NULL) && (daqBusGetFoundDevices( &g_scuDaqAdmin.oDaqDevs ) > 0) )
   {
      TASK_CREATE_OR_DIE( taskDaq, 512, TASK_PRIO_ADDAC_DAQ, &mg_taskDaqHandle );
      //vTaskDelay( pdMS_TO_TICKS( 1 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStopDaqIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskDaqHandle );
}


/*================================== EOF ====================================*/
