/*!
 * @file scu_etherbone.hpp
 * @brief Base class for wishbone or etherbone connections.\n
 *        Inheritable version for class EtherboneConnection.
 * @see scu_etherbone.cpp
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
#ifndef _SCU_ETHERBONE_HPP
#define _SCU_ETHERBONE_HPP
#include <EtherboneConnection.hpp>
#include <assert.h>


namespace EBC = FeSupport::Scu::Etherbone;

static_assert( sizeof( uint8_t )  == EB_DATA8,  "" );
static_assert( sizeof( uint16_t ) == EB_DATA16, "" );
static_assert( sizeof( uint32_t ) == EB_DATA32, "" );
static_assert( sizeof( uint64_t ) == EB_DATA64, "" );

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief This wrapper class makes it possible to inherit without changing
 *        other modules that also access etherbone.
 */
class EtherboneAccess
{
public:
   using PTR_T = EBC::EtherboneConnection::PTR_T;

private:
   /*!
    * @brief Class variable is the instances counter of this class.
    */
   static uint c_useCount;

   /*!
    * @brief Pointer to the object of type EtherboneConnection
    */
   PTR_T       m_pEbc;

   /*!
    * @brief Keeps the information for the destructor whether the etherbone object
    *        comes from external or was self created.
    */
   bool        m_fromExtern;

   /*!
    * @brief Keeps the information for the destructor whether the etherbone object
    *        was already connected or not.
    */
   bool        m_selfConnected;

public:

   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @see getEb
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   EtherboneAccess( PTR_T pEbc );

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Response timeout.
    */
   EtherboneAccess( const std::string& rScuName = EB_DEFAULT_CONNECTION,
                    uint timeout = EB_DEFAULT_TIMEOUT );

   /*!
    * @brief Destructur makes a disconnect, when this object has connected self
    *        and destroys the object of type EtherboneConnection
    *        if it was created by itself.
    */
   ~EtherboneAccess( void );

   /*!
    * @brief Returns the pointer to the object of type EtherboneConnection.
    *
    * It can be used as argument for constructors of further objects which
    * based on this class.
    */
   PTR_T getEb( void )
   {
      return m_pEbc;
   }

   /*!
    * @brief Returns the number of requested connections.
    * @note For debug purposes only.
    */
   uint getConnectionCounter( void ) const
   {
      return m_pEbc->getConnectionCounter();
   }

   /*!
    * @brief Returns "true" if the wishbone/etherbone connection has
    *        been successful opened.
    */
   bool isConnected( void )
   {
      return m_pEbc->isConnected();
   }

   /*!
    * @brief Returns the network address respectively
    *        the name of the wishbone device.
    */
   const std::string& getNetAddress( void )
   {
      return m_pEbc->getNetAddress();
   }

protected:
   /*!
    * @brief Searches for a particular device address
    * @return Found base address of a wishbone device.
    */
   uint64_t findDeviceBaseAddress( EBC::VendorId vendorId,
                                   EBC::DeviceId deviceId,
                                   uint32_t ind = 0 )
   {
      assert( m_pEbc->isConnected() );
      return m_pEbc->findDeviceBaseAddress( vendorId, deviceId, ind );
   }

   /*!
    * @brief Copies a data array in 1:1 manner form the bus.
    * @param eb_address Address to read from
    * @param pData Destination address to store data
    * @param format Or-link of endian convention and data format
    *               (8, 16, 32 or 64) bit.
    * @param size Length of data array.
    * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
    *                     By default (value is zero) no modulo operation
    *                     will made (normal operation).\n
    *                     Meaningful are the values 1 (no increment)
    *                     for 32-bit register, or 2 for 64-bit register.
    */
   void read( const etherbone::address_t eb_address,
              eb_user_data_t pData,
              const etherbone::format_t format,
              const uint size = 1,
              uint modWbAddrOfs = 0 )
   {
      assert( m_pEbc->isConnected() );
      m_pEbc->read( eb_address, pData, format, size, modWbAddrOfs );
   }

   /*!
    * @brief Copies a data array in 1:1 manner to the bus.
    * @param eb_address Address to write to
    * @param pData Array of data to write
    * @param format Or-link of endian convention and data format
    *               (8, 16, 32 or 64) bit.
    * @param size Length of data array.
    * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
    *                     By default (value is zero) no modulo operation
    *                     will made (normal operation).\n
    *                     Meaningful are the values 1 (no increment)
    *                     for 32-bit register, or 2 for 64-bit register.
    */
    void write( const etherbone::address_t eb_address,
                const eb_user_data_t pData,
                const etherbone::format_t format,
                const uint size = 1,
                uint modWbAddrOfs = 0 )
    {
       assert( m_pEbc->isConnected() );
       m_pEbc->write( eb_address, pData, format, size, modWbAddrOfs );
    }

#ifdef CONFIG_IMPLEMENT_DDR3_WRITE
   /*!
    * @brief Copies a data array of 64 bit items on the DDR3-RAM
    *
    * Unfortunately this function is necessary because the writing of
    * the SCU DDR3-memory needs a special handling. \n
    * In contrast to other devices the upper 32 bit of the 64 bit
    * memory unit of DDR3 has to write first and than the lower 32 bit.
    * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards
    *
    * @param eb_address Address to write to
    * @param pData Array of data to write
    * @param size Length in 64 bit items of data array.
    * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
    *                     By default (value is zero) no modulo operation
    *                     will made (normal operation).\n
    *                     Meaningful are the values 1 (no increment)
    *                     for 32-bit register, or 2 for 64-bit register.
    */
    void ddr3Write( const etherbone::address_t eb_address,
                    const uint64_t* pData,
                    const uint size = 1,
                    uint modWbAddrOfs = 0 )
    {
       assert( m_pEbc->isConnected() );
       m_pEbc->ddr3Write( eb_address, pData, size, modWbAddrOfs );
    }
#endif

};

} /* End namespace Scu */
#endif /* ifndef _SCU_ETHERBONE_HPP */
//================================== EOF ======================================
