/*!
 *  @file continueIntTest.c
 *  @brief Testprogram for DAQ-Continue-Test using interrupt.
 *  @date 24.04.2023
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT AqNY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <string.h>
#include <sdb_lm32.h>
#include <daq.h>
#include <lm32Interrupts.h>
#include <scu_msi.h>
#include <sw_queue.h>
#include <eb_console_helper.h>
#include <helper_macros.h>

DAQ_BUS_T g_allDaq;

#define CHANNEL 0

void* g_pScub_base;

QUEUE_CREATE_STATIC( g_daqQueue, 10, SCU_BUS_IRQ_QUEUE_T );

/*-----------------------------------------------------------------------------
 */
void _segfault(int sig)
{
   mprintf( ESC_ERROR "Segmentation fault: %d\n" ESC_NORMAL, sig );
   while( 1 );
}

/*-----------------------------------------------------------------------------
 */
void readFiFo( DAQ_CANNEL_T* pThis )
{
   int j = 0;
   DAQ_DESCRIPTOR_T descriptor;
   memset( &descriptor, 0, sizeof( descriptor ) );

   volatile unsigned int remaining;
   int i = 0;
   do
   {
      remaining = daqChannelGetDaqFifoWords( pThis );
      volatile uint16_t data = daqChannelPopDaqFifo( pThis );
#if 0
      mprintf( "%d: 0x%04X, %d\n", i, data, remaining );
#endif
      if( remaining < ARRAY_SIZE( descriptor.index ) )
      {
         SCU_ASSERT( j < ARRAY_SIZE( descriptor.index ) );
         descriptor.index[j++] = data;
      }
      i++;
   }
   while( remaining != 0 );

#if 0
   for( j = 0; j < ARRAY_SIZE( descriptor.index ); j++ )
      mprintf( "Descriptor %d: 0x%04X\n", j, descriptor.index[j] );
#endif
   daqDescriptorPrintInfo( &descriptor );
   DAQ_DESCRIPTOR_VERIFY_MY( &descriptor, pThis );
}

/*-----------------------------------------------------------------------------
 */
void printIntRegs( DAQ_DEVICE_T* pDevice )
{
   volatile uint16_t* volatile pIntr_In      = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_In];
   volatile uint16_t* volatile pIntr_Ena     = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_Ena];
   volatile uint16_t* volatile pIntr_pending = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_pending];
   volatile uint16_t* volatile pIntr_Active  = &((uint16_t*)daqDeviceGetScuBusSlaveBaseAddress( pDevice ))[Intr_Active];

   mprintf( "Intr_In:      0x%08X -> %04b", pIntr_In, *pIntr_In );
   mprintf( "\nIntr_Ena:     0x%08X -> %04b", pIntr_Ena, *pIntr_Ena );
   mprintf( "\nIntr_pending: 0x%08X -> %04b", pIntr_pending, *pIntr_pending );
   mprintf( "\nIntr_Active:  0x%08X -> %04b", pIntr_Active, *pIntr_Active );

 //  (*pIntr_Ena) |= ~0;
   if( (*pIntr_Active) & 0x01 )
      (*pIntr_Active) |= 0x01;
   mprintf( "\n" );
}

#define ADDR_SCUBUS 0x00
/*-----------------------------------------------------------------------------
 */
STATIC void onScuMSInterrupt( const unsigned int intNum,
                              const void* pContext UNUSED )
{
   MSI_ITEM_T m;
   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
      if( GET_LOWER_HALF( m.adr ) != ADDR_SCUBUS )
         continue;

      SCU_BUS_IRQ_QUEUE_T queueScuBusIrq =
      {
         .slot = GET_LOWER_HALF( m.msg ) + SCUBUS_START_SLOT
      };

      while( (queueScuBusIrq.pendingIrqs = scuBusGetAndResetIterruptPendingFlags( g_pScub_base, queueScuBusIrq.slot )) != 0 )
      {
         if( (queueScuBusIrq.pendingIrqs & ((1 << DAQ_IRQ_DAQ_FIFO_FULL) | (1 << DAQ_IRQ_HIRES_FINISHED))) == 0 )
            continue;

         if( !queuePush( &g_daqQueue, &queueScuBusIrq ) )
            mprintf( ESC_ERROR "ERROR: DAQ-queue overflow!\n" ESC_NORMAL );
      }
   }
}

/*-----------------------------------------------------------------------------
 */
