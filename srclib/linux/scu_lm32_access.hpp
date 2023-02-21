/*!
 * @file scu_lm32_access.hpp
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
#ifndef _SCU_LM32_ACCESS_HPP
#define _SCU_LM32_ACCESS_HPP
#include <scu_etherbone.hpp>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief An object of this class manages the read and write accesses
 *        of the LM32-memory.
 *
 * It is the base-class for data transfer between Linux-host and
 * LM32- application.
 */
class Lm32Access: public EtherboneAccess
{
protected:
   /*!
    * @brief Wishbone/Etherbone base address of LM32 memory.
    */
   uint m_baseAddress;

public:
   constexpr static uint OFFSET   = 0x10000000;
   constexpr static uint MEM_SIZE = 147456;
   constexpr static uint MAX_ADDR = MEM_SIZE + OFFSET;

   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   Lm32Access( EBC::EtherboneConnection* pEbc );

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Response timeout.
    */
   Lm32Access( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT );

   ~Lm32Access( void );

   /*!
    * @brief Writes data in the LM32-memory.
    * @param addr Relative target-memory address seen from LM32 perspective
    * @param pData Pointer to data source.
    * @param len Number of data units to write.
    * @param format Or-link of data element size in bytes and byte-order.
    */
   void write( uint addr, const void* pData, uint len, uint format );

   /*!
    * @brief Template writes data in the LM32-memory by recognizing
    *        the byte size of the data element.
    * @param addr Relative target-memory address seen from LM32 perspective
    * @param pData Pointer to data source.
    * @param len Number of data units to write.
    */
   template<typename TYPE>
   void write( uint addr, const TYPE* pData, uint len )
   {
      write( addr, pData, len, sizeof( TYPE ) | EB_BIG_ENDIAN );
   }

   /*!
    * @brief Reads data from the LM32-memory.
    * @param addr Relative source-memory address seen from LM32 perspective
    * @param pData Pointer to the target memory.
    * @param len Number of data units to read.
    * @param format Or-link of data element size in bytes and byte-order.
    */
   void read( uint addr, void* pData, uint len, uint format );

   /*!
    * @brief Template reads data from the LM32-memory by recognizing
    *        the byte size of the data element.
    * @param addr Relative source-memory address seen from LM32 perspective
    * @param pData Pointer to the target memory.
    * @param len Number of data units to read.
    */
   template<typename TYPE>
   void read( uint addr, TYPE* pData, uint len )
   {
      read( addr, pData, len, sizeof( TYPE ) | EB_BIG_ENDIAN );
   }

   /*!
    * @brief Returns the etherbone/wishbone base address of LM32
    */
   uint getBaseAddress( void )
   {
      return m_baseAddress;
   }

private:
   /*!
    * @brief Makes the common initialization for both constructors.
    */
   void init( void );
};

} /* Namespace Scu */

#endif /* ifndef _SCU_LM32_ACCESS_HPP */
//================================== EOF ======================================
