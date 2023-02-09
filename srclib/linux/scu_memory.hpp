/*!
 * @file scu_memory.hpp
 * @brief Access class for SCU-RAM: DDR3 in SCU 3 or SRAM in SCU 4
 * @note Header only!
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
#ifndef _SCU_MEMORY_HPP
#define _SCU_MEMORY_HPP
#include <scu_etherbone.hpp>

namespace Scu
{


///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Abstract base class which handles the access functions for
 *        the SCU-memory. In the case of SCU 3 it is the DDR3-RAM.
 *        And in the future for the SCU 4 the S-RAM.
 */
class RamAccess: public EtherboneAccess
{
protected:
   RamAccess( EBC::EtherboneConnection* pEbc )
      :EtherboneAccess( pEbc )
   {
   }

   RamAccess( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT )
      :EtherboneAccess( rScuName, timeout )
   {
   }

   virtual ~RamAccess( void )
   {
   }

public:
   virtual void read( const uint address, uint64_t* pData, const uint len, const bool burst = false ) = 0;
   virtual void write( const uint address, const uint64_t* pData, const uint len ) = 0;

};

}
#endif /* _SCU_MEMORY_HPP */
//================================== EOF ======================================
