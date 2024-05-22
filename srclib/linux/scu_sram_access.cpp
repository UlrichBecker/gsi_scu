/*!
 * @file scu_sram_access.cpp
 * @brief Access class for SCU- SRAM of SCU4
 *
 * @date 15.03.2023
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
#include <scu_sram.h>
#include "scu_sram_access.hpp"

using namespace Scu;


/*!----------------------------------------------------------------------------
 */
SramAccess::SramAccess( PTR_T pEbc )
   :RamAccess( pEbc )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
SramAccess::SramAccess( const std::string& rScuName, uint timeout )
   :RamAccess( rScuName, timeout )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
SramAccess::~SramAccess( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*!----------------------------------------------------------------------------
 */
void SramAccess::init( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   assert( isConnected() );

   m_baseAddress = findDeviceBaseAddress( EBC::gsiId, EBC::wb_pseudo_sram );
   DEBUG_MESSAGE( "SRAM: 0x" << std::hex << std::uppercase << m_baseAddress << std::dec );
}

/*!----------------------------------------------------------------------------
 */
uint SramAccess::getMaxCapacity64( void )
{
   return _32MB_IN_BYTE / sizeof(uint64_t);
}

constexpr uint MAX_CYCLE_LEN = 255;

/*!----------------------------------------------------------------------------
 */
void SramAccess::read( uint index64, uint64_t* pData, uint len )
{
   assert( (index64 + len) <= SRAM_MAX_INDEX64 );

   uint partLen = 0;
   while( len > 0 )
   {
      pData   += partLen;
      index64 += partLen;
      partLen =  std::min( len, MAX_CYCLE_LEN );
      len     -= partLen;
      EtherboneAccess::read( m_baseAddress + index64 * sizeof(uint64_t),
                             pData,
                             sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                             partLen * sizeof(uint64_t)/sizeof(uint32_t)
                           );
   }
}

/*!----------------------------------------------------------------------------
 */
void SramAccess::write( const uint index64, const uint64_t* pData, const uint len )
{
   assert( (index64 + len) <= SRAM_MAX_INDEX64 );

   uint partLen = 0;
   uint workLen = len;
   uint index = index64;
   while( workLen > 0 )
   {
      pData   += partLen;
      index   += partLen;
      partLen =  std::min( workLen, MAX_CYCLE_LEN );
      workLen -= partLen;
      EtherboneAccess::write( m_baseAddress + index * sizeof(uint64_t),
                              eb_user_data_t(pData),
                              sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                              partLen * sizeof(uint64_t)/sizeof(uint32_t)
                            );
   }
}

//================================== EOF ======================================
