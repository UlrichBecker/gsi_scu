/*!
 *  @file daq_eb_ram_buffer.cpp
 *  @brief Linux whishbone/etherbone interface for accessing the SCU-DDR3 RAM
 *
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
#include <shared_mmap.h>
#include <dbg.h>
#include <iostream>
using namespace Scu;
using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::EbRamAccess( DaqEb::EtherboneConnection* poEb )
   :m_poEb( poEb )
   ,m_connectedBySelf( false )
#ifndef CONFIG_NO_SCU_RAM
  #ifdef CONFIG_SCU_USE_DDR3
   ,m_ddr3TrModeBase( 0 )
   #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   ,m_ddr3BurstModeBase( 0 )
   #endif
  #endif
#endif
#ifdef CONFIG_EB_TIME_MEASSUREMENT
   , m_startTime( 0 )
#endif
{
   if( !m_poEb->isConnected() )
   {
      m_poEb->connect();
      m_connectedBySelf = true;
   }

   m_lm32SharedMemAddr = m_poEb->findDeviceBaseAddress( DaqEb::gsiId,
                                                        DaqEb::lm32_ram_user );
   /*
    * NOTE: The constant SHARED_OFFS is defined in generated/shared_mmap.h.
    * The file shared_mmap.h becomes generated by the makefile which builds the
    * LM32 binary.
    */
   m_lm32SharedMemAddr += SHARED_OFFS;

#ifndef CONFIG_NO_SCU_RAM
 #ifdef CONFIG_SCU_USE_DDR3
   m_ddr3TrModeBase = m_poEb->findDeviceBaseAddress( DaqEb::gsiId, DaqEb::wb_ddr3ram );
  #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   m_ddr3BurstModeBase = m_poEb->findDeviceBaseAddress( DaqEb::gsiId, DaqEb::wb_ddr3ram2 );
  #endif
 #endif
#endif

}

/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
EbRamAccess::~EbRamAccess( void )
{
   if( m_connectedBySelf )
      m_poEb->disconnect();
}

#ifndef CONFIG_NO_SCU_RAM
/*! ---------------------------------------------------------------------------
 * @see daq_eb_ram_buffer.hpp
 */
void EbRamAccess::readRam( RAM_DAQ_PAYLOAD_T* pData, std::size_t len,
                           RAM_RING_INDEXES_T& rIndexes
                        #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                           , RAM_DAQ_POLL_FT poll
                        #endif
                         )
{
 #if defined( CONFIG_DDR3_NO_BURST_FUNCTIONS )
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
 #else
   #warning At the moment the burst mode is slow!
   RAM_RING_INDEXES_T indexes = rIndexes;
   const uint = lenToEnd = ramRingGetUpperReadSize( &indexes );
   if( lenToEnd < len )
   {
      //TODO
      ret = ddr3FlushFiFo( ramRingGeReadIndex( &indexes ),
                           lenToEnd, pData, poll );
      if( ret != EB_OK )
         return ret;
      ramRingAddToReadIndex( &indexes, lenToEnd );
      len   -= lenToEnd;
      pData += lenToEnd;
   }
   //TODO
   ret = ddr3FlushFiFo( ramRingGeReadIndex( &indexes ),
                        len, pData, poll );
   if( ret != EB_OK )
      return ret;
   ramRingAddToReadIndex( &indexes, len );
   rIndexes = indexes;
 #endif
}
#endif /* #ifndef CONFIG_NO_SCU_RAM */

#ifdef CONFIG_EB_TIME_MEASSUREMENT
   #ifdef COMFIG_EB_TIME_MEASSUREMENT_TO_STDERR
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
   #ifdef COMFIG_EB_TIME_MEASSUREMENT_TO_STDERR
      std::cerr << "WB max duration: " << newDuration << " us" << std::endl;
   #endif
      m_oMaxDuration.m_duration  = newDuration;
      m_oMaxDuration.m_timestamp = m_startTime;
      m_oMaxDuration.m_eAccess   = access;
      m_oMaxDuration.m_dataSize  = size;
   }

   if( newDuration < m_oMinDuration.m_duration )
   {
   #ifdef COMFIG_EB_TIME_MEASSUREMENT_TO_STDERR
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

#endif // ifdef CONFIG_EB_TIME_MEASSUREMENT

//================================== EOF =======================================
