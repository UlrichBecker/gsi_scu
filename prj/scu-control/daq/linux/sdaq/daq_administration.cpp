/*!
 *  @file daq_administration.cpp
 *  @brief DAQ administration
 *
 *  @date 04.03.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <daq_administration.hpp>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <unistd.h>
#include <eb_console_helper.h>
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
#include <sys/time.h>
#endif
#include <message_macros.hpp>

using namespace Scu;
using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
bool DaqChannel::SequenceNumber::compare( uint8_t sequence )
{
   m_blockLost = (m_sequence != sequence) && m_continued;
   if( m_blockLost )
   {
      m_lostCount++;
      DEBUG_MESSAGE( "ERROR: Sequence is " << sequence
                      << ", expected: " << m_sequence );
   }
   m_continued = true;
   m_sequence = sequence;
   m_sequence++;
   return m_blockLost;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel::DaqChannel( const uint number )
   :m_number( number )
   ,m_pParent(nullptr)
   ,m_poSequence(nullptr)
{
   DEBUG_MESSAGE_M_FUNCTION( "ADDAC number: " << m_number );
   SCU_ASSERT( m_number <= DaqInterface::c_maxChannels );
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel::~DaqChannel( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "ADDAC number: " << m_number );
}

/*! ---------------------------------------------------------------------------
 */
