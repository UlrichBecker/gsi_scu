/*!
 * @file scu_ddr3_access.hpp
 * @brief Access class for SCU- DDR3-RAM
 *  @see
 *   <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroFür1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
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
#ifndef _SCU_DDR3_ACCESS_HPP
#define _SCU_DDR3_ACCESS_HPP
#include <scu_memory.hpp>
#include <scu_mutex.hpp>
#include <message_macros.hpp>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class administrates the access functions "write" and "read" for
 *        the SCU3-DDR3 memory:
 */
class Ddr3Access: public RamAccess
{
   /*!
    * @brief Specializing of class Scu::Mutex for DDR3 burst-transfer.
    */
   class Ddr3Mutex: public Mutex
   {
   public:
      Ddr3Mutex( const std::string& name );
      ~Ddr3Mutex( void );
   };

   /*!
    * @brief Base address of interface 1 (transparent mode).
    */
   uint m_if1Addr;

   /*!
    * @brief Base address of interface 2 (burst mode).
    */
   uint m_if2Addr;

   /*!
    * @brief Number of 64-bit words in transparent-mode until
    *        reading in burst-mode.
    */
   int  m_burstLimit;

   /*!
    * @brief Named mutex will used to protect the burst-transfer for
    *        concurrent accesses.
    */
   Ddr3Mutex m_oMutex;

public:
   constexpr static int ALWAYS_BURST = 0;
   constexpr static int NEVER_BURST  = -1;

   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    * @param burstLimit Number of 64-bit words in transparent-mode until
    *                   reading in burst-mode.\n
    *                   Value of -1 (default) means never reading in burst-mode.\n
    *                   Value of 0 means always reading in in burst-mode.
    */
   Ddr3Access( EBC_PTR_T pEbc, int burstLimit = NEVER_BURST );

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param burstLimit Number of 64-bit words in transparent-mode until
    *                   reading in burst-mode.\n
    *                   Value of -1 (default) means never reading in burst-mode.\n
    *                   Value of 0 means always reading in in burst-mode.
    * @param timeout Etherbone response timeout.
    */
   Ddr3Access( const std::string& rScuName = EB_DEFAULT_CONNECTION,
               int burstLimit = NEVER_BURST, uint timeout = EB_DEFAULT_TIMEOUT );

   /*!
    * @brief Destructur makes a disconnect, when this object has connected self
    *        and destroys the object of type EtherboneConnection
    *        if it was created by itself.
    */
   ~Ddr3Access( void ) override;

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
    * @brief Returns the currently used burst-limit for DDR3 reading.
    */
   int getBurstLimit( void )
   {
      return m_burstLimit;
   }

   /*!
    * @brief Sets a new burst-limit for DDR3 reading.
    */
   void setBurstLimit( int burstLimit = NEVER_BURST )
   {
      DEBUG_MESSAGE_M_FUNCTION( burstLimit );
      m_burstLimit = burstLimit;
   }

   /*!
    * @brief Returns the maximum addressable capacity in 64-bit units
    *        of DDR3-RAM.
    */
   uint getMaxCapacity64( void ) override;

   /*!
    * @brief Reads data from the DDR3 memory.
    * @param index64 Start-index (offset) in 64-bit words.
    * @param pData Target address in 64 bit units
    * @param len Length of the data array to read in 64-bit units.
    */
   void read( uint index64, uint64_t* pData, uint len ) override;

   /*!
    * @brief Writes data in the DDR3 - memory.
    * @param index64 Start-index (offset) in 64-bit words..
    * @param pData Source address in 64 bit units.
    * @param len Length of the data array to write in 64-bit units.
    */
   void write( const uint index64, const uint64_t* pData, const uint len ) override;

protected:
   /*!
    * @brief Optional callback function becomes invoked while waiting for the transfer of
    *        the burst transfer.
    * @param pollCount Can be used for e.g. initializing of a timer, if the value zero,
    *                  then it is the first call and the timer can be initialized.
    * @retval true
    * @retval false
    */
   virtual bool onBurstPoll( uint pollCount );

private:
   /*!
    * @brief Makes the common initialization for both constructors.
    */
   void init( void );

   /*!
    * @brief Returns the value of the FiFo status register.
    */
   uint32_t readFiFoStatus( void );

   /*!
    * @brief Flushes the FiFo
    */
   void flushFiFo( void );
};

} // namespace Scu
#endif // _SCU_DDR3_ACCESS_HPP
//================================== EOF ======================================