STATIC inline void enableMeassageSignaledInterruptsForScuBusSlave( const unsigned int slot )
{
   void* pScub_irq_base = find_device_adr( GSI, SCU_IRQ_CTRL );
   if( (int)pScub_irq_base == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: device address of SCU bus IRQ-base not found!\n" ESC_NORMAL );
      while( true );
   }

   scuBusSetSlaveValue16( scuBusGetSysAddr( g_pScub_base ), GLOBAL_IRQ_ENA, 0x20 );
   SET_REG32( pScub_irq_base, MSI_CHANNEL_SELECT, slot );
   SET_REG32( pScub_irq_base, MSI_SOCKET_NUMBER, slot );
   SET_REG32( pScub_irq_base, MSI_DEST_ADDR, (uint32_t)&((MSI_LIST_T*)g_pMyMsi)[0] );
   SET_REG32( pScub_irq_base, MSI_ENABLE, 1 << slot );
}


/*=============================================================================
 */
void main( void )
{
   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGENTA ESC_BOLD "DAQ Continous test, compiler: " COMPILER_VERSION_STRING ESC_NORMAL "\n");
#if 1
   g_pScub_base = find_device_adr( GSI, SCU_BUS_MASTER );
   if( (int)g_pScub_base == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: device address of SCU bus not found!\n" ESC_NORMAL );
      while( true );
   }

   if( daqBusFindAndInitializeAll( &g_allDaq, g_pScub_base ) <= 0 )
   {
      mprintf( ESC_ERROR "No usable DAQ found!\n" ESC_NORMAL );
      while( true );
   }
   mprintf( "%d DAQ- devices found %d channels\n",
            daqBusGetFoundDevices( &g_allDaq ),
            daqBusGetNumberOfAllFoundChannels( &g_allDaq ) );
   queueReset( &g_daqQueue );


   DAQ_CANNEL_T* pChannel = daqBusGetChannelObjectByAbsoluteNumber( &g_allDaq, CHANNEL );
   if( pChannel == NULL )
   {
      mprintf( ESC_ERROR "ERROR: Channel " TO_STRING( CHANNEL ) " not present!\n" ESC_NORMAL );
      return;
   }
   mprintf( "Using channel: " TO_STRING( CHANNEL ) " of ADDAC on slot: %u\n", daqChannelGetSlot( pChannel ) );


   printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );

   daqChannelPrintInfo( pChannel );

   printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pChannel ) );

   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );
   mprintf( "IRQ table configured: 0b%04b\n", irqGetMaskRegister() );
   enableMeassageSignaledInterruptsForScuBusSlave( daqChannelGetSlot( pChannel ) - SCUBUS_START_SLOT );
   irqEnable();

   daqChannelSetTriggerCondition( pChannel, 0x12345678 );
   mprintf( "Tag: 0x%08X\n", daqChannelGetTriggerCondition( pChannel ) );

  //!! daqChannelEnableTriggerMode( pChannel );
   daqChannelEnableExtrenTrigger( pChannel );
   //daqChannelEnableEventTrigger( pChannel );
   daqChannelSample1msOn( pChannel );
   daqChannelPrintInfo( pChannel );

   unsigned int receiveCount = 0;
   while( true )
   {
      SCU_BUS_IRQ_QUEUE_T queueScuBusIrq;
      if( !queuePopSave( &g_daqQueue, &queueScuBusIrq ) )
         continue;

      mprintf( ESC_FG_MAGENTA "SCU-bus slave MSI on slot: %u\n" ESC_NORMAL, queueScuBusIrq.slot );
      if( (queueScuBusIrq.pendingIrqs & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) == 0 )
         continue;

      DAQ_DEVICE_T* pAddac = daqBusGetDeviceBySlotNumber( &g_allDaq, queueScuBusIrq.slot );
      for( unsigned int i = 0; i < 4; i++ )
      {
         DAQ_CANNEL_T* pCurrentCannel = daqDeviceGetChannelObject( pAddac, i );
         if( !daqChannelTestAndClearDaqIntPending( pCurrentCannel ) )
            continue;

         daqChannelPrintInfo( pCurrentCannel );
         readFiFo( pCurrentCannel );
         printIntRegs( DAQ_CHANNEL_GET_PARENT_OF( pCurrentCannel ) );
         receiveCount++;
         if( receiveCount == 10 )
         {
            receiveCount = 0;
            daqChannelSample1msOff( pCurrentCannel );
            mprintf( "\n*** End ***\n" );
         }
      }
   }
#endif
   mprintf( "End...\n" );
}

/*================================== EOF ====================================*/