void DaqChannel::verifySequence( void )
{
   if( descriptorWasContinuous() )
      m_poSequence = &m_oSequenceContinueMode;
   else
      m_poSequence = &m_oSequencePmHighResMode;

   if( m_poSequence->compare( descriptorGetSequence() ) )
   {
      DaqAdministration* pAdmin = getParent()->getParent();
      SCU_ASSERT( dynamic_cast<DaqAdministration*>(pAdmin) != nullptr );
      pAdmin->readLastStatus();
      pAdmin->onBlockReceiveError();
   }
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( const uint number )
   :DaqBaseDevice( number )
   ,m_deviceNumber( 0 )
   ,m_slot( number )
   ,m_maxChannels( 0 )
   ,m_pParent(nullptr)
{
   DEBUG_MESSAGE_M_FUNCTION( number );
   SCU_ASSERT( m_deviceNumber <= DaqInterface::c_maxDevices );
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice::~DaqDevice( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   for( const auto& channel: *this )
      unregisterChannel( channel );
}

/* ----------------------------------------------------------------------------
 */
bool DaqDevice::registerChannel( DaqChannel* pChannel )
{
   SCU_ASSERT( dynamic_cast<DaqChannel*>(pChannel) != nullptr );
   SCU_ASSERT( m_channelPtrList.size() <= DaqInterface::c_maxChannels );

   for( const auto& i: m_channelPtrList )
   {
      if( pChannel->getNumber() == i->getNumber() )
         return true;
   }
   if( pChannel->m_number == 0 )
      pChannel->m_number = m_channelPtrList.size() + 1;
   pChannel->m_pParent = this;
   m_channelPtrList.push_back( pChannel );
   if( m_pParent != nullptr )
      pChannel->onInit();

   return false;
}

/* ----------------------------------------------------------------------------
 */
bool DaqDevice::unregisterChannel( DaqChannel* pChannel )
{
   SCU_ASSERT( dynamic_cast<DaqChannel*>(pChannel) != nullptr );

   if( pChannel->m_pParent != this )
      return true;

   for( const auto& i: m_channelPtrList )
   {
      if( i != pChannel )
         continue;
      //m_channelPtrList.erase( pChannel );
   }

   pChannel->m_pParent = nullptr;
   return false;
}

/* ----------------------------------------------------------------------------
 */
DaqChannel* DaqDevice::getChannel( const uint number )
{
   SCU_ASSERT( number > 0 );
   SCU_ASSERT( number <= DaqInterface::c_maxChannels );

   for( const auto& i: m_channelPtrList )
   {
      if( i->getNumber() == number )
         return i;
   }
   return nullptr;
}

/* ----------------------------------------------------------------------------
 */
void DaqDevice::init( void )
{
   for( const auto& i: m_channelPtrList )
      i->onInit();
}

/* ----------------------------------------------------------------------------
 */
void DaqDevice::reset( void )
{
   for( const auto& i: m_channelPtrList )
      i->onReset();
}

///////////////////////////////////////////////////////////////////////////////
//std::exception_ptr DaqAdministration::c_exceptionPtr = nullptr;

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqEb::EtherboneConnection* poEtherbone,
                                      const bool doReset,
                                      const bool doSendCommand
                                    )
   :DaqInterface( poEtherbone, doReset, doSendCommand )
   ,m_poBlockBuffer( nullptr )
   ,m_maxChannels( 0 )
   ,m_receiveCount( 0 )
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   ,m_elapsedTime( 0 )
#endif
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   m_poBlockBuffer = new BLOCK_BUFFER_T;
   ::memset( m_poBlockBuffer, 0, sizeof( BLOCK_BUFFER_T ) );
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqAccess* poEbAccess,
                                      const bool doReset,
                                      const bool doSendCommand
                                    )
   :DaqInterface( poEbAccess, doReset, doSendCommand )
   ,m_poBlockBuffer( nullptr )
   ,m_maxChannels( 0 )
   ,m_receiveCount( 0 )
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   ,m_elapsedTime( 0 )
#endif
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   m_poBlockBuffer = new BLOCK_BUFFER_T;
   ::memset( m_poBlockBuffer, 0, sizeof( BLOCK_BUFFER_T ) );
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::~DaqAdministration( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   if( m_poBlockBuffer != nullptr )
      delete m_poBlockBuffer;

   //for( const auto& dev: *this )
   //   unregisterDevice( dev );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdministration::registerDevice( DaqDevice* pDevice )
{
   SCU_ASSERT( dynamic_cast<DaqDevice*>(pDevice) != nullptr );
   SCU_ASSERT( m_devicePtrList.size() <= DaqInterface::c_maxDevices );

   for( const auto& i: m_devicePtrList )
   {
      if( pDevice->getDeviceNumber() == i->getDeviceNumber() )
         return true;
   }

   /*
    * Is device number forced?
    */
   if( pDevice->m_deviceNumber == 0 )
   { /*
      * No, allocation automatically.
      */
      if( pDevice->m_slot == 0  || !isLM32CommandEnabled() )
         pDevice->m_deviceNumber = m_devicePtrList.size() + 1;
      else
         pDevice->m_deviceNumber = getDeviceNumber( pDevice->m_slot );
   }

   if( isLM32CommandEnabled() )
   {
      if( pDevice->m_slot != 0 )
      {
         if( pDevice->m_slot != getSlotNumber( pDevice->m_deviceNumber ) )
            return true;
      }
      else
         pDevice->m_slot = getSlotNumber( pDevice->m_deviceNumber );

      pDevice->m_maxChannels = readMaxChannels( pDevice->m_deviceNumber );
   }
   else
      pDevice->m_maxChannels = DaqDevice::MAX_CHANNELS;

   m_maxChannels          += pDevice->m_maxChannels;
   pDevice->m_pParent     = this;
   m_devicePtrList.push_back( pDevice );
   pDevice->init();
   pDevice->m_deviceTyp = readDeviceType( pDevice->m_deviceNumber );

   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdministration::unregisterDevice( DaqDevice* pDevice )
{
   if( pDevice->m_pParent != this )
      return true;

   pDevice->m_pParent = nullptr;
   pDevice->m_deviceTyp = UNKNOWN;
   return false;
}

/*! ---------------------------------------------------------------------------
 */
int DaqAdministration::redistributeSlotNumbers( void )
{
   if( readSlotStatus() != DAQ_RET_OK )
   {
      for( const auto& i: m_devicePtrList )
      {
         i->m_slot = 0; // Invalidate slot number
      }
      return getLastReturnCode();
   }

   for( const auto& i: m_devicePtrList )
   {
      i->m_slot = getSlotNumber( i->m_deviceNumber );
   }

   return getLastReturnCode();
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDeviceByNumber( const uint number )
{
   SCU_ASSERT( number > 0 );
   SCU_ASSERT( number <= c_maxDevices );

   for( const auto& i: m_devicePtrList )
   {
      if( i->getDeviceNumber() == number )
         return i;
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDeviceBySlot( const uint slot )
{
   SCU_ASSERT( slot > 0 );
   SCU_ASSERT( slot <= c_maxSlots );

   for( const auto& i: m_devicePtrList )
   {
      if( i->getSlot() == slot )
         return i;
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelByAbsoluteNumber( uint absChannelNumber )
{
   SCU_ASSERT( absChannelNumber > 0 );
   SCU_ASSERT( absChannelNumber <= (c_maxChannels * c_maxDevices) );

   for( const auto& i: m_devicePtrList )
   {
      if( absChannelNumber > i->getMaxChannels() )
      {
         absChannelNumber -= i->getMaxChannels();
         continue;
      }
      return i->getChannel( absChannelNumber );
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelByDeviceNumber( const uint deviceNumber,
                                             const uint channelNumber )
{
   DAQ_ASSERT_CHANNEL_ACCESS( deviceNumber, channelNumber );

   DaqDevice* poDevice = getDeviceByNumber( deviceNumber );
   if( poDevice == nullptr )
      return nullptr;

   return poDevice->getChannel( channelNumber );
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelBySlotNumber( const uint slotNumber,
                                           const uint channelNumber )
{
   if( slotNumber == 0 )
      return nullptr;

   if( slotNumber > c_maxSlots )
      return nullptr;

   if( channelNumber == 0 )
      return nullptr;

   if( channelNumber > c_maxChannels )
      return nullptr;

   DaqDevice* poDevice = getDeviceBySlot( slotNumber );
   if( poDevice == nullptr )
      return nullptr;

   return poDevice->getChannel( channelNumber );
}

/*! ---------------------------------------------------------------------------
 */
void DaqAdministration::onErrorDescriptor( const DAQ_DESCRIPTOR_T& roDescriptor )
{
   throw( DaqException( "Erroneous descriptor" ) );
}

/*! ---------------------------------------------------------------------------
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen#Die_CRC_Pr_252fsumme
 */
uint16_t DaqAdministration::crcPolynom( uint16_t x )
{
   return static_cast<uint16_t>(1 + x * x + x * x * x * x * x);
}

/*! ---------------------------------------------------------------------------
 */
void DaqAdministration::onErrorCrc( void )
{
   //DEBUG_MESSAGE_M_FUNCTION( "" );
}

/*! ---------------------------------------------------------------------------
 */
uint DaqAdministration::distributeData( void )
{
   assert( m_poBlockBuffer != nullptr );
   /*
    * Getting the number of DDR3 memory items which has to be copied
    * in the m_poBlockBuffer buffer.
    */
   const uint toRead = std::min( getNumberOfNewData(),
                                 static_cast<uint>(sizeof( BLOCK_BUFFER_T ) / sizeof(RAM_DAQ_PAYLOAD_T)) );

   if( toRead == 0 )
      return toRead;

   static_assert( (c_ramBlockLongLen % c_ramBlockShortLen) == 0, "" );

   if( (toRead % c_ramBlockShortLen) != 0 )
   {
      DEBUG_MESSAGE( toRead << " items in ADDAC buffer not dividable by " << c_ramBlockShortLen );
      onDataError();
      return toRead;
   }

#ifdef CONFIG_DAQ_DEBUG
   ::memset( &m_poBlockBuffer, 0x7f, sizeof( BLOCK_BUFFER_T ) );
#endif

#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   USEC_T startTime = getSysMicrosecs();
#endif

   /*
    * Copying via wishbone/etherbone the DDR3-RAM data in the middle buffer.
    * This occupies the wishbone/etherbone bus!
    */
   readDaqData( &m_poBlockBuffer->ramItems[0], c_ramBlockShortLen );

#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   m_elapsedTime = std::max( getSysMicrosecs() - startTime, m_elapsedTime );
#endif
   /*
    * Rough check of the device descriptors integrity.
    */
   if( !::daqDescriptorVerifyMode( &m_poBlockBuffer->descriptor ) ||
       !gsi::isInRange( descriptorGetSlot(),
                        static_cast<uint>(Bus::SCUBUS_START_SLOT),
                        static_cast<uint>(Bus::MAX_SCU_SLAVES) ) ||
       !isDevicePresent( descriptorGetSlot() ) ||
       ( descriptorGetChannel() >= DaqDevice::MAX_CHANNELS )
     )
   {
      onErrorDescriptor( m_poBlockBuffer->descriptor );
      return getCurrentNumberOfData();
   }

   /*!
    * Holds the number of received payload data words without descriptor.
    */
   std::size_t wordLen;

   if( ::daqDescriptorIsLongBlock( &m_poBlockBuffer->descriptor ) )
   { /*
      * Long block has been detected, (high resolution or post mortem)
      * in this case the rest of the data has still to be read
      * from the DAQ-Ram-buffer.
      */
   #ifdef CONFIG_DAQ_TIME_MEASUREMENT
      startTime = getSysMicrosecs();
   #endif
      readDaqData( &m_poBlockBuffer->ramItems[c_ramBlockShortLen],
                   c_ramBlockLongLen - c_ramBlockShortLen );
   #ifdef CONFIG_DAQ_TIME_MEASUREMENT
      m_elapsedTime = std::max( getSysMicrosecs() - startTime, m_elapsedTime );
   #endif
      sendWasRead( c_ramBlockLongLen );
      wordLen = c_hiresPmDataLen - c_discriptorWordSize;
   }
   else
   { /*
      * Short block has been detected (continuous mode).
      * All data of this block has been already read.
      */
      sendWasRead( c_ramBlockShortLen );
      wordLen = c_contineousDataLen - c_discriptorWordSize;
   }

   /*
    *  See: https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen#Die_CRC_Pr_252fsumme
    * TODO It doesn't work yet! :-(
    */
   uint16_t crc = 0x001F;
   for( uint i = c_discriptorWordSize; i < wordLen; i++ )
   {
      crc += crcPolynom( m_poBlockBuffer->buffer[i] );
   }
   for( uint i = 0; i < (c_discriptorWordSize-1); i++ )
   {
      crc += crcPolynom( m_poBlockBuffer->buffer[i] );
   }
   if( daqDescriptorGetCRC( &m_poBlockBuffer->descriptor ) != static_cast<uint8_t>(crc & 0x00FF) )
   {
      onErrorCrc();
   }

   m_receiveCount++;

#ifdef CONFIG_USE_ADDAC_DAQ_BLOCK_STATISTICS
   /*
    * For statistics only.
    */
   onIncomingDescriptor( m_poBlockBuffer->descriptor );
#endif

   DaqChannel* pChannel = getChannelByDescriptor();

   if( pChannel != nullptr )
   {
      pChannel->verifySequence();
      pChannel->onDataBlock( &m_poBlockBuffer->buffer[c_discriptorWordSize], wordLen );
   }
   else
   {
      readLastStatus();
      onUnregistered( m_poBlockBuffer->descriptor );
   }

   return getCurrentNumberOfData();
}

/*! ---------------------------------------------------------------------------
 */
void DaqAdministration::reset( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   for( const auto& pDev: m_devicePtrList )
      pDev->reset();
}

//================================== EOF ======================================
