/*!
 * @file scu_etherbone.cpp
 * @brief Base class foe wishbone or etherbone connections
 * @see scu_etherbone.hpp
 * @see EtherboneConnection.hpp
 * @see EtherboneConnection.cpp
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
#include <assert.h>
#include "scu_etherbone.hpp"

using namespace Scu;

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::EtherboneAccess( EBC::EtherboneConnection* pEbc )
   :m_pEbc( pEbc )
   ,m_fromExtern( true )
   ,m_selfConnected( false )
{
   assert( dynamic_cast<EBC::EtherboneConnection*>(m_pEbc) != nullptr );
   if( !m_pEbc->isConnected() )
   {
      m_pEbc->connect();
      m_selfConnected = true;
   }
}

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::EtherboneAccess( std::string& rScuName, uint timeout )
   :m_pEbc( nullptr )
   ,m_fromExtern( false )
   ,m_selfConnected( true )
{
   m_pEbc = new EBC::EtherboneConnection( rScuName, timeout );
   m_pEbc->connect();
}

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::~EtherboneAccess( void )
{
   if( m_selfConnected )
      m_pEbc->disconnect();

   if( !m_fromExtern )
      delete m_pEbc;
}

//================================== EOF ======================================
