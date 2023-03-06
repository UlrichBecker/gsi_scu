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
   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   RamAccess( EBC::EtherboneConnection* pEbc )
      :EtherboneAccess( pEbc )
   {
   }

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Response timeout.
    */
   RamAccess( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT )
      :EtherboneAccess( rScuName, timeout )
   {
   }

public:
   /*!
    * @brief Destructur makes a disconnect, when this object has connected self
    *        and destroys the object of type EtherboneConnection
    *        if it was created by itself.
    */
   virtual ~RamAccess( void )
   {
   }

   /*!
    * @brief Returns the maximum capacity of the used memory in
    *        addressable 64-bit units.
    */
   virtual uint getMaxCapacity64( void ) = 0;

   /*!
    * @brief Reads from DDR3 of SCU3 or from SRAM of SCU4.
    * @param index64 Start-index (offset) in 64-bit words.
    * @param pData Pointer to target memory
    * @param len Length of data to read in 64-bit units.
    */
   virtual void read( uint index64, uint64_t* pData, uint len ) = 0;

   /*!
    * @brief Writes in DDR3 of SCU3 or in SRAM of SCU4.
    * @param index64 Start-index (offset) in 64-bit words.
    * @param pData Pointer to source memory.
    * @param len Length of data to write in 64-bit units.
    */
   virtual void write( const uint index64, const uint64_t* pData, const uint len ) = 0;

};

}
#endif /* _SCU_MEMORY_HPP */
//================================== EOF ======================================
