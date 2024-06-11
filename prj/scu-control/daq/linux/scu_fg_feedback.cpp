/*!
 * @file scu_fg_feedback.cpp
 * @brief Administration of data aquesition units for function generator
 *        feedback. Fusion of MIL- and ADDAC DAQ.
 *
 * @date 25.05.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#include <stdlib.h>
#include <daq_exception.hpp>
#include <scu_fg_feedback.hpp>
#include <message_macros.hpp>
#include <string.h>

using namespace Scu;

#ifndef CONFIG_OECORE_SDK_VERSION
   #warning "CAUTION: Module becomes not build by YOCTO SDK !"
#endif

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::Throttle::Throttle( Common* pParent )
   :m_pParent( pParent )
   ,m_lastForwardedValue( 0 )
   ,m_timeThreshold( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::Throttle::~Throttle( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
bool FgFeedbackChannel::Common::Throttle::operator()( const uint64_t timestamp,
                                                      const DAQ_T value )
{
   assert( m_pParent->m_pParent != nullptr );
   assert( m_pParent->m_pParent->m_pParent != nullptr );

   const FgFeedbackAdministration* pAdmin = m_pParent->m_pParent->m_pParent->m_pParent;
   assert( pAdmin != nullptr );

   if( ( static_cast<uint>(::abs( static_cast<int>(value - m_lastForwardedValue) )) < pAdmin->m_throttleThreshold ) &&
       (( timestamp < m_timeThreshold ) || ( pAdmin->m_throttleTimeout == 0 ))
     )
      return false;

   m_lastForwardedValue = value;
   m_timeThreshold = timestamp + pAdmin->m_throttleTimeout;
   return true;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::Common( FgFeedbackChannel* pParent )
   :m_pParent( pParent )
   ,m_oSetThrottle( this )
   ,m_oActThrottle( this )
   ,m_lastSupprTimestamp( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::~Common( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 * @brief Data-reduced forwarding to the higher software-layer.
 *
 * Only value changes outside of a threshold and/or the time between two
 * function calls exceeds a determined value becomes forwarded to the higher
 * software layer.
 */
