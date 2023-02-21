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
#include <daqt_messages.hpp>
#include "scu_ddr3_access.hpp"

using namespace Scu;

constexpr const char* NAME_APPENDIX = "_DDR3";

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( EBC::EtherboneConnection* pEbc )
   :RamAccess( pEbc )
   ,m_oMutex( pEbc->getNetAddress() + NAME_APPENDIX )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( std::string& rScuName, uint timeout )
   :RamAccess( rScuName, timeout )
   ,m_oMutex( rScuName + NAME_APPENDIX )
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
   m_if2Addr = findDeviceBaseAddress( EBC::gsiId, EBC::wb_ddr3ram2 );
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

#define CONFIG_NO_BURST_FIFO_POLL
/*!----------------------------------------------------------------------------
 */
void Ddr3Access::read( uint address, uint64_t* pData, uint len, const bool burst )
{
   if( !burst )
   { /*
      * Reading the DDR3-RAM in transparent mode.
      */
      EtherboneAccess::read( m_if1Addr + address * sizeof(uint64_t),
                             pData,
                             sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                             len * sizeof(uint64_t)/sizeof(uint32_t)
                           );
      return;
   }

   /*
    * Reading the DDR3-RAM in burst mode.
    */
   uint partLen = 0;
   while( len > 0 )
   {
      pData   += partLen;
      address += partLen;
      partLen =  std::min( len, static_cast<uint>(DDR3_XFER_FIFO_SIZE) );
      len     -= partLen;

      /*
       * Starting DDR3 burst mode
       */
      uint32_t start[2] = { address, partLen };
      static_assert( sizeof(start) == sizeof(uint64_t), "" );
      static_assert( DDR3_BURST_START_ADDR_REG_OFFSET+1 == DDR3_BURST_XFER_CNT_REG_OFFSET, "" );
      {
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
                                partLen * sizeof(uint64_t)/sizeof(uint32_t)
                                ,sizeof(uint64_t)
                              );
      }
   }
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::write( const uint address, const uint64_t* pData, const uint len )
{
   ddr3Write( m_if1Addr + address * sizeof(uint64_t), pData, len );
}

//================================== EOF ======================================
