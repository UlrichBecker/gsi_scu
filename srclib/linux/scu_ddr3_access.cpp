/*!
 * @file scu_ddr3_access.cpp
 * @brief Access class for SCU- DDR3-RAM
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
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
#include <scu_ddr3.h>
#include "scu_ddr3_access.hpp"

using namespace Scu;

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( EBC::EtherboneConnection* pEbc )
   :RamAccess( pEbc )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::Ddr3Access( std::string& rScuName, uint timeout )
   :RamAccess( rScuName, timeout )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Ddr3Access::~Ddr3Access( void )
{
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::init( void )
{
   assert( isConnected() );
   m_if1Addr = findDeviceBaseAddress( EBC::gsiId, EBC::wb_ddr3ram );
   m_if2Addr = findDeviceBaseAddress( EBC::gsiId, EBC::wb_ddr3ram2 );
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::read( const uint address, uint64_t* pData, const uint len, const bool burst )
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
    * Reading the DDR3-RAM in burst mode. //TODO!!
    */
#if 1
   /*
    * Starting DDR3 burst mode
    */
   static_assert( DDR3_BURST_START_ADDR_REG_OFFSET+1 == DDR3_BURST_XFER_CNT_REG_OFFSET, "" );
   uint32_t start[2] = { address, len };
   EtherboneAccess::write( m_if1Addr + DDR3_BURST_START_ADDR_REG_OFFSET * sizeof(uint32_t),
                           start,
                           sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                           ARRAY_SIZE( start ) / 2
                         );
#endif
#if 1
   /*
    * Reading the FiFo.
    */
   static_assert( DDR3_FIFO_LOW_WORD_OFFSET_ADDR+1 == DDR3_FIFO_HIGH_WORD_OFFSET_ADDR, "" );
   EtherboneAccess::read( m_if2Addr + DDR3_FIFO_LOW_WORD_OFFSET_ADDR * sizeof(uint32_t),
                          pData,
                          sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                          len * sizeof(uint64_t)/sizeof(uint32_t),
                          sizeof(uint64_t)
                        );
#endif
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::write( const uint address, const uint64_t* pData, const uint len )
{
   ddr3Write( m_if1Addr + address * sizeof(uint64_t), pData, len );
}

//================================== EOF ======================================
