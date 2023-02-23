/*!
 * @file scu_lm32_access.cpp
 * @brief Class handles the data transfer from and to the LM32-memory via
 *        etherbone/wishbone bus.
 *
 * @date 14.02.2023
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
#include <message_macros.hpp>
#include "scu_lm32_access.hpp"

using namespace Scu;

/*!----------------------------------------------------------------------------
 */
Lm32Access::Lm32Access( EBC::EtherboneConnection* pEbc )
   :EtherboneAccess( pEbc )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
Lm32Access::Lm32Access( std::string& rScuName, uint timeout )
   :EtherboneAccess( rScuName, timeout )
{
   init();
}

/*!----------------------------------------------------------------------------
 */
void Lm32Access::init( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   m_baseAddress = findDeviceBaseAddress( EBC::gsiId, EBC::lm32_ram_user );
}

/*!----------------------------------------------------------------------------
 */
Lm32Access::~Lm32Access( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*!----------------------------------------------------------------------------
 */
void Lm32Access::write( uint addr, const void* pData, uint len, uint format )
{
   EtherboneAccess::write( m_baseAddress + addr, eb_user_data_t(pData), format, len );
}

/*!----------------------------------------------------------------------------
 */
void Lm32Access::read( uint addr, void* pData, uint len, uint format )
{
   EtherboneAccess::read( m_baseAddress + addr, pData, format, len );
}

//================================== EOF ======================================
