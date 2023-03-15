/*!
 * @file scu_sram_access.hpp
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
#ifndef _SCU_SRAM_ACCESS_HPP
#define _SCU_SRAM_ACCESS_HPP
#include <scu_memory.hpp>
#include <message_macros.hpp>

namespace Scu
{
///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class administrates the access functions "write" and "read" for
 *        the SCU4-SRAM memory:
 */
class SramAccess: public RamAccess
{
   uint m_baseAddress;

public:
   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   SramAccess( EBC::EtherboneConnection* pEbc );

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Etherbone response timeout.
    */
   SramAccess( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT );

   /*!
    * @brief Destructur makes a disconnect, when this object has connected self
    *        and destroys the object of type EtherboneConnection
    *        if it was created by itself.
    */
   ~SramAccess( void ) override;

   /*!
    * @brief Returns the wishbone base address.
    * @note For test and debug purposes only.
    */
    uint getBase( void )
    {
       return m_baseAddress;
    }

   /*!
    * @brief Returns the maximum addressable capacity in 64-bit units
    *        of SRAM.
    */
   uint getMaxCapacity64( void ) override;

   /*!
    * @brief Reads data from the SRAM memory.
    * @param index64 Start-index (offset) in 64-bit words.
    * @param pData Target address in 64 bit units
    * @param len Length of the data array to read in 64-bit units.
    */
   void read( uint index64, uint64_t* pData, uint len ) override;

   /*!
    * @brief Writes data in the SRAM - memory.
    * @param index64 Start-index (offset) in 64-bit words..
    * @param pData Source address in 64 bit units.
    * @param len Length of the data array to write in 64-bit units.
    */
   void write( const uint index64, const uint64_t* pData, const uint len ) override;

private:
   /*!
    * @brief Makes the common initialization for both constructors.
    */
   void init( void );

};

} // namespace Scu
#endif /* ifndef _SCU_SRAM_ACCESS_HPP */
//================================== EOF ======================================