void FgFeedbackChannel::Common::evaluate( const uint64_t wrTimeStampTAI,
                                          const DAQ_T actValue,
                                          const DAQ_T setValue )
{
   assert( wrTimeStampTAI > 0 );

   if( m_oSetThrottle( wrTimeStampTAI, setValue ) ||
       m_oActThrottle( wrTimeStampTAI, actValue ) )
   {
      if( m_lastSupprTimestamp != 0 )
      { /*
         * When the last tuple was suppressed by throttling it becomes
         * necessary to forwarding it here, because in order to avoiding
         * a wrong line in a possible plot between two support dots.
         */
         m_pParent->onData( m_lastSupprTimestamp,
                            m_lastSupprActValue, m_lastSupprSetValue );
         m_lastSupprTimestamp = 0;
      }

      /*
       * Invoking the the callback function implemented in
       * the next higher software layer.
       */
      m_pParent->onData( wrTimeStampTAI, actValue, setValue );
   }
   else
   { /*
      * Storing of the suppressed tuple, maybe it will nevertheless
      * used at the next time. See above.
      */
      m_lastSupprTimestamp = wrTimeStampTAI;
      m_lastSupprSetValue  = setValue;
      m_lastSupprActValue  = actValue;
   }

   m_pParent->m_lastTimestamp = wrTimeStampTAI;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::Receive::Receive( AddacFb* pParent, const uint n )
   :daq::DaqChannel( n )
   ,m_pParent( pParent )
   ,m_timestamp( 0 )
   ,m_sampleTime( 0 )
   ,m_blockLen( 0 )
   ,m_sequence( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   assert( n > 0 );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::Receive::~Receive( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 * @brief Storing of a incoming ADDAC/ACU-DAQ data block.
 */
bool FgFeedbackChannel::AddacFb::Receive::onDataBlock( daq::DAQ_DATA_T* pData,
                                                       std::size_t wordLen )
{
   const auto timestamp = descriptorGetTimeStamp();
   const bool isSetData = (this == &m_pParent->m_oReceiveSetValue);

   m_pParent->m_pParent->onAddacDataBlock( isSetData,
                                           timestamp, pData, wordLen );

   if( !descriptorWasContinuous() )
   {
      m_pParent->m_pParent->onHighResPostMortemBlock( isSetData,
                                                      timestamp, pData, wordLen );
      return true;
   }

   if( wordLen >= ARRAY_SIZE(m_aBuffer) )
   {
      std::string str = "Size of received data out range. Actual: ";
      str += std::to_string( wordLen );
      str += ", maximum: ";
      str += std::to_string( ARRAY_SIZE(m_aBuffer) );
      throw daq::Exception( str );
   }

   m_blockLen   = wordLen;
   m_sequence   = descriptorGetSequence();
   m_sampleTime = descriptorGetTimeBase();

   /*
    * Setting the timestamp at the begining of the ADDAC-DAQ-block, that means
    * at the first sampled value of the block.
    */
   m_timestamp  = timestamp - m_sampleTime * m_blockLen;

   ::memcpy( m_aBuffer, pData, m_blockLen * sizeof(daq::DAQ_DATA_T) );

   m_pParent->finalizeBlock();

   return false;
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::AddacFb::Receive::onInit( void )
{
   if( this == &m_pParent->m_oReceiveActValue )
      m_pParent->m_pParent->onInit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::AddacFb::Receive::onReset( void )
{
   if( this == &m_pParent->m_oReceiveActValue )
      m_pParent->m_pParent->onReset();
}

/*! ---------------------------------------------------------------------------
 */
inline FgFeedbackChannel::DAQ_T
FgFeedbackChannel::AddacFb::Receive::operator[]( const std::size_t i ) const
{
   assert( i < ARRAY_SIZE(m_aBuffer) );
   return m_aBuffer[i] << FgFeedbackAdministration::VALUE_SHIFT;
}


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::AddacFb( FgFeedbackChannel* pParent,
                                     const daq::DAQ_DEVICE_TYP_T type )
   :Common( pParent )
   ,m_oReceiveSetValue( this, 1 + daq::daqGetSetDaqNumberOfFg( pParent->getFgNumber(), type ) )
   ,m_oReceiveActValue( this, 1 + daq::daqGetActualDaqNumberOfFg( pParent->getFgNumber(), type ) )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::~AddacFb( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 * @brief Forwarding of actual- and set- values to the higher software-layer
 *        once both data blocks has been received.
 */
void FgFeedbackChannel::AddacFb::finalizeBlock( void )
{  /*
    * At the first time one of both channels doesn't received yet,
    * in this case it's block length is still zero.
    */
   if( m_oReceiveSetValue.getBlockLen() == 0 )
      return;
   if( m_oReceiveActValue.getBlockLen() == 0 )
      return;

   DEBUG_MESSAGE( "set sequence: " << static_cast<uint>(m_oReceiveSetValue.getSequence()) );
   DEBUG_MESSAGE( "act sequence: " << static_cast<uint>(m_oReceiveActValue.getSequence()) );

   /*
    * Safety check: The data length of both blocks have to be equal!
    */
   if( m_oReceiveSetValue.getBlockLen() != m_oReceiveActValue.getBlockLen() )
   {
      std::string str = "Different block sizes received: set data: ";
      str += std::to_string( m_oReceiveSetValue.getBlockLen() );
      str += " actual data: ";
      str += std::to_string( m_oReceiveActValue.getBlockLen() );
      throw daq::Exception( str );
   }

   /*
    * Safety check: The sample interval time of set and actual data have to be equal!
    */
   if( m_oReceiveSetValue.getSampleTime() != m_oReceiveActValue.getSampleTime() )
   {
      std::string str = "Different sample intervals between set data (";
      str += std::to_string( m_oReceiveSetValue.getSampleTime() );
      str += " us) and actual data (";
      str += std::to_string( m_oReceiveActValue.getSampleTime() );
      str += " us) received!";
      throw daq::Exception( str );
   }

   auto timeStampSetVal = m_oReceiveSetValue.getTimestamp();

   if( m_pParent->m_pParent->m_pParent->isPairingBySequence() )
   { /*
      * +++ Pairing by sequence number +++
      * One of both channels has received first, in this case it has to be wait
      * for the second channel.
      * This will accomplished by comparing the sequence numbers.
      */
      const auto sequenceDeviation = ::abs( m_oReceiveSetValue.getSequence() -
                                            m_oReceiveActValue.getSequence() );
      if( sequenceDeviation != 0 )
      {
         if( sequenceDeviation == 1 || sequenceDeviation == static_cast<daq::DAQ_SEQUENCE_T>(~0) )
         { /*
            * A sequence number deviation of one means that at the moment only one channel has
            * received a new data block, therefore it must be wait for the incoming block
            * of the other channel.
            * This will happen every second function call in normal operation.
            */
            return;
         }

         /*
          * Throwing a exception if following function will not be overwritten.
          */
         m_pParent->onActSetBlockDeviation( m_oReceiveSetValue.getSequence(),
                                            m_oReceiveActValue.getSequence() );

         return;
      }
   }
   else
   { /*
      * +++ Pairing by timestamp +++
      * That should be the default.
      */
      const auto timeStampActVal = m_oReceiveActValue.getTimestamp();
      const uint diff = ::abs( static_cast<int64_t>(timeStampActVal - timeStampSetVal));
      static_assert( REL_PHASE_TOLERANCE == 1 || REL_PHASE_TOLERANCE == 2,
                     "Relative phase tolerance shall be one or two!" );
      if( diff > static_cast<uint>(REL_PHASE_TOLERANCE * m_oReceiveSetValue.getSampleTime()) )
      { /*
         * Is the time deviation between set- and actual- value-block
         * greater than a specific value, then it have to be wait for the
         * incoming block of the other channel.
         * Therefore this function will be terminated here.
         * This will happen every second function call in normal operation.
         */
         return;
      }

      if( diff > 0 )
      {
         m_pParent->onActSetTimestampDeviation( timeStampSetVal, timeStampActVal );
      }
   }

   /*
    * Forwarding of set- and actual- values to the higher software layer.
    */
   for( std::size_t i = 0; i < m_oReceiveSetValue.getBlockLen();
        i++, timeStampSetVal += m_oReceiveSetValue.getSampleTime() )
   {
      evaluate( timeStampSetVal, m_oReceiveActValue[i], m_oReceiveSetValue[i] );
   }
}

#ifdef CONFIG_MIL_FG
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::Receive::Receive( MilFb* pParent )
   :MiLdaq::DaqCompare( pParent->m_pParent->getFgNumber() )
   ,m_pParent( pParent )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::Receive::~Receive( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onData( uint64_t wrTimeStampTAI,
                                                MiLdaq::MIL_DAQ_T actlValue,
                                                MiLdaq::MIL_DAQ_T setValue )
{
   if( m_pParent->m_pParent->m_lastTimestamp < wrTimeStampTAI )
   {
      m_pParent->m_pParent->onMilData( wrTimeStampTAI, actlValue, setValue );

     /*
      * Just forwarding, that's all.
      */
      m_pParent->evaluate( wrTimeStampTAI, actlValue, setValue );
   }
   else
   {
      m_pParent->m_pParent->onTimestampError( wrTimeStampTAI, actlValue, setValue );
   }
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onInit( void )
{
   m_pParent->m_pParent->onInit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onReset( void )
{
   m_pParent->m_pParent->onReset();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::MilFb( FgFeedbackChannel* pParent )
   :Common( pParent )
   ,m_oReceive( this )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::~MilFb( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

#endif // ifdef CONFIG_MIL_FG

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::FgFeedbackChannel( const uint fgNumber )
      :m_fgNumber( fgNumber )
      ,m_pParent( nullptr )
      ,m_pCommon( nullptr )
      ,m_lastTimestamp( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION( fgNumber );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::~FgFeedbackChannel( void )
{
   DEBUG_MESSAGE_M_FUNCTION( m_fgNumber );

   if( m_pParent != nullptr )
      m_pParent->unregisterChannel( this );

   if( m_pCommon != nullptr )
      delete m_pCommon;
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice* FgFeedbackChannel::getParent( void )
{
   if( m_pParent == nullptr )
   {
      std::string str = "Feedback channel number ";
      str += std::to_string( m_fgNumber );
      str += " isn't registered!";
      throw daq::Exception( str );
   }
   return m_pParent;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_feedback.hpp
 */
std::string FgFeedbackChannel::getFgName( void )
{
   std::string str = "fg-";
   if( m_pParent == nullptr )
      str += "unknown";
   else
      str += std::to_string( getSocket() );
   str += '-';
   str += std::to_string( getFgNumber() );
   return str;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_feedback.hpp
 */
bool FgFeedbackChannel::isSetValueInvalid( void )
{
   if( m_pCommon == nullptr )
   { /*
      * This channel has not been registered yet, therefore
      * the set value is always invalid.
      */
      return true;
   }
#ifdef CONFIG_MIL_FG
   if( dynamic_cast<MilFb*>(m_pCommon) != nullptr )
   { /*
      * In the case of MIL-DAQ the set value could be invalid during
      * read back within a gap.
      */
      return static_cast<MilFb*>(m_pCommon)->isSetValueInvalid();
   }
#endif
   /*
    * In the case of ADDAC-DAQ the set value is always valid.
    */
   return false;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_feedback.hpp
 */
void FgFeedbackChannel::onActSetBlockDeviation( const uint setSequ, const uint actSequ )
{
   std::string str = "Deviation of sequence numbers of: ";
   str += getFgName();
   str += ": from set value input stream: ";
   str += std::to_string( setSequ );
   str += ", and actual value input stream: ";
   str += std::to_string( actSequ );
   str += " are greater than one!";
   throw daq::Exception( str );
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_feedback.hpp
 */
void FgFeedbackChannel::onActSetTimestampDeviation( const uint64_t setTimeStamp,
                                                    const uint64_t actTimestamp )
{
   std::string str = "Deviation of time stamps of ";
   str += getFgName();
   str += ": set: ";
   str += std::to_string( setTimeStamp );
   str += " us; act: ";
   str += std::to_string( actTimestamp );
   str += " us; difference: ";
   str += std::to_string( static_cast<int>(actTimestamp - setTimeStamp) );
   str += " us;";
   throw daq::Exception( str );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::FgFeedbackDevice( const uint socket )
   :m_poDevice( nullptr )
   ,m_pParent( nullptr )
{
   DEBUG_MESSAGE_M_FUNCTION( socket );

   if( ::isAddacFg( socket ) )
   {
      DEBUG_MESSAGE( "Creating ADDAC-device on slot: " << ::getFgSlotNumber( socket ) );
      m_poDevice = new daq::DaqDevice( socket );
   }
#ifdef CONFIG_MIL_FG
   else if( ::isMilFg( socket ) && (getFgSlotNumber( socket ) <= Bus::MAX_SCU_SLAVES))
   {
      DEBUG_MESSAGE( "Creating MIL-device on slot: " << ::getFgSlotNumber( socket ) );
      m_poDevice = new MiLdaq::DaqDevice( socket );
   }
#endif
   else
   {
      std::string str = "Unknown DAQ device type with socket: ";
      str += std::to_string( socket );
      throw daq::Exception( str );
   }

   DEBUG_MESSAGE( typeid(m_poDevice).name() << " created" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::~FgFeedbackDevice( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_pParent != nullptr )
       m_pParent->unregisterDevice( this );

   for( auto& channel: m_lChannelList )
      channel->m_pParent = nullptr;

   if( m_poDevice != nullptr )
   {
      DEBUG_MESSAGE( "Destructor of " << (m_poDevice->isAddac()? "ADDAC" : "MIL")
                     << "-device on slot: " << m_poDevice->getSlot() );

      delete m_poDevice;
   }
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration* FgFeedbackDevice::getParent( void )
{
   if( m_pParent == nullptr )
   {
      std::string str = "Feedback device socket number ";
      str += std::to_string( getSocket() );
      str += " isn't registered!";
      throw daq::Exception( str );
   }
   return m_pParent;
}

/*! ---------------------------------------------------------------------------
 * @brief Generates the channel kernel for all registered channels,
 *        depending on the device type if not already done.
 */
void FgFeedbackDevice::generateAll( void )
{
   for( const auto& pFeedbackChannel: m_lChannelList )
   {
      if( pFeedbackChannel->m_pCommon != nullptr )
         continue;
      generate( pFeedbackChannel );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Generates the channel kernel depending on the device type.
 */
void FgFeedbackDevice::generate( FgFeedbackChannel* pFeedbackChannel )
{
   assert( pFeedbackChannel->m_pCommon == nullptr );
   assert( m_pParent != nullptr );

   const auto type = getTyp();
   DEBUG_MESSAGE( "generating channel for device type: " << deviceType2String( type ) );

#ifdef CONFIG_MIL_FG
   auto pMilDev = dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice);
   /*
    * Is this object a MIL device?
    */
   if( pMilDev != nullptr )
   {  /*
       * The feedback channel object becomes registered in a MIL device so
       * a MIL feedback object will created.
       */
      pFeedbackChannel->m_pCommon = new FgFeedbackChannel::MilFb( pFeedbackChannel );
      pMilDev->registerDaqCompare( &static_cast<FgFeedbackChannel::MilFb*>(pFeedbackChannel->m_pCommon)->m_oReceive );
      return;
   }
#endif // ifdef CONFIG_MIL_FG

   auto pAddacDev = dynamic_cast<daq::DaqDevice*>(m_poDevice);
   /*
    * Here a ADDAC/ACU object is provided.
    */
   if( pAddacDev == nullptr )
   {
   #ifdef CONFIG_MIL_FG
      throw daq::Exception( "A ADDAC-device object is expected!" );
   #else
      throw daq::Exception( "Unknown device object!" );
   #endif
   }

   /*
    * The feedback channel object becomes registered in a ADDAC/ACU device so
    * a ADDAC/ACU feedback object will created.
    */
   pFeedbackChannel->m_pCommon = new FgFeedbackChannel::AddacFb( pFeedbackChannel, type );

   /*
    * Register receive channel for set-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveSetValue );
   /*
    * Register receive channel for actual-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveActValue );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackDevice::registerChannel( FgFeedbackChannel* pFeedbackChannel )
{
   if( pFeedbackChannel->m_pParent != nullptr )
   {
      std::string str = "Feedback channel number ";
      str += std::to_string( pFeedbackChannel->getFgNumber() );
      str += " already registered in device ";
      str += std::to_string( pFeedbackChannel->getSocket() );
      throw daq::Exception( str );
   }

   assert( pFeedbackChannel->m_pCommon == nullptr );
   pFeedbackChannel->m_pParent = this;

#ifdef CONFIG_MIL_FG
   /*
    * Is this object a MIL device?
    */
   if( dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice) != nullptr )
   {
      if( (pFeedbackChannel->getFgNumber() >= MAX_FG_MACROS) ||
          (pFeedbackChannel->getFgNumber() == 0) )
      {
         std::string str = "Function generator number for MIL-FG ";
         str += std::to_string( pFeedbackChannel->getFgNumber() );
         str += " is out of range from 1 to <" TO_STRING( MAX_FG_MACROS ) " !";
         throw daq::Exception( str );
      }
   }
   else
#endif // ifdef CONFIG_MIL_FG
   /*
    * Is this object a ADDAC- device?
    */
   if( dynamic_cast<daq::DaqDevice*>(m_poDevice) != nullptr )
   {
      if( pFeedbackChannel->getFgNumber() >= MAX_FG_PER_SLAVE )
      {
         std::string str = "Function generator number for ADDAC/ACU-FG ";
         str += std::to_string( pFeedbackChannel->getFgNumber() );
         str += " is out of range from 0 to <" TO_STRING( MAX_FG_PER_SLAVE ) " !";
         throw daq::Exception( str );
      }
   }
   else
   { /*
      * Nither ADDAC- nor MIL- dvice.
      */
      throw daq::Exception( "Unknown function generator can't be registered!" );
   }

   if( m_pParent != nullptr )
      generate( pFeedbackChannel );

   m_lChannelList.push_back( pFeedbackChannel );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackDevice::unregisterChannel( FgFeedbackChannel* pFeedbackChannel )
{
   if( pFeedbackChannel->m_pParent != this )
      return;

   m_lChannelList.remove( pFeedbackChannel );
   pFeedbackChannel->m_pParent = nullptr;
   DEBUG_MESSAGE( "Channel fg-" << getSocket() << '-'
                                << pFeedbackChannel->getFgNumber()
                                << " unregistered!" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel* FgFeedbackDevice::getChannel( const uint number )
{
   for( const auto& i: m_lChannelList )
   {
      if( i->getFgNumber() == number )
         return i;
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onUnregistered( daq::DAQ_DESCRIPTOR_T& roDescriptor )
{
   m_pParent->onUnregisteredAddacDaq( daq::daqDescriptorGetSlot( &roDescriptor ),
                                      daq::daqDescriptorGetChannel( &roDescriptor ) );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onBlockReceiveError( void )
{
   m_pParent->onAddacBlockError( descriptorGetSlot(), descriptorGetChannel() );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onDataTimeout( void )
{
   m_pParent->onDataTimeout( false );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onDataError( void )
{
   m_pParent->onDataError( false );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onFifoAlarm( void )
{
   m_pParent->onFifoAlarm( false );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onErrorDescriptor( const daq::DAQ_DESCRIPTOR_T& roDescriptor )
{
   m_pParent->onErrorDescriptor( roDescriptor );
}

#ifdef CONFIG_USE_ADDAC_DAQ_BLOCK_STATISTICS
/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::AddacAdministration::onIncomingDescriptor( daq::DAQ_DESCRIPTOR_T& roDescriptor )
{
   m_pParent->onIncomingDescriptor( roDescriptor );
}
#endif

///////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::MilDaqAdministration::onUnregistered( const FG_MACRO_T fg )
{
   m_pParent->onUnregisteredMilDevice( fg );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::MilDaqAdministration::onDataTimeout( void )
{
   m_pParent->onDataTimeout( true );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::MilDaqAdministration::onDataError( void )
{
   m_pParent->onDataError( true );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::MilDaqAdministration::onFifoAlarm( void )
{
   m_pParent->onFifoAlarm( true );
}

#endif /* ifdef CONFIG_MIL_FG */

///////////////////////////////////////////////////////////////////////////////
FgFeedbackAdministration::FgFeedbackAdministration( const std::string& netaddress,
                                                    const bool doRescan,
                                                    uint timeout )
   :m_oAddacDaqAdmin( this, DaqEb::EtherboneConnection::getInstance( netaddress, timeout ) )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( this, m_oAddacDaqAdmin.getEbAccess() )
#endif
   ,m_lm32Swi( m_oAddacDaqAdmin.getEbAccess() )
   ,m_throttleThreshold( DEFAULT_THROTTLE_THRESHOLD << VALUE_SHIFT )
   ,m_throttleTimeout( DEFAULT_THROTTLE_TIMEOUT * daq::NANOSECS_PER_MILISEC )
   ,m_ebSelfAcquired( true )
{
   DEBUG_MESSAGE_M_FUNCTION(getScuDomainName());
   scan( doRescan );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( EBC_PTR_T poEtherbone,
                                                    const bool doRescan )
   :m_oAddacDaqAdmin( this, poEtherbone )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( this, m_oAddacDaqAdmin.getEbAccess() )
#endif
   ,m_lm32Swi( m_oAddacDaqAdmin.getEbAccess() )
   ,m_throttleThreshold( DEFAULT_THROTTLE_THRESHOLD << VALUE_SHIFT )
   ,m_throttleTimeout( DEFAULT_THROTTLE_TIMEOUT * daq::NANOSECS_PER_MILISEC )
   ,m_ebSelfAcquired( false )
{
   DEBUG_MESSAGE_M_FUNCTION(getScuDomainName());
   scan( doRescan );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( DaqAccess* poEbAccess,
                                                    const bool doRescan )
   :m_oAddacDaqAdmin( this, poEbAccess )
#ifdef CONFIG_MIL_FG
  ,m_oMilDaqAdmin( this, poEbAccess )
#endif
  ,m_lm32Swi( poEbAccess )
  ,m_throttleThreshold( DEFAULT_THROTTLE_THRESHOLD << VALUE_SHIFT )
  ,m_throttleTimeout( DEFAULT_THROTTLE_TIMEOUT  * daq::NANOSECS_PER_MILISEC )
  ,m_ebSelfAcquired( false )
{
   DEBUG_MESSAGE_M_FUNCTION(getScuDomainName());
   scan( doRescan );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::~FgFeedbackAdministration( void )
{
   DEBUG_MESSAGE_M_FUNCTION(getScuDomainName());

   for( const auto& pDev: m_lDevList )
      pDev->m_pParent = nullptr;

   if( m_ebSelfAcquired )
      DaqEb::EtherboneConnection::releaseInstance( m_oAddacDaqAdmin.getEbAccess()->getEbPtr() );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::scan( const bool doRescan )
{
   m_vPollList.clear();

   if( doRescan )
      m_oFoundFgs.scan( &m_lm32Swi );
   else
      m_oFoundFgs.sync( getEbAccess() );

#ifdef CONFIG_MIL_FG
   if( getNumOfFoundMilFg() != 0 )
      m_vPollList.push_back( &m_oMilDaqAdmin );
#endif

   if( (getNumOfFoundNonMilFg() != 0)
    #ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
       && m_oAddacDaqAdmin.isAddacDaqSupport()
    #endif
     )
      m_vPollList.push_back( &m_oAddacDaqAdmin );

   m_vPollList.shrink_to_fit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::registerDevice( FgFeedbackDevice* poDevice )
{
   if( poDevice->m_pParent != nullptr )
   {
      std::string str = "Device on socket ";
      str += std::to_string( poDevice->getSocket() );
      str += " already registered!";
      throw daq::Exception( str );
   }

   if( !isSocketUsed( poDevice->getSocket() ) )
   {
      std::string str = "Device on socket ";
      str += std::to_string( poDevice->getSocket() );
      str += " not present!";
      throw daq::Exception( str );
   }

   if( poDevice->isAddac() )
   {
      m_oAddacDaqAdmin.registerDevice( poDevice->getAddac() );
   }
#ifdef CONFIG_MIL_FG
   else if( poDevice->isMil() )
   {
      m_oMilDaqAdmin.registerDevice( poDevice->getMil() );
   }
#endif
   else
   {
      throw daq::Exception( "FgFeedbackAdministration::registerDevice: Umknown device type!" );
   }
   poDevice->m_pParent = this;
   poDevice->generateAll();
   m_lDevList.push_back( poDevice );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::unregisterDevice( FgFeedbackDevice* poDevice )
{
   if( poDevice->m_pParent != this )
      return;

   m_lDevList.remove( poDevice );
   poDevice->m_pParent = nullptr;
   DEBUG_MESSAGE( "Feedback device " << poDevice->getSocket() << " unregistered!" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice* FgFeedbackAdministration::getDevice( const uint socket )
{
   for( const auto& i: m_lDevList )
   {
      if( i->getSocket() == socket )
         return i;
   }
   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
int FgFeedbackAdministration::sendSyncronizeTimestamps( const uint32_t timeOffset,
                                                        const uint32_t ecaTag )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
//   MiLdaq::DaqAdministration::distributeData();
 #endif
   daq::DaqAdministration::sendSyncronizeTimestamps();
#endif

   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->sendSyncronizeTimestamps( timeOffset, ecaTag );

   return 0;
}

/*! ---------------------------------------------------------------------------
 */
uint FgFeedbackAdministration::distributeData( void )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration::distributeData();
 #endif
   daq::DaqAdministration::distributeData();
#endif

   uint remainingData = 0;
   for( const auto& poDaqAdmin: m_vPollList )
      remainingData += poDaqAdmin->distributeData();

   return remainingData;
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::clearBuffer( const bool update )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration::clearBuffer();
 #endif
   daq::DaqAdministration::clearBuffer();
#endif /* ifdef __DOXYGEN__ */

   for( const auto& poDaqAdmin: m_vPollList )
   {
      poDaqAdmin->clearBufferRequest();
   }
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::reset( void )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration::reset();
 #endif
   daq::DaqAdministration::reset();
#endif

   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->reset();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::onDataReadingPause( const bool isMil )
{
#ifdef CONFIG_MIL_FG
   if( isMil )
   {
      m_oMilDaqAdmin.MiLdaq::DaqAdministration::onDataReadingPause();
      return;
   }
#endif
   m_oAddacDaqAdmin.daq::DaqAdministration::onDataReadingPause();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::onDataError( const bool isMil )
{
#ifdef CONFIG_MIL_FG
   if( isMil )
   {
      m_oMilDaqAdmin.MiLdaq::DaqAdministration::onDataError();
      return;
   }
#endif
   m_oAddacDaqAdmin.daq::DaqAdministration::onDataError();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::onErrorDescriptor( const daq::DAQ_DESCRIPTOR_T& roDescriptor )
{
   m_oAddacDaqAdmin.daq::DaqAdministration::onErrorDescriptor( roDescriptor );
}

#ifdef CONFIG_EB_TIME_MEASSUREMENT
/*! ---------------------------------------------------------------------------
 */
const char* FgFeedbackAdministration::accessConstantToString( const WB_ACCESS_T access )
{
   #define __ACCESS_TO_STRING( a ) case a: return #a
   switch( access )
   {
      __ACCESS_TO_STRING( UNKNOWN );
      __ACCESS_TO_STRING( LM32_READ );
      __ACCESS_TO_STRING( LM32_WRITE );
      __ACCESS_TO_STRING( DDR3_READ );
   }
   return "\0";
}
#endif /* ifdef CONFIG_EB_TIME_MEASSUREMENT */
//================================== EOF ======================================
