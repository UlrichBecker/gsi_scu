/*!
 *  @file daq_eb_ram_buffer.cpp
 *  @brief Linux whishbone/etherbone interface for accessing the
 *         LM32 shared memory and the
 *         DDR3-RAM for SCU3 or -- in future -- SRAM for SCU4.
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 19.06.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <daq_eb_ram_buffer.hpp>
/*
 * CAUTION: When the compiler miss the header file shared_mmap.h, then compile
 *          at first the LM32-application "scu_control.bin" which will
 *          generated this header file automatically.
 */
#include <shared_mmap.h>

#include <message_macros.hpp>
#include <BusException.hpp>
#include <string>

using namespace Scu;
using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 */
EbRamAccess::Lm32ShMemAccess::Lm32ShMemAccess( EBC_PTR_T poEb )
   :Lm32Access( poEb )
{
   /*
    * CAUTION: When the compiler doesn't find the constant SHARED_OFFS,
    * then compile at first the LM32-application scu_control.
    * The constant SHARED_OFFS is defined in shared_mmap.h which will
    * generated automatically by compiling the LM32 application.
    */
   m_baseAddress += SHARED_OFFS;
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::EbRamAccess( EBC_PTR_T poEb )
   :m_oLm32( poEb )
   ,m_poRamBuffer( nullptr )
#ifdef CONFIG_EB_TIME_MEASSUREMENT
   ,m_startTime( 0 )
#endif
{
   DEBUG_MESSAGE_M_FUNCTION("");

   try
   {
      m_poRamBuffer = new Ddr3Access( m_oLm32.getEb() );
      DEBUG_MESSAGE( "Using DDR3-RAM on SCU3" );
   }
   catch( DaqEb::BusException& e )
   {
      std::string exceptText = e.what();
      if( exceptText.find( "VendorId" ) == std::string::npos )
         throw DaqEb::BusException( e );

      m_poRamBuffer = new SramAccess( m_oLm32.getEb() );
      DEBUG_MESSAGE( "Using SRAM on SCU4" );
   }
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::~EbRamAccess( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_poRamBuffer != nullptr )
      delete m_poRamBuffer;
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
int EbRamAccess::getBurstLimit( void )
{
   if( dynamic_cast<Ddr3Access*>(m_poRamBuffer) != nullptr )
      return static_cast<Ddr3Access*>(m_poRamBuffer)->getBurstLimit();

   DEBUG_MESSAGE( "Function: " << __FUNCTION__ << " not available no using DDR3 !" );
   return Ddr3Access::NEVER_BURST;
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
void EbRamAccess::setBurstLimit( int burstLimit )
{
   if( dynamic_cast<Ddr3Access*>(m_poRamBuffer) != nullptr )
   {
      static_cast<Ddr3Access*>(m_poRamBuffer)->setBurstLimit( burstLimit );
      return;
   }
   DEBUG_MESSAGE( "Function: " << __FUNCTION__ << " not available no using DDR3 !" );
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
void EbRamAccess::readRam( RAM_DAQ_PAYLOAD_T* pData,
                           std::size_t len,
                           RAM_RING_INDEXES_T& rIndexes
                         )
{
   RAM_RING_INDEXES_T indexes = rIndexes;
   const std::size_t lenToEnd = ramRingGetUpperReadSize( &indexes );
   if( lenToEnd < len )
   {
      readRam( pData, lenToEnd, ramRingGetReadIndex( &indexes ) );
      ramRingAddToReadIndex( &indexes, lenToEnd );
      len   -= lenToEnd;
      pData += lenToEnd;
   }
   readRam( pData, len, ramRingGetReadIndex( &indexes ) );
   ramRingAddToReadIndex( &indexes, len );
   rIndexes = indexes;
}

#ifdef CONFIG_EB_TIME_MEASSUREMENT //==========================================
   #ifdef CONFIG_EB_TIME_MEASSUREMENT_TO_STDERR
     #warning "Timemeasurment output to stderr is active"
   #endif
/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
void EbRamAccess::stopTimeMeasurement( const std::size_t size, const WB_ACCESS_T access )
{
   const USEC_T newDuration = getSysMicrosecs() - m_startTime;

   if( newDuration > m_oMaxDuration.m_duration )
   {
   #ifdef CONFIG_EB_TIME_MEASSUREMENT_TO_STDERR
      std::cerr << "WB max duration: " << newDuration << " us" << std::endl;
   #endif
      m_oMaxDuration.m_duration  = newDuration;
      m_oMaxDuration.m_timestamp = m_startTime;
      m_oMaxDuration.m_eAccess   = access;
      m_oMaxDuration.m_dataSize  = size;
   }

   if( newDuration < m_oMinDuration.m_duration )
   {
   #ifdef CONFIG_EB_TIME_MEASSUREMENT_TO_STDERR
      std::cerr << "WB min duration: " << newDuration << " us" << std::endl;
   #endif
      m_oMinDuration.m_duration  = newDuration;
      m_oMinDuration.m_timestamp = m_startTime;
      m_oMinDuration.m_eAccess   = access;
      m_oMinDuration.m_dataSize  = size;
   }
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::WB_ACCESS_T
EbRamAccess::getWbMeasurementMaxTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize )
{
   rTimestamp = m_oMaxDuration.m_timestamp;
   rDuration  = m_oMaxDuration.m_duration;
   rSize      = m_oMaxDuration.m_dataSize;

   m_oMaxDuration.m_duration  = 0;
   m_oMaxDuration.m_timestamp = 0;
   m_oMaxDuration.m_dataSize  = 0;

   const WB_ACCESS_T ret = m_oMaxDuration.m_eAccess;
   m_oMaxDuration.m_eAccess = TIME_MEASUREMENT_T::UNKNOWN;

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::WB_ACCESS_T
EbRamAccess::getWbMeasurementMinTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize )
{
   rTimestamp = m_oMinDuration.m_timestamp;
   rDuration  = m_oMinDuration.m_duration;
   rSize      = m_oMinDuration.m_dataSize;

   m_oMinDuration.m_duration  = static_cast<USEC_T>(~0);
   m_oMinDuration.m_timestamp = 0;
   m_oMinDuration.m_dataSize  = 0;

   const WB_ACCESS_T ret = m_oMinDuration.m_eAccess;
   m_oMinDuration.m_eAccess = TIME_MEASUREMENT_T::UNKNOWN;

   return ret;
}

#endif // ifdef CONFIG_EB_TIME_MEASSUREMENT ===================================

//================================== EOF ======================================
