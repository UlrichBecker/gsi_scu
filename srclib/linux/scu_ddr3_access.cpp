/*!
 * @file scu_ddr3_access.cpp
 * @brief Access class for SCU- DDR3-RAM
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroFür1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *
 * @date 09.02.2023
 * @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <helper_macros.h>
#include <scu_ddr3.h>
#include "scu_ddr3_access.hpp"

using namespace Scu;


/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Mutex::Ddr3Mutex( const std::string& name )
   :Mutex( name + "_DDR3" )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Mutex::~Ddr3Mutex( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( EBC_PTR_T pEbc, int burstLimit )
   :RamAccess( pEbc )
   ,m_burstLimit( burstLimit )
   ,m_oMutex( pEbc->getNetAddress() )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( const std::string& rScuName, int burstLimit, uint timeout )
   :RamAccess( rScuName, timeout )
   ,m_burstLimit( burstLimit )
   ,m_oMutex( rScuName )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::~Ddr3Access( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::init( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   assert( isConnected() );

   m_if1Addr = findDeviceBaseAddress( EBC::gsiId, EBC::wb_ddr3ram );
   DEBUG_MESSAGE( "DDR3 IF1: 0x" << std::hex << std::uppercase << m_if1Addr << std::dec );

   m_if2Addr = findDeviceBaseAddress( EBC::gsiId, EBC::wb_ddr3ram2 );
   DEBUG_MESSAGE( "DDR3 IF2: 0x" << std::hex << std::uppercase << m_if2Addr << std::dec );

   flushFiFo();
}

/*-----------------------------------------------------------------------------
 */
void Ddr3Access::flushFiFo( void )
{ /*
   * Checking whether still data in FiFo.
   */
   const uint32_t words = readFiFoStatus() & DDR3_FIFO_STATUS_MASK_USED_WORDS;
   if( words == 0 )
      return;

   /*
    * FiFo isn't empty therefore it has to be flushed here by dummy-read.
    */
   DEBUG_MESSAGE( "Flushing DDR3-FiFo with " << words << " 64-bit items" );
   uint64_t dummyMem[words];
   EtherboneAccess::read( m_if2Addr
                            + DDR3_FIFO_LOW_WORD_OFFSET_ADDR * sizeof(uint32_t),
                          dummyMem,
                          sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                          words * sizeof(uint64_t)/sizeof(uint32_t),
                          sizeof(uint64_t)
                        );
}

/*-----------------------------------------------------------------------------
 */
uint Ddr3Access::getMaxCapacity64( void )
{
   return DDR3_MAX_INDEX64;
}

#ifndef CONFIG_NO_BURST_FIFO_POLL
/*-----------------------------------------------------------------------------
 */
bool Ddr3Access::onBurstPoll( uint pollCount )
{
   if( pollCount > 0 )
      ::usleep( 10000 );
   if( pollCount > 100 )
      return true;
   return false;
}
#endif

/*!----------------------------------------------------------------------------
 */
uint32_t Ddr3Access::readFiFoStatus( void )
{
   uint32_t status = 0;
   EtherboneAccess::read( m_if2Addr +
                            DDR3_FIFO_STATUS_OFFSET_ADDR * sizeof(uint32_t),
                          &status,
                          sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                          1
                        );
   return status;
}

constexpr uint MAX_PART_LEN = DDR3_XFER_FIFO_SIZE - 1;

#define CONFIG_NO_BURST_FIFO_POLL
#define CONFIG_DDR3_PARTITIONED_RW
/*!----------------------------------------------------------------------------
 */
void Ddr3Access::read( uint index64, uint64_t* pData, uint len )
{
   assert( (index64 + len) <= DDR3_MAX_INDEX64 );

   if( (m_burstLimit == NEVER_BURST) || (static_cast<int>(len) < m_burstLimit) )
   { /*
      * +++ Reading the DDR3-RAM in transparent mode. +++
      */
   #ifdef CONFIG_DDR3_PARTITIONED_RW
      uint partLen = 0;
      while( len > 0 )
      {
         pData   += partLen;
         index64 += partLen;
         partLen =  std::min( len, MAX_PART_LEN );
         len     -= partLen;
         EtherboneAccess::read( m_if1Addr + index64 * sizeof(uint64_t),
                                pData,
                                sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                                partLen * sizeof(uint64_t)/sizeof(uint32_t)
                              );
      }
   #else
      EtherboneAccess::read( m_if1Addr + index64 * sizeof(uint64_t),
                             pData,
                             sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                             len * sizeof(uint64_t)/sizeof(uint32_t)
                           );
   #endif
      return;
   }

   /*
    * +++ Reading the DDR3-RAM in burst mode. +++
    */
   assert( m_burstLimit != NEVER_BURST );
   uint partLen = 0;
   while( len > 0 )
   {
      pData   += partLen;
      index64 += partLen;
      partLen =  std::min( len, MAX_PART_LEN );
      len     -= partLen;

      /*
       * Starting DDR3 burst mode
       */
      uint32_t start[2] = { index64, partLen };
      static_assert( sizeof(start) == sizeof(uint64_t), "" );
      static_assert( DDR3_BURST_START_ADDR_REG_OFFSET+1 == DDR3_BURST_XFER_CNT_REG_OFFSET, "" );

      { /* Begin of mutex-scope. */
         AutoUnlock autoUnlock( m_oMutex );

         EtherboneAccess::write( m_if1Addr
                                 + DDR3_BURST_START_ADDR_REG_OFFSET * sizeof(uint32_t),
                                 start,
                                 sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                                 ARRAY_SIZE( start )
                               );

      #ifndef CONFIG_NO_BURST_FIFO_POLL
         /*
          * Possibly wait till the FiFo is ready.
          */
         uint32_t status;
         uint pollCount = 0;
         do
         {
            if( onBurstPoll( pollCount ) )
            {
               throw std::runtime_error( "DDR3-timeout: burst FiFo not full!" );
            }
            pollCount++;
            status = readFiFoStatus();
         }
         while( ((status & DDR3_FIFO_STATUS_MASK_EMPTY) != 0) &&
                ((status & DDR3_FIFO_STATUS_MASK_USED_WORDS) != partLen) );
      #endif

         /*
          * Reading the FiFo.
          */
         static_assert( DDR3_FIFO_LOW_WORD_OFFSET_ADDR+1 == DDR3_FIFO_HIGH_WORD_OFFSET_ADDR, "" );
         EtherboneAccess::read( m_if2Addr
                                + DDR3_FIFO_LOW_WORD_OFFSET_ADDR * sizeof(uint32_t),
                                pData,
                                sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                                partLen * sizeof(uint64_t)/sizeof(uint32_t),
                                sizeof(uint64_t)
                              );
      } /* End of mutex scope */
   }
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::write( const uint index64, const uint64_t* pData, const uint len )
{
   assert( (index64 + len) <= DDR3_MAX_INDEX64 );

#ifdef CONFIG_DDR3_PARTITIONED_RW
   uint workLen = len;
   uint workIndex = index64;
   uint64_t* pWorkData = const_cast<uint64_t*>(pData);
   uint partLen = 0;
   while( workLen > 0 )
   {
      pWorkData += partLen;
      workIndex += partLen;
      partLen   =  std::min( workLen, MAX_PART_LEN );
      workLen   -= partLen;
      ddr3Write( m_if1Addr + workIndex * sizeof(uint64_t), pWorkData, partLen );
   }
#else
   ddr3Write( m_if1Addr + index64 * sizeof(uint64_t), pData, len );
#endif
}

//================================== EOF ======================================
