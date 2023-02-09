/*!
 * @file scu_ddr3_access.cpp
 * @brief Access class for SCU- DDR3-RAM
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
   assert( !burst );
   EtherboneAccess::read( m_if1Addr + address * sizeof(uint64_t),
                          pData,
                          sizeof(uint32_t) | EB_LITTLE_ENDIAN,
                          len * sizeof(uint64_t)/sizeof(uint32_t)
                        );
}

/*!----------------------------------------------------------------------------
 */
void Ddr3Access::write( const uint address, const uint64_t* pData, const uint len )
{
   ddr3Write( m_if1Addr + address * sizeof(uint64_t), pData, len );
}

//================================== EOF ======================================
