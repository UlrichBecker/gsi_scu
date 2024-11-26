/*!
 * @file scu_task_daq.c
 * @brief FreeRTOS task for ADDAC-DAQs.
 * @see https://www-acc.gsi.de/wiki/Frontend/Firmware_SCU_Control
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
extern void* g_pScub_base;
DAQ_ADMIN_T g_scuDaqAdmin;

QUEUE_CREATE_STATIC( g_queueAddacDaq, 2 * MAX_FG_CHANNELS, SCU_BUS_IRQ_QUEUE_T );

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
int daqScanScuBus( DAQ_BUS_T* pDaqDevices, FG_MACRO_T* pFgList )
{
   const int ret = daqBusFindAndInitializeAll( pDaqDevices, g_pScub_base, pFgList );
   if( ret < 0 )
   {
      scuLog( LM32_LOG_ERROR, ESC_ERROR "Error in finding ADDAC-DAQs!\n" ESC_NORMAL );
      return DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND;
   }
   return DAQ_RET_OK;
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
STATIC inline void daqHandlePostMortem( void )
{
   const unsigned int maxDevices = daqBusGetFoundDevices( &g_scuDaqAdmin.oDaqDevs );
   for( unsigned int devNr = 0; devNr < maxDevices; devNr++ )
   {
      DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceObject( &g_scuDaqAdmin.oDaqDevs, devNr );
      daqDeviceScanPostMortem( pDaqDevice );
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup RTOS_TASK
 * @brief RTOS- task for ADDAC-DAQs respectively SCU-bus DAQs.
 */
//OPTIMIZE( "-O1"  ) //!@todo Necessary if LTO activated I don't know why yet!
STATIC void taskDaq( void* pTaskData UNUSED )
{
   taskInfoLog();
   queueResetSave( &g_queueAddacDaq );

   /*
    *         *** Main loop of ADDAC- DAQs ***
    */
   while( true )
   {
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

      daqHandlePostMortem();

      SCU_BUS_IRQ_QUEUE_T queueScuBusIrq;
      if( !queuePopSave( &g_queueAddacDaq, &queueScuBusIrq ) )
      {
      #if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_DAQ_TASK )
        /*
         * Sleep till wake up when queue is empty.
         */
         xTaskNotifyWait( pdFALSE, 0, NULL, portMAX_DELAY );
      #endif
         continue;
      }

      /*
       * Queue has at least one valid message.
       */
      DAQ_DEVICE_T* pDaqDevice = daqBusGetDeviceBySlotNumber( &g_scuDaqAdmin.oDaqDevs, queueScuBusIrq.slot );
      
      DAQ_REGISTER_T continousIntPending = 0;
      if( (queueScuBusIrq.pendingIrqs & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0 )
         continousIntPending = daqDeviceGetAndResetContinuousIntPendingBits( pDaqDevice );
      
      DAQ_REGISTER_T hiResPending = 0; 
      if( (queueScuBusIrq.pendingIrqs & (1 << DAQ_IRQ_HIRES_FINISHED)) != 0 )
         hiResPending = daqDeviceGetAndResetHighresIntPendingBits( pDaqDevice );
      
      const unsigned int maxChannels = daqDeviceGetMaxChannels( pDaqDevice );
      for( unsigned int channelNumber = 0; channelNumber < maxChannels; channelNumber++ )
      {
         DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( pDaqDevice, channelNumber );

         /*
          * Handling of DAQ continous interrupt
          */
         if( (continousIntPending & pChannel->intMask) != 0 )
         {
         #ifdef CONFIG_DAQ_SW_SEQUENCE
            pChannel->sequenceContinuous++;
         #endif
            if( daqChannelGetDaqFifoWords( pChannel ) > 0 )
            {
               ramPushDaqDataBlock( &g_scuDaqAdmin.oRam, pChannel, true );
               daqChannelDecrementBlockCounter( pChannel );
            }
         }
         
         /*
          * Handling of DAQ high-resolution interrupt
          */
         if( (hiResPending & pChannel->intMask) != 0 )
         {
            daqChannelDisableHighResolution( pChannel );
         #ifdef CONFIG_DAQ_SW_SEQUENCE
            pChannel->sequencePmHires++;
         #endif
            ramPushDaqDataBlock( &g_scuDaqAdmin.oRam, pChannel, false );
         }
      }
      //TASK_YIELD();
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStartDaqIfAnyPresent( void )
{
   if( mg_taskDaqHandle != NULL )
   { /*
      * Task is already running.
      */
      return;
   }

   if( daqBusGetFoundDevices( &g_scuDaqAdmin.oDaqDevs ) == 0 )
   { /*
      * No DAQ devices found, running of this task is meaningless.
      */
      return;
   }

   TASK_CREATE_OR_DIE( taskDaq, 512, TASK_PRIO_ADDAC_DAQ, &mg_taskDaqHandle );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskStopDaqIfRunning( void )
{
   taskDeleteIfRunning( &mg_taskDaqHandle );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void daqTaskSuspend( void )
{
   if( mg_taskDaqHandle != NULL )
      vTaskSuspend( mg_taskDaqHandle );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void daqTaskResume( void )
{
   if( mg_taskDaqHandle != NULL )
      vTaskResume( mg_taskDaqHandle );
}

#if (configUSE_TASK_NOTIFICATIONS == 1) && defined( CONFIG_SLEEP_DAQ_TASK )
#warning CONFIG_SLEEP_DAQ_TASK doesnt work yet!
/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskWakeupDaqFromISR( void )
{
   if( mg_taskDaqHandle != NULL )
      xTaskNotifyFromISR( mg_taskDaqHandle, 0, 0, NULL );
}

/*! ---------------------------------------------------------------------------
 * @see scu_task_daq.h
 */
void taskWakeupDaq( void )
{
   if( mg_taskDaqHandle != NULL )
   {
      xTaskNotify( mg_taskDaqHandle, 0, eNoAction );
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "taskWakeupDaq" ESC_NORMAL );
   }
}
#endif /* if (configUSE_TASK_NOTIFICATIONS == 1) */

/*================================== EOF ====================================*/
