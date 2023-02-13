/*!
 * @file scu_ddr3_access.hpp
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
#ifndef _SCU_DDR3_ACCESS_HPP
#define _SCU_DDR3_ACCESS_HPP
#include <scu_memory.hpp>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class administrates the access functions "write" and "read" for
 *        the SCU-DDR3 memory:
 */
class Ddr3Access: public RamAccess
{
   /*!
    * @brief Base address of interface 1 (transparent mode).
    */
   uint m_if1Addr;

   /*!
    * @brief Base address of interface 2 (burst mode).
    */
   uint m_if2Addr;

public:
   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   Ddr3Access( EBC::EtherboneConnection* pEbc );

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Response timeout.
    */
   Ddr3Access( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT );

   /*!
    * @brief Destructur makes a disconnect, when this object has connected self
    *        and destroys the object of type EtherboneConnection
    *        if it was created by itself.
    */
   ~Ddr3Access( void );

   /*!
    * @brief Returns the interface 1 address of DDR3-RAM.
    * @note For test and debug purposes only.
    */
   uint getIf1Address( void ) const
   {
      return m_if1Addr;
   }

   /*!
    * @brief Returns the interface 2 address of DDR3-RAM.
    * @note For test and debug purposes only.
    */
   uint getIf2Address( void ) const
   {
      return m_if2Addr;
   }

   /*!
    * @brief Reads data from the DDR3 memory.
    * @param address Start-index (source) of the DDR3 memory.
    * @param pData Target address in 64 bit units
    * @param len Length of the data array to read in 64-bit units.
    */
   void read( const uint address, uint64_t* pData, const uint len, const bool burst = false ) override;

   /*!
    * @brief Writes data in the DDR3 - memory.
    * @param address Start-index (target) of the DDR3 memory.
    * @param pData Source address in 64 bit units.
    * @param len Length of the data array to write in 64-bit units.
    */
   void write( const uint address, const uint64_t* pData, const uint len ) override;

private:
   /*!
    * @brief Makes the common initialization for both constructors.
    */
   void init( void );
};

} // namespace Scu
#endif // _SCU_DDR3_ACCESS_HPP
//================================== EOF ======================================
