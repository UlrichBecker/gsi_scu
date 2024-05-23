/*!
 * @file scu_etherbone.cpp
 * @brief Base class for wishbone or etherbone connections.\n
 *        Inheritable version for class EtherboneConnection.
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
#include <message_macros.hpp>
#include "scu_etherbone.hpp"

using namespace Scu;

uint EtherboneAccess::c_useCount = 0;

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::EtherboneAccess( EBC_PTR_T pEbc )
   :m_pEbc( pEbc )
   ,m_fromExtern( true )
   ,m_selfConnected( false )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   assert( dynamic_cast<EBC_PTR_T>(m_pEbc) != nullptr );
   if( !m_pEbc->isConnected() )
   {
      DEBUG_MESSAGE( "m_pEbc->connect();" );
      m_pEbc->connect();
      m_selfConnected = true;
   }
   c_useCount++;
}

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::EtherboneAccess( const std::string& rScuName, uint timeout )
   :m_pEbc( EBC::EtherboneConnection::getInstance( rScuName, timeout ) )
   ,m_fromExtern( false )
   ,m_selfConnected( true )
{
   DEBUG_MESSAGE_M_FUNCTION( rScuName );

   if( !m_pEbc->isConnected() )
   {
      DEBUG_MESSAGE( "m_pEbc->connect();" );
      m_pEbc->connect();
   }
   c_useCount++;
}

/*!----------------------------------------------------------------------------
 */
EtherboneAccess::~EtherboneAccess( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   assert( c_useCount > 0 );

   c_useCount--;
   if( m_selfConnected && m_pEbc->isConnected() && (c_useCount == 0) )
   {
      DEBUG_MESSAGE( "m_pEbc->disconnect();" );
      m_pEbc->disconnect();
   }

   if( !m_fromExtern )
   {
      DEBUG_MESSAGE( "delete m_pEbc;" );
      EBC::EtherboneConnection::releaseInstance(m_pEbc);
   }
}

//================================== EOF ======================================
